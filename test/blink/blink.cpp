
#include <Arduino.h>


#include "ttpins.h"
#include "ttsetup.h"
#include "teetools.h"
#include "cmdhandler.h"



extern "C" int main(void) {
  // initialize the digital pin as an output.
	ttSetup();

	const int incomingUsbSerialInfoSize = 32;
	char incomingUsbSerialInfo[incomingUsbSerialInfoSize];

	uint32_t msNow = millis();
	uint32_t fast250msPingTime = msNow;
	uint32_t oneSecondPingTime = msNow;
	uint32_t secondsCounter = 0;

	int mpw = maxPWM / 4.5;
	
	//analogWriteFrequency(ledPin, 915.527);
	int p = 0;
	int phase;
	const int phaseLen = 8000;
	int halfPhaseLen = phaseLen / 2;
	unsigned int hsTime = 0;
	const int hsPeriod = 4500;
	xmprintf(0, "4.1 started\n");
	while (1) {
		msNow = millis();
		phase = (msNow % phaseLen);
		if (phase < halfPhaseLen) {
			p = phase * mpw / halfPhaseLen;
		} else {
			p = mpw - (phase - halfPhaseLen)*mpw / halfPhaseLen;
		}
		if ( p > mpw) {
			  p = mpw;
		}
		if (msNow - hsTime > hsPeriod) {
			hsTime = msNow;
			uint8_t flt = digitalReadFast(m2flt);

			//usb_serial_write("md flt=%d\r\n", 9);
			xmprintf(0, "md flt=%d\r\n", flt);
		}


		
		//digitalWriteFast(13, HIGH);
		//delay(1000);
		//digitalWriteFast(13, LOW);
		//delay(1000);


		analogWrite(ledPin, p);
		//analogWrite(m1pwm, p);
		//analogWrite(m2pwm, p); 	
		delay(10);

				if (mainNow > (fast250msPingTime + 250)) {
			fast250msPingTime = mainNow;
			++fastCycleCounter;
			//if (/*(tcpClientWasConnected == 0) || */(loggingSensors)) {
			//	digitalWriteFast(ledPin, fastCycleCounter % 2);
			//}
			//Serial.printf("%d\n", mainNow);
			int bs1;
			while ((bs1 = usb_serial_available())) {
				int bs2 = (bs1 <= incomingUsbSerialInfoSize) ? bs1 : incomingUsbSerialInfoSize;
				int bs3 = usb_serial_read(incomingUsbSerialInfo, bs2);
				if (bs3 > 0) {
					//usb_serial_write(incomingUsbSerialInfo, bs3); //  this is for debugging and testing
					onIncomingInfo(incomingUsbSerialInfo, bs3);
				}
			}
			
			if (mainNow > (oneSecondPingTime + 1000)) {
				//Serial.println("+1s");
				oneSecondPingTime = mainNow;
				++secondsCounter;



		while ((int bs1 = usb_serial_available())) {
			int bs2 = (bs1 <= incomingUsbSerialInfoSize) ? bs1 : incomingUsbSerialInfoSize;
			int bs3 = usb_serial_read(incomingUsbSerialInfo, bs2);
			if (bs3 > 0) {
				//usb_serial_write(incomingUsbSerialInfo, bs3); //  this is for debugging and testing
				onIncomingInfo(incomingUsbSerialInfo, bs3);
			}
		}
	}

}

