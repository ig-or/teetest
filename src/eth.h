

#pragma once

enum GFrequest {
	gfFinish = 0,
	gfPleaseSendNext,
	gfPleaseRepeat,
	gfEmpty
};

typedef void(*EthInfoHandler)(GFrequest);

void ethSetup();
void ethLoop();
void ethPrint();
void ethFeed(char* s, int size);
void ethSetInfoHandler(EthInfoHandler h);