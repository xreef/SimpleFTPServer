# Changelog

Tutte le modifiche importanti al progetto `SimpleFTPServer` vengono documentate in questo file.

## [3.0.3] - 2026-10-17
### Correzioni di bug, conformità RFC e miglioramenti prestazionali (con `+0 byte` di overhead SRAM su AVR):

- **Supporto RFC 2428 `EPSV` (Extended Passive Mode)** ([#99](https://github.com/xreef/SimpleFTPServer/pull/99)):
  - Aggiunto il comando `EPSV` in `processCommand()` e l'annuncio `EPSV` nella risposta `FEAT`.
  - Aggiunta la guardia `if (dataServer.status() == CLOSED)` per `NETWORK_WiFiNINA` su `PASV`/`EPSV` per evitare l'esaurimento dei socket hardware sul modulo NINA-W102.
- **Gestione scritture TCP parziali in `doRetrieve()`** ([#100](https://github.com/xreef/SimpleFTPServer/pull/100)):
  - Riposizionamento del puntatore file (`file.seek(bytesTransfered + written)`) quando `data.write(buf, nb)` invia meno byte di quelli letti dallo storage in condizioni di backpressure TCP.
- **Fix callback `FTP_TRANSFER_STOP` e protezione overflow in `readChar()`** ([#98](https://github.com/xreef/SimpleFTPServer/pull/98)):
  - Catturato il nome del file prima di `file.close()` in `closeTransfer()` affinché la callback `FTP_TRANSFER_STOP` riceva sempre il nome corretto del file (su ESP32 VFS `file.name()` diventa vuoto dopo `close()`).
  - Corretto il controllo dei limiti in `readChar()` (`iCL < FTP_CMD_SIZE - 1`) per impedire che il terminatore null `'\0'` sovrascriva il primo byte di `cwdName[FTP_CWD_SIZE]`.
- **Supporto apostrofi nei nomi di file** ([#96](https://github.com/xreef/SimpleFTPServer/issues/96)):
  - Corretto `readChar()` sostituendo `if (c == '\'')` con `if (c == '\\')`, normalizzando solo i backslash Windows in `/` e preservando gli apostrofi nei nomi di file e directory.
- **Rimozione warning `WiFiClient::flush()` deprecato su ESP32 Core 3.3.7+** ([#94](https://github.com/xreef/SimpleFTPServer/issues/94)):
  - Rimossi i `data.flush()` ridondanti dopo `data.stop()` in `STOR`/`APPE` e protetto `data.flush()` con `#if defined(ESP8266)` in `doRetrieve()` e `closeTransfer()`.
- **Fix `isDir()` e comando `DELE` con client FTP avanzati (SpeedCommander, FileZilla, WinSCP)** ([#92](https://github.com/xreef/SimpleFTPServer/issues/92)):
  - Corretto `FtpServer::isDir()` su tutti i backend (`STORAGE_SD`, `STORAGE_LITTLEFS`, `STORAGE_FFAT`, `STORAGE_SD_MMC`, `STORAGE_SDFAT1`/`2`) affinché verifichi `isDirectory()`/`isDir()` e chiuda sempre l'handle temporaneo senza lasciare aperto `this->file`.
  - Anticipato l'invio di `250 Deleted <file>` prima della callback `FTP_FREE_SPACE_CHANGE` nel comando `DELE`.
- **Fix comando `APPE` (Append) su ESP32 `STORAGE_SD` e `STORAGE_SD_MMC`** ([#90](https://github.com/xreef/SimpleFTPServer/issues/90)):
  - Preservata la modalità `"a"` (`FILE_APPEND`) in `FtpServer::openFile(char path[], const char* readType)` invece di sovrascriverla con `"w"` (`FILE_WRITE`).
- **Fix `getFileModTime()` e conformità RFC 3659 per `MLSD` / `MLST`**:
  - Corretto `getFileModTime(uint16_t*, uint16_t*)` su `STORAGE_SD` e ESP32 VFS per evitare chiamate su `dir` chiuso e garantire il funzionamento di `MLST`.
  - Allineato l'output di `doMlsd()` al formato standard RFC 3659 (`Type=file|dir;Modify=...;Size=...; <name>`) anche per `STORAGE_FFAT`, `STORAGE_LITTLEFS` e `STORAGE_SEEED_SD`.
- **Eliminazione dei blocchi (15–70s) su `SdFat` e velocizzazione `FTP_CONNECT` / `FTP_DISCONNECT` / `QUIT`** ([#93](https://github.com/xreef/SimpleFTPServer/issues/93)):
  - Passato `0, 0` alla callback su `FTP_CONNECT` e `FTP_DISCONNECT` senza interrogare il filesystem.
  - Su `STORAGE_SDFAT1` e `STORAGE_SDFAT2`, evitata la chiamata bloccante a `freeClusterCount()` quando `MAINTAIN_FREE_CLUSTER_COUNT` è disabilitato (`0` di default in `SdFat`).
  - In `disconnectClient()` e `QUIT`, chiusi `client` e `data` prima della callback ed eliminato il doppio invio di `221 Goodbye`.
- **Supporto completo `Arduino Nano 33 IoT` (`SAMD21 + WiFiNINA`) ed esempi aggiornati** ([#90](https://github.com/xreef/SimpleFTPServer/issues/90), [#91](https://github.com/xreef/SimpleFTPServer/issues/91)):
  - Separati i blocchi `#ifndef DEFAULT_STORAGE_TYPE_*` e `#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_*` in `FtpServerKey.h` (e corretto `DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD`).
  - Aggiunti e aggiornati gli esempi `Arduino_WiFiNINA_SD`, `ESP32_SD_SdFat2`, `STM32_SPIFlash_enc28j60`, `STM32_SdFat_enc28j60`, `STM32_SdFat_w5500` e tutti gli sketch di esempio della libreria.

---

## [3.0.2] - 2025-11-28
- Fix `#elif defined` per ESP32 ed ESP8266 in `FtpServer.cpp` ([#88](https://github.com/xreef/SimpleFTPServer/issues/88)).
- Fix su `STORAGE_SPIFM` `getName`.

## [3.0.1] - 2025-11-22
- Aggiunto l'esempio `ESP32_SD_WiFi` (`SPI.begin()` + `SD.begin(CS)`).
- Migliorati i commenti e le callback negli esempi.

## [3.0.0] - 2025-10-13
- Aggiornata inizializzazione `SdFat2` per Arduino Mega e `W5500` (`Arduino_Mega_W5500_SdFat2`).
- Fix `READ` / `WRITE` / `APPEND`, ottimizzazione RAM e buffer statico/dinamico.
- Aggiunte callback FTP (`FTP_CONNECT`, `FTP_DISCONNECT`, `FTP_FREE_SPACE_CHANGE`, `FTP_TRANSFER_START`, `FTP_TRANSFER_STOP`, `FTP_TRANSFER_ERROR`).

