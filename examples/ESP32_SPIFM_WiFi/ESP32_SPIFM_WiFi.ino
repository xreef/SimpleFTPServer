/**
 * ESP32 and external SPI Flash FTP server
 *
 *
// esp32 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 		NETWORK_ESP32
	#define DEFAULT_STORAGE_TYPE_ESP32 					STORAGE_SPIFM
#endif
 *
 */

#include <Arduino.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include <WiFi.h>
#include <SimpleFTPServer.h>

Adafruit_FlashTransport_SPI flashTransport(SS, SPI); // Set CS and SPI interface
Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

const char* ssid = "<YOUR_SSID>";
const char* password = "<YOUR_PASSWD>";


FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println(F("FTP: Connected!"));
      break;
    case FTP_DISCONNECT:
      Serial.println(F("FTP: Disconnected!"));
      break;
    case FTP_FREE_SPACE_CHANGE:
    	Serial.print(F("FTP: Free space change, free "));
    	Serial.print(freeSpace);
    	Serial.print(F(" of "));
    	Serial.println(totalSpace);
      break;
    default:
      break;
  }
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      Serial.println(F("FTP: Upload start!"));
      break;
    case FTP_UPLOAD:
      Serial.print(F("FTP: Upload of file "));
      Serial.print(name);
      Serial.print(F(" "));
      Serial.print(transferredSize);
      Serial.println(F("bytes"));
      break;
    case FTP_TRANSFER_STOP:
      Serial.println(F("FTP: Finish transfer!"));
      break;
    case FTP_TRANSFER_ERROR:
      Serial.println(F("FTP: Transfer error!"));
      break;
    default:
      break;
  }

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
};

void setup()
{
	  // Initialize serial port and wait for it to open before continuing.
	  Serial.begin(115200);
	  while (!Serial) {
	    delay(100);
	  }
	  Serial.println("FTP with external SPIFlash");

	  WiFi.begin(ssid, password);
	  Serial.println("");

	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(500);
	    Serial.print(".");
	  }
	  Serial.println("");
	  Serial.print("Connected to ");
	  Serial.println(ssid);
	  Serial.print("IP address: ");
	  Serial.println(WiFi.localIP());
	  Serial.print("SN address: ");
	  Serial.println(WiFi.subnetMask());

	  if (flash.begin()) {
		  Serial.println(F("Device finded and supported!"));
	  } else {
		  Serial.println(F("Problem to discover and configure device, check wiring also!"));
		  while(1) yield();
	  }
	  // Set 4Mhz SPI speed
	  flashTransport.setClockSpeed(4000000, 4000000); // added to prevent speed problem

	  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
	  Serial.print("Flash size: "); Serial.println(flash.size());
	  Serial.flush();

	  // First call begin to mount the filesystem.  Check that it returns true
	  // to make sure the filesystem was mounted.
	  if (!fatfs.begin(&flash)) {
	    Serial.println("Error, failed to mount newly formatted filesystem!");
	    Serial.println("Was the flash chip formatted with the SdFat_format example?");
	    while(1) yield();
	  }
	  Serial.println("Mounted filesystem!");
	  Serial.println();

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      Serial.println("Starting FTP Server!");
      ftpSrv.begin("user","password");    //username, password for ftp.   (default 21, 50009 for PASV)
//      ftpSrv.beginAnonymous();

}

// The loop function is called in an endless loop
void loop()
{
	  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}
