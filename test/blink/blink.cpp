
#include <Arduino.h>

#include "ttpins.h"
#include "ttsetup.h"
#include "teetools.h"
#include "cmdhandler.h"
#include "motordriver.h"
#include "memsic.h"
#include "ir.h"
#include "logfile.h"

#include "xmfilter.h"

//#include "EventResponder.h"

void event100ms() {  //   call this 10 Hz
	irProces();
}
void event250ms() {	  //  call this 4  Hz
	h_usb_serial();	
}
void event1s() {   //  call this 1 Hz

}

PolyFilter<3> imuAX;
float axSmoothed = 0.0f;
int axSmoothCounter = 0;
void imuInfo(const xqm::ImuData& imu) {
	axSmoothed = imuAX.pfNext(imu.a[0]) / 12.0f;
	axSmoothCounter++;
}

extern "C" int main(void) {
	ttSetup();
	imuAX.pfInit(ca3_25_100, cb3_25_100);

	msNow = millis();
	uint32_t fast100msPingTime = msNow;
	uint32_t fast250msPingTime = msNow;
	uint32_t oneSecondPingTime = msNow;
	uint32_t secondsCounter = 0;

	int mpw = maxPWM / 4.5;
	
	//analogWriteFrequency(ledPin, 915.527);
	int p = 0;
	int phase;
	const int phaseLen = 2500;
	int halfPhaseLen = phaseLen / 2;
	unsigned int hsTime = 0;
	const int hsPeriod = 4500;
	xmprintf(0, "\r\n4.1 started\r\n");

/*
	EventResponder er;
	er.attachInterrupt(iTask200Hz);
	MillisTimer mt;
	mt.beginRepeating(5, er);
*/	
	while (1) {

		msNow = millis();
		//mksNow = micros();

		phase = (msNow % phaseLen);
		if (phase < halfPhaseLen) {
			p = phase * mpw / halfPhaseLen;
		} else {
			p = mpw - (phase - halfPhaseLen)*mpw / halfPhaseLen;
		}
		if ( p > mpw) {
			  p = mpw;
		}
		//if (msNow - hsTime > hsPeriod) {
		//	hsTime = msNow;
		//	uint8_t flt = digitalReadFast(m2flt);
//
			//usb_serial_write("md flt=%d\r\n", 9);
		//	xmprintf(0, "md flt=%d\r\n", flt);
		//}
		
		//digitalWriteFast(13, HIGH);
		//delay(1000);
		//digitalWriteFast(13, LOW);
		//delay(1000);


		analogWrite(ledPin, p);
		analogWrite(led1_pin, p >> 4);
		//digitalWriteFast(led1_pin, HIGH);
		//mdProcess();

		delay(2);
		axSmoothCounter = 0;
		processMemsicData(imuInfo);
		if (axSmoothCounter != 0) {
			setMSpeed(axSmoothed, axSmoothed);
		}

		lfProcess();

		if (msNow > (fast100msPingTime + 100)) {
			fast100msPingTime = msNow;
			event100ms();
			if (msNow > (fast250msPingTime + 250)) {
				fast250msPingTime = msNow;
				event250ms();
				if (msNow > (oneSecondPingTime + 1000)) {
					oneSecondPingTime = msNow;
					++secondsCounter;
					event1s();
				}
			}
		}
		yield();	
	}

}

