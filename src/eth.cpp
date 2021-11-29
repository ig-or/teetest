
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include "eth.h"
#include "teetools.h"
#include "rbuf.h"
#include "cmdhandler.h"

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
const IPAddress ip(192, 168, 0, 177);

const unsigned int localPortUdp = 8888; // local port to listen on
const unsigned int localPortTcp = 8889; // local port to listen on

// buffers for receiving and sending data
static const int maxPacketSize = 512;
char packetBuffer[maxPacketSize]; // buffer to hold incoming packet,
//char ReplyBuffer[] = "acknowledged\r\n";	   // a string to send back

const int rbSize = 1024;
unsigned char buf[rbSize];
static ByteRoundBuf rb;
static bool clientConnected = false;

const int bufSize2 = 256;
unsigned char buf2[bufSize2];

// An EthernetUDP instance to let us send and receive packets over UDP
//EthernetUDP udp;
static EthernetServer server;
static EthernetClient client;
EthInfoHandler infoHandler = 0;

void teensyMAC(uint8_t *mac);

void ethPrint() {
	xmprintf(2, "ETH status=%d; clientConnected=%s;  client=%s   \r\n", static_cast<int>(ethStatus),
		clientConnected ? "yes" : "no", client ? "yes" : "no");
}
void ethSetInfoHandler(EthInfoHandler h) {
	infoHandler = h;
}

void ethSetup() {
	// start the Ethernet
	//xmprintf(2, "eth starting ....  ");
	uint8_t mac[6];
	teensyMAC(mac);
	Ethernet.begin(mac, ip);
	//xmprintf(2, " .. begin .. ");

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
	//udp.begin(localPort);
	// start the server

  	server.begin(localPortTcp);
	ethStatus = sEthGood;
	//xmprintf(2, "eth started 1 \r\n");
	initByteRoundBuf(&rb, buf, rbSize);
	client = server.available();
	xmprintf(2, "eth started 2; client=%s \r\n", client ? "yes" : "no");
}

void ethFeed(char* s, int size) {
	bool irq = disableInterrupts();
	if (!clientConnected) {
		enableInterrupts(irq);
		return;
	}
	put_rb_s(&rb, s, size);
	enableInterrupts(irq);
	//xmprintf(2, "ethFeed: + %d bytes \r\n", size);
}

void ethLoop() {
	EthernetLinkStatus currentLinkStatus = Ethernet.linkStatus();
	if (currentLinkStatus != linkStatus) {
		linkStatus = currentLinkStatus;
		xmprintf(2, "ETH link status %d \r\n", linkStatus);
	}
	if (linkStatus == LinkOFF) {
		ethStatus = sEthNoLink;
		if (clientConnected) {
			clientConnected = false;
			xmprintf(2, "ETH client disconnected 3 \r\n");
		}
		return;
	}
	if (!client) {
		client = server.available();
	}
	if (!client) {
		if (clientConnected) {
			clientConnected = false;
			xmprintf(2, "ETH client disconnected 2 \r\n");
		}
		return;
	}
	bool con = client.connected();
	if (clientConnected != con) {
		clientConnected  = con;
		if (clientConnected) {
			bool irq = disableInterrupts();
			resetByteRoundBuf(&rb);
			enableInterrupts(irq);
			xmprintf(2, "ETH client connected \r\n");
		} else {
			xmprintf(2, "ETH client disconnected \r\n");
			//client.close();
			client.stop();
			return;
		}
	}
	if (!clientConnected) {
		return;
	}
	int bs, bs1;
	while ((bs = client.available()) > 0) {
		bs1 = client.read(packetBuffer, maxPacketSize);
		packetBuffer[maxPacketSize-1] = 0;
		//xmprintf(2, "ETH: got %d bytes {%s} \r\n", bs1, packetBuffer);
		if ((bs1 > 0)) {
		 	if (strncmp(packetBuffer, "console", 7) == 0 ) {  //  console client connected
				client.write("tee ", 4);
			} else 	if ((bs1 > 4) && (memcmp(packetBuffer, "TBWF", 4) == 0)) {
				if (infoHandler) {
					infoHandler(gfPleaseSendNext);
				}
			} else if ((bs1 > 4) && (memcmp(packetBuffer, "TBWN", 4) == 0)) {
				if (infoHandler) {
					infoHandler(gfPleaseRepeat);
				}
			} else {
				int bs2 = (bs1 >= maxPacketSize) ? maxPacketSize-1 : bs1;
				packetBuffer[bs2] = 0;
				processTheCommand(packetBuffer, bs2); // process the command
			}
		}
	}
	bool irq = disableInterrupts();
	int num = rb.num;
	enableInterrupts(irq);
	while (num > 0) {
		irq = disableInterrupts();
		int bs = get_rb_s(&rb, buf2, bufSize2-1);
		num = rb.num;
		enableInterrupts(irq);
		if (bs > 0) {
			int u = client.availableForWrite();
			buf2[bs] = 0; // !
			buf2[bufSize2-1] = 0; // ?
			int test = client.write(buf2, bs);
			
			//xmprintf(2, "\r\nETH availableForWrite=%d; test = %d; bs=%d *sending* {%s} \r\n", u, test, bs, buf2);
		}
	}


	/*
	// if there's data available, read a packet
	int packetSize = udp.parsePacket();
	if (packetSize) {
		IPAddress ra = udp.remoteIP();
		uint16_t rp = udp.remotePort();
		// read the packet into packetBufffer
		int bs = udp.read(packetBuffer, maxUdpSize);
		if (bs >= maxUdpSize) {
			bs = maxUdpSize-1;
		}
		packetBuffer[bs] = 0;  //  put zero AFTER the message
		xmprintf(2, "ETH got %d bytes from %d.%d.%d.%d port %d (%s)\r\n",
			bs,
			ra[0], 	ra[1], ra[2], ra[3],
			rp,
			packetBuffer
		);

		// send a reply to the IP address and port that sent us the packet we received
		udp.beginPacket(ra, rp);
		udp.write(ReplyBuffer);
		udp.endPacket();

		if ((bs > 0) && (packetBuffer[0] == '@')) {
			processTheCommand(packetBuffer + 1); // process the command
		}
		

		
  	} 
	*/  
	unsigned int ms = millis();
	if ((ms - aliveTime) > 1000) {
		aliveTime = ms;
		//udp.beginPacket("192.168.0.181", 8888);
		//udp.write("alive\r\n");
		//udp.endPacket();
		if (clientConnected) {
			client.write("ping\n", 6);
		}
	}
	
}

void teensyMAC(uint8_t *mac) {
	static char teensyMac[23];

	//Serial.println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
	for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
	for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
		
	sprintf(teensyMac, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//Serial.println(teensyMac);
}
