
#include <Arduino.h>

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include "EventResponder.h"
//#include "core_cm7.h"
#include "motordriver.h"
#include "ttpins.h"
#include "teetools.h"

//#include "rbuf.h"
#include "xmatrixplatform.h"
#include "xmroundbuf.h"
#include "xmatrix2.h"
const int maxMotorPWM = maxPWM * 0.95;


//const unsigned int minControlPeriod = 250; //  mks
constexpr float accTime = 5000.0f; // time for acceleration from 0.0 to 1.0, in milliseconds
constexpr int mdProcessRate = 2; // [ms]
constexpr int encReadRate = mdProcessRate << 1; // [ms]
const int encNumber = 1920; ///< impulses per revolution
constexpr float encTicks2Radians = TWO * pii / float(encNumber);
constexpr float encSpeed1 = 2.0f*pii*1000.0f/(encNumber * encReadRate); ///< speed = dFi * encSpeed1

constexpr float fStep = (static_cast<float>(mdProcessRate)) / accTime;

enum MMODE {
	mSimple, // just use what we have in 'mcp'
	mLinear  // 
};
static MMODE mmode = mLinear; // motor control mode

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
	 * \param di true if need to disable interrupts; set it to true when using from lower priority functions
	 */
	void mcUpdate(float mSpeed, unsigned int time, bool di = false) {
		if (mSpeed != mSpeed) {
			xmprintf(0, "mcUpdate bad speed\r\n");
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

		bool irq;
		if (di) {
			irq = disableInterrupts(); // ?????????
		}
		speed = speedCopy;
		dir = dirCopy;
		fSpeed = mSpeed;
		if (di) {
			enableInterrupts(irq);
		}
	}
};

struct Motor {
	enum MState {
		msCalibrate,
		msWork
	};
	MState mState;
	MotorControlParams mcp;		///< new params
	MotorControlParams mcpPrev; ///< previous params
	//MotorControlParams mcpPrevCopy; ///< previous params copy 
	Encoder enc; 
	volatile long encPos;
	static const int encBufSize = 8;
	//XMRoundBuf<int, encBufSize> encBuf;
	NativeRoundBuf<int, encBufSize> encBuf;
	float encTimeBuf[encBufSize]; ///< encoder time in seconds, starting from zero
	float encDataBuf[encBufSize]; ///< encoder data in radians, starting from zero
	float abc[3]; ///< for parabola coeffs

	volatile float encSpeed; ///< speed in radians/second

	/// the motor pins
	int pwmPin, dirPin, slpPin, fltPin, csPin;
	///  encoder pins:
	int encPin1, encPin2;
	int current, currentOffset;
	float fCurrent, maxFCurrent; // milliamps
	volatile int bigCurrentFlag;
	int id;  ///< motor ID
	bool invDir; ///< invert direction of the rotation
	unsigned int processCounter;
	volatile unsigned int bigCurrentCounter;
	static const int calibrationTime = 20; // ms
	Motor() {
	
	}
	void mSoftReset() {
		mState = msCalibrate;
		processCounter = 0; bigCurrentCounter = 0;
		current = 0;  currentOffset = 0; fCurrent = 0.0f; maxFCurrent = 0.0f;
	//	invDir = false;
		encPos = 0;
		bigCurrentFlag = 0;

		encBuf.clear();
		encSpeed = 0.0f;

		memset(encTimeBuf, 0, sizeof(float)*encBufSize);
		for (int i = 0; i < encBufSize; i++) {
			encTimeBuf[i] = i * encReadRate * 0.001; 
		}

		memset(encDataBuf, 0, sizeof(float)*encBufSize);
	}
	void setPins(int pwm, int dir, int slp, int flt, int cs) {
		pwmPin = pwm; dirPin = dir; slpPin = slp; fltPin = flt; csPin = cs;
	}
	void setEncPins(int p1, int p2) {
		encPin1 = p1;   encPin2 = p2;
	}
	void print() {
		xmprintf(0, "motor %d \tspeed=%.2f/%.2f; enc=%d fCurrent=%.1f mA, maxFCurrent=%.1f mA  [%d]\r\n", 
			id,  
			mcpPrev.fSpeed, mcp.fSpeed, 
			encPos,
			fCurrent, maxFCurrent,
			processCounter - millis());
	}

	void updateEncSpeed() {
		switch (encBuf.num) {
			case 0: encSpeed = 0.0f; break;
			case 1: encSpeed = (encBuf.x[0]) * encSpeed1;  break;
			case 2: encSpeed = (encBuf.x[1] - encBuf.x[0]) * encSpeed1;  break;
			case 3: encSpeed = (encBuf.x[2] - encBuf.x[1]) * encSpeed1;  break;
			default: {  //   FIXME: handle rollover here; what is rollover policy on encoder side?
				// calculate relative rotation angles ( between encBufSize counts ago and all the others)
				for (int i = 0; i < encBuf.num; i++) { // FIXME: start counting from 1
					encDataBuf[i] = (encBuf.x[i] - encBuf.x[0]) * encTicks2Radians;
				}
				parabola_appr(encTimeBuf, encDataBuf, encBuf.num, abc); // calculate 'abc'
				// current time is    encTimeBuf[encBuf.num - 1]
				encSpeed= 2.0f*abc[0]*encTimeBuf[encBuf.num - 1] + abc[1];
			}
		}
	}


	void processOutput(unsigned int ms) {
		fCurrent = (current - currentOffset) * a2ma; 
		if (fCurrent > maxFCurrent) {
			maxFCurrent = fCurrent;
		}
		if (fCurrent > 4000) {
			bigCurrentFlag = 1;
			mcp.mcUpdate(mcp.fSpeed *0.9, ms, false);
			++bigCurrentCounter;
		} else {
			bigCurrentFlag = 0;
		}

		if ((mcpPrev.speed == mcp.speed) && (mcpPrev.dir == mcp.dir)) {
			// we already set what was needed
			return;
		}
		
		switch (mmode) {
		case mSimple:
			memcpy(&mcpPrev, &mcp, sizeof(mcp));		//  just copy
			break;
		case mLinear:
			if (mcpPrev.fSpeed == mcp.fSpeed) {
				memcpy(&mcpPrev, &mcp, sizeof(mcp));		//  just copy
			} else  { //   change fSpeed:
				float x;
				if (mcpPrev.fSpeed < mcp.fSpeed) { //  go up
					x = mcpPrev.fSpeed + fStep;
					if (x > mcp.fSpeed) {
						x = mcp.fSpeed;
					}
				} else {  //  go down
					x = mcpPrev.fSpeed - fStep;
					if (x < mcp.fSpeed) {
						x = mcp.fSpeed;
					}		
				}
				mcpPrev.mcUpdate(x, ms, false);
			}
			break;
		};

		//now  mcpPrev have what we need
		analogWrite(pwmPin, mcpPrev.speed);
		int d = mcpPrev.dir;
		if (invDir) {
			d = (d == LOW) ? HIGH : LOW;
		}
		digitalWriteFast(dirPin, d);
	}

	void mProcess(unsigned int ms) {
		++processCounter;// supposed to be a  milliseconds counter
		current = analogRead(csPin);

		switch (mState) {
		case msCalibrate:
			currentOffset += current;
			if (processCounter >= calibrationTime) {
				currentOffset /= processCounter;
				//xmprintf(0, "motor %d currentOffset %d (%.2f mv)\r\n", id, 
				//	currentOffset, currentOffset*a2mv);
				mState = msWork;
			}
		break;
		default: break;
		};

		
		if (((processCounter % mdProcessRate) == 0) && (mState == msWork)) {
			processOutput(ms);
		}
		if ((processCounter % encReadRate) == 0) {   //  encoder
			int newPosition = enc.read();
			if (encPos != newPosition) {
				encPos = newPosition;
			}
			encBuf.rAdd(newPosition);
			updateEncSpeed();
		}



		//mcpPrevCopy = mcpPrev;
	}

	// called from lower priority (not interrupt)
	void setSpeed(float mSpeed, unsigned int time) {
			mcp.mcUpdate(mSpeed, time, true);
	}

	/**  call this after setting all the pins.
	 * 	one single motor setup.
	 */
	void mSetup(int id_) {
		mSoftReset();
		id = id_;
		invDir = ((id % 2) == 0);
		
		pinMode(csPin, INPUT);
		pinMode(fltPin, INPUT_PULLUP);

		pinMode(dirPin, OUTPUT);
		digitalWriteFast(dirPin, mcp.dir); 

		pinMode(pwmPin, OUTPUT);
		//analogWriteFrequency(pwmPin, 20000.0f);
		analogWriteFrequency(pwmPin, 18310.55f);
		analogWrite(pwmPin, mcp.speed);

		pinMode(slpPin, OUTPUT);
		digitalWriteFast(slpPin, HIGH);  

		enc.eSetup(encPin1, encPin2);
	}

};


static Motor m[2];


//void setMotorParams(int index, float mSpeed) {
//	m[index].mcp.mcUpdate(mSpeed);
//}

void setMSpeed(float m1Speed, float m2Speed) {
	unsigned int ms = millis();
	m[0].setSpeed(m1Speed, ms);
	m[1].setSpeed(m2Speed, ms);
}

void getMSpeed(float& m1Speed, float& m2Speed) {
	m1Speed = m[0].mcpPrev.fSpeed;
	m2Speed = m[1].mcpPrev.fSpeed;
}

void mdProcess(EventResponderRef r) {
	unsigned int ms = millis();
	m[0].mProcess(ms);
	m[1].mProcess(ms);
}
void mdPrint() {
	m[0].print();
	m[1].print();
}

EventResponder er;
MillisTimer mt;

int mdSetup() {
	m[0].setPins(m1pwm, m1dir, m1slp, m1flt, m1cs);
	m[1].setPins(m2pwm, m2dir, m2slp, m2flt, m2cs);

	m[0].setEncPins(m1encA, m1encB);
	m[1].setEncPins(m2encA, m2encB);

	m[0].mSetup(1);
	m[1].mSetup(2);

	// setup an interrupt for  'mdProcess'
	
	er.attachInterrupt(mdProcess);
	mt.beginRepeating(1, er);

	return 0;
}




