

#include "imu_alg.h"
#include "xmnavstruct.h"
#include "xmatrix2.h"
#include "xmfilter.h"
#include "teetools.h"
#include "logfile.h"
#include "robot.h"

ImuAlgState iaState = iaInit;
V3 a0, w0;
unsigned int imuCounter = 0;

const XMType maxStaticW = 0.05; // rad/sec 
const XMType G = 9.815;  //  m/s^2
XMCov2<3> wm, am;
V3 sWrms, sArms;
V3 a, w;
V3 ac, wc;  ///< for calibration
XMType ws;
constexpr double wrBound = 0.25*0.25; ///< for the calibration, will use only w^2 > wrBound

void imuAlgInit() {
	iaState = iaInit;
	imuCounter  = 0;
	//IMatrix<2, 1> test = xmCov2Test();
	//xmprintf(0, "xmCov2Test = [%f, %f] \r\n", test[0], test[1]);
}

void startImuCalibration() {
	//  need SD card to save the values
	if ((lfState != lfSGood)) { 
		xmprintf(0, "ERROR: cannot start IMU calibration: lfState = %d \r\n", lfState );
		return;
	}

	iaState = iaInit;
	icState = icInit;
	rState = rsImuCalibrate;

}
void endImuCalibration() {
	mxat(icState == icComplete);
	if ((lfState != lfSGood)) {
		xmprintf(0, "ERROR: cannot end IMU calibration: lfState = %d \r\n", lfState );
		return;
	}
	bool wasLogging = lfWriting;
	if (lfWriting) {  //   close the log file and then start again?
		lfStop();
	}


	if (wasLogging) {
		lfStart();
	}

}

void nextImuCalibration() {
	if (rState != rsImuCalibrate) {
		return;
	}
	switch (icState) {
		case icWaitRotation:
			imuCounter = 0;
			wm.reset(); am.reset();
			icState = icRotation;
			break;
		case icWaitStatic:
			imuCounter = 0;
			wm.reset(); am.reset();
			icState = icStatic;
			break;
	};
}

void imuAlgFeed(const xqm::ImuData& data) {
	XIMUData imu;
	int i;
	imu.idInit(data);

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
			//for (i = 0; i < 3; i++) {
			xmprintf(0, "imuCounter = %d; rState=%d; icState=%d\r\n", rState, icState, imuCounter);
			xmprintf(0, "IMU a0=[%f, %f, %f], w0=[%f, %f, %f]\r\n", a0[0], a0[1], a0[2], w0[0], w0[1], w0[2]);
			xmprintf(0, "IMU aRms=[%f, %f, %f], wRms=[%f, %f, %f]\r\n", sArms[0], sArms[1], sArms[2], sWrms[0], sWrms[1], sWrms[2]);
			//}
			iaState = iaGood;
			switch(rState) {
				case rsImuCalibrate: icState = icWaitRotation;  break;

			};
		}

		break;
	case iaGood:
		switch (rState) {
			case rsImuCalibrate:
				//rState = rsStop;
				switch (icState) {
					case icRotation:
						w = imu.w - w0;   
						ws = w.norma2();
						if (ws < wrBound) break;
						wm.add(w);
						++imuCounter;
						if (imuCounter > 100) {
							wc = wm.average();
							icState = icWaitStatic;
						}

						break;

					case icStatic:
						a = imu.a;
						am.add(a);
						if (imuCounter > 100) {
							ac = am.average();
							icState = icComplete;
							endImuCalibration();
							break;
						}
						break;
				};
				break;
		};


		break;
	};
}