# SimpleFTPServer Library

![SimpleFTPServer Logo](resources/SimpleFTPServerLogo.png)

[![arduino-library-badge](https://www.ardu-badge.com/badge/SimpleFTPServer.svg?)](https://www.ardu-badge.com/SimpleFTPServer)
[![](https://img.shields.io/badge/Platform-Arduino%20%7C%20ArduinoSAMD%20%7C%20ESP32%20%7C%20ESP8266%20%7C%20RP2040%20%7C%20STM32-green.svg)]()
[![](https://img.shields.io/badge/License-MIT-lightgrey.svg)](LICENSE.md)

A lightweight Arduino/embedded library to expose a device filesystem over FTP, allowing uploads, downloads and remote file management from standard FTP clients (FileZilla, WinSCP, Explorer, etc.). This version supports a single FTP session.

**Author:** Renzo Mischianti
**Website:** [www.mischianti.org](https://www.mischianti.org/category/my-libraries/simple-ftp-server/)
**GitHub:** [xreef/SimpleFTPServer](https://github.com/xreef/SimpleFTPServer)

---

## 📚 Documentation & Tutorials
Complete tutorials and platform-specific guides by the author are available on mischianti.org.

### General Documentation
- 🌐 **[Simple FTP Server Library Home](https://www.mischianti.org/category/my-libraries/simple-ftp-server/)** - Main library page with articles and examples
- 📖 **[FTP server on ESP8266 and ESP32](https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32)** - Getting started guide
- 📖 **[Simple FTP server library now with support for Wio Terminal and SD](https://www.mischianti.org/2021/07/01/simple-ftp-server-library-now-with-support-for-wio-terminal-and-sd/)**

---

## 📋 Table of Contents
- [Features](#-features)
- [Supported Platforms](#-supported-platforms)
- [Supported Network Interfaces](#-supported-network-interfaces)
- [Supported Storage Systems](#-supported-storage-systems)
- [Installation](#-installation)
- [Basic Usage](#-basic-usage)
- [Configuration](#-configuration)
- [Included Examples](#-included-examples)
- [Changelog](#-changelog)
- [License](#-license)
- [Contributing](#-contributing)
- [Support & Contact](#-support--contact)

## ✨ Features

- Expose device filesystem via a single FTP session (upload, download, delete, rename, list)
- Resume support (REST) and RFC-like commands (ALLO, STAT, SYST, HELP)
- UTF-8 filename support (configurable)
- Configurable buffers and timeouts via `FtpServerKey.h`
- Works with internal flash filesystems (SPIFFS, LittleFS, FFAT) and external SD (SD, SdFat)
- Small footprint and simple API: `begin()` and `handleFTP()`

## 🎯 Supported Platforms

| Platform | Filesystems | Notes |
|---|---|---|
| ESP32 | SPIFFS, LittleFS, FFAT, SD, SD_MMC | Full support, see examples |
| ESP8266 | SPIFFS, LittleFS, SD | Limited RAM vs ESP32 |
| RP2040 / Pico W | LittleFS, SD | See Pico W examples |
| STM32 | SdFat, SPI flash | Use SdFat examples |
| Arduino (AVR) | SD (8.3), SdFat | Use SdFat for better compatibility |
| Wio Terminal | SdFat, Seed SD, native FAT | See Wio examples |

> ⚠️ Note: Some older AVR boards (Uno/Mega) have limited RAM and may not support large transfers.

## 🌐 Supported Network Interfaces

- WiFi (ESP32, ESP8266, RP2040-W, WiFiNINA)
- Ethernet (W5x00, W5500, ENC28J60, ESP32-ETH)
- Seeed RTL8720DN

## 💾 Supported Storage Systems

### Internal flash
- SPIFFS (ESP8266/ESP32)
- LittleFS (ESP8266/ESP32/RP2040)
- FFAT (ESP32)

### External storage
- SD (Arduino SD library)
- SdFat / SdFat2 (recommended)
- SD_MMC (ESP32)
- Adafruit_SPIFlash

## 📦 Installation

### Arduino IDE (Library Manager)
1. Open Arduino IDE
2. Sketch > Include Library > Manage Libraries
3. Search for "SimpleFTPServer" and install

### Manual installation
1. Copy the repository folder into your Arduino `libraries/` folder
2. Restart Arduino IDE

### PlatformIO
Add to `platformio.ini`:
```ini
lib_deps = xreef/SimpleFTPServer
```

## 🚀 Basic Usage

### Quick start (ESP32 example)

```cpp
#include <WiFi.h>
#include "SPIFFS.h"
#include <SimpleFTPServer.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";

FtpServer ftpSrv;

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  // Mount filesystem before starting FTP
  if (SPIFFS.begin(true)) {
      Serial.println("SPIFFS opened!");
      // username, password for ftp.
      ftpSrv.begin("esp32","esp32");
  }
}

void loop(){
  ftpSrv.handleFTP();
}
```

## ⚙️ Configuration

See `FtpServerKey.h` for config defines and defaults. Key settings:
- `FTP_BUF_SIZE` — transfer buffer size
- `FTP_TIME_OUT` — inactivity timeout (seconds)
- `FTP_AUTH_TIME_OUT` — authentication timeout (seconds)
- `DEBUG_PRINTER` — debug output stream (defaults to `Serial`)
- `UTF8_SUPPORT` — enable/disable UTF-8 support

## 🧩 Included Examples

See the `examples/` folder for ready-to-use sketches for many platforms (ESP32, ESP8266, RP2040, STM32, Wio Terminal, Arduino). Adapt SSID, credentials and SD pins as needed.

## 📝 Changelog
- 2025-10-13 3.0.0 Fix: 
  - updated SdFat2 initialization for Arduino Mega; integrated W5500 initialization (example Arduino_Mega_W5500_SdFat2)
  - Fix READ WRITE APPEND problem
  - Fix ram usage
  - Set correct buffer size (Dynamic and Static)
  - Add FTP callbacks for connection events and file transfers; update data types for free space and transferred size
  - Improve file-open handling (ensure reads start at file beginning), increase passive data connection wait time, add robust FFat dir create/remove (fallback to /ffat and POSIX), add configurable minimum free-space check before uploads
  - Refactor FTP callback functions to use uint32_t for space parameters, improve serial output, and register callbacks for FTP events
- 2025-02-11 2.1.11 Management of relative and absolute path in command prompt (./ ../ /)
- 2025-01-28 2.1.11 Fix REST and add ALLO, and STAT commands
- 2025-01-13 2.1.11 Add addictional LOG to prevent multiple user issues
- 2025-01-12 2.1.11 Add REST (resume) command
- 2025-01-11 2.1.10 Multiple minor fix and optimization
- 2024-09-16 2.1.9 Fix LittleFS filename size.
- 2024-07-16 2.1.8 Add SYST command.
- 2023-10-07 2.1.7 Fix ESP32 rename on SD (thanks @scuba-hacker) #44
- 2023-02-02 2.1.6 Fix esp8266 Ethernet (w5x00) issue and explain solution for ESP32 Ethernet (w5x00), add new Networks management
- 2023-01-13 2.1.5 Fix SPIFM external SPI Flash date management (add SPIFM esp32 example)
- 2022-09-21 2.1.4 Add support for Raspberry Pi Pico W and rp2040 boards, Fix SD card config
- 2022-09-20 2.1.3 Soft AP IP management, more disconnect event and SD_MCC
- 2022-05-21 2.1.2 Fix SD path (#19)
- 2022-05-21 2.1.1 Minor fix
- 2022-03-30 2.1.0 Add UTF8 support and enabled It by default (Thanks to @plaber)
- 2022-03-30 2.0.0 Complete support for STM32 with SD and SPI Flash minor bux fix and HELP command support
- 2022-03-17 1.3.0 Fix enc28j60 and w5500 support and restructuring for local settings
- 2022-02-25 1.2.1 Fix anonymous user begin and fix SPIFFS wrong display
- 2022-02-22 1.2.0 Add anonymous user and implement correct RFC (#9 now work correctly with File Explorer)
- 2022-02-01 1.1.1 Add workaround to start FTP server before connection, add end and setLocalIP method.

## 📄 License
MIT License — see `LICENSE.md` for details.

Copyright (c) 2017-2025 Renzo Mischianti

## 🤝 Contributing
Contributions are welcome! Fork the repo, create a feature branch, commit and submit a Pull Request. Please ensure examples build on target platforms before opening a PR.

## 📞 Support & Contact
- Website: https://www.mischianti.org/category/my-libraries/simple-ftp-server/
- GitHub Issues: https://github.com/xreef/SimpleFTPServer/issues
- Forum / Articles: https://www.mischianti.org/

## 👤 Author
**Renzo Mischianti**
Website: https://www.mischianti.org
Email: renzo.mischianti@gmail.com
GitHub: [@xreef](https://github.com/xreef)

---

⭐ If this library helped your project, please star the repository on GitHub!
