/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form https://github.com/nailbuster/esp8266FTPServer
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */
#ifndef FtpServerKey_h
#define FtpServerKey_h

// Uncomment to enable printing out nice debug messages.
#define FTP_SERVER_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#define STORAGE_SD (0)
#define STORAGE_SPIFFS (1)
#define STORAGE_LITTLEFS (2)

#define NETWORK_ESP8266_ASYNC (0)
#define NETWORK_ESP8266 (1)
#define NETWORK_ESP8266_242 (6)
#define NETWORK_W5100 (2)
#define NETWORK_ENC28J60 (3)
#define NETWORK_ESP32 (4)
#define NETWORK_ESP32_ETH (5)
#define NETWORK_WiFiNINA (7)

#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 	NETWORK_ESP8266
	#define DEFAULT_STORAGE_TYPE_ESP8266 STORAGE_SPIFFS
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 	NETWORK_ESP32
	#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SPIFFS
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO 	NETWORK_W5100
	#define DEFAULT_STORAGE_TYPE_ARDUINO STORAGE_SD
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO_SAMD
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD 	NETWORK_WiFiNINA
	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SD
#endif

//#define SD_CS_PIN 4

#endif
