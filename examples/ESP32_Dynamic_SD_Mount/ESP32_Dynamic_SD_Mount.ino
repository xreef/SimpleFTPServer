/*
 *
 * FTP SERVER
 *
 *
 * FTP SERVER FOR W5500 ETHERNET SHIELD WITH SD
 *
 * Created by Renzo Mischianti (www.mischianti.org)
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32/
 *
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is

 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>

#define FTP_SERVER_DEBUG
// Select the NetworkInterface:
//   - NETWORK_W5100
//   - NETWORK_W5500
//   - NETWORK_ENC28J60
#define FTP_SERVER_NETWORK_TYPE_ARDUINO NETWORK_W5500

// Select the StorageType:
//   - STORAGE_SD
#define DEFAULT_STORAGE_TYPE_ARDUINO STORAGE_SD

#include <SimpleFTPServer.h>

// FTP server credentials
const char* ftp_user = "myuser";
const char* ftp_pass = "mypass";

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

// SD card chip select pin
const int sd_cs_pin = 4;

FtpServer ftpSrv;

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

  // ---- SD CARD ----
  Serial.println("Initializing SD card...");

  const int sdRetries = 3;
  for (int i = 0; i < sdRetries; ++i) {
    Serial.print("SD.begin attempt "); Serial.print(i+1); Serial.println("...");
    // Ensure SD CS is asserted high before initialization of other devices
    digitalWrite(sd_cs_pin, HIGH);
    delay(50);
    if (SD.begin(sd_cs_pin)) {
      sd_ok = true;
      break;
    }
  }
  Serial.println("initialization done.");

  // ---- FTP SERVER ----
  ftpSrv.begin(ftp_user, ftp_pass);
}

void loop() {
  ftpSrv.handleFTP();
}
