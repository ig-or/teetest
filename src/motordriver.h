#pragma once

#include "xmroundbuf.h"

int mdSetup();
void setMSpeed(float m1Speed, float m2Speed);
void getMSpeed(float& m1Speed, float& m2Speed);
void mdProcess();
void mdPrint();


enum MMODE {
	mSimple, // just use what we have in 'mcp'
	mLinear  // 
};

///   values which controls the motor
struct MotorControlParams {
	volatile int speed;
	volatile int dir;
	unsigned int ms;   //< time in milliseconds
	volatile float fSpeed;
	MotorControlParams();

	/**  setup params based on floaring point control value
	 * \param mSpeed "target" motor speed from -1 to 1
	 * \param current time time in milliseconds
	 * \param di true if need to disable interrupts; set it to true when using from lower priority functions
	 */
	void mcUpdate(float mSpeed, unsigned int time, bool di = false);
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
#ifndef PCTEST	
	Encoder enc; 
#endif
	volatile long encPos;
	static const int encBufSize = 5;
	//XMRoundBuf<int, encBufSize> encBuf;
	NativeRoundBuf<int, encBufSize> encBuf;
	float encTimeBuf[encBufSize]; ///< encoder time in seconds, starting from zero
	float encDataBuf[encBufSize]; ///< encoder data in radians, starting from zero
	float abc[3]; ///< for parabola coeffs

	volatile float encSpeed; ///< speed in radians/second
	volatile float encSpeedSimple; ///< speed in radians/second

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
	Motor();
	void mSoftReset();
	void setPins(int pwm, int dir, int slp, int flt, int cs);
	void setEncPins(int p1, int p2);
	void print();

	void updateEncSpeed();


	void processOutput(unsigned int ms);
	void mProcess(unsigned int ms);

	// called from lower priority (not interrupt)
	void setSpeed(float mSpeed, unsigned int time);

	/**  call this after setting all the pins.
	 * 	one single motor setup.
	 */
	void mSetup(int id_);

};


