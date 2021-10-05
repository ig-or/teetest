#pragma once

int mdSetup();
void setMSpeed(float m1Speed, float m2Speed, unsigned int mks);
void getMSpeed(float& m1Speed, float& m2Speed);
void mdProcess(unsigned int mks);