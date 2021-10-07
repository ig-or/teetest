
#include <Arduino.h>
#include "usb_serial.h"

#include "cmdhandler.h"
#include "teetools.h"
#include "ttpins.h"
#include "motordriver.h"


static const int incomingUsbSerialInfoSize = 32;
static char incomingUsbSerialInfo[incomingUsbSerialInfoSize];

static const int infoSize = 88;
static char info[infoSize];
static int infoIndex = 0;

void h_usb_serial() {
	int bs1;
	while ((bs1 = usb_serial_available())) {
		int bs2 = (bs1 <= incomingUsbSerialInfoSize) ? bs1 : incomingUsbSerialInfoSize;
		int bs3 = usb_serial_read(incomingUsbSerialInfo, bs2);
		if (bs3 > 0) {
			//usb_serial_write(incomingUsbSerialInfo, bs3); //  this is for debugging and testing
			onIncomingInfo(incomingUsbSerialInfo, bs3);
		}
	}
}

void onIncomingInfo(char* s, int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (i + infoIndex >= (infoSize-1)) {  //   overflow?
			infoIndex = 0;  //  reset!
		}
		if ((s[i] == 0) || (s[i] == '\n') || (s[i] == '\r')) {
			info[infoIndex] = 0;
			processTheCommand(info, infoIndex);

			infoIndex = 0;
		} else {
			info[infoIndex] = s[i];
			infoIndex++;
		}
	}
}

int processTheCommand(const char* s, int size) {
    xmprintf(0, "got cmd size=%d (%s)", size, s);
	if (size == 0) {
		size = strlen(s);
	}
	float ms1, ms2;
	if (strcmp(s, "inc speed") == 0) {
		getMSpeed(ms1, ms2);
		ms1 += 0.1; ms2 += 0.1;
		setMSpeed(ms1, ms2);
		return 0;
	}
	if (strcmp(s, "dec speed") == 0) {
		getMSpeed(ms1, ms2);
		ms1 -= 0.1; ms2 -= 0.1;
		setMSpeed(ms1, ms2);
		return 0;
	}
	if (strcmp(s, "stop") == 0) {
		setMSpeed(0.0f, 0.0f);
		return 0;
	}

    return 0;
}

