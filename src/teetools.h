#pragma once

#include "stdint.h"
#include "imxrt.h"
#include "core_pins.h"

extern uint32_t msNow;
extern uint32_t mksNow;

const int maxPWM = 65535;  // because of the 16 bit resolution
constexpr float a2mv = 3300.0f / maxPWM; // translation from adc reading into millivolts
constexpr float a2ma = 50 * 3300.0f / maxPWM; // translation from adc reading into milliamps for motor driver

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


inline bool disableInterrupts() {
	uint32_t primask;
	__asm__ volatile("mrs %0, primask\n" : "=r" (primask)::);
	__disable_irq();
	return (primask == 0) ? true : false;
}
inline void enableInterrupts(bool doit) {
	if (doit) __enable_irq();
}