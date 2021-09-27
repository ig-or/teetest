#include <Arduino.h>
#include "usb_serial.h"
#include <stdarg.h>

#include "teetools.h"


static uint32_t sprintCounter = 0;
static const int sbSize = 512;
static char sbuf[sbSize];

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

