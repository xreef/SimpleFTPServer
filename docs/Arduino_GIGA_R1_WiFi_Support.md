# Arduino GIGA R1 WiFi Support

This document explains how to use SimpleFTPServer on Arduino GIGA R1 WiFi boards.

## Overview

Arduino GIGA R1 WiFi features:
- **MCU**: STM32H747XI dual-core processor
- **WiFi Module**: Murata 1DX (same as used in some ESP32 boards)
- **Network Library**: Standard Arduino WiFi library
- **Recommended Storage**: SdFat2 library for SD card access

## Issue #78 Background

Previously, the library did not have explicit support for STM32 boards with WiFi. Users attempting to use the library on Arduino GIGA R1 WiFi would encounter:
- **Error 425 "No data connection"** when using FTP clients
- Issues with passive mode (PASV) connections
- Failure to establish data channels for directory listing (MLSD) or file transfers

## Solution

A new network type `NETWORK_STM32_WIFI` has been added to support STM32 boards with WiFi capabilities.

## Configuration

### Method 1: Define Network Type Before Including Library

In your sketch, define the network type before including the library:

```cpp
#define FTP_SERVER_NETWORK_TYPE NETWORK_STM32_WIFI
#define STORAGE_TYPE STORAGE_SDFAT2

#include <WiFi.h>
#include <SimpleFTPServer.h>
#include <SdFat.h>
```

### Method 2: Modify FtpServerKey.h (Advanced)

If you want to make STM32 WiFi the default for all STM32 boards, modify `FtpServerKey.h`:

```cpp
// STM32 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32
    #define DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32 NETWORK_STM32_WIFI  // Changed from NETWORK_W5100
    #define DEFAULT_STORAGE_TYPE_STM32 STORAGE_SDFAT2
#endif
```

## Example Sketch

```cpp
/*
 * SimpleFTPServer Example for Arduino GIGA R1 WiFi
 * 
 * Hardware:
 * - Arduino GIGA R1 WiFi
 * - SD card module connected via SPI
 */

// Define before including libraries
#define FTP_SERVER_NETWORK_TYPE NETWORK_STM32_WIFI
#define STORAGE_TYPE STORAGE_SDFAT2

#include <WiFi.h>
#include <SimpleFTPServer.h>
#include <SdFat.h>
#include <sdios.h>

// WiFi credentials
const char* ssid = "your-ssid";
const char* password = "your-password";

// FTP credentials
const char* ftp_user = "admin";
const char* ftp_pass = "admin";

// SD card
const int SD_CS_PIN = SS;  // Adjust for your setup
SdFat sd;

// FTP Server
FtpServer ftpServer;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("Arduino GIGA R1 WiFi - FTP Server");
  
  // Initialize SD card
  Serial.print("Initializing SD card... ");
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    Serial.println("FAILED!");
    while (1) delay(10);
  }
  Serial.println("OK");
  
  // Show SD card info
  Serial.print("Card size: ");
  Serial.print(sd.card()->sectorCount() / 2048.0);
  Serial.println(" MB");
  
  // Connect to WiFi in AP mode (Access Point)
  Serial.print("Starting WiFi AP... ");
  WiFi.beginAP(ssid, password);
  Serial.println("OK");
  
  // Or connect to existing WiFi network:
  // Serial.print("Connecting to WiFi... ");
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println(" Connected!");
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start FTP server
  ftpServer.begin(ftp_user, ftp_pass);
  Serial.println("FTP server started!");
  Serial.println("Connect with FTP client in PASSIVE mode");
}

void loop() {
  ftpServer.handleFTP();
}
```

## FTP Client Configuration

When connecting to the FTP server on Arduino GIGA R1 WiFi:

### FileZilla Settings
1. **Transfer Mode**: Passive (PASV)
2. **Transfer Settings**: 
   - Limit number of simultaneous connections: **1**
   - Maximum number of connections: **1**
3. **Connection**: 
   - Host: IP address shown in Serial Monitor
   - Port: 21
   - Protocol: FTP
   - Encryption: Use plain FTP

### WinSCP Settings
1. File protocol: FTP
2. Transfer mode: Passive
3. Encryption: No encryption

## Troubleshooting

### Error 425 "No data connection"

**Symptoms**: 
- Can connect and authenticate
- Cannot list directories or transfer files

**Solutions**:
1. Ensure `NETWORK_STM32_WIFI` is defined before including the library
2. Use PASSIVE mode in your FTP client
3. Set FTP client to use only 1 connection at a time
4. Check that WiFi connection is stable
5. Verify SD card is properly initialized

### Compilation Errors

**Error**: `no network type selected!`
- Make sure to define `FTP_SERVER_NETWORK_TYPE` as shown above

**Error**: Storage-related errors
- Ensure SdFat library is installed: `SdFat` by Bill Greiman (version 2.2.3+)
- Check that `STORAGE_TYPE` is set to `STORAGE_SDFAT2`

### WiFi Connection Issues

- If using AP mode, connect your computer directly to the Arduino's WiFi network
- If using Station mode, ensure the Arduino is on the same network as your computer
- Check firewall settings on your computer

## Network Types Reference

The following network types are now supported:

| Constant | Description | Use Case |
|----------|-------------|----------|
| `NETWORK_ESP8266` | ESP8266 WiFi | ESP8266 boards |
| `NETWORK_ESP32` | ESP32 WiFi | ESP32 boards |
| `NETWORK_STM32_WIFI` | STM32 + WiFi | Arduino GIGA R1 WiFi, STM32 with WiFi module |
| `NETWORK_ETHERNET_STM` | STM32 + Ethernet | STM32 with Ethernet shield |
| `NETWORK_W5100` | Ethernet W5100/W5500 | Arduino + Ethernet shield |
| `NETWORK_RP2040_WIFI` | Raspberry Pi Pico W | RP2040 with WiFi |

## Known Limitations

- Only one FTP client connection at a time
- Passive mode only (active mode not recommended for WiFi)
- Large file transfers may be slow depending on WiFi signal strength

## Additional Resources

- [Issue #78](https://github.com/xreef/SimpleFTPServer/issues/78) - Original bug report
- [Arduino GIGA R1 WiFi Documentation](https://docs.arduino.cc/hardware/giga-r1-wifi)
- [SdFat Library](https://github.com/greiman/SdFat)

## Credits

Support for Arduino GIGA R1 WiFi added in response to issue #78 reported by @Boby270.

