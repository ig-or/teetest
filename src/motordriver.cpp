
#include <Arduino.h>

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include "EventResponder.h"
#include "core_cm7.h"
#include "motordriver.h"
#include "ttpins.h"
#include "teetools.h"
const int maxMotorPWM = maxPWM * 0.95;
//const unsigned int minControlPeriod = 250; //  mks
const float accTime = 1000000.0f; // time for acceleration from 0.0 to 1.0, in microseconds
const int mdProcessRate = 2; // [ms]

enum MMODE {
	mSimple,
	mLinear
};
static MMODE mmode = mSimple; // motor control mode

///   values which controls the motor
struct MotorControlParams {
	volatile int speed;
	volatile int dir;
	unsigned int ms;   //< time in milliseconds
	volatile float fSpeed;
	MotorControlParams() : speed(0), dir(LOW), ms(0), fSpeed(0.0f) {}

	/**  setup params based on floaring point control value
	 * \param mSpeed "target" motor speed from -1 to 1
	 * \param current time time in milliseconds
	 */
	void mcUpdate(float mSpeed, unsigned int time) {
		if (mSpeed != mSpeed) {
			xmprintf(0, "mcUpdate bad speed\n");
			return;
		}
		if (mSpeed < -1.0f) 	{mSpeed = -1.0f; 	}
		if (mSpeed > 1.0f) 		{mSpeed =  1.0f; 	}	
		
		ms = time;
		int speedCopy;
		int dirCopy;

		if (mSpeed == 0.0f) {
			dirCopy = dir;
			speedCopy = 0;
		} else if (mSpeed > 0.0f) {
			dirCopy = LOW;
			speedCopy = mSpeed * maxMotorPWM;
		} else { // < 0.0f
			dirCopy = HIGH;
			speedCopy = mSpeed * maxMotorPWM;
			speedCopy *= -1;
		}

		if (speedCopy > maxMotorPWM) {
			speedCopy = maxMotorPWM;
		}
		if (speedCopy < 0) {
			speedCopy = 0;
		}

		bool irq = disableInterrupts(); // ?????????
		fSpeed = mSpeed;
		speed = speedCopy;
		dir = dirCopy;
		enableInterrupts(irq);
	}
};

struct Motor {
	MotorControlParams mcp;		///< new params
	MotorControlParams mcpPrev; ///< previous params
	MotorControlParams mcpPrevCopy; ///< previous params copy 
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
	void mProcess(unsigned int ms) {
		if ((mcpPrev.speed == mcp.speed) && (mcpPrev.dir == mcp.dir)) {
			// we already set what was needed
			return;
		}
		if (mmode == mSimple) {
			memcpy(&mcpPrev, &mcp, sizeof(mcp));		//  just copy
		} else if (mmode == mLinear) {

			//if (mcp.ms == mcpPrev.ms) { // too soon
			//	return;
			//}
			unsigned int dt; //   time diff in ms
			if (mcp.ms > mcpPrev.ms) {
				dt = mks - mcpPrev.mks;
			} else { //  rollover ? 
				dt = (0xffffffff - mcpPrev.mks) + mks;
			}
		

			mcpPrev.mks = mks; //  update command time

			float ds = mcp.fSpeed - mcpPrev.fSpeed;
			if (ds < 0.0f) {
				ds -= ds;
			}

		}

		//now  mcpPrev have what we need
		analogWrite(pwmPin, mcpPrev.speed);
		digitalWriteFast(dirPin, mcpPrev.dir);
		memcpy(&mcpPrevCopy, &mcpPrev, sizeof(mcpPrev));
	}

	//void setSpeed(float mSpeed) {
	//	mcp.mcUpdate(mSpeed, time);
	//}

	/**  call this after setting all the pins.
	 * 	one single motor setup.
	 */
	void mSetup() {
		pinMode(csPin, INPUT);
		pinMode(fltPin, INPUT_PULLUP);

		pinMode(dirPin, OUTPUT);
		digitalWriteFast(dirPin, mcp.dir); 

		pinMode(pwmPin, OUTPUT);
		analogWriteFrequency(pwmPin, 20000.0f);
		analogWrite(pwmPin, mcp.speed);

		pinMode(slpPin, OUTPUT);
		digitalWriteFast(slpPin, HIGH);  
	}

};


static Motor m[2];


void setMotorParams(int index, float mSpeed) {
	m[index].mcp.mcUpdate(mSpeed);
}

void setMSpeed(float m1Speed, float m2Speed) {
	unsigned int ms = millis();
	m[0].mcp.mcUpdate(m1Speed, ms);
	m[1].mcp.mcUpdate(m2Speed, ms);
}

void getMSpeed(float& m1Speed, float& m2Speed) {
	m1Speed = m[0].mcpPrev.fSpeed;
	m2Speed = m[1].mcpPrev.fSpeed;
}

void mdProcess(EventResponderRef r) {
	m[0].mProcess();
	m[1].mProcess();
}

int mdSetup() {
	m[0].setPins(m1pwm, m1dir, m1slp, m1flt, m1cs);
	m[1].setPins(m2pwm, m2dir, m2slp, m2flt, m2cs);

	m[0].setEncPins(m1encA, m1encB);
	m[1].setEncPins(m2encA, m2encB);

	m[0].mSetup();
	m[1].mSetup();

	// setup an interrupt for  'mdProcess'
	EventResponder er;
	er.attachInterrupt(mdProcess);
	MillisTimer mt;
	mt.beginRepeating(mdProcessRate, er);

	return 0;
}




