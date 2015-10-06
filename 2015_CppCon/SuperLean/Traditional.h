#pragma once

struct Server
{
	void Start();

private:
	void OnAccept(std::error_code ec, OsTcpSocket hdl);

	OsTcpSocket sock;
};

class tcp_reader
{
	char buf[ReaderSize];
	Tcp::Connection conn;

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
	{}
	static void start(WorkTracker& trk, int64_t total);
};

