#include <Arduino.h>

#include "ttpins.h"
#include "ttsetup.h"
#include "motordriver.h"

int ttSetup() {
	pinMode(ledPin, OUTPUT);
	analogWriteResolution(16);
    analogReadResolution(16);



    mdSetup();

	
	return 0;
}

