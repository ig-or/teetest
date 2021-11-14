

#include "imu_alg.h"
#include "xmnavstruct.h"
#include "xmfilter.h"

ImuAlgState iaState = iaInit;
V3 a0, w0;

const XMType maxStaticW = 0.05; // rad/sec 
const XMType G = 9.815;  //  m/s^2

void imuAlgInit() {
	iaState = iaInit;
}

void imuAlgFeed(const xqm::ImuData& imu) {
	XIMUData data;
	data.idInit(imu);

	switch (iaState) {
	case iaInit:
		iaState = iaStaticTest;
		a0.clear();  w0.clear();
	case iaStaticTest:
		a0 += imu.a;
		w0 += imu.w;

	case iaGood:

		break;
	};
}