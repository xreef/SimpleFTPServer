/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form https://github.com/nailbuster/esp8266FTPServer
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include "FtpServer.h"

FTP_SERVER_NETWORK_SERVER_CLASS ftpServer( FTP_CTRL_PORT);
FTP_SERVER_NETWORK_SERVER_CLASS dataServer( FTP_DATA_PORT_PASV);

void FtpServer::begin(String uname, String pword, boolean _bUnixLst) {
	// Tells the ftp server to begin listening for incoming connection
	_FTP_USER = uname;
	_FTP_PASS = pword;
	bUnixLst = _bUnixLst;

	ftpServer.begin();
	delay(10);
	dataServer.begin();
	delay(10);
	millisTimeOut = (uint32_t) FTP_TIME_OUT * 60 * 1000;
	millisDelay = 0;
	cmdStatus = 0;
	iniVariables();
}

void FtpServer::iniVariables() {
	// Default for data port
	dataPort = FTP_DATA_PORT_PASV;

	// Default Data connection is Active
	dataPassiveConn = true;

	// Set the root directory
	strcpy(cwdName, "/");

	rnfrCmd = false;
	transferStatus = 0;

}

void FtpServer::handleFTP() {
	if ((int32_t) (millisDelay - millis()) > 0)
		return;

	if (cmdStatus == 0) {
		if (client.connected()) {
			disconnectClient();
		}
		cmdStatus = 1;
	} else if (cmdStatus == 1)         // Ftp server waiting for connection
			{
		abortTransfer();
		iniVariables();
		DEBUG_PRINT(F("Ftp server waiting for connection on port "));
		DEBUG_PRINTLN(FTP_CTRL_PORT);
		cmdStatus = 2;
	} else if (cmdStatus == 2)         // Ftp server idle
			{

#if(FTP_SERVER_NETWORK_TYPE == NETWORK_W5100 || FTP_SERVER_NETWORK_TYPE == NETWORK_ENC28J60)

		if (!client.connected()) {
//		  	DEBUG_PRINTLN("CLIENT CONNECTED STATUS 2");
			client.stop();

			client = ftpServer.accept();
		}
#else
		  if (ftpServer.hasClient()) {
			  client.stop();
			  client = ftpServer.available();
		  }
	#endif

		if (client.connected())                // A client connected
		{
			DEBUG_PRINTLN("CLIENT CONNECTED FROM 2");

			clientConnected();
			millisEndConnection = millis() + 10 * 1000; // wait client id during 10 s.
			cmdStatus = 3;
		}
	} else if (readChar() > 0)         // got response
			{
		if (cmdStatus == 3)            // Ftp server waiting for user identity
				{
			if (!strcmp(command, "FEAT")) {
				client.println(F("211-Extensions suported:"));
				client.println(" MLSD");
				client.println(F("211 End."));
			} else if (!strcmp(command, "USER")) {
				int result = userIdentity();
				if (result == 1) {
					cmdStatus = 4;
				} else {
					cmdStatus = 0;
				}
			}
		} else if (cmdStatus == 4)   // Ftp server waiting for user registration
			if (userPassword()) {
				cmdStatus = 5;
				millisEndConnection = millis() + millisTimeOut;
			} else
				cmdStatus = 0;
		else if (cmdStatus == 5) {       // Ftp server waiting for user command
			if (!processCommand()) {
				cmdStatus = 0;
			} else {
				millisEndConnection = millis() + millisTimeOut;
			}
		}
	} else if (!client.connected() || !client) {
		cmdStatus = 1;
		DEBUG_PRINTLN("client disconnected");
	}

	if (transferStatus == 1)         // Retrieve data
			{
		if (!doRetrieve())
			transferStatus = 0;
	} else if (transferStatus == 2)    // Store data
			{
		if (!doStore())
			transferStatus = 0;
	} else if (cmdStatus > 2
			&& !((int32_t) (millisEndConnection - millis()) > 0)) {
		client.println("530 Timeout");
		millisDelay = millis() + 200;    // delay of 200 ms
		cmdStatus = 0;
	}
}

void FtpServer::clientConnected() {
	DEBUG_PRINTLN("Client connected!");

	client.println(F("220--- Welcome to FTP for ESP8266/ESP32 ---"));
	client.println(F("220---   By Renzo Mischianti   ---"));
	client.print(F("220 --   Version "));
	client.print(FTP_SERVER_VERSION);
	client.println(F("   --"));
	iCL = 0;
}

void FtpServer::disconnectClient() {
	DEBUG_PRINTLN(F(" Disconnecting client"));

	abortTransfer();
	client.println(F("221 Goodbye"));
	client.stop();
}

boolean FtpServer::userIdentity() {
	if (strcmp(command, "USER"))
		client.println(F("500 Syntax error"));
	if (strcmp(parameters, _FTP_USER.c_str()))
		client.println(F("530 user not found"));
	else {
		client.println(F("331 OK. Password required"));
		strcpy(cwdName, "/");
		return true;
	}
	millisDelay = millis() + 100;  // delay of 100 ms
	return false;
}

boolean FtpServer::userPassword() {
	if (strcmp(command, "PASS"))
		client.println(F("500 Syntax error"));
	else if (strcmp(parameters, _FTP_PASS.c_str()))
		client.println(F("530 "));
	else {
		DEBUG_PRINTLN(F("OK. Waiting for commands."));

		client.println(F("230 OK."));
		return true;
	}
	millisDelay = millis() + 100;  // delay of 100 ms
	return false;
}

boolean FtpServer::processCommand() {
	///////////////////////////////////////
	//                                   //
	//      ACCESS CONTROL COMMANDS      //
	//                                   //
	///////////////////////////////////////

	//
	//  CDUP - Change to Parent Directory
	//
	if (!strcmp(command, "CDUP")) {
		client.print(F("250 Ok. Current directory is "));
		client.println(cwdName);
	}
	//
	//  CWD - Change Working Directory
	//
	else if (!strcmp(command, "CWD")) {
//    char path[ FTP_CWD_SIZE ];
		if (strcmp(parameters, ".") == 0) { // 'CWD .' is the same as PWD command
			client.print(F("257 \""));
			client.print(cwdName);
			client.println(F("\" is your current directory"));
		} else {
			client.print(F("250 Ok. Current directory is "));
			client.println(cwdName);
		}

	}
	//
	//  PWD - Print Directory
	//
	else if (!strcmp(command, "PWD")) {
		client.print(F("257 \""));
		client.print(cwdName);
		client.println(F("\" is your current directory"));
		//
		//  QUIT
		//
	} else if (!strcmp(command, "QUIT")) {
		disconnectClient();
		return false;
	}

	///////////////////////////////////////
	//                                   //
	//    TRANSFER PARAMETER COMMANDS    //
	//                                   //
	///////////////////////////////////////

	//
	//  MODE - Transfer Mode
	//
	else if (!strcmp(command, "MODE")) {
		if (!strcmp(parameters, "S")) {
			client.println(F("200 S Ok"));
			// else if( ! strcmp( parameters, "B" ))
			//  client.println( "200 B Ok\r\n";
		} else {
			client.println(F("504 Only S(tream) is suported"));
		}
	}
	//
	//  PASV - Passive Connection management
	//
	else if (!strcmp(command, "PASV")) {
		if (data.connected())
			data.stop();
		//dataServer.begin();
		//dataIp = Ethernet.localIP();

#if(FTP_SERVER_NETWORK_TYPE == NETWORK_W5100 || FTP_SERVER_NETWORK_TYPE == NETWORK_ENC28J60)
		dataIp = Ethernet.localIP();
#else
		dataIp = client.localIP();
#endif

		dataPort = FTP_DATA_PORT_PASV;
		//data.connect( dataIp, dataPort );
		//data = dataServer.available();
		DEBUG_PRINTLN("Connection management set to passive");
		DEBUG_PRINT(dataIp[0]);
		DEBUG_PRINT(F(","));
		DEBUG_PRINT(dataIp[1]);
		DEBUG_PRINT(F(","));
		DEBUG_PRINT(dataIp[2]);
		DEBUG_PRINT(F(","));
		DEBUG_PRINTLN(dataIp[3]);

		DEBUG_PRINT("Data port set to ");
		DEBUG_PRINTLN(dataPort);

		client.print(F("227 Entering Passive Mode ("));
		client.print(dataIp[0]);
		client.print(F(","));
		client.print(dataIp[1]);
		client.print(F(","));
		client.print(dataIp[2]);
		client.print(F(","));
		client.print(dataIp[3]);
		client.print(F(","));
		client.print(dataPort >> 8);
		client.print(F(","));
		client.print(dataPort & 255);
		client.println(F(")."));
		dataPassiveConn = true;
	}
	//
	//  PORT - Data Port
	//
	else if (!strcmp(command, "PORT")) {
		if (data)
			data.stop();
		// get IP of data client
		dataIp[0] = atoi(parameters);
		char *p = strchr(parameters, ',');
		for (uint8_t i = 1; i < 4; i++) {
			dataIp[i] = atoi(++p);
			p = strchr(p, ',');
		}
		// get port of data client
		dataPort = 256 * atoi(++p);
		p = strchr(p, ',');
		dataPort += atoi(++p);
		if (p == NULL) {
			client.println(F("501 Can't interpret parameters"));
		} else {

			client.println(F("200 PORT command successful"));
			dataPassiveConn = false;
		}
	}
	//
	//  STRU - File Structure
	//
	else if (!strcmp(command, "STRU")) {
		if (!strcmp(parameters, "F")) {
			client.println(F("200 F Ok"));
			// else if( ! strcmp( parameters, "R" ))
			//  client.println( "200 B Ok\r\n";
		} else {
			client.println(F("504 Only F(ile) is suported"));
		}
	}
	//
	//  TYPE - Data Type
	//
	else if (!strcmp(command, "TYPE")) {
		if (!strcmp(parameters, "A"))
			client.println(F("200 TYPE is now ASII"));
		else if (!strcmp(parameters, "I"))
			client.println(F("200 TYPE is now 8-bit binary"));
		else
			client.println(F("504 Unknow TYPE"));
	}

	///////////////////////////////////////
	//                                   //
	//        FTP SERVICE COMMANDS       //
	//                                   //
	///////////////////////////////////////

	//
	//  ABOR - Abort
	//
	else if (!strcmp(command, "ABOR")) {
		abortTransfer();
		client.println(F("226 Data connection closed"));
	}
	//
	//  DELE - Delete a File
	//
	else if (!strcmp(command, "DELE")) {
		char path[ FTP_CWD_SIZE];
		if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(path)) {
			if (! STORAGE_MANAGER.exists(path)) {
				client.print(F("550 File "));
				client.print(parameters);
				client.println(F(" not found"));
			} else {
				if ( STORAGE_MANAGER.remove(path)) {
					client.print(F("250 Deleted "));
					client.println(parameters);
				} else {
					client.print(F("450 Can't delete "));
					client.println(parameters);
				}
			}
		}
	}
	//
	//  LIST - List
	//
	else if (!strcmp(command, "LIST")) {
		if (!dataConnect())
			client.println(F("425 No data connection"));
		else {
			client.println(F("150 Accepted data connection"));
			uint16_t nm = 0;
#if defined(ESP8266) && (STORAGE_TYPE != STORAGE_SD)
      Dir dir=STORAGE_MANAGER.openDir(cwdName);
     // if( !STORAGE_MANAGER.exists(cwdName))
     //   client.println( "550 Can't open directory " + String(cwdName) );
     // else
      {
        while( dir.next())
        {
			String fn, fs;
			fn = dir.fileName();
			DEBUG_PRINT(F("File -> "));
			DEBUG_PRINTLN(fn);
//#if (STORAGE_LITTLEFS == STORAGE_TYPE)
//			fn.remove(0, 1);
//#endif
			fs = String(dir.fileSize());
            data.print( "+r,s" + fs);
            data.println( ",\t" + fn );
            nm ++;
        }
        client.print( F("226 ") );
        client.print(nm);
        client.println(F(" matches total"));
      }
//#if (STORAGE_TYPE == STORAGE_SD)
//      dir.close();
//#endif
#elif defined(ESP32) || (STORAGE_TYPE == STORAGE_SD)
			File root = STORAGE_MANAGER.open(cwdName);
			if (!root) {
				client.print(F("550 Can't open directory "));
				client.println(cwdName);
				// return;
			} else {
				// if(!root.isDirectory()){
				// 		Serial.println("Not a directory");
				// 		return;
				// }

				File file = root.openNextFile();
				while (file) {
					if (file.isDirectory()) {
						if (bUnixLst) {
							data.println(
									"drwxrw-rw-   2 esp esp      "
											+ String("92") + " Jun 27  2011 "
											+ String(file.name()));
						} else {
							data.println("+r,s <DIR> " + String(file.name()));
						}
						// Serial.print("  DIR : ");
						// Serial.println(file.name());
						// if(levels){
						// 	listDir(fs, file.name(), levels -1);
						// }
					} else {
						String fn, fs;
						fn = file.name();
						// fn.remove(0, 1);
						fs = String(file.size());
						if (bUnixLst) {
#if (STORAGE_TYPE != STORAGE_SD)
							fn.remove(0, 1);
#endif
							data.println(
									"-rw-rw-rw-   1 esp esp      " + fs
											+ " Jun 27  2011 " + fn);
						} else {
							data.print("+r,s" + fs);
							data.println(",\t" + fn);
						}
						nm++;
					}
					file.close();
					file = root.openNextFile();
				}
				client.println("226 " + String(nm) + " matches total");
			}
#endif
			data.stop();
		}
	}
	//
	//  MLSD - Listing for Machine Processing (see RFC 3659)
	//
	else if (!strcmp(command, "MLSD")) {
		if (!dataConnect())
			client.println(F("425 No data connection MLSD"));
		else {
			client.println(F("150 Accepted data connection"));
			uint16_t nm = 0;
#if defined(ESP8266) && (STORAGE_TYPE != STORAGE_SD)
      Dir dir= STORAGE_MANAGER.openDir(cwdName);
//      char dtStr[ 15 ];
    //  if(!STORAGE_MANAGER.exists(cwdName))
    //    client.println( "550 Can't open directory " +String(parameters)+ );
    //  else
      {
        while( dir.next())
		{
			String fn,fs;
			fn = dir.fileName();
			DEBUG_PRINT(F("File -> "));
			DEBUG_PRINTLN(fn);

//#if (STORAGE_LITTLEFS == STORAGE_TYPE)
//			fn.remove(0, 1);
//#endif
			fs = String(dir.fileSize());
          data.println( "Type=file;Size=" + fs + ";"+"modify=20000101160656;" +" " + fn);
          nm ++;
        }
        client.println( F("226-options: -a -l") );
        client.println( "226 " + String(nm) + " matches total");
      }
#if (STORAGE_TYPE == STORAGE_SD)
      dir.close();
#endif
#elif defined(ESP32) || (STORAGE_TYPE == STORAGE_SD)
			File root = STORAGE_MANAGER.open(cwdName);
			// if(!root){
			// 		client.println( "550 Can't open directory " + String(cwdName) );
			// 		// return;
			// } else {
			// if(!root.isDirectory()){
			// 		Serial.println("Not a directory");
			// 		return;
			// }

			File file = root.openNextFile();
			while (file) {
				// if(file.isDirectory()){
				// 	data.println( "+r,s <DIR> " + String(file.name()));
				// 	// Serial.print("  DIR : ");
				// 	// Serial.println(file.name());
				// 	// if(levels){
				// 	// 	listDir(fs, file.name(), levels -1);
				// 	// }
				// } else {
				String fn, fs;
				fn = file.name();
#if (STORAGE_TYPE != STORAGE_SD)
						fn.remove(0, 1);
#endif
				fs = String(file.size());
				data.println(
						"Type=file;Size=" + fs + ";" + "modify=20000101160656;"
								+ " " + fn);
				nm++;
				// }
				file.close();
				file = root.openNextFile();
			}
			client.println(F("226-options: -a -l"));
			client.println("226 " + String(nm) + " matches total");
			// }
#endif
			data.stop();
		}
	}
	//
	//  NLST - Name List
	//
	else if (!strcmp(command, "NLST")) {
		if (!dataConnect())
			client.println(F("425 No data connection"));
		else {
			client.println(F("150 Accepted data connection"));
			uint16_t nm = 0;
#if defined(ESP8266) && (STORAGE_TYPE != STORAGE_SD)
      Dir dir=STORAGE_MANAGER.openDir(cwdName);
     // if( !STORAGE_MANAGER.exists( cwdName ))
     //   client.println( "550 Can't open directory " + String(parameters));
     // else
      {
        while( dir.next())
        {
          data.println( dir.fileName());
          nm ++;
        }
        client.println( "226 " + String(nm) + " matches total");
      }
//#if (STORAGE_TYPE == STORAGE_SD)
//      dir.close();
//#endif
#elif defined(ESP32) || (STORAGE_TYPE == STORAGE_SD)
			File root = STORAGE_MANAGER.open(cwdName);
			if (!root) {
				client.println("550 Can't open directory " + String(cwdName));
			} else {

				File file = root.openNextFile();
				while (file) {
					data.println(file.name());
					nm++;
					file = root.openNextFile();
				}
				client.println("226 " + String(nm) + " matches total");
			}
			root.close();
#endif
			data.stop();
		}
	}
	//
	//  NOOP
	//
	else if (!strcmp(command, "NOOP")) {
		// dataPort = 0;
		client.println(F("200 Zzz..."));
	}
	//
	//  RETR - Retrieve
	//
	else if (!strcmp(command, "RETR")) {
		char path[ FTP_CWD_SIZE];
		if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(path)) {
			DEBUG_PRINT(F("Load file "));
			DEBUG_PRINTLN(path);
			file.close();
			file = STORAGE_MANAGER.open(path, FILE_READ);
			if (!file)
				client.println("550 File " + String(parameters) + " not found");
			else if (!file)
				client.println("450 Can't open " + String(parameters));
			else if (!dataConnect())
				client.println(F("425 No data connection"));
			else {
				DEBUG_PRINT(F("Sending "));
				DEBUG_PRINTLN(parameters);

				client.println("150-Connected to port " + String(dataPort));
				client.println(
						"150 " + String(file.size()) + " bytes to download");
				millisBeginTrans = millis();
				bytesTransfered = 0;
				transferStatus = 1;
			}
		}
	}
	//
	//  STOR - Store
	//
	else if (!strcmp(command, "STOR")) {
		char path[ FTP_CWD_SIZE];
		if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(path)) {
			file.close();
			file = STORAGE_MANAGER.open(path, FILE_WRITE);
			if (!file)
				client.println("451 Can't open/create " + String(parameters));
			else if (!dataConnect() && !data.available()) {
				client.println("425 No data connection");
				file.close();
			} else {
				DEBUG_PRINT(F("Receiving "));
				DEBUG_PRINTLN(parameters);

				client.println("150 Connected to port " + String(dataPort));
				millisBeginTrans = millis();
				bytesTransfered = 0;
				transferStatus = 2;
			}
		}
	}
	//
	//  MKD - Make Directory
	//
	else if (!strcmp(command, "MKD")) {
		client.println("550 Can't create \"" + String(parameters)); //not support on espyet
	}
	//
	//  RMD - Remove a Directory
	//
	else if (!strcmp(command, "RMD")) {
		client.println("501 Can't delete \"" + String(parameters));

	}
	//
	//  RNFR - Rename From
	//
	else if (!strcmp(command, "RNFR")) {
		buf[0] = 0;
		if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(buf)) {
			if (! STORAGE_MANAGER.exists(buf))
				client.println("550 File " + String(parameters) + " not found");
			else {
				DEBUG_PRINT(F("Renaming "));
				DEBUG_PRINTLN(buf);

				client.println(
						F(
								"350 RNFR accepted - file exists, ready for destination"));
				rnfrCmd = true;
			}
		}
	}
	//
	//  RNTO - Rename To
	//
	else if (!strcmp(command, "RNTO")) {
		char path[ FTP_CWD_SIZE];
//    char dir[ FTP_FIL_SIZE ];
		if (strlen(buf) == 0 || !rnfrCmd)
			client.println("503 Need RNFR before RNTO");
		else if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(path)) {
			if ( STORAGE_MANAGER.exists(path))
				client.println("553 " + String(parameters) + " already exists");
			else {
				DEBUG_PRINT(F("Renaming "));
				DEBUG_PRINTLN(buf);
				DEBUG_PRINT(F(" to "));
				DEBUG_PRINTLN(path);

#if (STORAGE_SD == STORAGE_TYPE)
				File myFileIn = STORAGE_MANAGER.open(buf, 'r');
				File myFileOut = STORAGE_MANAGER.open(path, 'w');
				while (myFileIn.available()) {
					myFileOut.write(myFileIn.read());
				}
				bool operation = false;

				if (myFileIn.size() == myFileOut.size()) {
					operation = true;
				}

				myFileIn.close();
				myFileOut.close();

				STORAGE_MANAGER.remove(buf);
				if (operation) {
					client.println(F("250 File successfully renamed or moved"));
				} else {
					client.println(F("451 Rename/move failure"));
				}
#else
            if( STORAGE_MANAGER.rename( buf, path )) {
              client.println( F("250 File successfully renamed or moved") );
            } else {
				client.println( F("451 Rename/move failure") );
            }
#endif
			}
		}
		rnfrCmd = false;
	}

	///////////////////////////////////////
	//                                   //
	//   EXTENSIONS COMMANDS (RFC 3659)  //
	//                                   //
	///////////////////////////////////////

	//
	//  FEAT - New Features
	//
	else if (!strcmp(command, "FEAT")) {
		client.println(F("211-Extensions suported:"));
		client.println(F(" MLSD"));
		client.println(F("211 End."));
	}
	//
	//  MDTM - File Modification Time (see RFC 3659)
	//
	else if (!strcmp(command, "MDTM")) {
		client.println(F("550 Unable to retrieve time"));
	}

	//
	//  SIZE - Size of the file
	//
	else if (!strcmp(command, "SIZE")) {
		char path[ FTP_CWD_SIZE];
		if (strlen(parameters) == 0)
			client.println(F("501 No file name"));
		else if (makePath(path)) {
			file.close();
			file = STORAGE_MANAGER.open(path, FILE_READ);
			if (!file)
				client.println("450 Can't open " + String(parameters));
			else {
				client.println("213 " + String(file.size()));
				file.close();
			}
		}
	}
	//
	//  SITE - System command
	//
	else if (!strcmp(command, "SITE")) {
		client.print(F("500 Unknow SITE command "));
		client.println(parameters);
	}
	//
	//  SYST - System type
	//
	else if (!strcmp(command, "SYST")) {
		client.println(F("215 UNIX Type: L8"));
	}
	//
	//  Unrecognized commands ...
	else
		client.println(F("500 Unknow command"));

	return true;
}

boolean FtpServer::dataConnect() {
	unsigned long startTime = millis();
	//wait 5 seconds for a data connection
	if (!data.connected()) {
		DEBUG_PRINT("NOT CONNECTED");
		DEBUG_PRINTLN(dataPassiveConn);
		if (dataPassiveConn) {

			uint16_t count = 5000;

//  dataPassiveConn

			while (!data.connected() && count-- > 0) {

//	  while (!dataServer.hasClient() && millis() - startTime < 10000)
//	  {
//		  //delay(100);
//		  yield();
//	  }

#if(FTP_SERVER_NETWORK_TYPE == NETWORK_W5100 || FTP_SERVER_NETWORK_TYPE == NETWORK_ENC28J60)
				data = dataServer.accept();
#else
	if( dataServer.hasClient())
	{
	  data.stop();
	  data = dataServer.available();
	}
#endif
				delay(1);

//	  if (dataServer.hasClient()) {
//		  data.stop();
//		  data = dataServer.available();
//			DEBUG_PRINTLN(F("ftpdataserver client...."));
//	  }
			}
		} else if (!dataPassiveConn) {
			data.connect(dataIp, dataPort);
		}
	}
//    if( ! data.connected()) {
//        client.println( F("425 No data connection") );
//    } else {
//    	  	client.print( F("150 Accepted data connection to port ") );
//    		client.println( dataPort );
//      }
	return data.connected();

}
//boolean FtpServer::dataConnect()
//{
//  unsigned long startTime = millis();
//  //wait 5 seconds for a data connection
//  if (!data.connected())
//  {
//	  while (!dataServer.hasClient() && millis() - startTime < 10000)
//	  {
//		  //delay(100);
//		  yield();
//	  }
//	  if (dataServer.hasClient()) {
//		 // data.stop();
//		  data = dataServer.available();
//			#ifdef FTP_DEBUG
//		      Serial.println("ftpdataserver client....");
//			#endif
//
//	  }
//  }
//
//  return  data.connected() || data.available();
//
//}

boolean FtpServer::doRetrieve() {
	if (data.connected()) {
		int16_t nb = file.readBytes(buf, FTP_BUF_SIZE);
		if (nb > 0) {
			data.write((uint8_t*) buf, nb);
			bytesTransfered += nb;
			return true;
		}
	}
	closeTransfer();
	return false;
}

boolean FtpServer::doStore() {
	// Avoid blocking by never reading more bytes than are available
	int navail = data.available();

	if (navail > 0) {
		// And be sure not to overflow buf.
		if (navail > FTP_BUF_SIZE)
			navail = FTP_BUF_SIZE;
		int16_t nb = data.read((uint8_t*) buf, navail);
		// int16_t nb = data.readBytes((uint8_t*) buf, FTP_BUF_SIZE );
		if (nb > 0) {
			// Serial.println( millis() << " " << nb << endl;
			file.write((uint8_t*) buf, nb);
			bytesTransfered += nb;
		}
	}
	if (!data.connected() && (navail <= 0)) {
		closeTransfer();
		return false;
	} else {
		return true;
	}
}

void FtpServer::closeTransfer() {
	uint32_t deltaT = (int32_t) (millis() - millisBeginTrans);
	if (deltaT > 0 && bytesTransfered > 0) {
		client.println(F("226-File successfully transferred"));
		client.print(F("226 "));
		client.print(deltaT);
		client.print(F(" ms, "));
		client.print(bytesTransfered / deltaT);
		client.println(F(" kbytes/s"));
	} else
		client.println(F("226 File successfully transferred"));

	file.close();
	data.stop();
}

void FtpServer::abortTransfer() {
	if (transferStatus > 0) {
		file.close();
//    dir.close();
		client.println(F("426 Transfer aborted"));
		DEBUG_PRINTLN(F("Transfer aborted!"));

		transferStatus = 0;
	}
	data.stop();

}

// Read a char from client connected to ftp server
//
//  update cmdLine and command buffers, iCL and parameters pointers
//
//  return:
//    -2 if buffer cmdLine is full
//    -1 if line not completed
//     0 if empty line received
//    length of cmdLine (positive) if no empty line received 

int8_t FtpServer::readChar() {
	int8_t rc = -1;

	if (client.available()) {
		char c = client.read();
		// char c;
		// client.readBytes((uint8_t*) c, 1);
		DEBUG_PRINT(c);

		if (c == '\\')
			c = '/';
		if (c != '\r') {
			if (c != '\n') {
				if (iCL < FTP_CMD_SIZE)
					cmdLine[iCL++] = c;
				else
					rc = -2; //  Line too long
			} else {
				cmdLine[iCL] = 0;
				command[0] = 0;
				parameters = NULL;
				// empty line?
				if (iCL == 0)
					rc = 0;
				else {
					rc = iCL;
					// search for space between command and parameters
					parameters = strchr(cmdLine, ' ');
					if (parameters != NULL) {
						if (parameters - cmdLine > 4)
							rc = -2; // Syntax error
						else {
							strncpy(command, cmdLine, parameters - cmdLine);
							command[parameters - cmdLine] = 0;

							while (*(++parameters) == ' ')
								;
						}
					} else if (strlen(cmdLine) > 4) {
						rc = -2; // Syntax error.
					} else {
						strcpy(command, cmdLine);
					}
					iCL = 0;
				}
			}
		}
		if (rc > 0)
			for (uint8_t i = 0; i < strlen(command); i++)
				command[i] = toupper(command[i]);
		if (rc == -2) {
			iCL = 0;
			client.println("500 Syntax error");
		}
	}
	return rc;
}

// Make complete path/name from cwdName and parameters
//
// 3 possible cases: parameters can be absolute path, relative path or only the name
//
// parameters:
//   fullName : where to store the path/name
//
// return:
//    true, if done

boolean FtpServer::makePath(char *fullName) {
	return makePath(fullName, parameters);
}

boolean FtpServer::makePath(char *fullName, char *param) {
	if (param == NULL)
		param = parameters;

	// Root or empty?
	if (strcmp(param, "/") == 0 || strlen(param) == 0) {
		strcpy(fullName, "/");
		return true;
	}
	// If relative path, concatenate with current dir
	if (param[0] != '/') {
		strcpy(fullName, cwdName);
		if (fullName[strlen(fullName) - 1] != '/')
			strncat(fullName, "/", FTP_CWD_SIZE);
		strncat(fullName, param, FTP_CWD_SIZE);
	} else
		strcpy(fullName, param);
	// If ends with '/', remove it
	uint16_t strl = strlen(fullName) - 1;
	if (fullName[strl] == '/' && strl > 1)
		fullName[strl] = 0;
	if (strlen(fullName) < FTP_CWD_SIZE)
		return true;

	client.println(F("500 Command line too long"));
	return false;
}

// Calculate year, month, day, hour, minute and second
//   from first parameter sent by MDTM command (YYYYMMDDHHMMSS)
//
// parameters:
//   pyear, pmonth, pday, phour, pminute and psecond: pointer of
//     variables where to store data
//
// return:
//    0 if parameter is not YYYYMMDDHHMMSS
//    length of parameter + space

uint8_t FtpServer::getDateTime(uint16_t *pyear, uint8_t *pmonth, uint8_t *pday,
		uint8_t *phour, uint8_t *pminute, uint8_t *psecond) {
	char dt[15];

	// Date/time are expressed as a 14 digits long string
	//   terminated by a space and followed by name of file
	if (strlen(parameters) < 15 || parameters[14] != ' ')
		return 0;
	for (uint8_t i = 0; i < 14; i++)
		if (!isdigit(parameters[i]))
			return 0;

	strncpy(dt, parameters, 14);
	dt[14] = 0;
	*psecond = atoi(dt + 12);
	dt[12] = 0;
	*pminute = atoi(dt + 10);
	dt[10] = 0;
	*phour = atoi(dt + 8);
	dt[8] = 0;
	*pday = atoi(dt + 6);
	dt[6] = 0;
	*pmonth = atoi(dt + 4);
	dt[4] = 0;
	*pyear = atoi(dt);
	return 15;
}

// Create string YYYYMMDDHHMMSS from date and time
//
// parameters:
//    date, time 
//    tstr: where to store the string. Must be at least 15 characters long
//
// return:
//    pointer to tstr

char* FtpServer::makeDateTimeStr(char *tstr, uint16_t date, uint16_t time) {
	sprintf(tstr, "%04u%02u%02u%02u%02u%02u", ((date & 0xFE00) >> 9) + 1980,
			(date & 0x01E0) >> 5, date & 0x001F, (time & 0xF800) >> 11,
			(time & 0x07E0) >> 5, (time & 0x001F) << 1);
	return tstr;
}

