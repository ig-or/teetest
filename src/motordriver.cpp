
#include <Arduino.h>

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include "motordriver.h"
#include "ttpins.h"

struct Motor {
	int speed;
	int dir;
	float fSpeed;
	Motor() {
		speed = 0;
		dir = LOW;
		fSpeed = 0.0f;
	}
};

struct MotorS {

    Motor mNew[2];
	Motor mOld[2];
};

MotorS m;

int pwmPin[2] = {m1pwm, m2pwm};
int dirPin[2] = {m1dir, m2dir};

void setMotorParams(int index, float mSpeed) {
	if (mSpeed < -1.0f) {mSpeed = -1.0f; }
	if (mSpeed > 1.0f) {mSpeed = 1.0f; }

	if (mSpeed == 0.0f) {
		//m.mNew[index].dir = LOW;
		m.mNew[index].speed = 0;
	} else if (mSpeed > 0.0f) {
		m.mNew[index].dir = LOW;
		m.mNew[index].speed = mSpeed * maxPWM;
	} else { // < 0.0f
		m.mNew[index].dir = HIGH;
		m.mNew[index].speed = mSpeed * maxPWM;
		m.mNew[index].speed *= -1;
	}

	if (m.mNew[index].speed > maxPWM) {
		m.mNew[index].speed = maxPWM;
	}
	if (m.mNew[index].speed < 0) {
		m.mNew[index].speed = 0;
	}

	m.mNew[index].fSpeed = mSpeed;
}

void setMSpeed(float m1Speed, float m2Speed) {
	setMotorParams(0, m1Speed);
	setMotorParams(1, m2Speed);
}
void getMSpeed(float& m1Speed, float& m2Speed) {
	m1Speed = m.mNew[0].fSpeed;
	m2Speed = m.mNew[1].fSpeed;
}

void mdProcess() {
	int i;
	for (i = 0; i < 2; i++) {
		if (m.mOld[i].speed != m.mNew[i].speed) {
			m.mOld[i].speed = m.mNew[i].speed;
			analogWrite(pwmPin[i], m.mOld[i].speed);
			m.mOld[i].fSpeed = m.mNew[i].fSpeed; // need this?
		}
		if (m.mOld[i].dir != m.mNew[i].dir) {
			m.mOld[i].dir = m.mNew[i].dir;
			digitalWriteFast(dirPin[i], m.mOld[i].dir);
		}
	}
}

int mdSetup() {
	pinMode(m1cs, INPUT);
	pinMode(m2cs, INPUT);

	pinMode(m1flt, INPUT_PULLUP);
	pinMode(m2flt, INPUT_PULLUP);

	pinMode(m1slp, OUTPUT);
	pinMode(m2slp, OUTPUT);

	digitalWriteFast(m1slp, HIGH);  
	digitalWriteFast(m2slp, HIGH);

	pinMode(m1dir, OUTPUT);
	pinMode(m2dir, OUTPUT);

	digitalWriteFast(m1dir, LOW);  
	digitalWriteFast(m2dir, LOW);

	pinMode(m1pwm, OUTPUT);
	pinMode(m2pwm, OUTPUT);
	analogWriteFrequency(m1pwm, 10000.0f);
	analogWriteFrequency(m2pwm, 10000.0f);

	analogWrite(m1pwm, 0);
	analogWrite(m2pwm, 0);

	return 0;
}
