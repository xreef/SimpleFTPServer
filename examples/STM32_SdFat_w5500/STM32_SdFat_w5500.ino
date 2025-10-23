l /**
 * SimpleFTPServer ^1.3.0 on STM32 (need FLASH > 64K)
 * and ethernet w5500
 * SD connected on secondary SPI or primary
 *

#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32 	NETWORK_W5100
	#define DEFAULT_STORAGE_TYPE_STM32 STORAGE_SDFAT2
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
#include <SdFat.h>
#include <sdios.h>
#include <Ethernet.h>

#include <SimpleFtpServer.h>

// To use SD with primary SPI
// #define SD_CS_PIN PA4

// To use SD with secondary SPI
#define SD_CS_PIN PB12
static SPIClass mySPI2(PB15, PB14, PB13, SD_CS_PIN);
#define SD2_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(18), &mySPI2)

SdFat sd;

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
  Serial.begin( 115200 );

  while (!Serial) { delay(100); }

  Serial.print("\nInitializing SD card...");

  // Secondary SPI for SD
  if (!sd.begin(SD2_CONFIG)) {
  // Primary SPI for SD
  // if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    while (1);
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  // Show capacity and free space of SD card
  Serial.print(F("Capacity of card:   ")); Serial.print(long( sd.card()->sectorCount() >> 1 )); Serial.println(F(" kBytes"));

    // You can use Ethernet.init(pin) to configure the CS pin
    Ethernet.init(PA4);

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
