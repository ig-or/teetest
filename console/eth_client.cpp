

#include "eth_client.h"
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "xmutils.h"

std::mutex mu;
std::condition_variable cv;
static const int ethHeaderSize = 16;

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

bool EthClient::checkThePacket(const char* buf, int len) {
	if (len < ethHeaderSize) {
		std::cout << "EthClient::checkThePacket: len="<< len << "; packetsCounter = " << packetsCounter << std::endl;
		return false;
	}
	if (strncmp(buf, "TBWF", 4) != 0) {
		std::cout << "EthClient::checkThePacket: buf="<< buf << "; packetsCounter = " << packetsCounter <<  std::endl;
		return false;
	}
	int size; 
	memcpy(&size, buf + 4, 4);
	if (size != (len - ethHeaderSize)) {
		std::cout << "checkThePacket: size = "<< size << 
			"; (len - wfsHeaderSize) = " << (len - ethHeaderSize) << 
			"; packetsCounter = " << packetsCounter << std::endl;
		return false;
	}
	
	memcpy(&incomingPacketsCounter, buf + 8, 4);
	unsigned short int realCRC = CRC2::crc16(0, buf + ethHeaderSize, size);
	unsigned short int incomingCRC;
	memcpy(&incomingCRC, buf + 12, 2);
	if (incomingCRC != realCRC) {
		std::cout << "checkThePacket: crc error" << "; packetsCounter = " << packetsCounter << std::endl;
		return false;
	}
	if ((buf[14] != 0) || (buf[15] != 0)) {
		std::cout << "checkThePacket: EOH error" << "; packetsCounter = " << packetsCounter << std::endl;
		return false; 
	}
	if (packetsCounter != incomingPacketsCounter) {
		std::cout << "checkThePacket: packetsCounter = " << packetsCounter << 
			"; incomingPacketsCounter = " << incomingPacketsCounter << std::endl;
		packetsCounter = incomingPacketsCounter;
	}

	return true;
}


int EthClient::do_write(const char* s) {

	
	if (s == 0) {
		return 0;
	}
	int n = strlen(s);
	if (n == 0) {
		return 0;
	}
	std::cout << "EthClient::do_write  writing " << strlen(s) << " bytes:   " << s;
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

			printf("do_write: reading file %s [%s] \r\n", currentFileName.c_str(), s);
		}
	}
	return ret;
}

void EthClient::process(boost::system::error_code ec, std::size_t len) {
	if ((ec) || (pleaseStop)) {
		//socket.close();
		printf("EthClient::process error ec=%s; pleaseStop=%s \n", ec.message().c_str(), pleaseStop ? "yes" : "no");
		if ((ec == boost::asio::error::eof) || (ec == boost::asio::error::connection_reset)) {
			connected = false;
			socket.close();
			printf("disconnected ? ");
		}
		return;
	}
	//std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	if (len < 1) {
		printf("EthClient::process: len = %d\r\n", len);
		return;
	}
	if (ping1 != 0) {
		ping1();
	}
	if ((len >= 3) && (strncmp(buf, "tee", 3) == 0)) {
		connected = true;
		std::lock_guard<std::mutex> lk(mu);
		cv.notify_all();
		//printf("tee!\n");
		
	} else if ((len >= 4) && (strncmp(buf, "ping", 4) == 0)) { //  ping

	} else if (readingTheLogFile != 0) {
		readingTheFile(buf, len);
	} else	if (cb1 != 0) {
		cb1(buf, len);
	}
	do_read();
}

void EthClient::readingTheFile(char* buf, int len) {
	printf("EthClient::readingTheFile; len = %d \r\n", len);
	switch (readingTheLogFile) {
		case 1:
			cb1(buf, len);
			if (strncmp(buf, "ack,", 4) == 0) {
				fileSize = 0;
				int k = sscanf(buf + 4, "%d", &fileSize);
				if (k == 1) {
					std::cout << "\t got ack; fileSize = " << fileSize << std::endl;
					theFile = fopen(currentFileName.c_str(), "wb"); //  TODO: do this BEFORE sending the request to the TEENSY
					if (theFile == 0) {
						readingTheLogFile = 0;
						std::cout << "file write error: file " << currentFileName << " is probably locked" << std::endl  << std::endl;
						//gfState = 0;

					} else { //  all is good
						readingTheLogFile = 2;
						fileBytesCounter = 0;
						packetsCounter = 0;
						badPacketCounter = 0;
						counterErrorCounter = 0;
						std::cout << "file " << currentFileName << " opened; " << std::endl;
					}
				} else { //  cannot parce answer
					std::cout << "reply parsing error: got " << buf << std::endl;
					readingTheLogFile = 0;
				}

				break;
			}
			if (strncmp(buf, "nak,", 4) == 0) {
				readingTheLogFile = 0;
				std::cout << "cannot get this file (got NAK) " << std::endl;
				break;
			}

			break;

		case 2:
			if (checkThePacket(buf, len)) {
				//  send the ACK:
				int ret = socket.write_some(boost::asio::buffer(buf, ethHeaderSize), error);
				if (ret == 0) {
					readingTheLogFile = 0;
					std::cout << "write error while sending ACK to the teensy: " << error.message() << std::endl;
				} else { //  OK!
				}

				if (incomingPacketsCounter < packetsCounter) { //  looks like we got this already
					std::cout << "skipping the incoming packet #" << incomingPacketsCounter << 
						"(packetsCounter = " << packetsCounter << std::endl;
					counterErrorCounter += packetsCounter - incomingPacketsCounter;
				} else {
					fwrite(buf + ethHeaderSize, 1, len - ethHeaderSize, theFile);

					fileBytesCounter += (len - ethHeaderSize);
					++packetsCounter;
				}

				{
					int p = fileBytesCounter * 100 / fileSize;
					if ((p != lastReportedProgress) && ((p % 10) == 0)) {
						lastReportedProgress = p;
						std::cout << lastReportedProgress << "%" << std::endl;
					}
				}

				if (fileBytesCounter >= fileSize) {
					readingTheLogFile = 0;
					lastReportedProgress = 100;
					fclose(theFile);
					theFile = 0;
					std::cout << "reading the file finished; fileBytesCounter = " << fileBytesCounter << 
						"; fileSize = " << fileSize << 
						"; incomingPacketsCounter = " << incomingPacketsCounter <<
						"; packetsCounter = " << packetsCounter << 
						"; badPacketCounter = " << badPacketCounter <<
						"; counterErrorCounter = " << counterErrorCounter << std::endl;

					//using namespace boost::filesystem;
					//path e = path(currentFileName).extension();
					//if (e.string().compare(".log") == 0) { //  this was a log file
					//	parceLogFile(currentFileName.c_str());
					//}
				}
			} else {
				//  send the NAK:
				++badPacketCounter;
				memcpy(buf, "TBWN", 4);
				int ret = socket.write_some(boost::asio::buffer(buf, ethHeaderSize), error);
				if (ret == 0) {
					readingTheLogFile = 0;
					std::cout << "write error while sending NAK to the teensy: " << error.message() << std::endl;
				} else { //  OK!
				}
			}
			break;
		default: break;
	};

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