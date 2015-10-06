#pragma once
#include <cstdint>
#include <system_error>
#include <atomic>

const size_t ReaderSize = 4 * 1024;
const size_t WriterSize = 4 * 1024;

struct os_async_context
{
	uintptr_t Internal;
	uintptr_t  InternalHigh;
	union {
		struct {
			uint32_t Offset;
			uint32_t  OffsetHigh;
		} s;
		void* Pointer;
	} u;
	void* hEvent;

	os_async_context();
};

struct CompletionPacket
{
	uint32_t error;
	uint32_t  byteTransferred;
	os_async_context* overlapped;
};

using LPFN_COMPLETION_PROC = void(*)(CompletionPacket&);

void panic_if(bool cond, const char* message);

using native_handle_t = uintptr_t;
const native_handle_t kInvalidSocket = ~0;
const int kConnectionReset = 10054;
const int kSynchCompletion = 997;

struct endpoint
{
	endpoint() {}
	explicit endpoint(char const* str, unsigned short port);
	static constexpr size_t size() { return sizeof(buf); }
	void const* addr() const { return buf; }
private:
	char buf[16];
};

struct endpoint_name
{
	endpoint ep;
	char buf[16]; // padding
};

struct endpoint_name_pair {
	endpoint_name local;
	endpoint_name remote;
};

void os_sleep(int ms);

struct OsTcpSocket
{
	OsTcpSocket(OsTcpSocket const&) = delete;
	OsTcpSocket& operator = (OsTcpSocket const&) = delete;
	OsTcpSocket& operator = (OsTcpSocket&& rhs)
	{
		panic_if(handle_ != kInvalidSocket, "can only assign to an empty socket");
		handle_ = rhs.handle_;
		rhs.handle_ = kInvalidSocket;
		return *this;
	}

	OsTcpSocket(OsTcpSocket && rhs)
		: handle_(rhs.handle_)
	{
		rhs.handle_ = kInvalidSocket;
	}

	void Bind(endpoint const& ep);

	OsTcpSocket();

	void Listen();

	std::error_code Accept(native_handle_t newh, endpoint_name_pair & eps, os_async_context * o);

	std::error_code Connect(endpoint const& ep, os_async_context* o);

	std::error_code Send(void* buf, size_t len, os_async_context* o);

	std::error_code Receive(void* buf, size_t len, os_async_context* o);

	~OsTcpSocket();

	native_handle_t native_handle() const { return handle_; }

private:
	native_handle_t handle_;
};

struct ThreadPool
{
	static void AssociateHandle(native_handle_t h, LPFN_COMPLETION_PROC);
	static void Post(os_async_context*, int nBytes, LPFN_COMPLETION_PROC);
	static void Stop();

	explicit ThreadPool(int threadCount, bool enableSync);
	~ThreadPool();
};

struct WorkTracker
{
	WorkTracker(int work) : workCount(work) {}
	void completed() { release_work(); }
	void failed(std::error_code) { release_work(); }
private:
	void release_work() {
		if (--workCount == 0) {
			ThreadPool::Stop();
		}
	}
private:
	std::atomic<int> workCount;
};

