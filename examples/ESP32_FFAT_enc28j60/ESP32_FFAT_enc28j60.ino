/*
 * FtpServer esp32 with FFat and EthernetENC (or UIPEthernet)
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include "Arduino.h"
#include <SPI.h>
#include <EthernetENC.h>

#include "FS.h"
#include "FFat.h"

#include <MultiFTPServer.h>

#define MACADDRESS 0x00,0x01,0x02,0x03,0x04,0x05
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1

uint8_t macaddress[6] = {MACADDRESS};

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
	Serial.print(">>>>>>>>>>>>>>> _callback " );
	Serial.print(ftpOperation);
	/* FTP_CONNECT,
	 * FTP_DISCONNECT,
	 * FTP_FREE_SPACE_CHANGE
	 */
	Serial.print(" ");
	Serial.print(freeSpace);
	Serial.print(" ");
	Serial.println(totalSpace);

	// freeSpace : totalSpace = x : 360

	if (ftpOperation == FTP_CONNECT) Serial.println(F("CONNECTED"));
	if (ftpOperation == FTP_DISCONNECT) Serial.println(F("DISCONNECTED"));
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
	Serial.print(">>>>>>>>>>>>>>> _transferCallback " );
	Serial.print(ftpOperation);
	/* FTP_UPLOAD_START = 0,
	 * FTP_UPLOAD = 1,
	 *
	 * FTP_DOWNLOAD_START = 2,
	 * FTP_DOWNLOAD = 3,
	 *
	 * FTP_TRANSFER_STOP = 4,
	 * FTP_DOWNLOAD_STOP = 4,
	 * FTP_UPLOAD_STOP = 4,
	 *
	 * FTP_TRANSFER_ERROR = 5,
	 * FTP_DOWNLOAD_ERROR = 5,
	 * FTP_UPLOAD_ERROR = 5
	 */
	Serial.print(" ");
	Serial.print(name);
	Serial.print(" ");
	Serial.println(transferredSize);
};


void setup(void){
  Serial.begin(115200);

  Serial.println("Begin Ethernet");

  Ethernet.init(5);
  if (Ethernet.begin(macaddress)) { // Dynamic IP setup
      Serial.println("DHCP OK!");
  }else{
      Serial.println("Failed to configure Ethernet using DHCP");
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }

  	  IPAddress ip(MYIPADDR);
  	  IPAddress dns(MYDNS);
  	  IPAddress gw(MYGW);
  	  IPAddress sn(MYIPMASK);
  	  Ethernet.begin(macaddress, ip, dns, gw, sn);
		  Serial.println("STATIC OK!");
  }
  delay(5000);


  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());

 Serial.println("Ethernet Successfully Initialized");

  /////FTP Setup, ensure FFat is started before ftp;  /////////
  if (FFat.begin(true)) {
      Serial.println("FFat opened!");

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      ftpSrv.begin("user","password");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
      Serial.println("FTP server started!");
  } else {
	  Serial.println("FFat opened FAIL!!!!!");
  }

}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
 // server.handleClient();   //example if running a webserver you still need to call .handleClient();

}
