#include "logfile.h"
#include "SdFat.h"
#include "rbuf.h"
#include "teetools.h"


const uint64_t fileSize = 10485760LL;
// Use FIFO SDIO.
#define SD_CONFIG SdioConfig(FIFO_SDIO)

ByteRoundBuf rb;
static const int rbSize = 2048;
unsigned char rbBuf[rbSize];
static const int tmpSize = 512;
unsigned char tmpBuf[tmpSize];
SdFs sd;
FsFile file;
volatile int feedingFlag = 0;
volatile int ffErrorCounter = 0;
volatile int ffOverflowCounter = 0;
volatile bool lfWriting = false;
char fileNameCopy[64];
enum LFState {
	lfSInit,
	lfSGood,
	lfSError
};

LFState lfState = lfSInit;

void lfPrint() {
	xmprintf(0, "file %s: \tlfState=%d; lfWriting=%s; ffErrorCounter=%d; ffOverflowCounter=%d      \r\n", 
		fileNameCopy, (int)(lfState), lfWriting?"yes":"no", 
		ffErrorCounter, ffOverflowCounter);
}

void lfInit() {
	bool irq = disableInterrupts();
	initByteRoundBuf(&rb, rbBuf, rbSize);
	feedingFlag = 0;
	ffErrorCounter = 0;
	ffOverflowCounter = 0;
	lfWriting = false;
	enableInterrupts(irq);

	if (!sd.begin(SD_CONFIG)) {
    	//sd.initErrorHalt(&Serial);
		lfState = lfSError;
  	}
	lfState = lfSGood;
}

void lfStart(const char* fileName) {
	if ((lfState != lfSGood) || lfWriting) { // already
		return;
	}
	strncpy(fileNameCopy, fileName, 64);
	bool irq = disableInterrupts();
		resetByteRoundBuf(&rb);
		feedingFlag = 0;
		ffErrorCounter = 0;
		ffOverflowCounter = 0;
	enableInterrupts(irq);

	if (!file.open(fileName, O_CREAT | O_TRUNC | O_WRONLY)) {
		//sd.errorHalt("file.open failed");
		lfState = lfSError;
		return;
	}
	if (!file.preAllocate(fileSize)) {
		//sd.errorHalt("file.preAllocate failed");
		lfState = lfSError;
		return;
	}
	lfWriting = true;
	xmprintf(0, "log started\r\n");
}

void lfStop() {
	if ((lfState != lfSGood) || (!lfWriting)) { 
		return;
	}
	int n, bs;
	bool irq = disableInterrupts();
	lfWriting = false;
	n = rb.num;
	enableInterrupts(irq);

	while (n > 0) { // this can take some time, code can be interrupted
		bs = (n > tmpSize) ? tmpSize : n;
		bool irq = disableInterrupts();
		get_rb_s(&rb, tmpBuf, bs);
		n = rb.num;
		enableInterrupts(irq);
		file.write(tmpBuf, bs);
	}

	if (!file.truncate()) {
    	//sd.errorHalt("truncate failed");
		lfState = lfSError;
  	}
	file.close();
	xmprintf(0, "log stopped\r\n");
}

void lfFeed(void* data, int size) {
	if ((lfState != lfSGood) || (!lfWriting)) { 
		return;
	}

	// "lock"
	// FIXME: do this using special CPU instructions
	int test = feedingFlag;
	feedingFlag = 1;
	if (test != 0) { //   looks like we interrupted lower prio code
		++ffErrorCounter;
		feedingFlag = 0;
		return;
	}
	int freeBytes = rb.count - rb.num;
	if (freeBytes < size) { //   will not fit
		++ffOverflowCounter;
		feedingFlag = 0;
		return;
	}
	put_rb_s(&rb, (const unsigned char*)data, size); // add data to rb

	feedingFlag = 0;
}

void lfProcess() {
	if ((lfState != lfSGood) || (!lfWriting)) { 
		return;
	}
	int n;
	bool irq = disableInterrupts();
	n = rb.num;
	enableInterrupts(irq);

	while (n >= 512) {
		bool irq = disableInterrupts();
		get_rb_s(&rb, tmpBuf, tmpSize);
		n = rb.num;
		enableInterrupts(irq);
		file.write(tmpBuf, tmpSize);
	}
}







