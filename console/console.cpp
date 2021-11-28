#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <iostream>


#include "inproc.h"
#include "eth_client.h"


EthClient cli;

void teeData(char* s, int size) {
	s[size] = 0;
	//printf("\n teeData {%s} \n", s);
	printf("%s", s);
}
void inpData(char* s) {
	//printf(" INPDATA {%s} \n", s);
	cli.do_write(s);
}


int main(int argc, char *argv[]) {
	//printf("starting .. ");
	//boost::asio::io_context io_context1;
	//boost::asio::ip::tcp::socket s1(io_context1);

	
	using namespace std::chrono_literals;
	//printf("console starting .. ");
	//cli.startClient(teeData);


	std::thread inp(inputProc, inpData);
	std::thread tcp([&] { cli.startClient(teeData); } );

	//printf("main thread started \n");
	while (!inpExitRequest) {
		std::this_thread::sleep_for(200ms);
	}
	printf("exiting .. ");
	inpExitRequest = true;
	cli.StopClient();
	inp.join();
	printf("inp thread finished \n");
	tcp.join();
	printf("tcp thread finished \n");
	std::this_thread::sleep_for(20ms);

	printf("console stop\n");
	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	printf("AF: file %s, line %d, (%s)\n", file, line, str);
}

int XQSendInfo(const unsigned char* data, unsigned short int size) {

	return 0;
}