#include <Arduino.h>

#include "ttpins.h"
#include "ttsetup.h"
#include "motordriver.h"
#include "ir.h"
#include "teetools.h"
#include "memsic.h"
#include "eth.h"
#include "logfile.h"
#include "imu_alg.h"
#include "ADC.h"



adcHandler adch[2] = {0, 0};


void adc_isr1() {
	//xmprintf(0, "adc isr 1 \r\n");
	if (adch[1] == 0) {
		adc->adc1->readSingle();
		return;
	}
	adch[1]();
}
void adc_isr0() {
	//xmprintf(0, "adc isr 0 \r\n");
	if (adch[0] == 0) {
		adc->adc0->readSingle();
		return;
	}
	adch[0]();
}

void setAdcHandler(adcHandler h, int k) {
	if ((k < 0) || (k > 1)) {
		return;
	}
	adch[k] = h;
}

int ttSetup() {
	pinMode(imuPwrPin, OUTPUT);
	digitalWriteFast(imuPwrPin, LOW); //    no power for IMU yet

	pinMode(ledPin, OUTPUT);

	//   from https://www.pjrc.com/teensy/td_pulse.html, see the bottom of the page
	analogWriteResolution(pwmResolution);

	adc = new ADC();

    //analogReadResolution(adcResolution);
	adc->adc0->setResolution(adcResolution);
	adc->adc1->setResolution(adcResolution);

	adc->adc0->enableInterrupts(adc_isr0, 255);
	adc->adc1->enableInterrupts(adc_isr1, 255);

	//analogReadAveraging(8);
	
	lfInit();  //  SD log file init; will set up sdStarted = true; if SD card present
	ethSetup();

    mdSetup(); // motor driver setup
	irSetup(); // IR receiver setup

	pinMode(led1_pin, OUTPUT);
	analogWrite(led1_pin, maxPWM);
	
	setupMemsic_1();

	//digitalWriteFast(led1_pin, HIGH);
	delay(50);
	
	setupMemsic_2();
	digitalWriteFast(imuPwrPin, HIGH); // turn on IMU
	delay(500);
	
	setupMemsic_3(100);
	imuAlgInit();
	analogWrite(led1_pin, 0);
	
	return 0;
}

