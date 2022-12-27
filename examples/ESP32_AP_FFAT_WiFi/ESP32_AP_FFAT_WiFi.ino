/*
 * FtpServer esp32 with FFat FS
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include "Arduino.h"
#include "FS.h"
#include "FFat.h"

#include <SimpleFTPServer.h>

#ifdef STA_MODE
	const char* ssid = "<YOUR-SSID>";
	const char* password = "<YOUR-PASSWD>";
#endif
const char* ssid_AP = "ESP32";
const char* password_AP = "aabbccdd77";

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println(F("FTP: Connected!"));
      break;
    case FTP_DISCONNECT:
      Serial.println(F("FTP: Disconnected!"));
      break;
    case FTP_FREE_SPACE_CHANGE:
      Serial.printf("FTP: Free space change, free %u of %u!\n", freeSpace, totalSpace);
      break;
    default:
      break;
  }
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      Serial.println(F("FTP: Upload start!"));
      break;
    case FTP_UPLOAD:
      Serial.printf("FTP: Upload of file %s byte %u\n", name, transferredSize);
      break;
    case FTP_TRANSFER_STOP:
      Serial.println(F("FTP: Finish transfer!"));
      break;
    case FTP_TRANSFER_ERROR:
      Serial.println(F("FTP: Transfer error!"));
      break;
    default:
      break;
  }

  /* FTP_UPLOAD_START = 0,
   * FTP_UPLOAD = 1,
   *
   * FTP_DOWNLOAD_START = 2,
   * FTP_DOWNLOAD = 3,
   *
   * FTP_TRANSFER_STOP = 4,
   * FTP_DOWNLOAD_STOP = 4,
   * FTP_UPLOAD_STOP = 4,
   *
   * FTP_TRANSFER_ERROR = 5,
   * FTP_DOWNLOAD_ERROR = 5,
   * FTP_UPLOAD_ERROR = 5
   */
};

void setup(void){
  Serial.begin(115200);

  //AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_AP, password_AP);
  delay(1000);
  //IPAddress IP = IPAddress (10, 10, 10, 1);
  //IPAddress NMask = IPAddress (255, 255, 255, 0);
  //WiFi.softAPConfig(IP, IP, NMask);
  Serial.print("Set AP named:");
  Serial.println(ssid_AP);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

#ifdef STA_MODE
    // STA mode
    WiFi.begin(ssid, password);
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif

  /////FTP Setup, ensure FFAT is started before ftp;  /////////
  Serial.print(F("Inizializing FS..."));
  if (FFat.begin(true)){
      Serial.println(F("done."));
  }else{
      Serial.println(F("fail."));
      while (true) { delay(1000); };
  }

  ftpSrv.setCallback(_callback);
  ftpSrv.setTransferCallback(_transferCallback);

  Serial.println("Start FTP with user: user and passwd: password!");
  ftpSrv.begin("user","password");    //username, password for ftp.   (default 21, 50009 for PASV)
  ftpSrv.setLocalIp(myIP);
}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}
