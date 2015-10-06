#include "stdafx.h"

#include <experimental\resumable>
#include "PAL.h"

struct task {
	~task() {}
	struct promise_type {
		task get_return_object() { return task{}; }
		void return_void() {}
		bool initial_suspend() { return false; }
		bool final_suspend() { return false; }
	};
};

using namespace std::experimental;

namespace awaitable
{
	namespace detail
	{
		struct AwaiterBase : os_async_context
		{
			coroutine_handle<> resume;
			std::error_code err;
			int bytes;
		};

		void io_complete_callback(CompletionPacket& p)
		{
			auto me = static_cast<AwaiterBase*>(p.overlapped);
			me->err = std::error_code(p.error, std::system_category());
			me->bytes = (int)p.byteTransferred;
			me->resume();
		}
	}

	struct Tcp
	{
		struct Connection
		{
			OsTcpSocket sock;

			Connection(OsTcpSocket && sock) : sock(std::move(sock)) {}


			Connection() {
				sock.Bind(endpoint{ "127.0.0.1", 0 });
				ThreadPool::AssociateHandle(sock.native_handle(),
					&detail::io_complete_callback);
			}

			auto Read(void* buf, int len)
			{
				struct awaiter : detail::AwaiterBase {
					void* buf;
					int len;
					Connection* me;

					awaiter(Connection* me, void* buf, int len) : me(me), buf(buf), len(len) {}

					bool await_ready() { return false; }
					int await_resume() { return len; }

					bool await_suspend(coroutine_handle<> h) {
						this->resume = h;
						auto error = me->sock.Receive(buf, len, this);
						if (error.value() == kSynchCompletion) {
							return false;
						}
						if (error) {
							panic_if(!!error, "read failed");
						}
						return true;
					}
				};
				return awaiter{ this, buf, len };
			}

			auto Write(void* buf, int len)
			{
				struct awaiter : detail::AwaiterBase {
					void* buf;
					int len;
					Connection* me;

					awaiter(Connection* me, void* buf, int len) : me(me), buf(buf), len(len) {}

					bool await_ready() { return false; }
					int await_resume() { return len; }

					bool await_suspend(coroutine_handle<> h) {
						this->resume = h;
						auto error = me->sock.Send(buf, len, this);
						if (error.value() == kSynchCompletion) {
							return false;
						}
						if (error) {
							h.destroy();
						}
						return true;
					}
				};
				return awaiter{ this, buf, len };
			}
		};

		struct Listener
		{
			OsTcpSocket sock;

			Listener() {
				sock.Bind(endpoint{ "127.0.0.1", 13 });
				ThreadPool::AssociateHandle(sock.native_handle(), &detail::io_complete_callback);
				sock.Listen();
			}

			auto Accept() {
				struct awaiter : detail::AwaiterBase {
					Listener * me;
					OsTcpSocket sock;
					endpoint_name_pair ep;

					awaiter(Listener* me) : me(me) {}
					bool await_ready() { return false; }
					Connection await_resume() { return Connection{ std::move(sock) }; }

					void await_suspend(coroutine_handle<> h) {
						this->resume = h;

						ThreadPool::AssociateHandle(sock.native_handle(),
							&detail::io_complete_callback);

						me->sock.Accept(sock.native_handle(), ep, this);
					}
				};
				return awaiter{ this };
			}
		};

		static auto Connect(const char* s, unsigned short port)
		{
			struct awaiter : detail::AwaiterBase {
				Connection conn;
				endpoint ep;

				awaiter(const char* s, unsigned short port) : ep(s, port) {}
				bool await_ready() { return false; }
				Connection await_resume() {
					panic_if(!!this->err, "Connect failed");
					return std::move(conn);
				}

				void await_suspend(coroutine_handle<> h) {
					this->resume = h;
					conn.sock.Connect(ep, this);
				}
			};
			return awaiter{ s, port };
		}
	};

	struct repost : detail::AwaiterBase {
		bool await_ready() { return false; }
		void await_resume() {}
		void await_suspend(coroutine_handle<> h) {
			this->resume = h;
			ThreadPool::Post(this, 0, &detail::io_complete_callback);
		}
	};

	task TcpReader(WorkTracker& trk, int64_t total) {
		char buf[ReaderSize];
		auto conn = await Tcp::Connect("127.0.0.1", 13);
		while (total > 0) {
			auto bytes = await conn.Read(buf, sizeof(buf));
			if (bytes == 0) break;
			total -= bytes;
		}
		trk.completed();
	}

	task ServeClient(Tcp::Connection conn) {
		char buf[WriterSize];
		await repost();
		for (;;) {
			await conn.Write(buf, sizeof(buf));
		}
	}

	task Server()
	{
		Tcp::Listener s;
		for (;;) {
			ServeClient(await s.Accept());
		}
	}


	void run(int nReaders, uint64_t bytes, bool sync) {
		printf("awaitable readers %d sync %d ", nReaders, sync);

		ThreadPool q(nReaders * 2 + 8, sync);
		WorkTracker trk(nReaders);

		Server();
		for (int i = 0; i < nReaders; ++i)
			TcpReader(trk, bytes);

		os_sleep(100 * 1000);
	}
}