/*
 * FtpServer esp8266 and esp32 with SPIFFS
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif

#include <SimpleFTPServer.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";


FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial


void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

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


  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
  
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true)) {
#elif defined ESP8266
  if (SPIFFS.begin()) {
#endif
      Serial.println("SPIFFS opened!");
      ftpSrv.begin("esp8266","esp8266","Welcome to FTP for Arduino");    //username, password for ftp.   (default 21, 50009 for PASV), welcome mseeage (optional)
  }    
}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
 // server.handleClient();   //example if running a webserver you still need to call .handleClient();
 
}
