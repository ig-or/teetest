

#include "imu_alg.h"
#include "xmnavstruct.h"
#include "xmfilter.h"
#include "teetools.h"

ImuAlgState iaState = iaInit;
V3 a0, w0;

const XMType maxStaticW = 0.05; // rad/sec 
const XMType G = 9.815;  //  m/s^2

void imuAlgInit() {
	iaState = iaInit;

	IMatrix<2, 1> test = xmCov2Test();
	xmprintf(0, "xmCov2Test = [%f, %f] \r\n", test[0], test[1]);
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