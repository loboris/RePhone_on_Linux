# RePhone development on Linux

This repository contains RePhone application projects configured for Eclipse

###Guide for installation/configuration on Linux
[RePhone on Linux guide](https://github.com/loboris/RePhone_on_Linux/raw/master/Documents/RePhone%20on%20Linux.pdf)<br/>

###Links:
* [RePhone wiki](http://www.seeedstudio.com/wiki/Rephone)
* [RePhone Forum](http://www.seeedstudio.com/forum/viewforum.php?f=71)
* [LinkitAssist 2502 Forum](http://labs.mediatek.com/forums/forums/show/58.page)
* [Xadow GSM+BLE](http://www.seeedstudio.com/wiki/Xadow_GSM%2BBLE)
* [Linkit Assist SDK](http://download.labs.mediatek.com/MediaTek_LinkIt_Assist_2502_SDK_2_0_46.zip)
* [Eclipse](https://www.eclipse.org/downloads/)

# <p style='color:blue'>Lua on RePhone</p>

The main project in this repository is Lua implementation on RePhone.

####Main features:

* Complete <b>Lua 5.1</b> implementation
* Memory available to Lua programs ~700 KB + heap for c functions ~64 KB
* System functions for <b>watchdog</b>, <b>rtc</b>, scheduled <b>shut down</b> / <b>wake up</b> implemented
* <b>os</b> module expanded with many new functions ( <b>ntptime</b> function added for adjusting RTC time from NTP server )
* Lua <B>shell</b> is available on USB port (/dev/ttyACM0), on RePhone hardware UART and over Bluetooth
* Many new/RePhone specific modules (written in c)
  * <b>gpio</b>, with added fonctions for gpio toggle and EINT on any eint capable pin
  * <b>pwm</b> module
  * <b>adc</b> module
  * <b>https</b>, with added post mode (capable of sending file or params from table)
  * <b>gsm</b> module, with complete SMS functionality
  * <b>audio</b> module
  * <b>timer</b> module
  * <b>i2c</b> module
  * <b>spi</b> module (<i>in development</i>)
  * <b>tcp & udp</b> modules
  * <b>bt</b> Bluetooth module with possible Lua shell redirect 
  * <b>sensor</b> module (DS18B20, DHT-11, DHT-22 for now)
  * hash modules: <b>md5</b>, <b>SHA1</b>, <b>SHA2</b>
  * complete <b>json</b> module (cjson)
  * <b>struct</b> module (python struct library Lua implementation)
