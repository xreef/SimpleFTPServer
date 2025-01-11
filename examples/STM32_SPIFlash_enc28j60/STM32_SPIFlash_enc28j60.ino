/**
 * MultiFTPServer ^1.3.0 on STM32 (need FLASH > 64K)
 * and ethernet w5500
 * SPI Flash with Adafruit_SPIFlash and SdFat-Adafruit-Fork library
 *

#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32 	NETWORK_ETHERNET_ENC
	#define DEFAULT_STORAGE_TYPE_STM32 STORAGE_SPIFM
#endif

 *
 * @author Renzo Mischianti <www.mischianti.org>
 * @details https://www.mischianti.org/category/my-libraries/simple-ftp-server/
 * @version 0.1
 * @date 2022-03-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <Arduino.h>

#include <EthernetEnc.h>

#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

#include <MultiFTPServer.h>

Adafruit_FlashTransport_SPI flashTransport(SS, SPI); // Set CS and SPI interface
Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

#define ETHERNET_CS_PIN PA3

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
	  // Initialize serial port and wait for it to open before continuing.
	  Serial.begin(115200);
	  while (!Serial) {
	    delay(100);
	  }
	  Serial.println("Adafruit SPI Flash FatFs Full Usage Example");

	  // Initialize flash library and check its chip ID.
	  if (!flash.begin()) {
	    Serial.println("Error, failed to initialize flash chip!");
	    while(1) yield();
	  }

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


    // You can use Ethernet.init(pin) to configure the CS pin
    Ethernet.init(ETHERNET_CS_PIN);

    if (Ethernet.begin(mac)) { // Dynamic IP setup
        Serial.println(F("DHCP OK!"));
    }else{
        Serial.println(F("Failed to configure Ethernet using DHCP"));
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
          while (true) {
            delay(1); // do nothing, no point running without Ethernet hardware
          }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println(F("Ethernet cable is not connected."));
        }

    	  IPAddress ip(MYIPADDR);
    	  IPAddress dns(MYDNS);
    	  IPAddress gw(MYGW);
    	  IPAddress sn(MYIPMASK);
    	  Ethernet.begin(mac, ip, dns, gw, sn);
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
    Serial.println();


  // Initialize the FTP server
  ftpSrv.begin("user","password");
  Serial.println("Ftp server started!");
}

void loop()
{
  ftpSrv.handleFTP();

  // more processes...
}
