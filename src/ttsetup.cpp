#include <Arduino.h>

#include "ttpins.h"
#include "ttsetup.h"
#include "motordriver.h"
#include "ir.h"

int ttSetup() {
	pinMode(ledPin, OUTPUT);
	analogWriteResolution(16);
    analogReadResolution(16);



    mdSetup();
	irSetup();

	
	return 0;
}

