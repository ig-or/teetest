
/**  Data structures for the navigation algorithm.

\file xmnavstruct.h
\date    2012
\version 2.0

*/



#ifndef XMNAVSTRUCT_H_FILE
#define XMNAVSTRUCT_H_FILE

#include "xstdef.h"
#include "xmatrix2.h"

namespace xqm {
	struct GpsData;
	struct ImuData;
	struct VisorData;
};

/** info from IMU, common for all IMU devices. Used inside navigation engine.

*/
struct XIMUData {
	/** accelerometer data
	*/
	Vector3	a; 
	/** gyro data
	*/
	Vector3 w;
	tstype timestamp;
	/** 0 - data are OK
		not '0' - something is wrong
	*/
	//unsigned char status; 
	void idInit(const xqm::ImuData& ird);
};

/** info from GPS, common for all GPS devices. Used inside navigation engine.

*/
struct XGPSData {
	V3	p;	///< position, [m]
	V3	v;  ///< velocity   [m/s]
	/**
	0 - no data
	1 - SA data
	2 - Code-Diff data (what is it?)
	3 - float solution
	4 - FIX solution
	5 - something else
	*/
	unsigned char status;

	/** looks like sometimes we have no velocity
		1 - we have velocity
		0 - no velocity, and no velocity RMS
	*/
	unsigned char hasVelo;
	Vector3	pRMS; ///< position RMS, [m]  
	Vector3	vRMS;  ///< velocity RMS, [m/s]  
	tstype timestamp;
	tstype gpsTime;

	void gdInit(const xqm::GpsData& gps);
	XGPSData();
};


#endif



