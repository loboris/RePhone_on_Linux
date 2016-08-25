# RePhone development on Linux

This repository contains RePhone application projects configured for Eclipse.
The main purpose of this repository is <b>Lua</b> implementation on RePhone (Xadow GSM+BLE).

### Guide for installation/configuration on Linux
[RePhone on Linux guide](https://github.com/loboris/RePhone_on_Linux/raw/master/Documents/RePhone%20on%20Linux.pdf)<br/>

### Links:
* [RePhone wiki](http://www.seeedstudio.com/wiki/Rephone)
* [RePhone Forum](http://www.seeedstudio.com/forum/viewforum.php?f=71)
* [LinkitAssist 2502 Forum](http://labs.mediatek.com/forums/forums/show/58.page)
* [Xadow GSM+BLE](http://www.seeedstudio.com/wiki/Xadow_GSM%2BBLE)
* [Linkit Assist SDK](http://download.labs.mediatek.com/MediaTek_LinkIt_Assist_2502_SDK_2_0_46.zip)
* [Eclipse](https://www.eclipse.org/downloads/)

# &#x1F539; Lua on RePhone &#x1F539;

Lua programming language implementation on RePhone (Xadow GSM+BLE).

<i>"Lua is a powerful, efficient, lightweight, embeddable scripting language. It supports procedural programming, object-oriented programming, functional programming, data-driven programming, and data description.

Lua combines simple procedural syntax with powerful data description constructs based on associative arrays and extensible semantics.
Lua is dynamically typed, runs by interpreting bytecode with a register-based virtual machine, and has automatic memory management with incremental garbage collection, making it ideal for configuration, scripting, and rapid prototyping"</i>

This implementation is specially oriented to IoT applications building.


#### Main features:

* Complete <b>Lua 5.1</b> implementation
* Memory available to Lua programs 650~700 KB + heap for c functions ~64 KB
* <b>os</b> module expanded with many new functions ( copy, mkdir, rmdir, list, compile )
* <b>sys</b> module added with many RePhone related functions, non-volatile system parameters & variables
* <b>Watchdog</b> implemented for maximum security with variable timeout.
* Full <b>RTC</b> support with scheduled shut down / wake up / alarm
* Lua <B>shell</b> is available on USB port (/dev/ttyACM0), on RePhone hardware UART and over Bluetooth
* Many new/RePhone specific modules (written in c)
  * <b>gpio</b>, with added fonctions for gpio toggle and EINT on any eint capable pin
  * <b>pwm</b> module (part of gpio module)
  * <b>adc</b> module (part of gpio module)
  * <b>eint</b> external interrupt; Lua callback function (part of gpio module)
  * <b>wt2812</b> module; WT2812 - NeoPixel support (part of gpio module)
  * <b>https</b>, with added post mode (capable of sending file or params from table)
  * <b>gsm</b> module, with complete SMS functionality (read,write,list,delete,callbacks,...)
  * <b>mqtt</b> module, complete mqtt client
  * <b>email</b> module, only <b>smtp</b> client for now (send email)
  * <b>ftp</b> module, all basic <b>ftp</b> client commands available
  * <b>audio</b> module, playback and recording
  * <b>timer</b> module
  * <b>i2c</b> hw i2c module
  * <b>spi</b> hw spi module
  * <b>net</b> module, tcp & udp support, ntp support (set RTC time from NTP server)
  * <b>bt</b> Bluetooth module with possible Lua shell redirect 
  * <b>uart</b> Hardware UART (two ports) module with possible Lua shell redirect 
  * <b>sensor</b> module: DS18B20, DS18S20, DS1822 and DS28EA00, DHT-11, DHT-22, BME280 sensors
  * hash modules: <b>md5</b>, <b>SHA1</b>, <b>SHA2</b>
  * complete <b>json</b> module (cjson)
  * <b>struct</b> module (python struct library Lua implementation)
  * <b>bit</b> module, adds bitwise operations on numbers
  * <b>lcd</b> module; full support for Xadow 1.54" Touchscreen V0/V1 (with touch support) and ILI9341 & ST7735 based TFT modules in 4-wire SPI mode. Supported  are many graphics elements, fixed width and proportional fonts (unlimited number of fonts, loaded from file), jpeg, bmp and raw bitmap images. Does not require frame buffer memory, very fast.
  * <b>term</b> module; operations for ansi/vt100 terminals; console file editior, ymodem file transfer
