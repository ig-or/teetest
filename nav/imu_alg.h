

#pragma once
#include "xmatrix2.h"
#include "xmessage.h"

enum ImuAlgState {
    iaInit,
    iaStaticTest,
    iaWait,
    iaGood
};



void imuAlgInit();
void imuAlgFeed(const xqm::ImuData& imu);