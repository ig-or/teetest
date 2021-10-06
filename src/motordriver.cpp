
#include <Arduino.h>

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include "motordriver.h"
#include "ttpins.h"
#include "teetools.h"
const int maxMotorPWM = maxPWM * 0.95;
const unsigned int minControlPeriod = 250; //  mks
const float accTime = 1000000.0f; // time for acceleration from 0.0 to 1.0, in microseconds

///   values which controls the motor
struct MotorControlParams {
	int speed;
	int dir;
	unsigned int mks;   //< time in microseconds
	float fSpeed;
	MotorControlParams() : speed(0), dir(LOW), mks(0), fSpeed(0.0f) {}

	/**  setup params based on floaring point control value
	 * \param mSpeed "target" motor speed from -1 to 1
	 * \param current time time in microseconds
	 */
	void mcUpdate(float mSpeed, unsigned int time) {
		if (mSpeed != mSpeed) {
			xmprintf(0, "mcUpdate bad speed\n");
			return;
		}
		if (mSpeed < -1.0f) 	{mSpeed = -1.0f; 	}
		if (mSpeed > 1.0f) 		{mSpeed =  1.0f; 	}	
		fSpeed = mSpeed;
		mks = time;

		if (mSpeed == 0.0f) {
			//m.mNew[index].dir = LOW;
			speed = 0;
		} else if (mSpeed > 0.0f) {
			dir = LOW;
			speed = mSpeed * maxMotorPWM;
		} else { // < 0.0f
			dir = HIGH;
			speed = mSpeed * maxMotorPWM;
			speed *= -1;
		}

		if (speed > maxMotorPWM) {
			speed = maxMotorPWM;
		}
		if (speed < 0) {
			speed = 0;
		}
	}
};

struct Motor {
	MotorControlParams mcp;		///< new params
	MotorControlParams mcpPrev; ///< previous params
	Encoder enc; 

	/// the motor pins
	int pwmPin, dirPin, slpPin, fltPin, csPin;
	///  encoder pins:
	int encPin1, encPin2;
	Motor() {
	}
	void setPins(int pwm, int dir, int slp, int flt, int cs) {
		pwmPin = pwm; dirPin = dir; slpPin = slp; fltPin = flt; csPin = cs;
	}
	void setEncPins(int p1, int p2) {
		encPin1 = p1;   encPin2 = p2;
	}
	void mProcess(unsigned int mks) {
		if ((mcpPrev.speed == mcp.speed) && (mcpPrev.dir == mcp.dir)) {
			return;
		}
		if (mks == mcpPrev.mks) { // too soon
			return;
		}
		unsigned int dt; //   time diff in mks
		if (mks > mcpPrev.mks) {
			dt = mks - mcpPrev.mks;
		} else { //  rollover ? 
			dt = (0xffffffff - mcpPrev.mks) + mks;
		}
		if (dt < minControlPeriod) { //  too soon
			return; 
		}

		mcpPrev.mks = mks; //  update command time

		float ds = mcp.fSpeed - mcpPrev.fSpeed;
		if (ds < 0.0f) {
			ds -= ds;
		}


/*

		if (mcpPrev.speed != mcp.speed) {
			mcpPrev.speed = mcp.speed;
			analogWrite(pwmPin[i], m.mOld[i].speed);
			m.mOld[i].fSpeed = m.mNew[i].fSpeed; // need this?
		}
		if (m.mOld[i].dir != m.mNew[i].dir) {
			m.mOld[i].dir = m.mNew[i].dir;
			digitalWriteFast(dirPin[i], m.mOld[i].dir);
		}
		*/
	}

	//void setSpeed(float mSpeed) {
	//	mcp.mcUpdate(mSpeed, time);
	//}

	///  call this after setting all the pins
	void mSetup() {
		pinMode(csPin, INPUT);
		pinMode(fltPin, INPUT_PULLUP);

		pinMode(dirPin, OUTPUT);
		digitalWriteFast(dirPin, mcp.dir); 

		pinMode(pwmPin, OUTPUT);
		analogWriteFrequency(pwmPin, 10000.0f);
		analogWrite(pwmPin, mcp.speed);

		pinMode(slpPin, OUTPUT);
		digitalWriteFast(slpPin, HIGH);  
	}

};


static Motor m[2];


void setMotorParams(int index, float mSpeed, unsigned int time) {
	m[index].mcp.mcUpdate(mSpeed, time);
}

void setMSpeed(float m1Speed, float m2Speed, unsigned int mks) {
	m[0].mcp.mcUpdate(m1Speed, mks);
	m[1].mcp.mcUpdate(m2Speed, mks);
}

void getMSpeed(float& m1Speed, float& m2Speed) {
	m1Speed = m[0].mcpPrev.fSpeed;
	m2Speed = m[1].mcpPrev.fSpeed;
}

void mdProcess(unsigned int mks) {
	m[0].mProcess(mks);
	m[1].mProcess(mks);
}

int mdSetup() {
	m[0].setPins(m1pwm, m1dir, m1slp, m1flt, m1cs);
	m[1].setPins(m2pwm, m2dir, m2slp, m2flt, m2cs);

	m[0].setEncPins(m1encA, m1encB);
	m[1].setEncPins(m2encA, m2encB);

	m[0].mSetup();
	m[1].mSetup();

	return 0;
}
