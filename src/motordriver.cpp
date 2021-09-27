
#include <Arduino.h>

#include "motordriver.h"
#include "ttpins.h"

int mdSetup() {
	pinMode(m1cs, INPUT);
	pinMode(m2cs, INPUT);

	pinMode(m1flt, INPUT_PULLUP);
	pinMode(m2flt, INPUT_PULLUP);

	pinMode(m1slp, OUTPUT);
	pinMode(m2slp, OUTPUT);

	// disable driver at the very beginning
	//stop the motors ? 
	digitalWriteFast(m1slp, HIGH);  
	digitalWriteFast(m2slp, HIGH);

	pinMode(m1dir, OUTPUT);
	pinMode(m2dir, OUTPUT);

	digitalWriteFast(m1dir, LOW);  
	digitalWriteFast(m2dir, LOW);

	pinMode(m1pwm, OUTPUT);
	pinMode(m2pwm, OUTPUT);
	analogWriteFrequency(m1pwm, 20000.0f);
	analogWriteFrequency(m2pwm, 20000.0f);

	//stop the motors
	analogWrite(m1pwm, 0);
	analogWrite(m2pwm, 0);

	return 0;
}
