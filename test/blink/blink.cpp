
#include <Arduino.h>
#include "ADC.h"

#include "ttpins.h"
#include "ttsetup.h"
#include "teetools.h"
#include "cmdhandler.h"
#include "motordriver.h"
#include "memsic.h"
#include "ir.h"
#include "logfile.h"
#include "eth.h"
#include "power.h"

#include "xmfilter.h"
#include "imu_alg.h"

//#include "EventResponder.h"

void event100ms() {  //   call this 10 Hz
	irProces();
}


static int lastBatteryInfo = 0, nextBatteryInfo = 0;

void event250ms() {	  //  call this 4  Hz
	h_usb_serial();	

	if (lastBatteryInfo != nextBatteryInfo) {
		lastBatteryInfo = nextBatteryInfo;
		batteryUpdate(lastBatteryInfo);
	}
}

void event1s() {   //  call this 1 Hz
	//xmprintf(0, ".");
	//batteryUpdate();
	//adc->readSingle(1);
	//bool test1 = adc->adc1->startSingleRead(m1cs);
	bool test2 = adc->adc0->startSingleRead(bvPin);

	//xmprintf(0, " .%d,%d ", test1?1:2, test2?1:2);

	//float p = getBattPercent();
	//if (p < 10.0f) {
	//	led1.liSetMode(LedIndication::LIRamp, 8.0);
//
	//}
}

void powerStatusChangeH(PowerStatus from, PowerStatus to) {
	xmprintf(0, "POWER: %s -> %s \r\n", pwrStatusText[from], pwrStatusText[to]);
	mxat(from != to);
	switch(to) {
		case pwVeryLow:
			mdStop();
			canUseMotors(false);
			led1.liSetMode(LedIndication::LIRamp, 8.0);
			break;

		default:
			if (from == pwVeryLow) {
				canUseMotors(true);
				led1.liSetMode(LedIndication::LIRamp, 0.8);
			}
	};
}


void onAdc0() {
	nextBatteryInfo =  adc->adc0->readSingle();
}

PolyFilter<3> imuAX;
float axSmoothed = 0.0f;
int axSmoothCounter = 0;
void imuInfo(const xqm::ImuData& imu) {
	imuAlgFeed(imu);
	lfSendMessage(&imu);			//  put IMU measurement to the log file
	

	axSmoothed = imuAX.pfNext(imu.a[0]) / 12.0f;
	axSmoothCounter++;
}

extern "C" int main(void) {

	pinMode(13, OUTPUT);
	while (1) {
		digitalWriteFast(13, HIGH);
		delay(20);
		digitalWriteFast(13, LOW);
		delay(150);
	}


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
	const int phaseLen = 1500;
	int halfPhaseLen = phaseLen / 2;
	//unsigned int hsTime = 0;
	xmprintf(0, "\r\n4.1 started\r\n");

	serPwStatusChangeHandler(powerStatusChangeH);

/*
	EventResponder er;
	er.attachInterrupt(iTask200Hz);
	MillisTimer mt;
	mt.beginRepeating(5, er);
*/	

	setAdcHandler(onAdc0, 0);

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

		analogWrite(ledPin, p);
		analogWrite(led1_pin, led1.liGet(msNow));

		//delay(2);
		axSmoothCounter = 0;
		processMemsicData(imuInfo);
		//if (axSmoothCounter != 0) {
		//	setMSpeed(axSmoothed, axSmoothed);
		//}

		ethLoop();		//  ethernet
		lfProcess();  	//  log file

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

