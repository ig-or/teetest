

#pragma once
#include "boost/asio.hpp"

typedef void(*vf)(void);
typedef void(*data_1)(char* s, int size);
typedef void(*data_2)(char* s);

class EthClient {
public:
	bool connected;
	vf pingReply;

	EthClient();
	void startClient(data_1 cb);
	void StopClient();
	int do_write(const char* s);


private:
	volatile bool pleaseStop = false;
	int fileSize, fileBytesCounter, packetsCounter, badPacketCounter, counterErrorCounter;
	int readingTheLogFile, lastReportedProgress;
	int incomingPacketsCounter = 0;
	std::string currentFileName;
	FILE*	theFile = 0;
	
	boost::asio::io_context io_context;	
	boost::asio::ip::tcp::socket socket;
	static const int bufSize = 1024;
	char buf[bufSize];
	boost::system::error_code error;

	

	
	void do_read();
	void process(boost::system::error_code ec, std::size_t len);
	data_1 cb1 = 0;
};


