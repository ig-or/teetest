#include <Arduino.h>
#include "usb_serial.h"
#include <stdarg.h>
#include "SdFat.h"

#include "teetools.h"
#include "motordriver.h"
#include "eth.h"

uint32_t msNow = 0;
uint32_t mksNow = 0;

int ptf(FsFile& file, const char* s, ...); 

//  from https://github.com/sstaub/TeensyID/blob/master/TeensyID.cpp
static uint32_t getTeensySerial(void) {
	uint32_t num;
	num = HW_OCOTP_MAC0 & 0xFFFFFF;
	return num;
}
void teensySN(uint8_t *sn) {
	uint32_t num = getTeensySerial();
	sn[0] = num >> 24;
	sn[1] = num >> 16;
	sn[2] = num >> 8;
	sn[3] = num;
}

const char* teensySN(void) {
	uint8_t serial[4];
	static char teensySerial[16];
	teensySN(serial);
	sprintf(teensySerial, "%02x-%02x-%02x-%02x", serial[0], serial[1], serial[2], serial[3]);
	return teensySerial;
}

void logSetup(const char* fileName) {
	char fn[32];
	xmprintf(0, "logSetup! %s\r\n", fileName);
	strncpy(fn, fileName, 16);
	strcat(fn, "_s.txt");
	FsFile f;

	if (!f.open(fn, O_CREAT | O_TRUNC | O_WRONLY)) {
		xmprintf(0, "could not open file %s for writing \r\n", fn);
		return;
	}
	if (!f.preAllocate(1024)) {
		xmprintf(0, "can not preallocate space for a file %s \r\n", fn);
		return;
	}

	ptf(f, "%s %s settings\n\ntime %.3f s", 
		teensySN(), fileName, (((float)(millis())) / 1000.0f));

	for (int i = 0; i < 2; i++) {
		ptf(f, "m%d_PID %f  %f  %f\n", m[i].id,  m[i].pid.P, m[i].pid.I, m[i].pid.D);
	}

	ptf(f, "pwmResolution %d;  maxPWM %d\nadcResolution %d;  maxADC %d\na2mv %f;  a2ma %f", 
		pwmResolution, maxPWM, adcResolution, maxADC, a2mv, a2ma);


	if (!f.truncate()) {

  	}
	f.close();

}

LedIndication::LedIndication() {

}
void LedIndication::liSetMode(LIMode m, float f) {
	liMode = m;
	if (f < 0.05f) {
		f = 1.0f;
	}
	period = std::ceil(1000.0f / f);
	halfPeriod = period >> 1;
	mph = maxPower / halfPeriod;
}

int LedIndication::liGet(unsigned int ms) {
	int phase = (ms % period);
	int p = 0;
	switch (liMode) {
		case LIStop:
			break;
		case LIRamp:
			if (phase < halfPeriod) {
				p = phase * mph;
			} else {
				p = maxPower - (phase - halfPeriod)*mph;
			}
			if ( p > maxPower) {
					p = maxPower;
			}
			break;
	};
	return p;
}
LedIndication led1;

static uint32_t sprintCounter = 0;
static const int sbSize = 512;
static char sbuf[sbSize];

int ptf(FsFile& file, const char* s, ...) {
	va_list args;
	va_start(args, s);
	int ok;

	ok = vsnprintf_P(sbuf, sbSize - 1, s, args);
	va_end(args);

	if ((ok <= 0) || (ok >= (sbSize))) {
		return 1;
	}
	unsigned int eos = strlen(sbuf);
	if (eos >= sbSize) {
		eos = sbSize-1;
	}
	sbuf[eos] = 0;
	int test = file.write(sbuf, eos+1);
	if (test <= 0) {
		return 2;
	}

	return 0;
}

int xmprintf(int dst, const char* s, ...) {
	va_list args;
	va_start(args, s);
	int ok;

	int bs = 0;

	if (dst & 1) {
		bs = snprintf(sbuf, 32, "%u\t%.3f\t", sprintCounter, 1000.0f / 1000.0f);
		if ((bs <= 0) || (bs >= 32)) {
			strcpy(sbuf, "errror 1\n");
			goto writeHere;
		}
	}

	ok = vsnprintf_P(sbuf + bs, sbSize - 1 - bs, s, args);
	va_end(args);
	if ((ok <= 0) || (ok >= (sbSize - bs))) {
		strcpy(sbuf, " errror 2\n");
	}

writeHere:
	sbuf[sbSize - 1] = 0;
	int eos = strlen(sbuf);

//	if (usbSerialWorking /*&& (dst & 4)*/) {
	if ((dst & 1) == 0) {
		usb_serial_write((void*)(sbuf), eos);
	}
	if ((dst & 2) == 0) {
		ethFeed(sbuf, eos);
	}
	
	sprintCounter++;

	return 0;
}

void assert_failed(const char* file, unsigned int line, const char* str) {
	xmprintf(0, "AF: file %s, line %d, %s \r\n", file, line, str);
}

int  xm_printf(const char * s, ...) {
	va_list args;
	va_start(args, s);
	int ok;

	int bs = 0;

	ok = vsnprintf_P(sbuf + bs, sbSize - 1 - bs, s, args);
	if ((ok <= 0) || (ok >= (sbSize - bs))) {
		strcpy(sbuf, " errror 2\n");
	}

writeHere:
	int eos = strlen(sbuf);
	sbuf[sbSize - 1] = 0;

//	if (usbSerialWorking /*&& (dst & 4)*/) {
		usb_serial_write((void*)(sbuf), eos);
//	}

	va_end(args);
	sprintCounter++;

	return 0;
}

