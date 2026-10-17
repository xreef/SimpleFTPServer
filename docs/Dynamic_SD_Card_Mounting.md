# Dynamic SD Card Mounting

This document explains how to mount and unmount the SD card dynamically based on FTP connection state, addressing Issue #79.

## Problem Statement

When an SD card is shared between multiple devices or when you want to minimize power consumption, automatically mounting the SD card at startup is not ideal. You may want to:

1. **Mount** the SD card only when an FTP client connects
2. **Unmount** the SD card when the FTP client disconnects
3. Make the SD card available to other devices when not in use

## Solution Overview

SimpleFTPServer provides callback functions that are triggered on FTP connection events:
- `FTP_CONNECT` - Called when a client connects
- `FTP_DISCONNECT` - Called when a client disconnects
- `FTP_FREE_SPACE_CHANGE` - Called when storage space changes

These callbacks can be used to dynamically mount/unmount the SD card.

## Implementation

### Using Standard SD Library

```cpp
#include <WiFi.h>
#include <SD.h>
#include <SimpleFTPServer.h>

const int SD_CS_PIN = 5;
FtpServer ftpServer;
bool sdCardMounted = false;

void ftpCallback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println("FTP Client connected - Mounting SD card...");
      if (SD.begin(SD_CS_PIN)) {
        sdCardMounted = true;
        Serial.println("SD card mounted successfully");
      } else {
        Serial.println("SD card mount failed!");
      }
      break;
      
    case FTP_DISCONNECT:
      Serial.println("FTP Client disconnected - Unmounting SD card...");
      if (sdCardMounted) {
        SD.end();
        sdCardMounted = false;
        Serial.println("SD card unmounted");
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi...
  WiFi.begin("ssid", "password");
  
  // Set callback BEFORE starting FTP server
  ftpServer.setCallback(ftpCallback);
  
  // Start FTP server (SD card is NOT mounted yet)
  ftpServer.begin("user", "password");
}

void loop() {
  ftpServer.handleFTP();
}
```

### Using SdFat2 Library

```cpp
#include <WiFi.h>
#include <SimpleFTPServer.h>
#include <SdFat.h>

const int SD_CS_PIN = 5;
SdFat sd;
FtpServer ftpServer;
bool sdCardMounted = false;

void ftpCallback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println("FTP Client connected - Mounting SD card...");
      if (sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
        sdCardMounted = true;
        Serial.println("SD card mounted successfully");
      } else {
        Serial.println("SD card mount failed!");
      }
      break;
      
    case FTP_DISCONNECT:
      Serial.println("FTP Client disconnected - Unmounting SD card...");
      if (sdCardMounted) {
        sd.end();
        sdCardMounted = false;
        Serial.println("SD card unmounted");
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi...
  WiFi.begin("ssid", "password");
  
  // Set callback BEFORE starting FTP server
  ftpServer.setCallback(ftpCallback);
  
  // Start FTP server (SD card is NOT mounted yet)
  ftpServer.begin("user", "password");
}

void loop() {
  ftpServer.handleFTP();
}
```

## Complete Examples

Two complete working examples are provided in the `examples` folder:

1. **ESP32_Dynamic_SD_Mount** - Using standard SD library
2. **ESP32_Dynamic_SD_Mount_SdFat** - Using SdFat2 library

Both examples demonstrate:
- WiFi connection
- FTP server initialization without mounting SD
- Automatic SD mounting on FTP connect
- Automatic SD unmounting on FTP disconnect
- Detailed serial output for debugging

## Key Points

### 1. Callback Registration
Always register the callback **before** calling `ftpServer.begin()`:
```cpp
ftpServer.setCallback(ftpCallback);
ftpServer.begin("user", "password");
```

### 2. Do Not Mount in setup()
Remove or comment out SD mounting code from `setup()`:
```cpp
void setup() {
  // DON'T do this:
  // SD.begin(SD_CS_PIN);
  
  // Instead, mount in FTP_CONNECT callback
  ftpServer.setCallback(ftpCallback);
  ftpServer.begin("user", "password");
}
```

### 3. Track Mount State
Use a boolean flag to track SD card mount state:
```cpp
bool sdCardMounted = false;
```

This prevents trying to mount an already-mounted card or unmount an unmounted card.

### 4. Proper Cleanup
Always call `.end()` to properly unmount the SD card:
```cpp
// For SD library:
SD.end();

// For SdFat2:
sd.end();
```

## Shared SD Card Between Devices

When sharing an SD card between multiple devices:

### Hardware Considerations
1. Use proper pull-up resistors on SPI lines
2. Ensure only one device accesses the card at a time
3. Add physical indicators (LEDs) to show which device is using the card

### Software Considerations
1. Always unmount before allowing other device access
2. Add delays between mount/unmount and device switching
3. Handle mount failures gracefully

### Example: Two-Device System

**Device A (ESP32 with FTP Server):**
```cpp
// Mounts SD only when FTP client connects
// Unmounts when FTP client disconnects
ftpServer.setCallback(ftpCallback);
```

**Device B (Data Logger):**
```cpp
// Check if SD is available before mounting
if (SD.begin(CS_PIN)) {
  // Log data
  SD.end();
} else {
  // SD is in use by FTP server, try later
}
```

## Troubleshooting

### SD Card Won't Mount in Callback
**Symptoms**: SD mount fails when FTP client connects

**Solutions**:
1. Check CS pin number is correct
2. Verify SPI pins are properly configured
3. Add delay before mounting:
   ```cpp
   delay(100);
   SD.begin(SD_CS_PIN);
   ```
4. Check for proper power supply (SD cards need stable voltage)

### SD Card Still Busy After Unmount
**Symptoms**: Other device cannot access SD after FTP disconnect

**Solutions**:
1. Ensure all files are closed before unmounting
2. Add delay after `.end()`:
   ```cpp
   SD.end();
   delay(500);  // Give SD card time to finalize
   ```
3. Power cycle the SD card if supported by hardware

### Connection Drops During Transfer
**Symptoms**: FTP connection drops while transferring files

**Solutions**:
1. Increase FTP client timeout settings
2. Check WiFi signal strength
3. Reduce FTP buffer size if memory constrained
4. Monitor serial output for errors

## Advanced Usage

### Multiple Callbacks

You can also use transfer callbacks for fine-grained control:

```cpp
void transferCallback(FtpTransferOperation op, const char* name, unsigned int size) {
  switch (op) {
    case FTP_UPLOAD_START:
      Serial.print("Starting upload: ");
      Serial.println(name);
      break;
      
    case FTP_DOWNLOAD_START:
      Serial.print("Starting download: ");
      Serial.println(name);
      break;
      
    case FTP_TRANSFER_STOP:
      Serial.println("Transfer complete");
      break;
  }
}

ftpServer.setTransferCallback(transferCallback);
```

### Custom Mount Logic

```cpp
void ftpCallback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  if (ftpOperation == FTP_CONNECT) {
    // Try multiple mount attempts
    int attempts = 3;
    while (attempts-- > 0) {
      if (SD.begin(SD_CS_PIN)) {
        sdCardMounted = true;
        Serial.println("SD mounted");
        return;
      }
      delay(1000);
    }
    Serial.println("SD mount failed after 3 attempts");
  }
}
```

## Benefits

1. **Reduced Power Consumption**: SD card is only powered when needed
2. **Shared Access**: Other devices can access SD when FTP is not active
3. **Cleaner Startup**: System boots without waiting for SD initialization
4. **Better Error Handling**: Mount failures don't prevent system startup

## References

- [Issue #79](https://github.com/xreef/SimpleFTPServer/issues/79) - Original feature request
- [ESP32_Dynamic_SD_Mount example](../examples/ESP32_Dynamic_SD_Mount/)
- [ESP32_Dynamic_SD_Mount_SdFat example](../examples/ESP32_Dynamic_SD_Mount_SdFat/)

## Credits

Dynamic SD mounting feature documentation created in response to issue #79 reported by @dreamer1234.

