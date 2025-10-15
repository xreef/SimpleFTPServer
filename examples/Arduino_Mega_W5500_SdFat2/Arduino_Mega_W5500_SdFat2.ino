/*
 * FTP SERVER
 *
 * FTP SERVER FOR W5500 ETHERNET SHIELD WITH SdFat2
 *
 * Based on Arduino_Mega_W5500_SD example, adapted to use SdFat (SdFat2) library
 *
 * Created by automated edit
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SimpleFTPServer.h>
#include <SdFat.h>

// FTP server credentials
const char* ftp_user = "myuser";
const char* ftp_pass = "mypass";

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

// SD card chip select pin
const int sd_cs_pin = 4;

FtpServer ftpSrv;

// SdFat objects
SdFat sd;

bool sd_ok = false;

void setup() {
  Serial.begin(115200);

  // Ensure SPI and CS pins are in a safe state for Arduino Mega
  // On AVR the SS pin (53 on Mega) must be set as OUTPUT to keep SPI in master mode
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH); // keep SS high by default

  // Typical Ethernet shield uses pin 10 as Ethernet CS
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  // SD card chip select pin - put it HIGH while initializing Ethernet
  pinMode(sd_cs_pin, OUTPUT);
  digitalWrite(sd_cs_pin, HIGH);

  // ---- ETHERNET ----
  Serial.println("Starting Ethernet...");

  const int ethRetries = 3;
  bool eth_ok = false;
  for (int i = 0; i < ethRetries; ++i) {
    Serial.print("Ethernet.begin attempt "); Serial.print(i+1); Serial.println("...");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // try to configure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
    }

    // small delay to let interface come up
    delay(1000);

    IPAddress local = Ethernet.localIP();
    Serial.print("Local IP after attempt: ");
    Serial.println(local);
    if (local[0] != 0 || local[1] != 0 || local[2] != 0 || local[3] != 0) {
      eth_ok = true;
      break;
    }
    Serial.println("Ethernet not ready yet, retrying...");
  }

  if (!eth_ok) {
    Serial.println("Warning: Ethernet may not be initialized correctly. Continuing anyway.");
  }

  Serial.print("FTP Server IP address: ");
  Serial.println(Ethernet.localIP());

  // ---- SD CARD (SdFat) ----
  Serial.println("Initializing SdFat (SdFat2) card...");

  const int sdRetries = 3;
  for (int i = 0; i < sdRetries; ++i) {
    Serial.print("sd.beginChipSelect attempt "); Serial.print(i+1); Serial.println("...");
    // Ensure SD CS is asserted high before initialization of other devices
    digitalWrite(sd_cs_pin, HIGH);
    delay(50);
    if (sd.begin(sd_cs_pin, SD_SCK_MHZ(50))) {
      sd_ok = true;
      break;
    }
    delay(100);
  }

  if (!sd_ok) {
    Serial.println("SdFat initialization failed. FTP server will still start but file operations may fail.");
  } else {
    Serial.println("SdFat initialized.");
  }

  // ---- FTP SERVER ----
  ftpSrv.begin(ftp_user, ftp_pass);
}

void loop() {
  ftpSrv.handleFTP();
}
