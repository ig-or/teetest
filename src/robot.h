#pragma once

///   global robot state
enum RState {
	rsInit,
	rsStop,
	rsPlay,
	rsImuCalibrate,

	rStateCount
};

/// imu calibration state
enum ICState {
	icInit,
	icWaitRotation,
	icRotation,
	icWaitStatic,
	icStatic,
	icComplete
};

extern RState rState;
extern ICState icState;

