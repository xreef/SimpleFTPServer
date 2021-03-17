# SimpleFTPServer

[Instruction on FTP server on esp8266 and esp32](https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32)

Simple FTP Server for 
 - esp8266 (SPIFFs, LittleFS)
 - esp32 (SPIFFS)
 - Arduino (SD with 8.3 file format)

<!-- wp:paragraph -->
<p>When I develop a new solution I'd like to divide the application in layer, and so I'd like focus my attention in only one aspect at time. </p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p> In detail I separate the REST layer (written inside the microcontroller) and the Front-End (written in Angular, React/Redux or vanilla JS), so I'd like to upload new web interface directly to the microcontroller via FTP. </p>
<!-- /wp:paragraph -->

<!-- wp:image {"align":"center","id":2155} -->
<div class="wp-block-image"><figure class="aligncenter"><img width="450px" src="https://www.mischianti.org/wp-content/uploads/2019/06/FTPTransferEsp8266-1024x662.jpg" alt="" class="wp-image-2155"/><figcaption>FTP file transfer on esp8266 or esp32</figcaption></figure></div>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p>For static information (Web pages for examples), that not change frequently, esp8266 or esp32 have internal SPIFFS (SPI Flash File System) and you can upload data via Arduino IDE as explained in the article  "<a href="https://www.mischianti.org/2019/08/30/wemos-d1-mini-esp8266-integrated-spiffs-filesistem-part-2/">WeMos D1 mini (esp8266), integrated SPIFFS Filesystem</a>" for esp8266 or "<a rel="noreferrer noopener" href="https://www.mischianti.org/2020/06/04/esp32-integrated-spiffs-filesystem-part-2/" target="_blank">ESP32: integrated SPIFFS FileSystem</a>" for esp32 or with LittleFS "<a href="https://www.mischianti.org/2020/06/22/wemos-d1-mini-esp8266-integrated-littlefs-filesystem-part-5/">WeMos D1 mini (esp8266), integrated LittleFS Filesystem</a>" but for fast operation and future support It's usefully use FTP.</p>
<!-- /wp:paragraph -->