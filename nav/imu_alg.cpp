

#include "imu_alg.h"
#include "xmnavstruct.h"
#include "xmatrix2.h"
#include "xmfilter.h"
#include "teetools.h"
#include "logfile.h"
#include "robot.h"
#include "SdFat.h"

#include "motordriver.h"

ImuAlgState iaState = iaInit;
V3 a0, w0;
unsigned int imuCounter = 0, globalImuCounter = 0;;

const XMType maxStaticW = 0.05; // rad/sec 
const XMType G = 9.815;  //  m/s^2
XMCov2<3> wm, am;
V3 sWrms, sArms;
V3 a, w;
const char cfName[] = "irc.txt"; /// imu->robot calibration file name
V3 ac, wc;  ///< for calibration
XMType ws;
constexpr double wrBound = 0.45*0.45; ///< for the calibration, will use only w^2 > wrBound

V3 rYimu, rXimu, rZimu; ///< robot axis in IMU frame
bool haveImuCalibration = false;

void calibration() {
	
	V3 wc1 = wc;
	V3 ac1 = ac;
	wc1.normalize();
	ac1.normalize();

	rYimu = wc1;
	rXimu = cross(ac1, wc1);
	rZimu = cross(rXimu, rYimu);
	haveImuCalibration = true;
	xmprintf(0, "IMU->robot calibration processed\r\n");
	xmprintf(0, "rx=[%.4f, %.4f, %.4f], ry=[%.4f, %.4f, %.4f], rz=[%.4f, %.4f, %.4f]\r\n",
		rXimu[0], rXimu[1], rXimu[2], 
		rYimu[0], rYimu[1], rYimu[2], 
		rZimu[0], rZimu[1], rZimu[2]);
}

void imuAlgInit() {
	iaState = iaInit;
	imuCounter  = 0;
	//IMatrix<2, 1> test = xmCov2Test();
	//xmprintf(0, "xmCov2Test = [%f, %f] \r\n", test[0], test[1]);

	//  try to read IMU calibration
	if (lfState == lfSGood) {
		bool writing = lfWriting;
		if (writing) {
			lfStop();
		}
		FsFile f;
		if (!f.open(cfName)) { //  open a new file
			xmprintf(0, "WARNING: cannot open file %s; no IMU->robot calibration \r\n", cfName);
		} else {
			char stmp[512];
			int bs = f.fgets(stmp, 512);
			if (bs < 5) {
				xmprintf(0, "WARNING: error 1 in file %s; got (%s) \r\n", cfName, stmp);
			} else {
				//int k = sscanf(stmp, "%.9f %.9f %.9f %.9f %.9f %.9f", 
				int k = sscanf(stmp, "%f %f %f %f %f %f", 
					&(ac[0]), &(ac[1]), &(ac[2]), &(wc[0]), &(wc[1]), &(wc[2]));
				if (k == 6) {
					calibration();
				} else {
					xmprintf(0, "WARNING: bad file %s; k=%d; bs = %d; stmp = (%s) \r\n", 
						cfName, k, bs, stmp);
				}
			}

			f.close();
		}
		if (writing) {
			lfStart();
		}
	}
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
	led1.liSetMode(LedIndication::LIRamp, 3.0f);

}
void endImuCalibration() {
	mxat(icState == icComplete);

	calibration();
	rState = rsStop;

	if ((lfState != lfSGood)) {
		xmprintf(0, "ERROR: cannot end IMU calibration: lfState = %d \r\n", lfState );
		return;
	}
	bool wasLogging = lfWriting;
	if (lfWriting) {  //   close the log file and then start again?
		lfStop();
	}

	FsFile f;
	if (!f.open(cfName, O_CREAT | O_TRUNC | O_WRONLY)) { //  create a new file
		xmprintf(0, "ERROR: cannot create a file %s \r\n", cfName);
		
	} else {
		char stmp[512];
		int bs = snprintf(stmp, 512, "%.9f %.9f %.9f %.9f %.9f %.9f \r\n", 
			ac[0], ac[1], ac[2], wc[0], wc[1], wc[2]);
		f.write(stmp, bs);
		f.sync();
		f.close();
		xmprintf(0, "endImuCalibration: %s", stmp);
	}
	if (wasLogging) {
		lfStart();
	}
}

void nextImuCalibration() {
	if (rState != rsImuCalibrate) {
		xmprintf(0, "ERROR: nextImuCalibration: rState  = %d\r\n", rState);
		return;
	}
	switch (icState) {
		case icWaitRotation:
			imuCounter = 0;
			wm.reset(); am.reset();
			icState = icRotation;
			led1.liSetMode(LedIndication::LIRamp, 3.0f);
			xmprintf(0, "nextImuCalibration: rotation \r\n");
			break;
		case icWaitStatic:
			imuCounter = 0;
			wm.reset(); am.reset();
			icState = icStatic;
			led1.liSetMode(LedIndication::LIRamp, 3.0f);
			xmprintf(0, "nextImuCalibration: static \r\n");
			break;
		default:
			xmprintf(0, "ERROR: nextImuCalibration: icState  = %d\r\n", icState);
	};
}

void imuAlgFeed(const xqm::ImuData& data) {
	XIMUData imu;
	int i;
	imu.idInit(data);
	++globalImuCounter;

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
				case rsImuCalibrate: 
					icState = icWaitRotation;  
					led1.liSetMode(LedIndication::LIRamp, 1.0f);
					break;

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
							led1.liSetMode(LedIndication::LIRamp, 1.0f);
							xmprintf(0, "imuAlgFeed: end of rotation \r\n");
						}

						break;

					case icStatic:
						a = imu.a;
						am.add(a);
						++imuCounter;
						if (imuCounter > 100) {
							ac = am.average();
							icState = icComplete;
							xmprintf(0, "imuAlgFeed: end of static \r\n");
							endImuCalibration();
							led1.liSetMode(LedIndication::LIStop, 1.0f);
							break;
						}
						break;
				};
				break;
			case rsPlay:
				w = imu.w - w0;   
				ws = w.length();
				{
					V3 gm = imu.a;
					gm.normalize();
					float pr = sp(gm, rXimu);
					float q = pr * 4.1f;
					setMSpeed(q, q);

					if (globalImuCounter % 100 == 0) {
						xmprintf(0, "pr = %f\r\n", pr);
					}


				}


				break;
		};


		break;
	};
}


void rStay() {
	if (!haveImuCalibration) {
		xmprintf(0, "need imu->robot calibration \r\n");
		return;
	}
	if (rState != rsStop) {
		xmprintf(0, "rState = %d \r\n", rState);
	}

	rState = rsPlay;
	led1.liSetMode(LedIndication::LIRamp, 2.0f);
}

void rStop() {
	rState = rsStop;
	mdStop();
	led1.liSetMode(LedIndication::LIStop, 1.0f);
}