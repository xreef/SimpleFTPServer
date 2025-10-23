/*
 * FtpServer Arduino with Ethernet library and w5100 shield
 * With SdFat version > 2 (full name and more size)
 *
 * #ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
 *	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO 	NETWORK_W5100
 *	#define DEFAULT_STORAGE_TYPE_ARDUINO STORAGE_SDFAT2
 * #endif
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <SdFat.h>
#include <sdios.h>
#include <FtpServer.h>
#include <FreeStack.h>

// Define Chip Select for your SD card according to hardware
// #define CS_SDCARD 4  // SD card reader of Ehernet shield
#define CS_SDCARD 4 // Chip Select for SD card reader on Due

// Define Reset pin for W5200 or W5500
// set to -1 for other ethernet chip or if Arduino reset board is used
 #define W5x00_RESET -1
//#define W5x00_RESET 8  // on Due
// #define W5x00_RESET 3  // on MKR


// Object for File system
SdFat sd;

// Object for FtpServer
//  Command port and Data port in passive mode can be defined here
// FtpServer ftpSrv( 221, 25000 );
// FtpServer ftpSrv( 421 ); // Default data port in passive mode is 55600
FtpServer ftpSrv; // Default command port is 21 ( !! without parenthesis !! )

// Mac address of ethernet adapter
// byte mac[] = { 0x90, 0xa2, 0xda, 0x00, 0x00, 0x00 };
byte mac[] = { 0x00, 0xaa, 0xbb, 0xcc, 0xde, 0xef };

// IP address of FTP server
// if set to 0, use DHCP for the routeur to assign IP
// IPAddress serverIp( 192, 168, 1, 40 );
IPAddress serverIp( 0, 0, 0, 0 );

// External IP address of FTP server
// In passive mode, when accessing the serveur from outside his subnet, it can be
//  necessary with some clients to reply them with the server's external ip address
// IPAddress externalIp( 192, 168, 1, 2 );

ArduinoOutStream cout( Serial );

// Callback called on connect/disconnect and when free space changes
void _callback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace) {
  // Log basic info to serial
  cout << F("[FTP callback] op=") << (int)ftpOperation
       << F(" free=") << (unsigned long)freeSpace
       << F(" total=") << (unsigned long)totalSpace << endl;

  // ftpOperation values include:
  // FTP_CONNECT, FTP_DISCONNECT, FTP_FREE_SPACE_CHANGE
  // Add custom handling here if needed (e.g., LED, status output)
}

// Callback called during file transfers
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, uint32_t transferredSize) {
  cout << F("[FTP transfer] op=") << (int)ftpOperation
       << F(" name=") << name
       << F(" size=") << (unsigned long)transferredSize << endl;

  // ftpOperation values include:
  // FTP_UPLOAD_START, FTP_UPLOAD, FTP_DOWNLOAD_START, FTP_DOWNLOAD,
  // FTP_TRANSFER_STOP, FTP_TRANSFER_ERROR
  // Add custom handling here if needed (e.g., progress indicator)
}

void setup()
{
  Serial.begin( 115200 );
  cout << F("=== Test of FTP Server with SdFat ") << SD_FAT_VERSION << F(" file system ===") << endl;

  // If other chips are connected to SPI bus, set to high the pin connected
  // to their CS before initializing Flash memory
  pinMode( 4, OUTPUT );
  digitalWrite( 4, HIGH );
  pinMode( 10, OUTPUT );
  digitalWrite( 10, HIGH );

  // Mount the SD card memory
  cout << F("Mount the SD card memory... ");
  if( ! sd.begin( CS_SDCARD, SD_SCK_MHZ( 50 )))
  {
    cout << F("Unable to mount SD card") << endl;
    while( true ) ;
  }
  pinMode( CS_SDCARD, OUTPUT );
  digitalWrite( CS_SDCARD, HIGH );
  cout << F("ok") << endl;

  // Show capacity and free space of SD card
  cout << F("Capacity of card:   ") << long( sd.card()->sectorCount() >> 1 )
       << F(" kBytes") << endl;
  cout << F("Free space on card: ")
       << long( sd.vol()->freeClusterCount() * sd.vol()->sectorsPerCluster() >> 1 )
       << F(" kBytes") << endl;

  // Send reset to Ethernet module
  if( W5x00_RESET > -1 )
  {
    pinMode( W5x00_RESET, OUTPUT );
    digitalWrite( W5x00_RESET, LOW );
    delay( 200 );
    digitalWrite( W5x00_RESET, HIGH );
    delay( 200 );
  }

  // Initialize the network
  cout << F("Initialize ethernet module ... ");
  if( serverIp[0] != 0 )
    Ethernet.begin( mac, serverIp );
  else if( Ethernet.begin( mac ) == 0 )
  {
    cout << F("failed!") << endl;
    while( true ) ;
  }
  uint16_t wizModule[] = { 0, 5100, 5200, 5500 };
  cout << F("W") << wizModule[ Ethernet.hardwareStatus()] << F(" ok") << endl;
  serverIp = Ethernet.localIP();
  cout << F("IP address of server: ")
       << int( serverIp[0]) << "." << int( serverIp[1]) << "."
       << int( serverIp[2]) << "." << int( serverIp[3]) << endl;

  // Register callbacks so user code receives FTP events and transfer notifications
  ftpSrv.setCallback(_callback);
  ftpSrv.setTransferCallback(_transferCallback);

  // Initialize the FTP server
  ftpSrv.begin("user","password");
  // ftpSrv.init( externalIp );
  // ftpSrv.init( IPAddress( 11, 22, 33, 44 ));

  // Default username and password are set to 'arduino' and 'test'
  //  but can then be changed by calling ftpSrv.credentials()
  // ftpSrv.credentials( "myname", "123" );

  cout << F("Free stack: ") << FreeStack() << endl;

  cout << "Viaaa!";
}

void loop()
{
  ftpSrv.handleFTP();

  // more processes...
}
