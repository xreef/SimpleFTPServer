# SimpleFTPServer compatibility with SdFat2 on PlatformIO

This guide helps you solve compilation and usage issues with the SimpleFTPServer library and SdFat2 on PlatformIO, especially for ESP8266 and ESP32.

## Common problem

When selecting `STORAGE_SDFAT2` on PlatformIO, you may get an error like:

```
src\FTPServer.cpp:2323:50: error: no match for 'operator=' (operand types are 'FsFile' and 'File32')
file = STORAGE_MANAGER.open( path, readTypeInt );
```

On Arduino IDE, instead, compilation works fine.

## Causes
- **SdFat library version**: PlatformIO may install a different version than Arduino IDE.
- **File type**: SdFat2 uses `FsFile` (or `File32` in some versions) instead of `File`.
- **Configuration macros**: The file type used by the library must match the installed SdFat2 version.

## Step-by-step solution

### 1. Force the correct SdFat version in PlatformIO
In your `platformio.ini` file add:

```
libraries =
    greiman/SdFat@2.2.3
```

Replace the version with the one that works for you in Arduino IDE, if different.

### 2. Correctly select the storage type
Make sure the `STORAGE_TYPE` macro is set to `STORAGE_SDFAT2`.
You can do this by adding to your PlatformIO configuration:

```
build_flags = -DSTORAGE_TYPE=STORAGE_SDFAT2
```

### 3. Code verification
In `FtpServer.h` the following section correctly handles SdFat2:

```cpp
#elif (STORAGE_TYPE == STORAGE_SDFAT2)
    #include <SdFat.h>
    #include <sdios.h>
    #define STORAGE_MANAGER sd
    #define FTP_FILE FsFile
    #define FTP_DIR FsFile
    extern SdFat STORAGE_MANAGER;
```

Make sure `FTP_FILE` is used everywhere in the code to declare file variables.

### 4. Clean PlatformIO cache
If you changed library version or macros, run:

```
pio run --target clean
```

### 5. Rebuild the project

## Additional notes
- If you use a SdFat version that uses `File32` instead of `FsFile`, change the `FTP_FILE` macro accordingly.
- See the [official SdFat documentation](https://github.com/greiman/SdFat) for details on versions and APIs.

## Example PlatformIO configuration

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    greiman/SdFat@2.2.3
build_flags = -DSTORAGE_TYPE=STORAGE_SDFAT2
```

## References
- [Issue #76 on GitHub](https://github.com/xreef/SimpleFTPServer/issues/76)
- [SimpleFTPServer repository](https://github.com/xreef/SimpleFTPServer)
- [SdFat repository](https://github.com/greiman/SdFat)
