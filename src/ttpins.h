#pragma once 

const int ledPin = 13;
const int maxPWM = 65535;  // because of the 16 bit resolution

/*Motor speed inputs: A PWM (pulse-width modulation) signal 
on these pins corresponds to a PWM output on the corresponding channel’s motor outputs. 
When a PWM pin is low, the corresponding motor brakes low 
(both A and B are shorted together through ground). 
When it is high, the motor is on. 
The maximum allowed PWM frequency is 100 kHz.
*/
const int m1pwm = 14;
const int m2pwm = 15;

/*Current sense outputs: These pins output voltages proportional to the motor currents 
when the H-bridges are driving (but not while they are braking, including when current limiting is active). 
The output voltage is about 10 mV/A for the 18v22 and 20 mV/A for the other versions, 
plus an approximate offset of 50 mV.
*/
const int m1cs = 16;
const int m2cs = 17;

/*
Motor direction inputs: When DIR is low, 
motor current flows from output A to output B; 
when DIR is high, current flows from B to A.
*/
const int m1dir = 18;
const int m2dir = 19;

/*Fault indicators: These open-drain outputs drive low when a fault is occurring. 
In order to use these outputs, they must be pulled up to your system’s logic voltage 
(such as by enabling the internal pull-ups on the Arduino pins they are connected to). 
*/
const int m1flt = 20;
const int m2flt = 21;

/*
	Inverted sleep inputs: These pins are pulled high, enabling the driver by default. 
    Driving these pins low puts the motor driver channels into a low-current sleep mode 
    and disables the motor outputs (setting them to high impedance).
*/
const int m1slp = 22;
const int m2slp = 23;

/**   IR
 * 
 * */
const int IR_RECV_PIN = 11;
