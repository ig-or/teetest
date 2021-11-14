
/** a number of 'common' functions for navigation

\file xmnavtools.h
\date    2012 - 2015
\version 2.0

*/


#ifndef XMNAVTOOLS_H_FILE
#define XMNAVTOOLS_H_FILE

#include "xmatrix2.h"

/** NOT TESTED
calculate orientation from accelerometer measurements (in body frame?)
@param[in] accelerometer measurements vector ("g" in imu frame)
@return orientation quaternion: rotation from IMU frame into ENU frame (or backward????)

*/
Qua	orientationFromAccelerations(const Vector3& mv);



#endif


