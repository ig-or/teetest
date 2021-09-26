
#include <Arduino.h>
#include "usb_serial.h"

#include "ttpins.h"
#include "ttsetup.h"


extern "C" int main(void) {
  // initialize the digital pin as an output.
	ttSetup();
		
	uint32_t now = millis();
	int mpw = maxPWM / 2;
	
	//analogWriteFrequency(ledPin, 915.527);
	int p = 0;
	int phase;
	unsigned int hsTime = 0;
	const int hsPeriod = 2000;
	while (1) {
		now = millis();
		phase = (now % 2000);
		if (phase < 1000) {
			p = phase * mpw / 1000;
		} else {
			p = mpw - (phase - 1000)*mpw / 1000;
		}
		if ( p > mpw) {
			  p = mpw;
		}
		if (now - hsTime > hsPeriod) {
			hsTime = now;
			usb_serial_write("hello!\r\n", 9);
		}
		
		//digitalWriteFast(13, HIGH);
		//delay(1000);
		//digitalWriteFast(13, LOW);
		//delay(1000);


		analogWrite(ledPin, p);
		delay(10);
	}

}

