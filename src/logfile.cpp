#include "logfile.h"
#include "SdFat.h"
#include "rbuf.h"
#include "teetools.h"
#include "xmessage.h"
#include "xmessagesend.h"

const uint64_t fileSize = 10485760LL;
constexpr uint64_t fileSize2 = fileSize - 16*512;
// Use FIFO SDIO.
#define SD_CONFIG SdioConfig(FIFO_SDIO)

volatile ByteRoundBuf rb;
static const int rbSize = 4096; //8192;
static unsigned char __attribute__((aligned(32))) rbBuf[rbSize];

//  buffer for writing to file
static const int tmpSize = 512;
unsigned char tmpBuf[tmpSize];
SdFs sd;
FsFile file;
volatile int feedingFlag = 0;
volatile int ffErrorCounter = 0;
volatile int ffOverflowCounter = 0;
volatile bool lfWriting = false;
int fileCounter = 0;
unsigned long long fileBytesCounter = 0LL;
char fileNameCopy[64];

volatile LFState lfState = lfSInit;
volatile static bool sdStarted = false;

void lfPrint() {
	xmprintf(0, "file %s: \tlfState=%d; lfWriting=%s; ffErrorCounter=%d; ffOverflowCounter=%d; rb.overflow=%d; rb.error=%d; sdStarted=%s      \r\n", 
		fileNameCopy, (int)(lfState), lfWriting?"yes":"no", 
		ffErrorCounter, ffOverflowCounter,
		rb.overflowCount, rb.errorCount,
		sdStarted?"yes":"no"	
	);
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
		xmprintf(0, "cannot start CD card (no card in slot?)\r\n");
		lfState = lfSError;
		sdStarted = false;
  	} else {
		sdStarted = true;
		lfState = lfSGood;
		xmprintf(0, "sd card started \r\n");
	}
}

void lfStart(const char* fileName) {
	xmprintf(0, "lfStart %s \r\n", fileName);
	if (lfWriting) { // already
		xmprintf(0, "writing %s already \r\n", fileNameCopy);
		return;
	}
	if (!sdStarted) { //  try to start?
		lfInit();
		if (!sdStarted) {
			xmprintf(0, "cannot2 start CD card (no card in slot?)\r\n");
			return;
		}
	}

	strncpy(fileNameCopy, fileName, 64);
	fileCounter = 0;
	fileBytesCounter = 0LL;

	bool irq = disableInterrupts();
		resetByteRoundBuf(&rb);
		feedingFlag = 0;
		ffErrorCounter = 0;
		ffOverflowCounter = 0;
	enableInterrupts(irq);

	if (!file.open(fileName, O_CREAT | O_TRUNC | O_WRONLY)) {
		//sd.errorHalt("file.open failed");
		lfState = lfSError;
		xmprintf(0, "cannot open file %s for writing \r\n", fileName);
		return;
	}
	if (!file.preAllocate(fileSize)) {
		//sd.errorHalt("file.preAllocate failed");
		lfState = lfSError;
		xmprintf(0, "cannot preallocate space for log file; card full?\r\n");
		return;
	}
	lfWriting = true;
	lfState = lfSGood;
	xmprintf(0, "log started\r\n");
}

void lfStop() {
	if (!lfWriting) {
		xmprintf(0, "not writing now \r\n");
	}
	if (!sdStarted) {
		xmprintf(0, "sd card didnt started \r\n");
		return;
	}

	if (lfState != lfSGood){ 
		xmprintf(0, "state not good \r\n");
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
	//int test = feedingFlag;
	feedingFlag = 1;
	//if (test != 0) { //   looks like we interrupted lower prio code
	//	++ffErrorCounter;
	//	feedingFlag = 0;
	//	return;
	//}
	bool irq = disableInterrupts();
	int freeBytes = rb.count - rb.num;
	if (freeBytes < size) { //   will not fit
		++ffOverflowCounter;
		feedingFlag = 0;
		enableInterrupts(irq);
		return;
	}
	put_rb_s(&rb, (const unsigned char*)data, size); // add data to rb
	enableInterrupts(irq);
	feedingFlag = 0;
}


#define  headerSize (sizeof(xqm::MsgHeader))
static volatile unsigned char counter = 0;  // log file message counter

/** THIS FUNCTION BELOW  SHOULD BE REENTRANT!!!!
 *  It is called simultaneosly from different interupt levels
 * */
int lfSendMessage(const unsigned char* data, unsigned char type, unsigned short int size) {
	// IS IT OK TO USE STATIC VARIABLES BELOW?
	if ((!sdStarted) || (lfState != lfSGood) || (!lfWriting)) { 
		return;
	}
	xqm::MsgHeader hdr;
	int ret = size + headerSize + 2;  //  size of all the message
	unsigned char cr[2];
	unsigned char crc = 0;
	unsigned char counterCopy = counter; //  COUNTER MAY BE NOT CORRECT !!!!
	// the solution could be to move counter read/increment inside interrupt disable block,
	// but this is not good since we have to disable interrupts for longer time..
	// TODO: create better solution

	// create message header
	hdr.hdr[0] = 'X'; hdr.hdr[1] = 'M'; hdr.hdr[2] = 'R'; hdr.hdr[3] = ' ';
	hdr.count = counterCopy; hdr.length = size; hdr.type = type; 
	// update crc
	XQ_updateCRC(crc, (unsigned char*)(&hdr), headerSize);
	XQ_updateCRC(crc, data, size);
	//  message tail
	cr[0] = crc; cr[1] = '\n';

	// *******************************************************************
	bool irq = disableInterrupts();
	int freeBytes = rb.count - rb.num;
	if (freeBytes < ret) { //   will not fit
		++ffOverflowCounter;
		enableInterrupts(irq);
		return 0;
	}
	put_rb_s(&rb, (const unsigned char*)(&hdr), headerSize); 	// add header
	put_rb_s(&rb, data, size); 									// add message body
	put_rb_s(&rb, cr, 2); 										// add message tail
	++counter;
	enableInterrupts(irq);
	// ***************************************************************
	return ret;
}

/*
int XQSendInfo(const unsigned char* data, unsigned short int size) {
	lfFeed((void*)data, size);
	return 0;
}
*/
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
		fileBytesCounter += tmpSize;
		if (fileBytesCounter > fileSize2) { //  start another file
			if (!file.truncate()) {
    			//sd.errorHalt("truncate failed");
				lfState = lfSError;
  			}
			file.close();

			//  create another file name
			fileCounter++;
			fileBytesCounter = 0LL;
			char fn[75];
			char ntmp[16];
			strncpy(fn, fileNameCopy, 64);
			snprintf(ntmp, 16, "_%d", fileCounter);
			strncat(fn, ntmp, 8);

			// open another file
			if (!file.open(fn, O_CREAT | O_TRUNC | O_WRONLY)) {
				lfState = lfSError;
				return;
			}
			if (!file.preAllocate(fileSize)) {
				lfState = lfSError;
				return;
			}
		}
	}
}







