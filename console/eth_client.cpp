
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

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

bool EthClient::checkThePacket(char* buf, int len) {
	using namespace std::chrono_literals;
	if (len < ethHeaderSize) {
		std::cout << "checkThePacket: len="<< len << "; packetsCounter = " << packetsCounter << std::endl;
		return false;
	}
	if (strncmp(buf, "TBWF", 4) != 0) {
		printf("checkThePacket: buf= {%s}, packetsCounter = %u\r\n", buf, packetsCounter);
		return false;
	}
	int size; 
	memcpy(&incomingPacketsCounter, buf + 8, 4);
	memcpy(&size, buf + 4, 4);
	if (size != (len - ethHeaderSize)) {
		
		static const int tc = 5;
		int tCounter = 0;
		size_t bs = 0;

		do {
			std::this_thread::sleep_for(2ms);
			while ((bs = socket.available()) > 0) {
				int haveSpaceInBuffer = bufSize - len;
				int needed = size + ethHeaderSize - len;
				bs = (bs < haveSpaceInBuffer) ? bs : haveSpaceInBuffer;
				bs = (bs < needed) ? bs : needed;
				bs = socket.read_some(boost::asio::buffer((char*)(buf+len), bs), error);
				//bs = socket.read_some((buf+len, bs), error);
				len += bs;

				if (len >= (size+ethHeaderSize)) {
					break;
				}
			}
			if (bs < 1) {
				break;
			}
			tCounter++;
		} while (tCounter < tc);

		if (len >= (size+ethHeaderSize)) {
		} else {
			printf("checkThePacket: size = %d; (len - wfsHeaderSize) = %d; packetsCounter = %d; incomingPacketsCounter=%d\r\n", 
				size, (len - ethHeaderSize), packetsCounter, incomingPacketsCounter);
			
			return false;
		}
	}
	
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
		printf("checkThePacket: packetsCounter = %u, incomingPacketsCounter = %u \r\n", packetsCounter, incomingPacketsCounter);
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
	//printf("do_write  writing %d bytes\r\n", n);

	if (strncmp(s, "get ", 4) == 0) {
		readingTheLogFile = 1;
		currentFileName = s + 4;
		lastReportedProgress = 0;
		while ((currentFileName.length() > 1) && ((currentFileName.back() == '\n') || (currentFileName.back() == '\r'))) {
			currentFileName.pop_back();
		}
		printf("do_write: reading file %s [%s] \r\n", currentFileName.c_str(), s);
	}

	int ret = socket.write_some(boost::asio::buffer(s, n), error);
	if (ret == 0) {
		readingTheLogFile = 0;
		std::cout << "write error: " << error.message() << std::endl;
	} else {
		//printf("socket.write_some = %d\n", ret);
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
		printf("EthClient::process: len = %zd\r\n", len);
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
		assert(len < bufSize);
		buf[bufSize-1] = 0;
		readingTheFile(buf, len);
	} else	if (cb1 != 0) {
		cb1(buf, len);
	}
	do_read();
}

void EthClient::readingTheFile(char* buf, int len) {
	using namespace std::chrono_literals;
	//printf("reading len = %d; (%d) \r\n", len, readingTheLogFile);
	const char* sch;
	switch (readingTheLogFile) {
		case 1:
			cb1(buf, len);
			if (((sch = strstr(buf, "ack,")) != NULL) && (sch[4] != 0)) {
				fileSize = 0;
				int k = sscanf(sch + 4, "%d", &fileSize);
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

						memset(buf, 0, ethHeaderSize);
						memcpy(buf, "TBWF", 4);
						int ret = socket.write_some(boost::asio::buffer(buf, ethHeaderSize), error);

						printf("file %s  opened; \r\n", currentFileName.c_str());
					}
				} else { //  cannot parce answer
					std::cout << "reply parsing error: got " << buf << std::endl;
					readingTheLogFile = 0;
				}

				break;
			}
			if (((sch = strstr(buf, "nak,")) != NULL)) {
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
						printf("%%%d\r\n", lastReportedProgress);
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
				// ?
				std::this_thread::sleep_for(2ms);
				size_t bs;
				while ((bs = socket.available()) > 0) {
					bs = (bs < bufSize) ? bs : bufSize;
					bs = socket.read_some(boost::asio::buffer(buf, bs), error);
					printf("got %d bytes (nak) \r\n", bs);
				}

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