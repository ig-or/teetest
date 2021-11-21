#pragma once

#include "stdint.h"
//#include "imxrt.h"
//#include "core_pins.h"

extern uint32_t msNow;
extern uint32_t mksNow;

//const int maxPWM = 65535;  // because of the 16 bit resolution
const unsigned int pwmResolution = 13;
constexpr unsigned int maxPWM = (1 << pwmResolution) - 1;  // 8191 for    13 bit
const unsigned int adcResolution = 10;
constexpr unsigned int maxADC = (1 << adcResolution)  - 1;
constexpr float a2mv = 3300.0f / maxADC; // translation from adc reading into millivolts; like 3.3v is a full range
constexpr float a2ma = 50 * 3300.0f / maxADC; // translation from adc reading into milliamps for motor driver; 50 for every millivolt

void logSetup(const char* fileName);

struct LedIndication {
	enum LIMode {
		LIRamp,
		LIStop
	};
	LIMode liMode = LIStop;
	int period = 1000; ///< period in ms
	int halfPeriod = 500;
	static constexpr int maxPower = maxPWM * 0.8f;
	float mph = maxPower / 500;
	LedIndication();
	/**
	 *  \param m mode
	 *  \param f frequency in Hz
	 * */
	void liSetMode(LIMode m, float f = 0.0);

	/**
	 * \return PWM value for the led
	 * */
	int liGet(unsigned int ms);
};

extern LedIndication led1;
/*
struct TTime {
	uint32_t mks;
	uint32_t ms;

	///  set time to current time
	void setNow() {
		__disable_irq();
		mks = micros();
		ms = millis();
		__enable_irq();
		mks %= 1000;
	}
	int msDiff(const TTime& t1, const TTime& t2) {


	}
};
*/
int xmprintf(int dst, const char* s, ...);

