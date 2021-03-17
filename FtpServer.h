/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form https://github.com/nailbuster/esp8266FTPServer
 *
 * AUTHOR:  Renzo Mischianti
 * VERSION: 0.1.0
 *
 * https://www.mischianti.org/
 *
 */

#include "FtpServerKey.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if(NETWORK_ESP8266_242 == DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266)
	#define ARDUINO_ESP8266_RELEASE_2_4_2
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 NETWORK_ESP8266
#endif
//
//#if(NETWORK_ESP8266_SD == DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266)
//	#define ESP8266_GT_2_4_2_SD_STORAGE_SELECTED
//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 NETWORK_ESP8266
//#endif

#if !defined(FTP_SERVER_NETWORK_TYPE)
// select Network type based
	#if defined(ESP8266) || defined(ESP31B)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ESP8266
	#elif defined(ESP32)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ESP32
	#elif defined(ARDUINO_ARCH_SAMD)
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_SAMD
	#else
		#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
		#define STORAGE_TYPE DEFAULT_STORAGE_TYPE_ARDUINO
	//	#define STORAGE_SD_ENABLED
	#endif
#endif

#if defined(ESP8266) || defined(ESP31B)
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
	#ifndef STORAGE_SPIFFS_FORCE_DISABLE
		#define STORAGE_SPIFFS_ENABLED
	#endif
#elif defined(ESP32)
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
	#ifndef STORAGE_SPIFFS_FORCE_DISABLE
		#define STORAGE_SPIFFS_ENABLED
	#endif
#else
	#ifndef STORAGE_SD_FORCE_DISABLE
		#define STORAGE_SD_ENABLED
	#endif
#endif


// Includes and defined based on Network Type
#if(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266_ASYNC)

// Note:
//   No SSL/WSS support for client in Async mode
//   TLS lib need a sync interface!

#if defined(ESP8266)
#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#elif defined(ESP32)
#include <WiFi.h>
//#include <WiFiClientSecure.h>

#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#elif defined(ESP31B)
#include <ESP31BWiFi.h>
#else
#error "network type ESP8266 ASYNC only possible on the ESP mcu!"
#endif
//
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncTCPbuffer.h>
//#define FTP_CLIENT_NETWORK_CLASS AsyncTCPbuffer
//#define FTP_SERVER_NETWORK_SERVER_CLASS AsyncServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266 || FTP_SERVER_NETWORK_TYPE == NETWORK_ESP8266_242)

#if !defined(ESP8266) && !defined(ESP31B)
#error "network type ESP8266 only possible on the ESP mcu!"
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <ESP31BWiFi.h>
#endif
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_W5100)

#ifdef STM32_DEVICE
#define FTP_CLIENT_NETWORK_CLASS TCPClient
#define FTP_SERVER_NETWORK_SERVER_CLASS TCPServer
#else
#include <Ethernet.h>
#include <SPI.h>
#define FTP_CLIENT_NETWORK_CLASS EthernetClient
#define FTP_SERVER_NETWORK_SERVER_CLASS EthernetServer
#endif

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ENC28J60)

#include <UIPEthernet.h>

#define FTP_CLIENT_NETWORK_CLASS UIPClient
#define FTP_SERVER_NETWORK_SERVER_CLASS UIPServer

//#include <UIPEthernet.h>
//UIPClient base_client;
//SSLClient client(base_client, TAs, (size_t)TAs_NUM, A5);
//
//#define FTP_CLIENT_NETWORK_CLASS SSLClient
//#define FTP_SERVER_NETWORK_SERVER_CLASS UIPServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP32)

#include <WiFi.h>
//#include <WiFiClientSecure.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiClientSecure
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_ESP32_ETH)

#include <ETH.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#elif(FTP_SERVER_NETWORK_TYPE == NETWORK_WiFiNINA)

#include <WiFiNINA.h>
#define FTP_CLIENT_NETWORK_CLASS WiFiClient
//#define FTP_CLIENT_NETWORK_SSL_CLASS WiFiSSLClient
#define FTP_SERVER_NETWORK_SERVER_CLASS WiFiServer

#else
#error "no network type selected!"
#endif

#if(STORAGE_TYPE == STORAGE_SPIFFS)
	#ifdef STORAGE_SPIFFS_ENABLED
		#if defined(ESP32)
//			#define FS_NO_GLOBALS
			#include <SPIFFS.h>
		#else
			#ifdef ARDUINO_ESP8266_RELEASE_2_4_2
				#define FS_NO_GLOBALS
			#endif
			#include "FS.h"
		#endif
		#define STORAGE_MANAGER SPIFFS
	#endif
#elif(STORAGE_TYPE == STORAGE_LITTLEFS)
	#include "LittleFS.h"
	#define STORAGE_MANAGER LittleFS
#elif(STORAGE_TYPE == STORAGE_SD)
	#ifdef STORAGE_SD_ENABLED
		#include <SPI.h>
		#include <SD.h>
	#endif
	#define STORAGE_MANAGER SD
#endif

//#ifdef FTP_CLIENT_NETWORK_SSL_CLASS
//#define FTP_CLIENT_NETWORK_CLASS FTP_CLIENT_NETWORK_SSL_CLASS
//#endif

#define OPEN_CLOSE_SPIFFS
#define OPEN_CLOSE_SD

// Setup debug printing macros.
#ifdef FTP_SERVER_DEBUG
	#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
	#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
	#define DEBUG_PRINT(...) {}
	#define DEBUG_PRINTLN(...) {}
#endif

#ifndef FTP_SERVER_H
#define FTP_SERVER_H

////#include "Streaming.h"
//#include <FS.h>
//#include <WiFiClient.h>

#define FTP_SERVER_VERSION "FTP-2017-10-18"

#define FTP_CTRL_PORT    21          // Command port on wich server is listening  
#define FTP_DATA_PORT_PASV 50009     // Data port in passive mode

#define FTP_TIME_OUT  5           // Disconnect client after 5 minutes of inactivity
#define FTP_CMD_SIZE 255 + 8 // max size of a command
#define FTP_CWD_SIZE 255 + 8 // max size of a directory name
#define FTP_FIL_SIZE 255     // max size of a file name
//#define FTP_BUF_SIZE 1024 //512   // size of file buffer for read/write
#define FTP_BUF_SIZE 2*1460 //512   // size of file buffer for read/write

#ifndef FILE_READ
	#define FILE_READ "r"
#endif
#ifndef FILE_WRITE
	#define FILE_WRITE "w"
#endif


class FtpServer
{
public:
  void    begin(String uname, String pword, boolean _bUnixLst = false);
  void    handleFTP();

private:
  void    iniVariables();
  void    clientConnected();
  void    disconnectClient();
  boolean userIdentity();
  boolean userPassword();
  boolean processCommand();
  boolean dataConnect();
  boolean doRetrieve();
  boolean doStore();
  void    closeTransfer();
  void    abortTransfer();
  boolean makePath( char * fullname );
  boolean makePath( char * fullName, char * param );
  uint8_t getDateTime( uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                       uint8_t * phour, uint8_t * pminute, uint8_t * second );
  char *  makeDateTimeStr( char * tstr, uint16_t date, uint16_t time );
  int8_t  readChar();

  IPAddress      dataIp;              // IP address of client for data
  FTP_CLIENT_NETWORK_CLASS client;
  FTP_CLIENT_NETWORK_CLASS data;
  
  File file;
  
  boolean  dataPassiveConn;
  uint16_t dataPort;
  char     buf[ FTP_BUF_SIZE ];       // data buffer for transfers
  char     cmdLine[ FTP_CMD_SIZE ];   // where to store incoming char from client
  char     cwdName[ FTP_CWD_SIZE ];   // name of current directory
  char     command[ 5 ];              // command sent by client
  boolean  rnfrCmd;                   // previous command was RNFR
  char *   parameters;                // point to begin of parameters sent by client
  uint16_t iCL;                       // pointer to cmdLine next incoming char
  int8_t   cmdStatus,                 // status of ftp command connexion
           transferStatus;            // status of ftp data transfer
  uint32_t millisTimeOut,             // disconnect after 5 min of inactivity
           millisDelay,
           millisEndConnection,       // 
           millisBeginTrans,          // store time of beginning of a transaction
           bytesTransfered;           //
  String   _FTP_USER;
  String   _FTP_PASS;
  boolean  bUnixLst;
};

#endif // FTP_SERVER_H


