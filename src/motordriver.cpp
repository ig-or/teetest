
#ifndef PCTEST
	#include <Arduino.h>

	#define ENCODER_USE_INTERRUPTS
	#define ENCODER_OPTIMIZE_INTERRUPTS
	#include <Encoder.h>
	#include "EventResponder.h"
	#include "IntervalTimer.h"
	
#endif
#include "teetools.h"
//#include "core_cm7.h"
#include "motordriver.h"
#include "ttpins.h"


//#include "rbuf.h"
#include "xmatrixplatform.h"
#include "xmroundbuf.h"
#include "xmatrix2.h"
#include "logfile.h"
#include "xmessage.h"
#include "xmessagesend.h"

#ifdef PCTEST

const int LOW = 0;
const int HIGH = 1;
int xmprintf(int dst, const char* s, ...);
bool disableInterrupts() { return true; }
void enableInterrupts(bool irq) {}
unsigned int millis() {
	return 0;
}
void analogWrite(int pin, int pwm) { }
void digitalWriteFast(int pin, int val) {}
int analogRead(int pin) {return 0;}
#endif

const int maxMotorPWM = maxPWM * 0.95;
//const unsigned int minControlPeriod = 250; //  mks
constexpr float accTime = 5000.0f; // time for acceleration from 0.0 to 1.0, in milliseconds
constexpr int mdProcessRate = 2; // [ms]
constexpr int encReadRate = 10; // [ms]
constexpr float fEncReadRate = float(encReadRate) * 0.001f;
const int encNumber = 1920; ///< impulses per revolution
constexpr float encTicks2Radians = TWO * pii / float(encNumber);
constexpr float encSpeed1 = 2.0f*pii*1000.0f/(encNumber * encReadRate); ///< speed = dFi * encSpeed1

constexpr float fStep = (static_cast<float>(mdProcessRate)) / accTime;

PID::PID() {
	P = 0.16f;
	I = 0.5f;
	D = 0.00f;
	
	eInt = 0.0f;
	ms = 0;
	error = 0.0f;
	ret = 0.0f;

	smallError = 2.0f*encTicks2Radians;
}

float PID::u(float error_, float errorSpeed, unsigned int ms_){  //   FIXME: time rollover
	float mError = abs(error_);
	if (mError <= smallError) {
		eInt = 0.0f;
		ret = 0.0f;
	} else {
		if (ms == 0) { // init
			eInt = 0.0f;
		} else {
			eInt += (error + error_) * 0.5f * (ms_ - ms) / 1000.0f;
		}
		ret = P * error + I * eInt - errorSpeed * D;
	}
	ms = ms_;
	error = error_;
	return ret;
}
void PID::pPrint() {
	xmprintf(0, "PID: eInt=%f; error=%f; ret=%f \r\n", eInt, error, ret);
}


MotorControlParams::MotorControlParams() : speed(0), dir(LOW), ms(0), fSpeed(0.0f) {}

/**  setup params based on floaring point control value
 * \param mSpeed "target" motor speed from -1 to 1
 * \param current time time in milliseconds
 * \param di true if need to disable interrupts; set it to true when using from lower priority functions
 */
void MotorControlParams::mcUpdate(float mSpeed, unsigned int time, bool di) {
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


Motor::Motor() {
	mmode = mLinear; // mLinear; // mAngle
	changeAngleFlag = 0;
	targetEnc = 0;
	mc_e = 0.0;
	mc_u = 0.0;
}
void Motor::mSoftReset() {
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

	changeAngleFlag = 0;
	df = 0.0f;
}
void Motor::setPins(int pwm, int dir, int slp, int flt, int cs) {
	pwmPin = pwm; dirPin = dir; slpPin = slp; fltPin = flt; csPin = cs;
}
void Motor::setEncPins(int p1, int p2) {
	encPin1 = p1;   encPin2 = p2;
}
void Motor::print() {
	xmprintf(0, "motor%d%s \tspeed=%.2f/%.2f; enc=%d fCurrent=%.1f mA, maxFCurrent=%.1f mA  [%d] mmode=%d \t", 
		id,  
		invDir?"inv":" ",
		mcpPrev.fSpeed, mcp.fSpeed, 
		encPos,
		fCurrent, maxFCurrent,
		processCounter - millis(),
		static_cast<int>(mmode));

	pid.pPrint();
}

void Motor::updateEncSpeed() {
	int n = encBuf.num;
	//static const int needCounts = 18; //   will try to use this number of counts for speed estimation
	switch (n) {
		case 0: 
			encSpeedSimple = 0.0f;
			encSpeed = encSpeed;
			break;
		case 1: 
			encSpeedSimple = (encBuf.x[0]) * encSpeed1;
			encSpeed = encSpeed;
			break;
		case 2:
			encSpeedSimple = (encBuf.x[1] - encBuf.x[0]) * encSpeed1;  
			encSpeed = encSpeed;
			break;
		case 3: 
			encSpeedSimple = (encBuf.x[2] - encBuf.x[1]) * encSpeed1;  
			encSpeed = encSpeed;
			break;
		default: {  //   FIXME: handle rollover here; what is rollover policy on encoder side?
			// calculate relative rotation angles ( between encBufSize counts ago and all the others)
			for (int i = 0; i < n; i++) { // FIXME: start counting from 1
				encDataBuf[i] = (encBuf.x[i] - encBuf.x[0]) * encTicks2Radians;
			}
			if (abs(encDataBuf[n - 1]) > 18) { //  if we have some rotation angle inside the buffer
				parabola_appr(encTimeBuf, encDataBuf, n, abc); // calculate 'abc'
				// current time is    encTimeBuf[n - 1]
				encSpeed = 2.0f * abc[0] * encTimeBuf[n - 1] + abc[1];
			}
			else {
				linear_appr(encTimeBuf, encDataBuf, n, abc); // calculate 'ab'
				encSpeed = abc[0];
			}

			encSpeedSimple = float(encBuf.x[n-1] - encBuf.x[n-2]) * (encTicks2Radians * 1000.0f) / encReadRate;
		}
	}
}


void Motor::processOutput(unsigned int ms) {
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


	switch (mmode) {
	case mSimple:
		if ((mcpPrev.speed == mcp.speed) && (mcpPrev.dir == mcp.dir)) {
			// we already set what was needed
			return;
		}
		memcpy(&mcpPrev, &mcp, sizeof(mcp));		//  just copy
		break;
	case mLinear:
		if ((mcpPrev.speed == mcp.speed) && (mcpPrev.dir == mcp.dir)) {
			// we already set what was needed
			return;
		}

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
	case mAngle:
		if ((processCounter % encReadRate) != 0) {   //  encoder
			encPos = enc.read();
			if (!invDir) {  // invert encoder measurements also
				encPos = -encPos;
			}
		}
		{
			mc_e = (targetEnc - encPos) * encTicks2Radians; //  angle error in radians
			//float u = e*0.25;
			mc_u = pid.u(mc_e, encSpeed, ms); //  target speed is zero
			mcpPrev.mcUpdate(mc_u, ms, false);
			//if (ms % 2000 == 0) {
			//	xmprintf(0, "mAngle \te = %f; u = %f; fSpeed=%f; speed = %d \n", e, u, mcpPrev.fSpeed, mcpPrev.speed);
			//}
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

void Motor::mProcess(unsigned int ms) {
	++processCounter;// supposed to be a  milliseconds counter
	//current = analogRead(csPin);

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

	if ((processCounter % encReadRate) == 0) {   //  encoder
#ifndef PCTEST
		encPos = enc.read();
#else
		encPos  = 0;
#endif
		if (!invDir) {  // invert encoder measurements also
			encPos = -encPos;
		}

		encBuf.rAdd(encPos);
		updateEncSpeed();

		if (changeAngleFlag != 0) {  //   target angle change
			changeAngleFlag = 0;

			targetEnc =  encPos + df / encTicks2Radians;
			xmprintf(0, "dEnc = %d \r\n", df / encTicks2Radians);
		}
	}

	if (((processCounter % mdProcessRate) == 0) && (mState == msWork)) {
		processOutput(ms);
	}

	//mcpPrevCopy = mcpPrev;

}

void Motor::changeAngle(float a) {
	//mmode = mAngle;
	df = a;
	changeAngleFlag = 1;

}

// called from lower priority (not interrupt)
void Motor::setSpeed(float mSpeed, unsigned int time) {
		mcp.mcUpdate(mSpeed, time, true);
}

void Motor::mStop() {
	//bool irq = disableInterrupts();
	mmode = mLinear;
	//memcpy(&mcp, &mcpPrev, sizeof(mcp));
	mcp.mcUpdate(0.0f, millis(), true);
	//enableInterrupts(irq);

}

/**  call this after setting all the pins.
 * 	one single motor setup.
 */
void Motor::mSetup(int id_) {
	mSoftReset();
	id = id_;
	invDir = ((id % 2) == 0);
#ifndef PCTEST
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
#endif
}




Motor m[2];


//void setMotorParams(int index, float mSpeed) {
//	m[index].mcp.mcUpdate(mSpeed);
//}

void setMSpeed(float m1Speed, float m2Speed) {
	unsigned int ms = millis();
	m[0].setSpeed(m1Speed, ms);
	m[1].setSpeed(m2Speed, ms);
}

void changeAngle(float da1, float da2) {
	m[0].changeAngle(da1);
	m[1].changeAngle(da2);
}

void getMSpeed(float& m1Speed, float& m2Speed) {
	m1Speed = m[0].mcpPrev.fSpeed;
	m2Speed = m[1].mcpPrev.fSpeed;
}
void mdStop() {
	m[0].mStop();
	m[1].mStop();
}

xqm::TeeMotor mInfo[2];
#ifndef PCTEST
void mdProcess(EventResponderRef r) {
	unsigned int ms = millis();
	m[0].mProcess(ms);
	m[1].mProcess(ms);

	/*char stmp[32];
	int test = snprintf(stmp, 32, "%u\t%d\t%d\n", 
		ms, 
		floor((m[0].current - m[0].currentOffset) * a2ma),
		floor((m[1].current - m[1].currentOffset) * a2ma)
		);

	if ((test > 0) && (test < 32)) {
		lfFeed(stmp, test+1);
	}
	*/
	for (int i = 0; i < 2; i++) {
		mInfo[i].timestamp = ms;
		mInfo[i].id = m[i].id;
		mInfo[i].e = m[i].mc_e;
		mInfo[i].pid_eInt = m[i].pid.eInt;
		mInfo[i].u = m[i].mc_u;

		mInfo[i].enc = m[i].encPos;
		mInfo[i].targetEnc = m[i].targetEnc;
		mInfo[i].encSpeed = m[i].encSpeed;
		mInfo[i].encSpeedSimple = m[i].encSpeedSimple;

		lfSendMessage(&(mInfo[i]));
	}
	
}
#endif
void mdPrint() {
	m[0].print();
	m[1].print();
}
#ifndef PCTEST
EventResponder er;
MillisTimer mt;
IntervalTimer iTimer;
#endif

//char //[32];
xqm::TeeCurrent tcu;
void onCurrent() {
	unsigned int mks = micros();
	m[0].current = analogRead(m[0].csPin);
	m[1].current = analogRead(m[1].csPin);

	//int bs = snprintf(curTmp, 32, "%u\t%d\t%d\n", mks, m[0].current, m[1].current);
	//if ((bs > 0) && (bs < 32)) {
	//		lfFeed(curTmp, bs+1);
	//	}

	tcu.timestamp= mks;
	tcu.cu[0] = m[0].current;
	tcu.cu[1] = m[1].current;
	lfSendMessage(&tcu);
}


int mdSetup() {
	m[0].setPins(m1pwm, m1dir, m1slp, m1flt, m1cs);
	m[1].setPins(m2pwm, m2dir, m2slp, m2flt, m2cs);

	m[0].setEncPins(m1encA, m1encB);
	m[1].setEncPins(m2encA, m2encB);

	m[0].mSetup(1);
	m[1].mSetup(2);

	// setup an interrupt for  'mdProcess'
#ifndef PCTEST	
	er.attachInterrupt(mdProcess);
	mt.beginRepeating(1, er);
#endif
	iTimer.priority(145);
	iTimer.begin(onCurrent, 100);
	return 0;
}




