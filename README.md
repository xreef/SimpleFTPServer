# SimpleFTPServer Library 3.0.4

![SimpleFTPServer Logo](resources/SimpleFTPServerLogo.png)

[![arduino-library-badge](https://www.ardu-badge.com/badge/SimpleFTPServer.svg?)](https://www.ardu-badge.com/SimpleFTPServer)
[![](https://img.shields.io/badge/Platform-Arduino%20%7C%20ArduinoSAMD%20%7C%20UNO%20R4%20WiFi%20%7C%20ESP32%20%7C%20ESP8266%20%7C%20RP2040%20%7C%20STM32-green.svg)]()
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
- Passive (`PASV`, `EPSV`) and active (`PORT`) data connections
- A second client (e.g. a FileZilla transfer connection) is accepted when the current session is idle (ESP32/ESP8266 WiFi)
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
| Arduino (AVR) | SD (8.3), SdFat | Mega 2560 + W5100/W5500 shield; use SdFat for better compatibility |
| Arduino SAMD (Nano 33 IoT, MKR WiFi 1010) | SD, SdFat | WiFiNINA (firmware >= 1.5.0), see `Arduino_WiFiNINA_SD` |
| Arduino UNO R4 WiFi | SD (8.3) | WiFiS3 + SPI microSD module (CS pin in `SD.begin()`) |
| Wio Terminal | SdFat, Seed SD, native FAT | See Wio examples |

> ⚠️ Note: Some older AVR boards (Uno/Mega) have limited RAM and may not support large transfers.

## 🌐 Supported Network Interfaces

- WiFi (ESP32, ESP8266, RP2040-W, WiFiNINA, WiFiS3 on Arduino UNO R4 WiFi)
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
- `FTP_DATA_CONNECT_TIMEOUT_MS` — max wait for the data connection, passive or active (default `5000`); `handleFTP()` blocks meanwhile, so keep it well below the client timeout (FileZilla: 20 s)
- `FTP_TRANSFER_STALL_TIMEOUT_MS` — an upload with no data for this time is aborted with `426` (default `10000`)
- `FTP_DATA_CLOSE_TIMEOUT_MS` — `NETWORK_W5100` only: max wait for the client to close the data connection at the end of a transfer before the Ethernet library resets it (default `5000`; with the library default of 1 s a slow client lost the end of the file)
- `FTP_IDLE_PREEMPT_MS` — ESP32/ESP8266 WiFi only: when another client is waiting and the logged-in session has been idle for this time, the idle session is closed and the new client accepted (default `1500`, `0` disables it)
- `FTP_SERVER_DEBUG` — enable debug messages (e.g. `-D FTP_SERVER_DEBUG` in `build_flags`)
- `DEBUG_PRINTER` — debug output stream (defaults to `Serial`)
- `UTF8_SUPPORT` — enable/disable UTF-8 support

### Passive and active mode

Prefer **passive mode** (the FileZilla default). In active mode (`PORT`) the board opens the data connection towards the PC, so the PC firewall must accept incoming connections for the FTP client on the network profile in use: on Windows a rule that allows FileZilla only on *Public* networks blocks it on a *Private* one, and every listing or transfer fails with `425 No data connection` after a few seconds.

With WiFiNINA (Arduino Nano 33 IoT, MKR WiFi 1010) the NINA module can close an active-mode data connection while the board is sending to a slower receiver: the server answers `426` and the transfer has to be repeated. Passive mode is not affected.

## 🧩 Included Examples

See the `examples/` folder for ready-to-use sketches for many platforms (ESP32, ESP8266, RP2040, STM32, Wio Terminal, Arduino). Adapt SSID, credentials and SD pins as needed.

## 📝 Changelog
- 2026-10-23 3.0.4
  - Add `FTP_DATA_CONNECT_TIMEOUT_MS`, `FTP_TRANSFER_STALL_TIMEOUT_MS` and `FTP_IDLE_PREEMPT_MS` in `FtpServerKey.h`: waiting for the passive/active data connection no longer blocks `handleFTP()` beyond the FileZilla timeout (it was up to 60 s)
  - Non-blocking `doStore()`: a pause during an upload no longer ends it with `226` and a truncated file; an upload with no data for `FTP_TRANSFER_STALL_TIMEOUT_MS` is aborted with `426`
  - Idle-session preemption (ESP32/ESP8266 WiFi): a waiting client (e.g. a FileZilla transfer connection) is accepted when the logged-in session is idle, instead of waiting up to `FTP_TIME_OUT`
  - `readChar()` reads the whole available command line in one call
  - A failed `STOR` (`425`, e.g. active mode blocked by the PC firewall) no longer deletes or truncates the existing file
  - An interrupted download is reported with `426` instead of `226` in `doRetrieve()` (e.g. WiFiNINA in active mode)
  - A complete download is no longer reported with `426` when the client closes the data connection right after the last byte (e.g. curl)
  - `NETWORK_W5100`: downloads to slow clients no longer lose the end of the file (`EthernetClient::stop()` reset the connection after 1 s); new `FTP_DATA_CLOSE_TIMEOUT_MS`
  - Fix ESP32/ESP8266 + Ethernet (`W5100`, `ENC28J60`) build: `setNoDelay()` only with the ESP `WiFiClient`
  - Add `NETWORK_WiFiS3` (Arduino UNO R4 WiFi)
- 2026-10-17 3.0.3
  - Add RFC 2428 `EPSV` (Extended Passive Mode) command and `FEAT` announcement with `WiFiNINA` socket reuse guard (#99)
  - Fix partial TCP write rewind in `doRetrieve()` when `data.write()` writes fewer bytes than read from storage under TCP backpressure (#100)
  - Fix `FTP_TRANSFER_STOP` callback receiving an empty filename in `closeTransfer()` and prevent `readChar()` null-terminator overflow into `cwdName` (#98)
  - Fix `readChar()` replacing single quotes (`'`) with `/` in filenames instead of Windows backslashes (`\`) (#96)
  - Fix `WiFiClient::flush()` deprecation warnings on ESP32 Arduino Core 3.3.7+ by removing redundant calls after `stop()` and guarding with `#if defined(ESP8266)` (#94)
  - Fix `isDir()` leaking `this->file` handle and misidentifying regular files as directories on `STORAGE_SD`, `STORAGE_LITTLEFS`, `STORAGE_FFAT`, and `STORAGE_SD_MMC`; send `250 Deleted` before `FTP_FREE_SPACE_CHANGE` callback in `DELE` (#92)
  - Fix `APPE` (append) mode on ESP32 `STORAGE_SD` and `STORAGE_SD_MMC` in `openFile()` (`"a"` was overwritten with `"w"`) (#90)
  - Fix `getFileModTime()` on `STORAGE_SD` and ESP32 VFS (`MLST` command support across SD backends)
  - Fix RFC 3659 `MLSD` output format on `STORAGE_FFAT`, `STORAGE_LITTLEFS`, and `STORAGE_SEEED_SD`
  - Fix `WiFiNINA` (`NETWORK_WiFiNINA`) server socket exhaustion and `accept()`/`available()` handling on SAMD21 (Arduino Nano 33 IoT); add `Arduino_WiFiNINA_SD` and `ESP32_SD_SdFat2` examples
  - Avoid blocking `free()` FAT cluster scans on `FTP_CONNECT`/`FTP_DISCONNECT` and on `SdFat` when `MAINTAIN_FREE_CLUSTER_COUNT` is disabled (#93)
  - Decouple `#ifndef DEFAULT_STORAGE_TYPE_*` and `#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_*` guards in `FtpServerKey.h` and update all library examples (#90, #91)
- 2025-11-28 3.0.2 
  - Fix #elif define (#88)
  - Fix on SPIFM getName
- 2025-11-22 3.0.1 Fix / Example:
  - Add example `ESP32_SD_WiFi` demonstrating SD (SPI) + WiFi usage with SimpleFTPServer.
  - Update ESP32 project example to initialize SD via SPI (SPI.begin() + SD.begin(CS)) and start the FTP server using the SD storage.
  - Minor linter-friendly renames for example callbacks and improved example comments.

- 2025-10-13 3.0.0 Fix: 
  - updated SdFat2 initialization for Arduino Mega; integrated W5500 initialization (example Arduino_Mega_W5500_SdFat2)
  - Fix READ WRITE APPEND problem
  - Fix ram usage
  - Set correct buffer size (Dynamic and Static)
  - Add FTP callbacks for connection events and file transfers; update data types for free space and transferred size
  - Improve file-open handling (ensure reads start at file beginning), increase passive data connection wait time, add robust FFat dir create/remove (fallback to /ffat and POSIX), add configurable minimum free-space check before uploads
  - Refactor FTP callback functions to use uint32_t for space parameters, improve serial output, and register callbacks for FTP events
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

Copyright (c) 2017-2026 Renzo Mischianti

## 🤝 Contributing
Contributions are welcome! Fork the repo, create a feature branch, commit and submit a Pull Request. Please ensure examples build on target platforms before opening a PR.

## 📞 Support & Contact
- Website: https://www.mischianti.org/category/my-libraries/simple-ftp-server/
- GitHub Issues: https://github.com/xreef/SimpleFTPServer/issues
- Forum: https://mischianti.org/forums/forum/mischiantis-libraries/simple-ftp-server/
- Articles: https://mischianti.org/category/my-libraries/simple-ftp-server/
- 
## 👤 Author
**Renzo Mischianti**
Website: https://www.mischianti.org
Email: renzo.mischianti@gmail.com
GitHub: [@xreef](https://github.com/xreef)

---

⭐ If this library helped your project, please star the repository on GitHub!
