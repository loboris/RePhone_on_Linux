# ############################################
# The beta version for a linux firmware upload
# ############################################

# ############################################
# Thanks to:
# Stefan.Mandl1@gmail.com
# https://github.com/mandl/LinkIt_Assist_2502
# ############################################

!!PYTHON3 must be installed!!

udev rule
=========

Since Rephone is based on a Mediatek chip, ModemManager will, by default,
try to treat it as a modem and make it available for network connections.
This is undesirable.

To work around this problem, create a udev rule under /etc/udev/rules.d/
called 71-rephone.rules with the following contents:

    SUBSYSTEM=="tty", ATTRS{idVendor}=="0e8d",\
        ATTRS{idProduct}=="0003",\
        MODE="0660", SYMLINK+="rephone"

    ACTION=="add|change", SUBSYSTEM=="usb",\
        ENV{DEVTYPE}=="usb_device", ATTRS{idVendor}=="0e8d",\
        ATTRS{idProduct}=="0003",\
        ENV{ID_MM_DEVICE_IGNORE}="1"

Upload process
==============

1. Install the udev rule "71-rephone.rules"
2. Disconnect the battery.


We need in the same folder
==========================

1. Download_Agent/6261/NOR_FLASH_TABLE   This is the data for the NOR flash detection.
2. Download_Agent/6261/EXT_RAM           This is the download agent.
3. Download_Agent/6261/INT_SYSRAM        This is the download agent.
4. RePhone/EXT_BOOTLOADER
5. RePhone/INT_BOOTLOADER                This is the Rephone bootloader (SEEED02A_DEMO_BOOTLOADER_V005_MT2502_MAUI_11CW1418SP5_W15_19.bin).
6. RePhone/ROM                           (W15.19.p2-uart).
7. RePhone/VIVA                          (W15.19.p2-uart).
8. firmwareUploader.py                   Upload script

------------------------------------------------------
-- You can replace 5, 6 & 7 with different versions --
------------------------------------------------------


Doing the upload
================

1. Disconnect the battery and disconnect the device (USB) from PC
2. Start the *python3 firmwareUploader.py --nobattery* or *python3 firmwareUploader.py --nobattery --nofatformat* (to preserve the fat data)
3. Connect the device to USB
4. The upload is running. The red led goes on dimmed.
5. See progress info.... wait for **Update done !!!!!!!!**...
6. Disconnect the device from USB.

