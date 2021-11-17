

#include "imu_alg.h"
#include "xmnavstruct.h"
#include "xmfilter.h"
#include "teetools.h"

ImuAlgState iaState = iaInit;
V3 a0, w0;
unsigned int imuCounter = 0;

const XMType maxStaticW = 0.05; // rad/sec 
const XMType G = 9.815;  //  m/s^2
XMCov2<3> wm, am
V3 sWrms, sArms;

void imuAlgInit() {
	iaState = iaInit;
	imuCounter  = 0;
	//IMatrix<2, 1> test = xmCov2Test();
	//xmprintf(0, "xmCov2Test = [%f, %f] \r\n", test[0], test[1]);
}

void imuAlgFeed(const xqm::ImuData& imu) {
	XIMUData data;
	int i;
	data.idInit(imu);

	switch (iaState) {
	case iaInit:
		iaState = iaStaticTest;
		a0.clear();  w0.clear();
		wm.reset(); am.reset();
		imuCounter  = 0;
		sWrms.clear(); sArms.clear();

	case iaStaticTest:
		a0 += imu.a; 		w0 += imu.w;
		++imuCounter;
		am.add(imu.a); 		wm.add(imu.w);

		if (imuCounter > 100) {
			a0.ddiv(imuCounter); w0.ddiv(imuCounter);
			sArms = am.std();      sWrms = wm.std();
			for (i = 0; i < 3; i++) {
				xmprintf(0, "IMU a0=[%f, %f, %f], w0=[%f, %f, %f]\r\n", a0[0], a0[1], a0[2], w0[0], w0[1], w0[2])
				xmprintf(0, "IMU aRms=[%f, %f, %f], wRms=[%f, %f, %f]\r\n", sArms[0], sArms[1], sArms[2], sWrms[0], sWrms[1], sWrms[2])
			}
			iaState = iaGood;
		}

		break;
	case iaGood:

		break;
	};
}