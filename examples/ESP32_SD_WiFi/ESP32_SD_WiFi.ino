/*
  ESP32 example: Simple FTP Server using SD (SPI)
  Folder/name aligned with other examples: ESP32_SD_WiFi

  AUTHOR: adapted from project example

  NOTES:
  - Change WIFI_SSID and WIFI_PASSWORD to your WiFi network.
  - Change FTP_USER and FTP_PASS to desired FTP credentials.
  - If your SD module uses a different CS pin, change SD_CS_PIN.
*/

#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <SimpleFTPServer.h>

// WiFi credentials (replace before use)
const char* WIFI_SSID = "<YOUR_SSID>";
const char* WIFI_PASSWORD = "<YOUR_PASSWORD>";

// FTP credentials (replace before use)
const char* FTP_USER = "user";
const char* FTP_PASS = "password";

// SD chip select pin (change if your board/module uses a different pin)
#ifndef SD_CS_PIN
#define SD_CS_PIN SS
#endif

FtpServer ftpSrv;
bool ftpStarted = false;

// Callbacks (avoid leading underscore names to reduce linter warnings)
void ftpStatusCallback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println(F("FTP: Connected!"));
      break;
    case FTP_DISCONNECT:
      Serial.println(F("FTP: Disconnected!"));
      break;
    case FTP_FREE_SPACE_CHANGE:
      Serial.printf("FTP: Free space change, free %u of %u!\n", freeSpace, totalSpace);
      break;
    default:
      break;
  }
}

void ftpTransferCallback(FtpTransferOperation ftpOperation, const char* name, uint32_t transferredSize) {
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      Serial.println(F("FTP: Upload start!"));
      break;
    case FTP_UPLOAD:
      Serial.printf("FTP: Upload of file %s byte %u\n", name, transferredSize);
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
}

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }
  delay(100);

  Serial.println();
  Serial.print("Connecting to WiFi '"); Serial.print(WIFI_SSID); Serial.println("'...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_11dBm);

  // Wait for connection
  uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - started) < 20000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to "); Serial.println(WIFI_SSID);
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi not connected (timeout). Proceeding anyway.");
  }

  Serial.print(F("Initializing SD (CS=")); Serial.print(SD_CS_PIN); Serial.print(F(")..."));
  // Initialize SPI (use default VSPI pins on most ESP32 boards)
  SPI.begin();
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("fail."));
    Serial.println(F("SD initialization failed - FTP will not start. Check wiring and CS pin."));
    ftpStarted = false;
    return;
  }
  Serial.println(F("done."));

  // setup callbacks and start ftp server
  ftpSrv.setCallback(ftpStatusCallback);
  ftpSrv.setTransferCallback(ftpTransferCallback);

  Serial.println("Starting FTP server...");
  ftpSrv.begin(FTP_USER, FTP_PASS);
  ftpStarted = true;
}

void loop() {
  if (ftpStarted) {
    ftpSrv.handleFTP();
  } else {
    delay(1000);
  }
}

