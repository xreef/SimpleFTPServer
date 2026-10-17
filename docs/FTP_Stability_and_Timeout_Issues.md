# FTP Server Stability and Timeout Issues

This document addresses connection stability issues, timeout errors, and incomplete file transfers (Issue #80).

## Problem Description

Users may experience:
- **Directory listing freezes** after viewing several folders
- **Connection timeout errors** (530 Timeout, "Connection timed out after 20 seconds")
- **Incomplete file transfers** (some files transfer, others don't)
- **Server becomes unresponsive** and requires restart
- **Large file downloads fail** after transferring only a small percentage (Issue #81)

### Symptoms in Logs

**FileZilla:**
```
Command: MLSD
Response: 150 Accepted data connection to port 50009
Response: 226-options: -a -l
Response: 226 5 matches total
Error: Connection timed out after 20 seconds of inactivity
Error: Failed to retrieve directory listing
```

**ESP32 Serial:**
```
List of file!!Dir opened!!Connected!!
DIR MLSD
All file read!!
530 Timeout  // ← Problem occurs here
```

**Large File Download (Issue #81):**

FTP client shows:
```
ftp> get test.pdf
local: test.pdf remote: test.pdf
227 Entering Passive Mode (192,168,4,1,195,89)
150-Connected to port 50009
150 5802771 bytes to download
  5% |***...                                                               |   329 KiB  328.99 KiB/s    00:16 ETA
ftp: Reading from network: Interrupted system call
426 Data connection closed. Transfer aborted
```

ESP32 Serial shows:
```
NB --> 1024
NB --> 1024
NB --> 1024
...
[76783][D][NetworkClient.cpp:562] connected(): Disconnected: RES: 0, ERR: 128
```

**Key indicators:**
- Small files (<100KB) transfer successfully
- Large files (>1MB) fail after 5-10% transfer
- Error code `ERR: 128` = Connection reset by peer
- Transfer stops suddenly with "Interrupted system call"

## Root Cause

The FTP server requires **frequent and regular calls** to `handleFTP()` to:
1. Process incoming commands
2. Send data over the data connection
3. Close connections properly
4. Handle timeouts

**Common mistake:**
```cpp
void loop() {
    // Wrong: handleFTP() called too infrequently
    
    if (millis() - prevMillis > 1000) {  // Check every 1 second
        checkButtons();
        updateScreen();
        prevMillis = millis();
    }
    
    ftpServer.handleFTP();  // Only called after other tasks!
}
```

When `handleFTP()` is not called frequently enough:
- Data transfers stall
- Timeouts occur
- Connections are not properly closed
- The server becomes unresponsive

## Solutions

### Solution 1: Call handleFTP() First and Often

**Correct approach:**
```cpp
void loop() {
    // Call handleFTP() FIRST and as often as possible
    ftpServer.handleFTP();
    
    // Then do other tasks at intervals
    if (millis() - prevMillis > 1000) {
        checkButtons();
        updateScreen();
        prevMillis = millis();
    }
}
```

### Solution 2: Minimize Blocking Code

**Bad:**
```cpp
void loop() {
    ftpServer.handleFTP();
    
    delay(100);  // ← NEVER use delay() with FTP server!
    
    readSensors();
    longCalculation();  // ← Avoid long-running operations
}
```

**Good:**
```cpp
void loop() {
    ftpServer.handleFTP();  // Called every iteration
    
    // Use millis() for timing, never delay()
    if (millis() - lastSensorRead > 100) {
        readSensors();
        lastSensorRead = millis();
    }
    
    // Break long operations into smaller chunks
    if (millis() - lastCalc > 50) {
        doSmallCalculation();
        lastCalc = millis();
    }
}
```

### Solution 3: Dedicated FTP Task (Advanced)

For complex applications with multiple tasks, use FreeRTOS tasks on ESP32:

```cpp
// Task for FTP server (high priority)
void ftpTask(void *parameter) {
    while (true) {
        ftpServer.handleFTP();
        vTaskDelay(1);  // Yield to other tasks
    }
}

// Task for UI/other operations (lower priority)
void uiTask(void *parameter) {
    while (true) {
        checkButtons();
        updateScreen();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    // Initialize WiFi, SD, FTP server...
    
    // Create tasks
    xTaskCreate(ftpTask, "FTP", 4096, NULL, 2, NULL);  // Priority 2
    xTaskCreate(uiTask, "UI", 4096, NULL, 1, NULL);    // Priority 1
}

void loop() {
    // Empty - tasks handle everything
}
```

## Issue #80 Specific Fix

The user's code that caused the problem:

```cpp
void RunFTP() {
    while (currentOperation == OPERATION_FTP_SERVER) {
        // These checks delay handleFTP() call!
        if (millis() - prevMillis > KEYS_CHECK_INTERVAL) {
            key_pressed = GetKey();
            if (last_key != key_pressed) {
                last_key = key_pressed;
                ProcessButtons();
            }
            prevMillis = millis();
        }
        
        if (millis() - prevMillis_2 > UPDATE_SCREEN_INTERVAL) {
            ScreenUpdate();
            prevMillis_2 = millis();
        }
        
        FTP_server.handleFTP();  // Called last, too infrequently!
    }
}
```

**Fixed version:**

```cpp
void RunFTP() {
    // Initialize SD card and connection here
    
    unsigned long prevMillis = 0;
    unsigned long prevMillis_2 = 0;
    const unsigned long KEYS_CHECK_INTERVAL = 50;     // 50ms for keys
    const unsigned long UPDATE_SCREEN_INTERVAL = 200; // 200ms for screen
    
    while (currentOperation == OPERATION_FTP_SERVER) {
        // ALWAYS call handleFTP() first and every iteration!
        FTP_server.handleFTP();
        
        // Check keys at intervals
        unsigned long currentMillis = millis();
        if (currentMillis - prevMillis >= KEYS_CHECK_INTERVAL) {
            prevMillis = currentMillis;
            key_pressed = GetKey();
            if (last_key != key_pressed) {
                last_key = key_pressed;
                ProcessButtons();
            }
        }
        
        // Update screen at intervals
        if (currentMillis - prevMillis_2 >= UPDATE_SCREEN_INTERVAL) {
            prevMillis_2 = currentMillis;
            ScreenUpdate();
        }
    }
    
    // Disconnect and exit operations here
}
```

## Issue #81 Specific Fix - Large File Downloads

**Problem:** Downloads of large files (>1MB) fail after transferring only 5-10% of data.

**Why it happens:**

The FTP server sends data in chunks of 1024 bytes (FTP_BUF_SIZE) per call to `handleFTP()`. For a 5.8MB file:
- Total chunks needed: ~5,700
- If `handleFTP()` is called every 100ms: Transfer takes 570 seconds (9.5 minutes)
- TCP connection times out before completion

**Symptoms:**
```
NB --> 1024    // Server sending data
NB --> 1024
...
Disconnected: RES: 0, ERR: 128  // Connection reset
```

**Solution:**

Ensure `handleFTP()` is called **every loop iteration** without delays:

```cpp
void loop() {
    // Critical: Call this EVERY iteration for large file transfers
    ftpServer.handleFTP();
    
    // NO delays or slow operations here!
}
```

**For Issue #81 user's code:**

If you have a gist link showing code like this:
```cpp
void loop() {
    // Some task that takes time
    doSomething();
    
    ftpServer.handleFTP();  // Called too late!
}
```

Change to:
```cpp
void loop() {
    // FTP FIRST - critical for large file transfers
    ftpServer.handleFTP();
    
    // Then other tasks at intervals
    static unsigned long lastTask = 0;
    if (millis() - lastTask >= 100) {
        doSomething();
        lastTask = millis();
    }
}
```

**Transfer Speed Calculation:**

To transfer large files successfully, you need high loop frequency:

| Loop Frequency | Transfer Speed (1024 byte chunks) | 5MB File Time |
|----------------|-----------------------------------|---------------|
| 10 loops/sec   | 10 KB/s                          | ~500 seconds  |
| 100 loops/sec  | 100 KB/s                         | ~50 seconds   |
| 1,000 loops/sec| 1 MB/s                           | ~5 seconds    |
| 10,000 loops/sec| 10 MB/s                         | ~0.5 seconds  |

**Target:** 10,000+ loops per second for optimal file transfer performance.

**Why some clients work and others don't:**

- **lftp works**: More tolerant of slow transfers, larger TCP buffers
- **Ubuntu FTP fails**: Standard timeout settings, smaller buffers
- **FileZilla**: Depends on timeout configuration

**Additional optimizations for large files:**

1. **Increase FTP buffer size** (optional, but helps):

In `FtpServerKey.h`:
```cpp
#ifndef FTP_BUF_SIZE
    #define FTP_BUF_SIZE 2048  // Double the default (was 1024)
#endif
```

2. **Use faster SD card and SPI speed**:
```cpp
// ESP32 with SD library
SD.begin(SD_CS, 40000000);  // 40MHz

// ESP32 with SdFat2 (recommended)
sd.begin(SD_CS, SD_SCK_MHZ(50));  // 50MHz
```

3. **Monitor transfer with callback**:
```cpp
void transferCallback(FtpTransferOperation op, const char* name, unsigned int size) {
    if (op == FTP_DOWNLOAD) {
        Serial.print("Downloading: ");
        Serial.print(name);
        Serial.print(" - ");
        Serial.print(size);
        Serial.println(" bytes");
    }
}

void setup() {
    // ...
    ftpServer.setTransferCallback(transferCallback);
}
```

This helps identify if transfers are progressing or stalling.

## Best Practices

### 1. Loop Structure
```cpp
void loop() {
    ftpServer.handleFTP();  // FIRST
    
    // Other tasks with millis() timing
    // ...
    
    // NO delay(), NO blocking operations
}
```

### 2. Check Loop Execution Time

Add timing diagnostics to ensure your loop runs fast enough:

```cpp
void loop() {
    static unsigned long lastLoop = 0;
    unsigned long loopTime = millis() - lastLoop;
    
    if (loopTime > 100) {  // Warning if loop takes >100ms
        Serial.print("WARNING: Slow loop detected: ");
        Serial.print(loopTime);
        Serial.println(" ms");
    }
    lastLoop = millis();
    
    ftpServer.handleFTP();
    // ... other tasks
}
```

### 3. Optimize Screen Updates

Screen updates can be slow. Do them less frequently during FTP activity:

```cpp
void loop() {
    ftpServer.handleFTP();
    
    static unsigned long lastScreenUpdate = 0;
    // Update screen only every 500ms when FTP is active
    if (millis() - lastScreenUpdate > 500) {
        updateScreen();
        lastScreenUpdate = millis();
    }
}
```

### 4. SD Card Performance

Slow SD cards can cause delays:

```cpp
// Use faster SPI speed
SD.begin(CS_PIN, 40000000);  // 40MHz instead of default

// For SdFat2:
sd.begin(CS_PIN, SD_SCK_MHZ(50));  // 50MHz
```

### 5. Increase FTP Timeout (if needed)

In `FtpServerKey.h`, you can increase the timeout:

```cpp
// Default is 5 minutes (300 seconds)
#ifndef FTP_TIME_OUT
    #define FTP_TIME_OUT  (10 * 60)  // 10 minutes
#endif
```

**Note:** This is a workaround. The real fix is calling `handleFTP()` frequently.

## FileZilla Configuration

To help with stability issues, configure FileZilla:

1. **Edit → Settings → Connection → Timeout**: Increase to 60 seconds
2. **Edit → Settings → Transfers → Concurrent transfers**: Limit to 1
3. **Edit → Settings → FTP → Passive mode**: Always use passive
4. **Edit → Settings → Transfers → File Types**: Binary mode

## Testing Your Fix

After implementing the fixes, test with this code:

```cpp
void loop() {
    static unsigned long loopCount = 0;
    static unsigned long lastReport = 0;
    
    ftpServer.handleFTP();
    
    loopCount++;
    
    // Report loops per second
    if (millis() - lastReport > 1000) {
        Serial.print("Loops/sec: ");
        Serial.println(loopCount);
        loopCount = 0;
        lastReport = millis();
    }
}
```

**Expected result:** Should see 10,000+ loops per second on ESP32. If less than 1,000, you have blocking code somewhere.

## Common Mistakes to Avoid

| ❌ Wrong | ✅ Correct |
|---------|-----------|
| `delay(100);` | Use `millis()` timing |
| Call `handleFTP()` inside `if()` | Call `handleFTP()` every iteration |
| Long calculations in loop | Break into smaller chunks |
| `while()` loops without `handleFTP()` | Add `handleFTP()` inside `while()` |
| Blocking SD card operations | Use faster SD cards, optimize SPI speed |
| Screen updates every loop | Update screen at intervals (100-500ms) |

## Complete Working Example

```cpp
#include <WiFi.h>
#include <SD.h>
#include <SimpleFTPServer.h>

const char* ssid = "your-ssid";
const char* password = "your-password";
const int SD_CS = 5;

FtpServer ftpServer;

// Timing variables
unsigned long lastKeyCheck = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastLoopReport = 0;
unsigned long loopCount = 0;

void setup() {
    Serial.begin(115200);
    
    // Connect WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    
    // Initialize SD
    if (!SD.begin(SD_CS)) {
        Serial.println("SD initialization failed!");
        return;
    }
    Serial.println("SD initialized");
    
    // Start FTP
    ftpServer.begin("user", "password");
    Serial.println("FTP server started");
}

void loop() {
    // ALWAYS call handleFTP() first and every iteration
    ftpServer.handleFTP();
    
    loopCount++;
    
    // Check keys every 50ms
    if (millis() - lastKeyCheck >= 50) {
        lastKeyCheck = millis();
        checkKeys();
    }
    
    // Update screen every 200ms
    if (millis() - lastScreenUpdate >= 200) {
        lastScreenUpdate = millis();
        updateScreen();
    }
    
    // Report loop performance every second
    if (millis() - lastLoopReport >= 1000) {
        Serial.print("Loops/sec: ");
        Serial.println(loopCount);
        loopCount = 0;
        lastLoopReport = millis();
    }
}

void checkKeys() {
    // Your key checking code here
    // Keep it fast!
}

void updateScreen() {
    // Your screen update code here
    // Keep it fast!
}
```

## Related Issues

- [Issue #80](https://github.com/xreef/SimpleFTPServer/issues/80) - Unstable working, timeouts
- [Issue #79](https://github.com/xreef/SimpleFTPServer/issues/79) - Dynamic SD mounting

## Summary

**The key to stable FTP operation:**
1. Call `handleFTP()` **every loop iteration**
2. Never use `delay()`
3. Keep loop execution time < 10ms
4. Use `millis()` for timing other tasks
5. Minimize blocking operations

Follow these guidelines and timeout issues will be eliminated.
