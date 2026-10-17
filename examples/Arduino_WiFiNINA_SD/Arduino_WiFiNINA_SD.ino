/*
 * Simple FTP Server Example with SD Card and WiFiNINA
 * (Arduino Nano 33 IoT, Arduino MKR WiFi 1010, Arduino UNO WiFi Rev.2, Nano RP2040 Connect)
 *
 * AUTHOR: Renzo Mischianti
 * URL: https://www.mischianti.org
 *
 * DESCRIPTION:
 * This example demonstrates how to use the SimpleFTPServer library
 * with a WiFiNINA-based board and an SPI SD card module.
 * It checks the u-blox NINA-W102 firmware version (requires >= 1.5.0 for
 * WiFiServer::accept() FTP support), connects to WiFi, mounts the SD card,
 * and starts the FTP server on port 21 (Passive data port 50009).
 *
 * https://www.mischianti.org/category/my-libraries/simple-ftp-server/
 *
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>
#include <SimpleFTPServer.h>

// WiFi credentials
const char* WIFI_SSID     = "<YOUR-SSID>";      // Replace with your WiFi SSID
const char* WIFI_PASSWORD = "<YOUR-PASSWD>";    // Replace with your WiFi password

// SD card chip select pin (e.g. D10 on Nano 33 IoT, D4 on MKR SD Proto Shield)
const int CHIP_SELECT_PIN = 10;

// FTP server instance
FtpServer ftpSrv;

void _callback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println(F("FTP: Client connected!"));
      break;
    case FTP_DISCONNECT:
      Serial.println(F("FTP: Client disconnected!"));
      break;
    case FTP_FREE_SPACE_CHANGE:
      Serial.print(F("FTP: Free space change -> "));
      Serial.print(freeSpace);
      Serial.print(F(" / "));
      Serial.println(totalSpace);
      break;
    default:
      break;
  }
}

void _transferCallback(FtpTransferOperation ftpOperation, const char* name, uint32_t transferredSize) {
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      Serial.print(F("FTP: Upload start -> "));
      Serial.println(name);
      break;
    case FTP_DOWNLOAD_START:
      Serial.print(F("FTP: Download start -> "));
      Serial.println(name);
      break;
    case FTP_TRANSFER_STOP:
      Serial.print(F("FTP: Transfer completed ("));
      Serial.print(transferredSize);
      Serial.println(F(" bytes)"));
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
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    delay(10);
  }

  // 1. Check WiFiNINA module communication and firmware version
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("ERROR: Communication with WiFiNINA module failed!"));
    while (true) {
      delay(100);
    }
  }

  String fv = WiFi.firmwareVersion();
  Serial.print(F("WiFiNINA Firmware Version: "));
  Serial.println(fv);
  if (fv < "1.5.0") {
    Serial.println(F("WARNING: WiFiNINA firmware < 1.5.0 detected!"));
    Serial.println(F("Please upgrade the NINA-W102 firmware to >= 1.5.0 (required for WiFiServer::accept() FTP support)."));
  } else if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print(F("NOTE: A newer WiFiNINA firmware ("));
    Serial.print(WIFI_FIRMWARE_LATEST_VERSION);
    Serial.println(F(") is available."));
  }

  // 2. Initialize SD card
  Serial.print(F("Initializing SD card (CS="));
  Serial.print(CHIP_SELECT_PIN);
  Serial.print(F(")..."));
  while (!SD.begin(CHIP_SELECT_PIN)) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(" OK!"));

  // 3. Connect to WiFi network
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("\nWiFi connected!"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // 4. Start FTP server with username and password
  ftpSrv.setCallback(_callback);
  ftpSrv.setTransferCallback(_transferCallback);
  ftpSrv.begin("user", "password"); // Replace with your desired FTP credentials
  Serial.println(F("FTP server started on port 21 (Passive Mode)!"));
}

void loop() {
  // Continuously process FTP requests
  ftpSrv.handleFTP();
}

