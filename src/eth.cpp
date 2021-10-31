
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include "eth.h"
#include "teetools.h"

enum EthStatus
{
	sEthInit,
	sEthError,
	sEthNoLink,
	sEthGood
};

EthStatus ethStatus = sEthInit;
EthernetLinkStatus linkStatus = Unknown;
unsigned int aliveTime = 0;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);

unsigned int localPort = 8888; // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";	   // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP udp;
void teensyMAC(uint8_t *mac);

void ethPrint() {
	xmprintf(0, "ETH	status=%d\r\n", static_cast<int>(ethStatus));
}

void ethSetup() {
	// start the Ethernet
	//xmprintf(0, "eth starting ....  ");
	uint8_t mac[6];
	teensyMAC(mac);
	Ethernet.begin(mac, ip);
	//xmprintf(0, " .. begin .. ");

	// Check for Ethernet hardware present
	if (Ethernet.hardwareStatus() == EthernetNoHardware) 	{
		ethStatus = sEthError;
		return;
	}
	linkStatus = Ethernet.linkStatus();
	if (linkStatus == LinkOFF) 	{
		ethStatus = sEthNoLink;
	}

	// start UDP
	udp.begin(localPort);
	ethStatus = sEthGood;
	//xmprintf(0, "eth started \r\n");
}

void ethLoop() {

	EthernetLinkStatus currentLinkStatus = Ethernet.linkStatus();
	if (currentLinkStatus != linkStatus) {
		linkStatus = currentLinkStatus;
		xmprintf(0, "ETH link status %d \r\n", linkStatus);
	}
	if (linkStatus == LinkOFF) {
		ethStatus = sEthNoLink;
		return;
	}
	
	// if there's data available, read a packet
	int packetSize = udp.parsePacket();
	if (packetSize) {
		IPAddress ra = udp.remoteIP();
		uint16_t rp = udp.remotePort();
		// read the packet into packetBufffer
		udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
		packetBuffer[UDP_TX_PACKET_MAX_SIZE-1] = 0;
		xmprintf(0, "ETH got %d bytes from %d.%d.%d.%d port %d (%s)\r\n",
			packetSize,
			ra[0], 	ra[1], ra[2], ra[3],
			rp,
			packetBuffer
		);

		// send a reply to the IP address and port that sent us the packet we received
		udp.beginPacket(ra, rp);
		udp.write(ReplyBuffer);
		udp.endPacket();
  	} 
	  
	unsigned int ms = millis();
	if ((ms - aliveTime) > 250) {
		aliveTime = ms;
		udp.beginPacket("192.168.0.181", 8888);
		udp.write("alive\r\n");
		udp.endPacket();
	}

}

void teensyMAC(uint8_t *mac) {

	static char teensyMac[23];

	#if defined(HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
		Serial.println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
		for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
		for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;

		#define MAC_OK

	#else

		mac[0] = 0x04;
		mac[1] = 0xE9;
		mac[2] = 0xE5;

		uint32_t SN=0;
		__disable_irq();

		#if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
			Serial.println("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
			
			FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
			FTFL_FCCOB0 = 0x41;
			FTFL_FCCOB1 = 15;
			FTFL_FSTAT = FTFL_FSTAT_CCIF;
			while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
			SN = *(uint32_t *)&FTFL_FCCOB7;

			#define MAC_OK
			
		#elif defined(HAS_KINETIS_FLASH_FTFE)
			Serial.println("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
			
			kinetis_hsrun_disable();
			FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
			*(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
			FTFL_FSTAT = FTFL_FSTAT_CCIF;
			while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
			SN = *(uint32_t *)&FTFL_FCCOBB;
			kinetis_hsrun_enable();

			#define MAC_OK
			
		#endif

		__enable_irq();

		for(uint8_t by=0; by<3; by++) mac[by+3]=(SN >> ((2-by)*8)) & 0xFF;

	#endif

	#ifdef MAC_OK
		sprintf(teensyMac, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		Serial.println(teensyMac);
	#else
		Serial.println("ERROR: could not get MAC");
	#endif
}
