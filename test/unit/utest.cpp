
#include "../../src/motordriver.h"
#include "stdio.h"
#include <stdarg.h>
#include "xmatrixplatform.h"

bool encTest() {
	Motor m;
	m.mSoftReset();
	FILE* f = fopen("enctest.csv", "wt");
	if (f == 0) {
		printf("ERROR cannot open output file \n");
		return false;
	}
	printf("encTest \n");

	constexpr float time1 = 1.0f;
	const int encNumber = 1920; ///< impulses per revolution
	constexpr int ep = 20; // ms
	constexpr int tSize = time1 * 1000.0f / float(ep);
	constexpr float tEnd = float(tSize * ep) / 1000.0f; // end of the test , seconds
	float t[tSize];
	float p[1][tSize];
	//float ret[1][tSize];
	int i;

	float s1 = 0.1; // rad/sec
	for (i = 0; i < tSize; i++) {
		t[i] = float(i*ep) / 1000.0f;

		p[0][i] = s1 * t[i] * encNumber / (2.0 * pii);
	}

	//  test
	for (i = 0; i < tSize; i++) {
		m.encBuf.rAdd(p[0][i]);

		m.updateEncSpeed();
		//ret[0][i] = m.encSpeed;
		fprintf(f, "%.3f\t%f\t%f\n", t[i], m.encSpeed, m.encSpeedSimple);
	}


	fclose(f);
	return true;	
}

int main() {
	encTest();


	return 0;
}

void assert_failed(const char* file, unsigned int line, const char* str) {
	printf("AF %s line %d   %s \n", file, line, str);

}

uint32_t msNow = 0;
uint32_t mksNow = 0;


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

	ok = vsnprintf(sbuf + bs, sbSize - 1 - bs, s, args);
	if ((ok <= 0) || (ok >= (sbSize - bs))) {
		strcpy(sbuf, " errror 2\n");
	}

writeHere:
	int eos = strlen(sbuf);
	sbuf[sbSize - 1] = 0;

//		usb_serial_write((void*)(sbuf), eos);
	printf(sbuf);

	va_end(args);
	sprintCounter++;

	return 0;
}

