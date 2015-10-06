#include "stdafx.h"
#include "PAL.h"
#include <memory>
#include <system_error>
#include <atomic>

namespace improved {
	namespace detail
	{
		struct OverlappedBase : os_async_context {
			virtual void Invoke(std::error_code, int bytes) = 0;
			virtual ~OverlappedBase() {}
		};

		void io_complete_callback(CompletionPacket& p)
		{
			auto me = static_cast<OverlappedBase*>(p.overlapped);
			me->Invoke(std::error_code(p.error, std::system_category()), (int)p.byteTransferred);
		}

		template <typename Fn>
		struct CompletionWithSizeT : OverlappedBase, private Fn
		{
			CompletionWithSizeT(Fn fn) : Fn(std::move(fn)) {}
			void Invoke(std::error_code ec, int count) override
			{
				Fn::operator()(ec, count);
			}
		};

		template <typename Fn>
		std::unique_ptr<OverlappedBase> make_handler_with_size_t(Fn && fn)
		{
			return std::make_unique < CompletionWithSizeT <
				std::decay_t<Fn >> >(std::forward<Fn>(fn));
		}

		struct OverlappedAcceptBase : OverlappedBase {
			endpoint_name_pair ep;
			OsTcpSocket sock;

			OverlappedAcceptBase() {}
		};

		template <typename Fn>
		struct CompletionWithConnection : OverlappedAcceptBase, private Fn
		{
			template <typename F>
			CompletionWithConnection(F && fn) : Fn(std::forward<F>(fn)) {}

			void Invoke(std::error_code ec, int) override
			{
				ThreadPool::AssociateHandle(sock.native_handle(),
					&detail::io_complete_callback);
				Fn::operator()(ec, std::move(sock));
			}
		};

		template <typename Fn>
		std::unique_ptr<OverlappedAcceptBase> make_accept_handler(Fn && fn)
		{
			return std::make_unique < CompletionWithConnection <
				std::decay_t<Fn >> >(std::forward<Fn>(fn));
		}
	}

	struct Tcp
	{
		struct Connection
		{
			template <typename F>
			bool Read(void* buf, int& len, F && cb) {
				return Read(buf, len,
					detail::make_handler_with_size_t(std::forward<F>(cb)));
			}

			template <typename F>
			bool Write(void* buf, int& len, F cb) {
				return Write(buf, len,
					detail::make_handler_with_size_t(std::forward<F>(cb)));
			}

			Connection() {}

			Connection(Connection&& rhs) = default;
			Connection& operator= (Connection&& rhs) = default;

			template <typename F>
			void Connect(char const* str, unsigned short port, F cb)
			{
				Connect(str, port,
					detail::make_handler_with_size_t(std::forward<F>(cb)));
			}

			Connection(OsTcpSocket s)
				: sock(std::move(s))
			{
			}

			bool Read(void* buf, int& len, detail::OverlappedBase* o)
			{
				auto error = sock.Receive(buf, len, o);
				if (error.value() == kSynchCompletion) {
					return true;
				}
				if (error) {
					o->Invoke(error, 0);
				}
				return false;
			}

			bool Write(void* buf, int& len, detail::OverlappedBase* o)
			{
				auto error = sock.Send(buf, len, o);
				if (error.value() == kSynchCompletion) {
					return true;
				}
				if (error) {
					o->Invoke(error, 0);
				}
				return false;
			}

		private:

			bool Read(void* buf, int& len, std::unique_ptr<detail::OverlappedBase> o)
			{
				auto error = sock.Receive(buf, len, o.get());
				if (error.value() == kSynchCompletion) {
					return true;
				}
				if (error) {
					o->Invoke(error, 0);
				}
				o.release();
				return false;
			}

			bool Write(void* buf, int& len, std::unique_ptr<detail::OverlappedBase> o)
			{
				auto error = sock.Send(buf, len, o.get());
				if (error.value() == kSynchCompletion) {
					return true;
				}
				if (error) {
					o->Invoke(error, 0);
				}
				o.release();
				return false;
			}

			void Connect(char const* str, unsigned short port, std::unique_ptr<detail::OverlappedBase> o)
			{
				sock.Bind(endpoint{ "127.0.0.1", 0 });
				ThreadPool::AssociateHandle(sock.native_handle(),
					&detail::io_complete_callback);

				sock.Connect(endpoint("127.0.0.1", 13), o.get());

				o.release();
			}
			OsTcpSocket sock;
		};

		struct Listener
		{
			Listener() {
				endpoint ep("127.0.0.1", 13);
				sock.Bind(ep);
				ThreadPool::AssociateHandle(sock.native_handle(), &detail::io_complete_callback);
				sock.Listen();
			}

			template <typename Fn>
			void Accept(Fn cb)
			{
				Accept(detail::make_accept_handler(std::forward<Fn>(cb)));
			}

			void Accept(std::unique_ptr<detail::OverlappedAcceptBase> o)
			{
				sock.Accept(o->sock.native_handle(), o->ep, o.get());
				o.release();
			}

			OsTcpSocket sock;
		};
	};

	class tcp_reader
	{
		char buf[ReaderSize];
		Tcp::Connection conn;
		std::unique_ptr<detail::OverlappedBase> wo;

		WorkTracker& tracker;

		int64_t total;
		void OnConnect(std::error_code ec);
		void OnRead(std::error_code ec, int bytesRead);
		void OnError(std::error_code ec);
		void OnComplete();
	public:
		explicit tcp_reader(WorkTracker& trk, int64_t total)
			: total(total)
			, tracker(trk)
		{
			wo = detail::make_handler_with_size_t(
				[this](auto ec, int nBytes) { this->OnRead(ec, nBytes); });
		}
		static void start(WorkTracker& trk, int64_t total);
	};

	void tcp_reader::start(WorkTracker& trk, int64_t total) {
		auto p = std::make_unique<tcp_reader>(trk, total);
		p->conn.Connect("127.0.0.1", 13,
			[raw = p.get()](auto ec, auto) { raw->OnConnect(ec); });
		p.release(); // operation launched. let it fly
	}

	void tcp_reader::OnConnect(std::error_code ec)
	{
		if (ec) return OnError(ec);
		int len = sizeof(buf);
		while (
			conn.Read(buf, len, wo.get())
			);
	}

	void tcp_reader::OnRead(std::error_code ec, int bytesRead) {
		if (ec) return OnError(ec);
		do {
			total -= (int)bytesRead;
			if (total <= 0 || bytesRead == 0) return OnComplete();
			bytesRead = sizeof(buf);
		} while (conn.Read(buf, bytesRead, wo.get()));
	}
	void tcp_reader::OnError(std::error_code ec) {
		printf("Reader got error: %d\n", ec.value());
		auto cleanMe = std::unique_ptr<tcp_reader>(this);
		tracker.failed(ec);
	}
	void tcp_reader::OnComplete() {
		auto cleanMe = std::unique_ptr<tcp_reader>(this);
		tracker.completed();
	}

	struct tcp_writer
	{
		std::unique_ptr<detail::OverlappedBase> wo;

		tcp_writer(OsTcpSocket sock)
			: conn(std::move(sock))
		{
			wo = detail::make_handler_with_size_t(
				[this](auto ec, int nBytes) { this->OnWrite(ec, nBytes); });

			int len = sizeof(buf);
			bool completedSync = conn.Write(buf, len, wo.get());

			if (completedSync) {
				ThreadPool::Post(wo.get(), len, &detail::io_complete_callback);
			}
		}
		void OnWrite(std::error_code ec, int nBytes)
		{
			if (ec) return OnError(ec);
			do {
				nBytes = sizeof(buf);
			} while ( conn.Write(buf, nBytes, wo.get()) );
		}
		void OnError(std::error_code ec) {
			if (ec.value() == kConnectionReset) OnComplete();
		}
		void OnComplete() {}

		static void start(OsTcpSocket sock)
		{
			auto p = std::make_unique<tcp_writer>(std::move(sock));
			p.release();
		}

	private:
		char buf[WriterSize];
		Tcp::Connection conn;
	};

	struct Server
	{
		void Start()
		{
			sock.Accept([this](auto ec, auto b) { OnAccept(ec, std::move(b)); });
		}

		void OnAccept(std::error_code ec, OsTcpSocket hdl) {
			panic_if(!!ec, "Accept failed");
			tcp_writer::start(std::move(hdl));
			sock.Accept([this](auto ec, auto b) { OnAccept(ec, std::move(b)); });
		}

		Tcp::Listener sock;
	};

	void run(int nReaders, uint64_t bytes, bool sync)
	{
		ThreadPool q(nReaders * 2 + 8, sync);

		WorkTracker trk(nReaders);

		for (int i = 0; i < nReaders; ++i)
			tcp_reader::start(trk, bytes);

		Server server;
		server.Start();

		printf("improved readers %d sync %d ", nReaders, sync);
		os_sleep(120 * 1000);
	}
}