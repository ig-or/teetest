

#include "logsend.h"
#include "logfile.h"

#include "SdFat.h"
#include "sdios.h"
#include "rbuf.h"
#include "teetools.h"
#include "xmessage.h"

#include "xmutils.h"
#include "eth.h"
#include "xmessagesend.h"


static unsigned char sendFileViaEthID = 0;
//#define	NAK (xmprintf(0, "nak,%d\n", n))
//#define ACK(n)  (xmprintf(0, "ack,%d\n", n))
static unsigned int  ethFileStartTime = 0;
static unsigned int ethFileBytesCounter = 0;
static int sendFilePacketsCounter = 0;
static FsFile ethFile;
unsigned long long  ethFileSize = 0LL;
static unsigned int blockSendTime = 0;
static int repeatSendCounter = 0;
static const unsigned int ethSendTimeout = 1200;  ///< [ms]
static const int maximumRepeats = 5;
static unsigned int fileReadInfoSize = 0;
//unsigned char* rbb = 0;

void sendFileViaEthHandler(GFrequest r);
/*!
 * reply on 'get file' request.
 * 
 * \param yes
 * \param info   the file size
 */
void ethACK(bool yes, int info) {
	char s[32];
	if (yes) {
		snprintf(s, 32, "ack,%d\r\n", info);
	} else {
		snprintf(s, 32, "nak,%d\r\n", info);
	}
	//xmprintf(1, s);
	ethFeed(s, strlen(s));
}

/*!
 *  send file state.
 * 
 */
enum SFWState {
	sfwStart = 0,  ///< the very beginning
	sfwWorking,
	sfwComplete,
	sfwError
};
static SFWState sfwState = sfwStart;
static const int ethHeaderSize = 16;

int lfGetFile(const char* name) {
	//rbb = getRBuf();
	if (lfWriting) {
		ethACK(false, 0);	
		xmprintf(0, "writing file %s right now \r\n", fileNameCopy);
		return 1;
	}

	if (lfState != lfSGood) { // 
		ethACK(false, 0);	
		xmprintf(0, "SD didnt started ? \r\n");
		return 2;
	}
	//mxat(lfState == lfSGood);
	mxat(!ethFile.isOpen());
	if (!ethFile.open(name)) { 
		ethACK(false, 0);	
		xmprintf(0, "cannot open file {%s} (bad file name?) \r\n", name);
		return 3;
	}
	ethFileSize = ethFile.size();
	if (ethFileSize <= 0) {
		ethACK(false, 0);	
		xmprintf(0, "file empty \r\n");
		return 4;
	}

	ethFileBytesCounter = 0;
	sendFilePacketsCounter = 0;

	printfToEth(false);
	ethSetInfoHandler(sendFileViaEthHandler);
	ethFileStartTime = millis();
	ethACK(true, ethFileSize);	// lets go!
	led1.liSetMode(LedIndication::LIRamp, 3.5);
	xmprintf(2, "start log file %s reading \r\n", name);

	return 0;
}

void sendFileViaEthComplete() {
	ethSetInfoHandler(0);
	ethFile.close();
	unsigned int t2 = millis();
	printfToEth(true);
	//removeWifiTxEmptyHandler();
	led1.liSetMode(LedIndication::LIRamp, 0.8);
	//resetByteRoundBuf(&rb); // ????????
	xmprintf(0, "getFileViaEth ended in %d ms; bytesCounter = %u; %d packets were sent; sfwState = %d\r\n", 
		t2 - ethFileStartTime, ethFileBytesCounter, sendFilePacketsCounter, sfwState);
	//sfwState = sfwComplete;
}

/*!
 *  send out the info.
 * 
 * \param buf all the info to send. useful info starts at 'buf + EthHeaderSize' address
 * \param size size of the useful info. So we supposed to send (size + wfsHeaderSize) bytes
 */
void sendEthPacket(char* buf, int size, unsigned int counter) {
	// create a header:
	memcpy(buf, "TBWF", 4);			// header
	memcpy(buf + 4, &size, 4);		// info size
	memcpy(buf + 8, &counter, 4);	//  counter
	unsigned short int crc = CRC2::crc16(0, buf + ethHeaderSize, size); // CRC
	memcpy(buf + 12, &crc, 2);
	buf[14] = 0;  buf[15] = 0;		//  END

	//asyncSendInfo(sendFileViaWifiID, buf, size + wfsHeaderSize);
	ethFeed(buf, size+ethHeaderSize);

	blockSendTime = millis();	// time
}


void sendFileViaEthHandler(GFrequest r) {
	//if (tcpConnections[sendFileViaWifiID] == 0) {
	//	sendFileViaWifiComplete();
	//}

	
	//xmprintf(2, "sendFileViaEthHandler: %d\r\n", r);

	const int bSize = 1024;
	unsigned int now = millis();

	switch (r) {
	case gfEmpty:
		switch (sfwState) {
		case sfwStart: //  initial send
			ethFileBytesCounter = 0;
			sendFilePacketsCounter = 0;
			fileReadInfoSize = ethFile.read(rbBuf + ethHeaderSize, bSize - ethHeaderSize);
			ethFileBytesCounter += fileReadInfoSize;
			if (fileReadInfoSize > 0) {
				sendEthPacket(rbBuf, fileReadInfoSize, sendFilePacketsCounter);
				sfwState = sfwWorking;
				repeatSendCounter = 0;
			} else {  //  error: failed to read any info from the file
				sfwState = sfwError;
				// send error message to the client....
				xmprintf(0, "sendFileViaEthHandler: 1: fileReadInfoSize = %d\r\n", fileReadInfoSize);
				sendFileViaEthComplete();
			}
			break;

		case sfwWorking: // timeout handling
			if ((now - blockSendTime) > ethSendTimeout) {
				++repeatSendCounter;
				if (repeatSendCounter >= maximumRepeats) {
					sfwState = sfwError;
					// (try to) send error message to the client....
					xmprintf(0, "sendFileViaWifiHandler: 2: repeatSendCounter >= maximumRepeats; repeatSendCounter = %d\r\n", repeatSendCounter);

					sendFileViaEthComplete();
				} else {
					sendEthPacket(rbBuf, fileReadInfoSize, sendFilePacketsCounter); // lets send it again
				}
			}
			break;
		};
		break;

	case gfPleaseSendNext:
		repeatSendCounter = 0;
		++sendFilePacketsCounter;
		fileReadInfoSize = ethFile.read(rbBuf + ethHeaderSize, bSize - ethHeaderSize);
		ethFileBytesCounter += fileReadInfoSize;
		if (fileReadInfoSize > 0) {
			sendEthPacket(rbBuf, fileReadInfoSize, sendFilePacketsCounter);
		} else { //  EOF?
			if (ethFileBytesCounter == ethFileSize) { //  yes!
				// send message to the client....
				sfwState = sfwComplete;
				xmprintf(0, "sendFileViaEthHandler: 3  sfwComplete!\r\n");
			} else { //  error?
				sfwState = sfwError;
				xmprintf(0, "sendFileViaEthHandler: 4:  sfwError! fileReadInfoSize = %d; wifiFileBytesCounter = %d; wFileSize = %d\r\n", 
						  fileReadInfoSize, ethFileBytesCounter, ethFileSize);
			}
			sendFileViaEthComplete();
		}
		break;

	case gfPleaseRepeat:
		++repeatSendCounter;
		if (repeatSendCounter >= maximumRepeats) {
			sfwState = sfwError;
			// (try to) send error message to the client....
			xmprintf(0, "sendFileViaEthHandler: wehPleaseRepeat: sfwError! repeatSendCounter >= maximumRepeats; repeatSendCounter = %d  \r\n", 
				repeatSendCounter);

			sendFileViaEthComplete();
		} else {
			sendEthPacket(rbBuf, fileReadInfoSize, sendFilePacketsCounter); // lets send it again
		}
		break;

	};
}







