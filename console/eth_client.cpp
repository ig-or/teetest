

#include "eth_client.h"
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

std::mutex mu;
std::condition_variable cv;

EthClient::EthClient() : socket(io_context) {
	pleaseStop = false;
	connected = false;
	readingTheLogFile = 0;
	theFile = 0;
	pingReply = 0;
	cb1 = 0;
}

void EthClient::startClient(data_1 cb, vf ping) {
	using boost::asio::ip::tcp;
	using boost::asio::ip::address;

	pleaseStop = false;
	connected = false;
	readingTheLogFile = 0;
	theFile = 0;
	lastReportedProgress = 0;
	cb1 = cb;
	ping1 = ping;

	try {
		//tcp::resolver resolver(io_service);
		std::cout << "connecting .... ";
		
		tcp::endpoint ep(address::from_string("192.168.0.177"), 8889);
		
		socket.async_connect(ep,
			[this](boost::system::error_code ec) {
			if (!ec) {
				printf("connected to server\n");
				do_write("console\r\n");
				do_read();
			} else {
				int itmp = 0;
			}
		});
		
		//std::cout << "OK!" << std::endl;  this is not true at this point
		io_context.run();
		
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

int EthClient::do_write(const char* s) {

	//std::cout << "writing " << strlen(s) << " bytes:   ";
	if (s == 0) {
		return 0;
	}
	int n = strlen(s);
	if (n == 0) {
		return 0;
	}
	int ret = socket.write_some(boost::asio::buffer(s, n), error);
	if (ret == 0) {
		std::cout << "write error: " << error.message() << std::endl;
	} else {
		//printf("socket.write_some = %d\n", ret);
		if (strncmp(s, "get ", 4) == 0) {
			readingTheLogFile = 1;
			currentFileName = s + 4;
			lastReportedProgress = 0;

			while ((currentFileName.length() > 1) && ((currentFileName.back() == '\n') || (currentFileName.back() == '\r'))) {
				currentFileName.pop_back();
			}
		}
	}
	return ret;
}

void EthClient::process(boost::system::error_code ec, std::size_t len) {
	if ((ec) || (pleaseStop)) {
		//socket.close();
		printf("EthClient::process error ec=%s \n", ec.message().c_str());
		if ((ec == boost::asio::error::eof) || (ec == boost::asio::error::connection_reset)) {
			connected = false;
			socket.close();
			printf("disconnected ? ");
		}
		return;
	}
	//std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	if (len < 1) {
		return;
	}
	if (ping1 != 0) {
		ping1();
	}
	if (strncmp(buf, "tee", 3) == 0) {
		connected = true;
		std::lock_guard<std::mutex> lk(mu);
		cv.notify_all();
		//printf("tee!\n");
		
	} else if (strncmp(buf, "ping", 4) == 0) { //  ping

	} else 	if (cb1 != 0) {
		cb1(buf, len);
	}
	do_read();
}

void EthClient::do_read() {
	
	socket.async_read_some(boost::asio::buffer(buf, bufSize - 2), 
		[this](boost::system::error_code ec, std::size_t len) { process(ec, len);  });

	if (pleaseStop) {
		return;
	}
	
}


void EthClient::StopClient() {
	pleaseStop = true;
	//socket.close();
	connected = false;
	io_context.post([this]() { 
		socket.close(); 
	});
}	