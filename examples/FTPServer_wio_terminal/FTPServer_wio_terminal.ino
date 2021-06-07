/*
 *  This sketch sends data via HTTP GET requests to examle.com service.
 */

#include <rpcWiFi.h>
//#ifdef USESPIFLASH    // if you want to use flash FS . default mode is spi mode
//#define DEV SPIFLASH
//#include "SFUD/Seeed_SFUD.h"
//#else                 // if you want to use SD FS
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
//#endif
#include <FtpServer.h>



FtpServer ftpSrv;

const char *ssid = "<YOUR-SSID>";
const char *password = "<YOUR-PASSWD>";

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    Serial.print("Listing directory: ");
    Serial.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);

    while (!SD.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
//    while (!DEV.begin(104000000UL)) {
        Serial.println("Card Mount Failed");
        return;
    }

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.mode(WIFI_STA);


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(1000);

    Serial.print("Starting SD.");

//    SD.mkdir("ciao");

    Serial.println("finish!");

    Serial.print("Listing directory: ");
    Serial.println("/");

    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
//    file.close();
//    root.close();

    ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.

  }

  void loop(void) {
  	  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
  }
