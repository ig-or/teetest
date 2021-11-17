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

int ttSetup() {
	pinMode(imuPwrPin, OUTPUT);
	digitalWriteFast(imuPwrPin, LOW); //    no power for IMU yet

	pinMode(ledPin, OUTPUT);

	//   from https://www.pjrc.com/teensy/td_pulse.html, see the bottom of the page
	analogWriteResolution(pwmResolution);
    analogReadResolution(adcResolution);
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
	
	return 0;
}

