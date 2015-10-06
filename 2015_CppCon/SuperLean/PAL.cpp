#include "stdafx.h"
#include "PAL.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <Mswsock.h>
#include <mstcpip.h>
#include <atomic>

static_assert(sizeof(os_async_context) == sizeof(OVERLAPPED), "os_async_context != OVERLAPPED");

struct Timer
{
	Timer();
	void Stop();
private:
	int64_t freq;
	int64_t start;
};

Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
}

void Timer::Stop()
{
	int64_t stop;
	QueryPerformanceCounter((LARGE_INTEGER*)&stop);
	auto diff = (double)(stop - start) / (double)freq;
	printf("Measured: %f seconds\n", diff);
}

void panic_if(bool cond, const char* message)
{
	if (cond) {
		printf("Error at: %s %d\n", message, WSAGetLastError());
		exit(1);
	}
}

void os_sleep(int ms)
{
	Sleep(ms);
}

#pragma comment(lib, "Ws2_32.lib")

endpoint::endpoint(char const* str, unsigned short port)
{
	auto thisHost = gethostbyname(str);
	auto ip = inet_ntoa(*(struct in_addr *) *thisHost->h_addr_list);
	sockaddr_in& service = *(sockaddr_in*)(addr());

	static_assert(sizeof(sockaddr_in) == size(), "endpoint size != sockaddr_in size");

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(ip);
	service.sin_port = htons(port);
}

struct SocketStarter
{
	WSADATA wsaData;
	LPFN_CONNECTEX ConnectExPtr;
	LPFN_ACCEPTEX AcceptExPtr;
	bool syncCompletion;
	SocketStarter();
};

SocketStarter startSockets;

struct CompletionQueue
{
	Timer timer;

	static DWORD WINAPI ThreadProc(
		_In_ LPVOID lpParameter
		)
	{
		auto me = (CompletionQueue*)lpParameter;
		for (;;) {
			CompletionPacket packet;
			ULONG_PTR key;

			auto success = GetQueuedCompletionStatus(
				me->iocp,
				(LPDWORD)&packet.byteTransferred,
				&key,
				(LPOVERLAPPED*)&packet.overlapped,
				INFINITE
				);

			if (packet.overlapped == nullptr)
			{
				panic_if(!success, "Failed to deque op");
				break;
			}

			packet.error = success ? 0 : GetLastError();

			panic_if(key == 0, "iocpl: key is null");

			((LPFN_COMPLETION_PROC)key)(packet);
		}
		me->release();
		return 0;
	}

	CompletionQueue(int threadCount)
		: iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0))
		, count(threadCount)
		, threadCount(threadCount)
	{
		panic_if(iocp == nullptr, "CreateIoCompletionPort");

		for (int i = 0; i < threadCount; ++i)
		{
			t[i] = CreateThread(
				nullptr, 0,
				CompletionQueue::ThreadProc,
				this,
				0,
				nullptr
				);
			panic_if(t[i] == nullptr, "Failed to create thread");
		}
	}

	void release()
	{
		if (--count == 0) {
			timer.Stop();
			exit(0);
		}
	}

	void Stop()
	{
		timer.Stop();
		exit(0);
		for (int i = 0; i < threadCount; ++i)
		{
			PostQueuedCompletionStatus(
				iocp,
				0,
				0,
				0
				);
		}
	}

	~CompletionQueue()
	{
		Stop();
	}

	void AssociateHandle(HANDLE ioHandle, LPFN_COMPLETION_PROC fn)
	{
		auto result = CreateIoCompletionPort(ioHandle, iocp, (ULONG_PTR)fn, 0);
		panic_if(result != iocp, "association handle failed");
	}

	int threadCount;
	std::atomic<int> count;
	HANDLE t[64];

	HANDLE iocp;
};

void OsTcpSocket::Listen() {
	auto iResult = ::listen(handle_, 100);
	panic_if(iResult == SOCKET_ERROR, "listen failed with error");
}


std::error_code OsTcpSocket::Connect(endpoint const& ep, os_async_context* o)
{
	DWORD unused;
	auto ok = (*startSockets.ConnectExPtr)(
		handle_, (const sockaddr*)ep.addr(), (int)ep.size(),
		nullptr, 0,
		&unused,
		(OVERLAPPED*)o
		);
	int status;
	if (ok) {
		status = 0;
		panic_if(true, "did not expect Connect to complete synchronously");
	}
	else {
		status = WSAGetLastError();
		panic_if(status != ERROR_IO_PENDING, "Connect failed");
	}
	return{ status, std::system_category() };
}

std::error_code OsTcpSocket::Send(void* buf, size_t len, os_async_context* o)
{
	WSABUF b;
	b.buf = (char*)buf;
	b.len = (int)len;

	DWORD nBytes;

	auto iResult = WSASend(
		handle_,
		&b,
		1,
		&nBytes,
		0,
		(OVERLAPPED*)o,
		nullptr
		);

	int status;

	if (iResult == SOCKET_ERROR)
	{
		status = GetLastError();
		if (status == ERROR_IO_PENDING)
			status = 0; // no action required, therefore success
	}
	else {
		status = (startSockets.syncCompletion) ? ERROR_IO_PENDING : 0;
	}
	return{ status, std::system_category() };
}

std::error_code OsTcpSocket::Receive(void* buf, size_t len, os_async_context* o)
{
	WSABUF b;
	b.buf = (char*)buf;
	b.len = (int)len;

	DWORD nBytes;
	DWORD flags = 0;

	auto iResult = WSARecv(
		handle_,
		&b,
		1,
		&nBytes,
		&flags,
		(OVERLAPPED*)o,
		nullptr
		);

	int status;

	if (iResult == SOCKET_ERROR)
	{
		status = GetLastError();
		if (status == ERROR_IO_PENDING)
			status = 0;
	}
	else {
		status = (startSockets.syncCompletion) ? ERROR_IO_PENDING : 0;
	}
	return{ status, std::system_category() };
}

OsTcpSocket::~OsTcpSocket()
{
	if (handle_ != INVALID_SOCKET)
		closesocket(handle_);
}

void OsTcpSocket::Bind(endpoint const& ep)
{
	auto result = ::bind(handle_, (const sockaddr*)ep.addr(), (int)ep.size());
	panic_if(result == SOCKET_ERROR, "bind failed");
}

OsTcpSocket::OsTcpSocket()
	: handle_(::WSASocketW(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP,
		nullptr, 0,
		WSA_FLAG_OVERLAPPED))
{
	panic_if(handle_ == INVALID_SOCKET, "WsaSocketW");

	int OptionValue = 1;
	DWORD NumberOfBytesReturned = 0;

	int status = 0;
	
	status = WSAIoctl(
		handle_,
		SIO_LOOPBACK_FAST_PATH,
		&OptionValue,
		sizeof(OptionValue),
		NULL,
		0,
		&NumberOfBytesReturned,
		0,
		0);

	panic_if(SOCKET_ERROR == status, "SetFlags");

	INT optval = 1;
	status = setsockopt(handle_,
		IPPROTO_TCP,
		TCP_NODELAY,
		(PCHAR)&optval,
		sizeof(INT));

	panic_if(status != 0, "Failed to set TCP_NODELAY socket option");
}

SocketStarter::SocketStarter()
{
	auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	panic_if(iResult != NO_ERROR, "WSAStartup");

	OsTcpSocket s;
	static GUID connGuid = WSAID_CONNECTEX;
	static GUID acceptGuid = WSAID_ACCEPTEX;
	DWORD numBytes;

	iResult = ::WSAIoctl(s.native_handle(), SIO_GET_EXTENSION_FUNCTION_POINTER,
		&connGuid, sizeof(connGuid), &ConnectExPtr, sizeof(ConnectExPtr),
		&numBytes, NULL, NULL);

	panic_if(iResult != NO_ERROR, "GetConnextEx");

	iResult = ::WSAIoctl(s.native_handle(), SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptGuid, sizeof(acceptGuid), &AcceptExPtr, sizeof(AcceptExPtr),
		&numBytes, NULL, NULL);

	panic_if(iResult != NO_ERROR, "GetAcceptEx");

	WSAPROTOCOL_INFOW wsaProtocolInfo;
	int wsaProtocolInfoSize = sizeof(wsaProtocolInfo);
	iResult = getsockopt(
		s.native_handle(),
		SOL_SOCKET,
		SO_PROTOCOL_INFOW,
		reinterpret_cast<char*>(&wsaProtocolInfo),
		&wsaProtocolInfoSize);
	panic_if(iResult == SOCKET_ERROR, "SOL_SOCKET");

	//syncCompletion = false;
	syncCompletion = (wsaProtocolInfo.dwServiceFlags1 & XP1_IFS_HANDLES) != 0;
}

std::error_code OsTcpSocket::Accept(native_handle_t newh, endpoint_name_pair& eps, os_async_context* o)
{
	DWORD unused;
	auto  bRetVal = (*startSockets.AcceptExPtr)(handle_, newh, &eps,
		0, sizeof(eps.local), sizeof(eps.remote),
		&unused, (LPOVERLAPPED)o);

	int status;

	if (bRetVal == FALSE) {
		status = WSAGetLastError();
		panic_if(status != ERROR_IO_PENDING, "AcceptEx failed");
	}
	else {
		status = 0;
		panic_if(true, "Strange, completed synchronously\n");
	}
	return{ status, std::system_category() };
}

CompletionQueue* queue = nullptr;

void ThreadPool::AssociateHandle(native_handle_t h, LPFN_COMPLETION_PROC fn)
{
	panic_if(queue == nullptr, "ThreadPool is not initialized");
	if (startSockets.syncCompletion) {
		auto success = SetFileCompletionNotificationModes(
			(HANDLE)h, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS);
		panic_if(!success, "FILE_SKIP_COMPLETION_PORT_ON_SUCCESS");
	}
	queue->AssociateHandle((HANDLE)h, fn);
}

void ThreadPool::Post(os_async_context * o, int nBytes, LPFN_COMPLETION_PROC fn)
{
	panic_if(queue == nullptr, "ThreadPool is not initialized");

	auto success = PostQueuedCompletionStatus(
		queue->iocp,
		nBytes,
		(ULONG_PTR)fn,
		(LPOVERLAPPED)o
		);
	panic_if(!success, "PostQueuedCompletionStatus");
}

ThreadPool::ThreadPool(int threadCount, bool enableSync)
{
	panic_if(queue != nullptr, "ThreadPool is already initialized");
	queue = new CompletionQueue(threadCount);
	if (enableSync == false) {
		startSockets.syncCompletion = false;
	}
}

void ThreadPool::Stop()
{
	if (queue)
		queue->Stop();
}

ThreadPool::~ThreadPool()
{
	delete queue;
	queue = nullptr;
}

os_async_context::os_async_context()
{
	memset(this, 0, sizeof(*this));
}
