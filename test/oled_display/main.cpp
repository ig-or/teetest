
#include <Arduino.h>
const int ledPin = 13;
const int maxPWM = 65535;  // because of the 16 bit resolution

#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

extern "C" int main(void) {
  // initialize the digital pin as an output.
	pinMode(ledPin, OUTPUT);	
	uint32_t now = millis();
	int mpw = maxPWM / 2;
	analogWriteResolution(16);
	//analogWriteFrequency(ledPin, 915.527);
	int p = 0;
	int phase;
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
		
		//digitalWriteFast(13, HIGH);
		//delay(1000);
		//digitalWriteFast(13, LOW);
		//delay(1000);


		analogWrite(ledPin, p);
		analogWrite(m2pwm, p);
		delay(10);
	}

}

