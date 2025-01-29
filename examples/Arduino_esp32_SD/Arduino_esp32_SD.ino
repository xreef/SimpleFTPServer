/*
 * Simple FTP Server Example with SD Card on ESP32
 *
 * AUTHOR: Renzo Mischianti
 * URL: https://www.mischianti.org
 *
 * DESCRIPTION:
 * This example demonstrates how to use the SimpleFTPServer library
 * with an ESP32 and an SD card module. The ESP32 connects to a WiFi network
 * and initializes an FTP server for file transfers.
 *
 * FEATURES:
 * - WiFi connection to local network
 * - SD card initialization for file storage
 * - FTP server setup for file uploads and downloads
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <WiFi.h>
#include <SimpleFTPServer.h>
#include <SPI.h>
#include <SD.h>

// WiFi credentials
const char* WIFI_SSID = "<YOUR-SSID>";    		// Replace with your WiFi SSID
const char* WIFI_PASSWORD = "<YOUR-PASSWD>";    // Replace with your WiFi password

// SD card chip select pin
const int CHIP_SELECT_PIN = SS;               // Default SS pin for SPI

// FTP server instance
FtpServer ftpServer;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial) {
    // Wait for serial port to connect (required for native USB ports)
  }

  // Connect to WiFi network
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.printf("Connected to: %s\n", WIFI_SSID);
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());

  // Wait for a short delay before initializing SD card
  delay(1000);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  while (!SD.begin(CHIP_SELECT_PIN)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nSD card initialized successfully!");

  // Start FTP server with username and password
  ftpServer.begin("user", "password"); // Replace with your desired FTP credentials
  Serial.println("FTP server started!");
}

void loop() {
  // Handle FTP server operations
  ftpServer.handleFTP(); // Continuously process FTP requests
}
