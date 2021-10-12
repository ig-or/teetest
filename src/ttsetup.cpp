#include <Arduino.h>

#include "ttpins.h"
#include "ttsetup.h"
#include "motordriver.h"
#include "ir.h"
#include "teetools.h"

int ttSetup() {
	pinMode(ledPin, OUTPUT);

	//   from https://www.pjrc.com/teensy/td_pulse.html, see the bottom of the page
	analogWriteResolution(13);
    analogReadResolution(13);


    mdSetup();
	irSetup();

	pinMode(led1_pin, OUTPUT);
	//analogWrite(led1_pin, maxPWM);
	//digitalWriteFast(led1_pin, HIGH);

	
	return 0;
}

