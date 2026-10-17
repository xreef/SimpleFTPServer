/*
 * SimpleFTPServer - Dynamic SD Card Mounting Example (Standard SD Library)
 *
 * This example demonstrates how to mount the SD card only when an FTP
 * connection is established, and unmount it when the connection is closed.
 *
 * This is useful when:
 * - The SD card is shared between multiple devices
 * - You want to minimize power consumption
 * - You need to access the SD card from another device
 *
 * Hardware:
 * - ESP32 board
 * - SD card module connected via SPI
 *
 * Related Issue: #79
 * https://github.com/xreef/SimpleFTPServer/issues/79
 *
 * Author: Renzo Mischianti
 * www.mischianti.org
 */

#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <SimpleFTPServer.h>

// WiFi credentials
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

// FTP credentials
const char* FTP_USER = "admin";
const char* FTP_PASS = "admin";

// SD Card configuration
const int SD_CS_PIN = 5;  // Adjust to your wiring

// FTP Server
FtpServer ftpServer;

// SD Card state
bool sdCardMounted = false;

// Forward declaration
void ftpCallback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace);

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }
  delay(200);

  Serial.println("\n\n=================================");
  Serial.println("Dynamic SD Mounting (Standard SD)");
  Serial.println("=================================\n");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize SPI bus (do not mount SD card yet - it will be mounted on FTP connect)
  SPI.begin();
  Serial.println("\nSD card NOT mounted at startup");
  Serial.println("It will be mounted when FTP client connects\n");

  // Set up FTP callback to handle mount/unmount
  ftpServer.setCallback(ftpCallback);

  // Start FTP server (without mounting SD)
  ftpServer.begin(FTP_USER, FTP_PASS);

  Serial.println("FTP server started");
  Serial.println("Waiting for FTP connection...");
  Serial.println("=================================\n");
}

void loop() {
  ftpServer.handleFTP();
}

// Callback function called on FTP events
void ftpCallback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println("\n>>> FTP Client connected!");

      // Mount SD card when client connects
      if (!sdCardMounted) {
        Serial.print(">>> Mounting SD card... ");
        if (SD.begin(SD_CS_PIN)) {
          sdCardMounted = true;
          Serial.println("SUCCESS");
        } else {
          Serial.println("FAILED!");
          Serial.println(">>> SD card mount error. Check wiring and card.");
        }
      } else {
        Serial.println(">>> SD card already mounted");
      }
      break;

    case FTP_DISCONNECT:
      Serial.println("\n>>> FTP Client disconnected!");

      // Unmount SD card when client disconnects
      if (sdCardMounted) {
        Serial.print(">>> Unmounting SD card... ");
        SD.end();
        sdCardMounted = false;
        Serial.println("DONE");
        Serial.println(">>> SD card is now available for other devices");
      }

      Serial.println("\nWaiting for next FTP connection...");
      Serial.println("=================================\n");
      break;

    case FTP_FREE_SPACE_CHANGE:
      Serial.println(">>> Storage space changed");
      Serial.print(">>> Free space: ");
      Serial.print((unsigned long)freeSpace);
      Serial.print(" kB / Total: ");
      Serial.print((unsigned long)totalSpace);
      Serial.println(" kB");
      break;

    default:
      break;
  }
}
