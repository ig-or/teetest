#pragma once

#include "stdint.h"
#include "imxrt.h"
#include "core_pins.h"

extern uint32_t msNow;
extern uint32_t mksNow;

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