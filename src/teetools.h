#pragma once

#include "stdint.h"
#include "imxrt.h"
#include "core_pins.h"

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
