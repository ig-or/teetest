
/**  Data structures for the navigation algorithm.

\file xmnavstruct.cpp
\date    2012
\version 2.0

*/


#include "xmnavstruct.h"
#include "xmatrixplatform.h"
#include "xmessage.h"
#include "xmutils.h"

void XIMUData::idInit(const xqm::ImuData& ird) {
	int i;
	for(i = 0; i < 3; i++) {
		a.x[i] = ird.a[i];
		w.x[i] = ird.w[i];
	}
	timestamp = ird.timestamp;
}


XGPSData::XGPSData() {
	status = 0;
}

void XGPSData::gdInit(const xqm::GpsData& gps) {
	int i;
	hasVelo = gps.hasVelo;
	for(i = 0; i < 3; i++) {
		p.x[i] = gps.pos[i];
		v.x[i] = gps.velo[i];
		if (rcvIsNan(gps.velo[i])) {
			hasVelo = 0;
		}

		pRMS.x[i] = gps.posRms[i];
		vRMS.x[i] = gps.veloRms[i];
	}
	
	status = gps.solutionType;
	gpsTime = gps.gpsTime;
	timestamp = gps.timestamp;
}





