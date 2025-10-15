/*
 * SimpleFTPServer - Dynamic SD Card Mounting Example (SdFat2)
 *
 * This example demonstrates how to mount the SD card only when an FTP
 * connection is established, and unmount it when the connection is closed.
 *
 * This version uses the SdFat2 library for better performance and features.
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
#include <SimpleFTPServer.h>
#include <SdFat.h>
#include <sdios.h>

// WiFi credentials
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

// FTP credentials
const char* FTP_USER = "admin";
const char* FTP_PASS = "admin";

// SD Card configuration
const int SD_CS_PIN = 5;  // Adjust to your wiring

// SdFat object
SdFat sd;

// FTP Server
FtpServer ftpServer;

// SD Card state
bool sdCardMounted = false;

// Forward declaration
void ftpCallback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=================================");
  Serial.println("Dynamic SD Mounting (SdFat2)");
  Serial.println("=================================\n");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // DO NOT mount SD card here - it will be mounted on FTP connect
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
void ftpCallback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println("\n>>> FTP Client connected!");

      // Mount SD card when client connects
      if (!sdCardMounted) {
        Serial.print(">>> Mounting SD card... ");

        // Initialize with 50MHz SPI speed
        if (sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
          sdCardMounted = true;
          Serial.println("SUCCESS");

          // Show SD card info
          Serial.print(">>> SD Card Size: ");
          Serial.print(sd.card()->sectorCount() / 2048.0);
          Serial.println(" MB");

          Serial.print(">>> Free Space: ");
          Serial.print(sd.vol()->freeClusterCount() * sd.vol()->sectorsPerCluster() / 2048.0);
          Serial.println(" MB");

          Serial.print(">>> Card Type: ");
          switch (sd.card()->type()) {
            case SD_CARD_TYPE_SD1:
              Serial.println("SD1");
              break;
            case SD_CARD_TYPE_SD2:
              Serial.println("SD2");
              break;
            case SD_CARD_TYPE_SDHC:
              Serial.println("SDHC");
              break;
            default:
              Serial.println("Unknown");
          }
        } else {
          Serial.println("FAILED!");
          Serial.println(">>> SD card mount error. Check wiring and card.");
          Serial.println(">>> Error code: ");
          sd.initErrorPrint(&Serial);
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

        // Close all open files before unmounting
        sd.end();

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
      Serial.print(freeSpace);
      Serial.print(" kB / Total: ");
      Serial.print(totalSpace);
      Serial.println(" kB");
      break;
  }
}

