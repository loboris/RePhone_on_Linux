#!/usr/bin/env python3
#
# Copyright (C) 2016  Stefan Mandl
#
# A Updater Tool for Rephone
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
# Street, Fifth Floor, Boston, MA 02110-1301 USA.

import argparse
import os
import serial
import struct
import sys
import time
import traceback
from time import sleep


# http://www.bunniestudios.com/blog/?p=4297
#  mtk_cmd_old_write16 = 0xa1,
#  mtk_cmd_old_read16 = 0xa2,
#  mtk_checksum16 = 0xa4,
#  mtk_remap_before_jump_to_da = 0xa7,
#  mtk_jump_to_da = 0xa8,
#  mtk_send_da = 0xad,
#  mtk_jump_to_maui = 0xb7,
#  mtk_get_version = 0xb8,
#  mtk_close_usb_and_reset = 0xb9,
#  mtk_cmd_new_read16 = 0xd0,
#  mtk_cmd_new_read32 = 0xd1,
#  mtk_cmd_new_write16 = 0xd2,
#  mtk_cmd_new_write32 = 0xd4,
#  // mtk_jump_to_da = 0xd5,
#  mtk_jump_to_bl = 0xd6,
#  mtk_get_sec_conf = 0xd8,
#  mtk_send_cert = 0xe0,
#  mtk_get_me = 0xe1, /* Responds with 22 bytes */
#  mtk_send_auth = 0xe2,
#  mtk_sla_flow = 0xe3,
#  mtk_send_root_cert = 0xe5,
#  mtk_do_security = 0xfe,
#  mtk_firmware_version = 0xff,
# usb.data_len > 0 && usb.src == host
# booloader
# d7  
# load address 0x00 00 00 00
# length       0x00 00 00 00
# https://github.com/xobs/fernly


class MTKFirmwareUploader(object):

    MTK_COMMAND_WRITE16 = 0xD2
    MTK_COMMAND_WRITE32 = 0xD4
    MTK_COMMAND_READ32 = 0xD1

    # write 16 bit    0xA1  + Address 32 bit + lentgh (words) + data
    MTK_COMMAND_WRITE16_OLD = 0xA1
    
    # read 16bit    0xA1 + Address 32 bit + length ( words )
    MTK_COMMAND_READ16_OLD = 0xA2

    MTK_COMMAND_DOWNLOAD_IMAGE = 0xD7

    MTK_RUN_CODE = 0xd5
    
    
    DA_ACK = 0x5A
    DA_NACK = 0xA5
    DA_CONT = 0x69
    DA_FLUSH = 0xE2
    
    # internal bootloader upload size
    WRITE_SIZE = 1024
    
    # DA Upload size
    DA_WRITE_SIZE = 0x1000
    
    # NOR Flash data table
    FilenameNorTable = "Download_Agent/6261/NOR_FLASH_TABLE"
    
    # This is the DA Loader program
    FilenameINT_SYSRAM = "Download_Agent/6261/INT_SYSRAM"
    FilenameEXT_RAM = "Download_Agent/6261/EXT_RAM"
    
    UNCHANED_DATA_BLOCKS = 1
    
    
    
    def __init__(self):
        # global flag for EMI test
        self.flagEMI_Ok = False

        
    def checkFilesExit(self):
        
        if not os.path.isfile(self.FilenameNorTable):
            raise Exception('Can not open file: %s') % (self.FilenameNorTable)
        if not os.path.isfile(self.FilenameINT_SYSRAM):
            raise Exception('Can not open file: %s') % (self.FilenameINT_SYSRAM)
        if not os.path.isfile(self.FilenameEXT_RAM):
            raise Exception('Can not open file: %s') % (self.FilenameEXT_RAM)
        
    # open the comport
    def open(self, port):
        
        print ('Try to open port %s. Press ctrl+c for break' % port)
        while 1:
            try:
                self.ser = serial.Serial(port, 115200, timeout=5, dsrdtr=True, rtscts=True)
                print ('Connect to Port %s' % port)
                break
            except:
                time.sleep(0.1)
                continue

    """ connect to the internal bootloader in rom """
    def connectBootloader(self):

        print ('Try to connect bootloader...')
        self.ser.write(struct.pack('B', 0xa0))
        if  self.ser.read(1) != b'\x5F':
            raise Exception('sync lost 1')
        self.ser.write(struct.pack('B', 0x0a)) 
        if self.ser.read(1) != b'\xf5':
            raise Exception('sync lost 2') 
        self.ser.write(struct.pack('B', 0x50))
        if self.ser.read(1) != b'\xaf' :
            raise Exception('sync lost 3')
        self.ser.write(struct.pack('B', 0x05)) 
        if self.ser.read(1) != b'\xfa' :
            raise Exception('sync lost 4')
        print ('Connect to bootloader...')

        
    
    def close(self):
        self.ser.close()
        
    """ write a 16 bit register """
    def write16(self, adr, data):
        pkt = struct.pack('>I', adr)
        
        command = struct.pack('B', self.MTK_COMMAND_WRITE16)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command')
        self.ser.write(pkt)
        # write address        
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address')
        pkt = struct.pack('>I', 0x01)
        # write length         
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length')
        self.ser.read(2)
        pkt = struct.pack('>H', data)
        # write data        
        self.ser.write(pkt)
        if self.ser.read(2) != pkt:
            raise Exception('Invalid response data')
        self.ser.read(2)
        
    """ write a 16 bit register """
    def write16_old(self, adr, data):
        pkt = struct.pack('>I', adr)
        
        command = struct.pack('B', self.MTK_COMMAND_WRITE16_OLD)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command')
        self.ser.write(pkt)
        # write address        
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address')
        pkt = struct.pack('>I', 0x01)
        # write length         
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length')
        
        pkt = struct.pack('>H', data)
        # write data        
        self.ser.write(pkt)
        if self.ser.read(2) != pkt:
            raise Exception('Invalid response data')
          
    
    """ Read a 16 bit register old way"""
    def read16_old(self, adr):
        pkt = struct.pack('>I', adr)
        command = struct.pack('B', self.MTK_COMMAND_READ16_OLD)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command(Read 16 bit)')
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address(Read 16 bit)')
        pkt = struct.pack('>I', 0x01)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length(Read 16 bit)')
        val = self.ser.read(2)
        value, = struct.unpack('>H', val)
        return value

    """ write a 32 bit register """
    def write32(self, adr, data):
        pkt = struct.pack('>I', adr)
        
        command = struct.pack('B', self.MTK_COMMAND_WRITE32)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command')
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address')
        pkt = struct.pack('>I', 0x01)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length')
        self.ser.read(2)
        pkt = struct.pack('>I', data)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response data')
        self.ser.read(2)
        
    """ read a 32 bit register """
    def read32(self, adr):
        pkt = struct.pack('>I', adr)
        
        command = struct.pack('B', self.MTK_COMMAND_READ32)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command')
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address')
        pkt = struct.pack('>I', 0x01)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length')
        
        self.ser.read(2)
        val = self.ser.read(4)
        value, = struct.unpack('>I', val)
        self.ser.read(2)
        return value

    def getSize(self, filename):
        st = os.stat(filename)
        return st.st_size
    
    def DA_WaitForSync(self):
        print ('Wait for DA sync.......')
        time.sleep(0.3)
        value = 0x00
        
        # Get SYNC_CHAR C0 DA return BB = 142 
        #                Version 
        # 0xC0  0x03 0x02 0x8e
       
        while value != 0xC0:
            print ('Wait for 0XC0')
            val = self.ser.read(1)
            value, = struct.unpack('B', val)
        version, = struct.unpack('>H', self.ser.read(2))
        print ('DA_Version 0x%x' % version)
       
        versionBB = struct.unpack('B', self.ser.read(1))
        print ('DA Version BB 0x%d' % versionBB)
        print ('DA sync done')
        if version != 0x302:
            
            raise Exception('Load wrong DA Version....Stop')

    def getFileCrc(self, filename, crcsum=0x0000):
        if not os.path.isfile(filename):
            raise Exception('Filename is wrong')
    
        
        with open(filename, 'rb') as f:
            integer = f.read(2)  # In fact, you could read all in.
            while len(integer) >= 2:
                # if integer is not a string longer than 4, next line crash.
                s, = struct.unpack('>H', integer)  # The return value is a tuple with integers.
               
                crcsum = (crcsum & 0x0000FFFF) ^ s
                integer = f.read(2)
     
        f.close()
        print(('  Checksum of %s is 0x%x') % (filename, crcsum))
        return crcsum

    def getFileCrcByte(self, filename, crcsum=0x0000):
        
        if not os.path.isfile(filename):
            raise Exception('Filename is wrong')
    
      
        with open(filename, 'rb') as f:
            Buffer = f.read()
                        
            for byte in Buffer:
                crcsum = (crcsum + byte) & 0x0000FFFF

        f.close()
        print("  Checksum of {0:s} is 0x{1:x}".format(filename, crcsum))
        return crcsum
    
    
    def getBufferCrc(self, Buffer):               
        crcsum = 0x0000
        for byte in Buffer:
            crcsum = (crcsum + byte) & 0x0000FFFF
        
        #print("Checksum is 0x{0:4x}".format(crcsum))
        return crcsum
    
   
    # upload file
    def writeFile(self, adr, filename):
        
        print ("  Write file {0:s} to address 0x{1:x}".format(filename,adr))
        
        if not os.path.isfile(filename):
            raise Exception('Can not open file: %s') % (filename)
        
        f = open(filename, 'rb')
        
        fileSize = self.getSize(filename) 
        print ("  Bytes to send {0:d}".format(fileSize))
        if fileSize == 0:
            raise Exception('File is empty')
         
        
        command = struct.pack('B', self.MTK_COMMAND_DOWNLOAD_IMAGE)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command(writeFile)')
        
        pkt = struct.pack('>I', adr)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address(writeFile)')
        
        pkt = struct.pack('>I', fileSize)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response length(writeFile)')

        # signature length
        pkt = struct.pack('>I', 0x100)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response dummy(writeFile)')

        self.ser.read(2)
        # write block
        for i in range(fileSize // self.WRITE_SIZE):
            var = f.read(self.WRITE_SIZE)
            self.ser.write(var)
            #print ("Block  {0:2d} send 1024k".format(i))
        
        if fileSize % self.WRITE_SIZE:
            
            var = f.read()
            print ("  Send last data {0:d}".format(len(var)))
            self.ser.write(var)
        # read CRC
        crc = self.ser.read(2)
        crc_val, = struct.unpack('<H', crc)
        print ("  Upload CRC 0x{0:x}".format(crc_val))
        self.ser.read(2)
        return crc_val
           
    def startProg(self, adr):
        print (' ')
        print ("--------------------------------------")
        print ("Start program at 0x{0:x}. good luck".format(adr))
        print ("--------------------------------------")
        command = struct.pack('B', self.MTK_RUN_CODE)
        self.ser.write(command)
        if self.ser.read(1) != command:
            raise Exception('Invalid response command start')
        pkt = struct.pack('>I', adr)
        self.ser.write(pkt)
        if self.ser.read(4) != pkt:
            raise Exception('Invalid response address')   
        self.ser.read(2)
        print ("------------------")
        print ("Program is running")
        print ("------------------")
        print (" ")

    # Doing the internal PSRAM Calibration
    # 
    def BL_EMI_PSRAM_Calibration(self):

        print ('BL_EMI_PSRAM_Calibration')
        
        # 338  External memory block
        # https://github.com/xobs/fernly/blob/master/include/fernvale-emi.h
        self.write32(0xa0050090, 0x0)
        # 358
        self.write32(0xa0050160, 0x13030000)
        # 378
        # EMI_CTRL_ODLA
        self.write32(0xa00500f8, 0x0)
        # 398
        # EMI_CTRL_ODLD
        self.write32(0xa0050100, 0x0)
        # 418
        # EMI_CTRL_ODLE
        self.write32(0xa0050108, 0x0)
        # 438
        # EMI_CTRL_ODLF
        self.write32(0xa0050110, 0x0)
        # 458
        # EMI_CTRL_CONM
        self.write32(0xa0050060, 0x1001)
        # 478
        # EMI_CTRL_GENA
        self.write32(0xa0050070, 0x11)
        # 498
        # EMI_CTRL_RDCT
        self.write32(0xa0050088, 0x01)
        # 518
        # EMI_CTRL_IOA 
        self.write32(0xa0050130, 0x10001)
        # 541
        # EMI_CTRL_IOB
        self.write32(0xa0050138, 0x10001)
        # 558
        self.write32(0xa0050150, 0x5000)
        # 578
        self.write32(0xa0050158, 0x1001)
        # 598
        # EMI_CTRL_DSRAM
        self.write32(0xa0050090, 0x0)
                
        EMI_Start = 0x1f1f1f1f
        EMI_Next  = 0x01010101
        
        self.flagEMI_Ok = False
        for x in range(0, 31):
            #print ("EMI 0x{0:08x} ".format(EMI_Start))
            # EMI_CTRL_IDLC
            self.write32(0xa00500d0, EMI_Start)
            # 872
            # EMI_CTRL_IDLD
            self.write32(0xa00500d8, EMI_Start)
            # 892
            # EMI_CTRL_IDLE 
            self.write32(0xa00500e0, 0x0)
            # 912
            self.write32(0xa0050300, 0x0)
            # 932
            # Reset MBIST engine 
            self.write32(0xa0050300, 0x0)
            # 952
            # Set range (start and length) 0x2000 003f
            #  3F00 = 16k 
            #
            self.write32(0xa0050308, 0x2000003f)
            # 972
            # load test pattern 0xA55A
            self.write32(0xa0050300, 0xa55a0000)
            # 992 0xa55a0000
            value = self.read32(0xa0050300)
            # 1010
            # EMI_CTRL_MBISTB_START_KEY 0x325
            # EMI_CTRL_MBISTB_RANDOMIZE (1 << 12)
            # EMI_CTRL_MBISTB_INVERT (1 << 13)
            # Kick off the test 
            self.write32(0xa0050300, 0xa55a0325)
            # 1033  0x03
            
            if self.waitForReadyAndGetTest() == True:
                self.timingOk()
            EMI_Start = EMI_Start - EMI_Next
                     
            # reset MBIST engine
            self.write32(0xa0050300, 0x0)
               
            # loop 
        if  self.flagEMI_Ok == False:
            raise Exception('EMI Training fail...... stop')
        
        print ('BL_EMI_PSRAM_Calibration')
            
    def timingOk(self):  
        print ('timingOk')
        # reset MBIST engine
        self.write32(0xa0050300, 0x0)   
        # To Do Fixme   
        self.write32(0xa0050308, 0x2000003f)
        self.write32(0xa0050300, 0xa55a2000)
        self.read32(0xa0050300)
        # kick off test INVERT
        self.write32(0xa0050300, 0xa55a2325)
        # 0x03
        if self.waitForReadyAndGetTest() == True:
            self.timingDeepTesting()
        # 7993
        # reset MBIST engine
        self.write32(0xa0050300, 0x0)
        
        
    # Doing deep pattern testing    
    def timingDeepTesting(self):   
        
        print ('timingDeepTesting') 
        # To Do Fixme 
        self.write32(0xa0050308, 0x2000003f)
        # 8160
        self.write32(0xa0050300, 0xffff0000)
        # 8189
        self.read32(0xa0050300)
        # 8202
        # kick off test
        self.write32(0xa0050300, 0xffff0325)
        # 8218 0x3ffc0002
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 1 fail ')
        
        # 8254
        self.write32(0xa0050300, 0x0)
        self.write32(0xa0050308, 0x2000003f)
        self.write32(0xa0050300, 0xffff2000)
        self.read32(0xa0050300)
        # kick off test RANDOMIZE  and  INVERT
        # kick off test INVERT
        self.write32(0xa0050300, 0xffff2325)
        # 0x3ffc0002
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 2 fail ')
    
    
        # 8391
        
        self.write32(0xa0050300, 0x0)
        self.write32(0xa0050308, 0x2000003f)
        self.write32(0xa0050300, 0xa55a1000)
        self.read32(0xa0050300)
        # kick off test RANDOMIZE
        self.write32(0xa0050300, 0xa55a1325)
        # 0x3ffc0002
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 3 fail ')
         
        # 8522
        self.write32(0xa0050300, 0x0)
         # 8542
        self.write32(0xa0050308, 0x2000003f)
         # 8562
         # load pattern
        self.write32(0xa0050300, 0xa55a3000)
         # 8582  0xa55a3000
        self.read32(0xa0050300)
         # 8600
         # kick off test RANDOMIZE and INVERT
        self.write32(0xa0050300, 0xa55a3325)
         # 8620 0x3ffc002
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 4 fail ')
          
        # 8659
        self.write32(0xa0050300, 0x0) 
        # 8679
        self.write32(0xa0050308, 0x2000003f)
        # 8696
        self.write32(0xa0050300, 0xffff1000)
        # 8716 0xffff1000
        self.read32(0xa0050300)
        # 8734
        # kick off RANDOMIZE
        self.write32(0xa0050300, 0xffff1325)
        # 8757 0x3ffc0002
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 5 fail ')
        
        # 8793
        self.write32(0xa0050300, 0x0)
        # 8810
        self.write32(0xa0050308, 0x2000003f)
        # 8833 
        self.write32(0xa0050300, 0xffff3000)
        # 8853 0xffff3000
        self.read32(0xa0050300)
        # 8868
        # kick off RANDOMIZE and INVERT
        self.write32(0xa0050300, 0xffff3325)
        if self.waitForReadyAndGetTest() == False:
            raise Exception('Pattern 6 fail ')
        
        self.flagEMI_Ok = True
     
    def waitForReadyAndGetTest(self):
       
        # EMI_CTRL_MBISTD (EMI_CTRL_ADDR + 0x318)
        # EMI_CTRL_MBISTD_FAILURE (1 << 0)
        # EMI_CTRL_MBISTD_FINISHED (1 << 1)
        
        # wait for ready
        EMI_CTRL_MBISTD_FAILURE = 1
        EMI_CTRL_MBISTD_FINISHED = 2
        
        # wait for ready
        while 1:
            flag = self.read32(0xa0050318)
            
            if flag & EMI_CTRL_MBISTD_FINISHED:
                break
                   
        val = self.read32(0xa0050318)  
        if val & EMI_CTRL_MBISTD_FAILURE :
            # test fail
            return False
        # test ok
        return True 
     

    def retrySend(self, var):
        while(1):
            time.sleep(1)
            print ('      wait for DA ready')
            ack = self.ser.read(1)
            if len(ack) == 0:
                raise Exception('No response')
            gotAck, = struct.unpack('B', ack)
            print ('      Got %x' % gotAck)
            if gotAck == self.DA_FLUSH:
                break
        
        print ('      DA flush ready:')
        self.ser.write(struct.pack('B', self.DA_CONT))
         # send again
        self.ser.write(struct.pack('B', self.DA_ACK))
       
        self.ser.write(var)
        crc = self.getBufferCrc(var)
        self.ser.write(struct.pack('>H', crc))
        val, = struct.unpack('B', self.ser.read(1))
        print ('      Ack: %x' % val)
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_CONT:
            raise Exception('Send CRC error')
        return crc, val, gotAck

    def DA_LoadImage(self, filename, wt):
        print ('  DA_LoadImage')
        
        print ('    Write file %s' % filename)
        
        if not os.path.isfile(filename):
            raise Exception('Can not open file: %s') % (filename)
        
        f = open(filename, 'rb')
        
        fileSize = self.getSize(filename) 
        print ('    Bytes to send %d' % (fileSize))
        if fileSize == 0:
            raise Exception('File is empty')
        # send a 4 k block
        print ('    Sending...')
        for i in range(fileSize // self.DA_WRITE_SIZE):
            #time.sleep(1)
            var = f.read(self.DA_WRITE_SIZE)
            # print var.encode("hex")
           
            self.ser.write(struct.pack('B', self.DA_ACK))  
            self.ser.write(var)
            
            #print ("Block  %d send 4k" % i)
            crc = self.getBufferCrc(var)
            self.ser.write(struct.pack('>H', crc))
            val, = struct.unpack('B', self.ser.read(1))
            if  val != self.DA_CONT:
                print ('    Got Error: %x' % val)
                errorCode, = struct.unpack('>I', self.ser.read(4))
                print ('    Error code: %x' % errorCode)
                self.retrySend(var)                                    

        if fileSize % self.DA_WRITE_SIZE:
            var = f.read(fileSize % self.DA_WRITE_SIZE)
            crc = self.getBufferCrc(var)
            print ('    Send last data %d' % len(var))
            self.ser.write(struct.pack('B', self.DA_ACK))  
            self.ser.write(var)
            
            # wait some time ??
            #time.sleep(1)
            self.ser.write(struct.pack('>H', crc))
            if wt == 1:
                val, = struct.unpack('B', self.ser.read(1))
                print ('    response: %x' % val)
                if val != self.DA_CONT:
                    print ('    Error: %x' % val)
                    errorCode, = struct.unpack('>I', self.ser.read(4))
                    print ('    Error code: %x' % errorCode)
                    self.retrySend(var)

        f.close()   
        print ('  DA_LoadImage done')
        
    def DA_CheckEFuse(self):
        print (' ')
        print ('Check Efuse backup data...')
        
            
        # 10630
        # da_check_efuse_backup_data: maker (0xabababab), efuse_backup_data_1 (0x00013072), efuse_backup_data_2 (0x0001d88c)
        self.ser.write(b"\x75")
        val, = struct.unpack('>I', self.ser.read(4))
        print ('  maker 0x%x' % val)
        val, = struct.unpack('>I', self.ser.read(4))
        print ('  efuse_backup_data_1 0x%x' % val)
        val, = struct.unpack('>I', self.ser.read(4))
        print ('  efuse_backup_data_2 0x%x' % val)
    
    def DA_CBRInfo(self):
        print (' ')
        print ('DA_CBRInfo')
        # Write NOR images...
        # get_CBRinfo...
        # 10656
        self.ser.write(b"\x52")
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_NACK:
            print ('val: 0x%x' % val)
            raise Exception('no nack')
        
        # cbr not found
        val, = struct.unpack('>I', self.ser.read(4))
        print (('  Code: %d') % val)
        # should be ok
        
    
    def DA_SetMemBlock(self, FileName1, FileName2):  
        print (' ')
        print ('DA_SetMemBlock')
        # 10668
        # SV5_CMD_SetMemBlock...
        # send MEM_BEGIN_ADDR(0x00020000)
        # send MEM_END_ADDR  (0x001D60CF)
        
        fileSize1 = self.getSize(FileName1)
        fileSize2 = self.getSize(FileName2)
        print ('  %s size: 0x%x' % (FileName1, fileSize1))
        print ('  %s size: 0x%x' % (FileName2, fileSize2))
        # send command
        self.ser.write(b"\xd3")
        
        # send NACK ???
        self.ser.write(b"\xa5")
        
        # block count  write 2 images
        self.ser.write(b"\x02")
        
        # FIX ME  Is this dynamic ???
        startAddress1 = 0x20000
        #startAddress1 = 0x10020000
        print ('  startAddress1 0x%x ' % startAddress1)
        # Mem begin Address  address  image 1  ROM
        self.ser.write(struct.pack('>I', startAddress1))
        # Mem end Address
        endAddress1 = startAddress1 + fileSize1 - 1
        print ('  endAddress1 0x%x' % endAddress1)
        self.ser.write(struct.pack('>I', endAddress1))
        # Type
        self.ser.write(b"\x00\x00\x01\00")
        
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
           
            error, = struct.unpack('>I', self.ser.read(4))
            print ('  Error Code : %d' % error)
            raise Exception('no ack')
        
        # fix me is this dynamic
        startAddress2 = 0x001D64D0
        #startAddress2 = 0x101D64D0
        print ('  startAddress2 0x%x' % startAddress2)
        # send MEM_BEGIN_ADDR(0x001D64D0)  address of image 2  VIVA
        self.ser.write(struct.pack('>I', startAddress2))
        
        # send MEM_END_ADDR(0x004D7BBB)
        endAddress2 = startAddress2 + fileSize2 - 1
        print ('  endAddress2 0x%x' % endAddress2)
        self.ser.write(struct.pack('>I', endAddress2))
        
        # send IMAGE_TYPE(0x00000108)
        self.ser.write(b"\x00\x00\x01\x08")
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            print ('  val: 0x%x' % val)
            raise Exception('no ack')
        
        
        
        # UNCHANED_DATA_BLOCKS=(0x02)
        self.UNCHANED_DATA_BLOCKS, = struct.unpack('B', self.ser.read(1))  # 0x02
        print ('  UNCHANED_DATA_BLOCKS %d' % self.UNCHANED_DATA_BLOCKS)
        val, = struct.unpack('B', self.ser.read(1))
        if val == self.DA_ACK:
            print ('  Got ACK: 0x%x' % val)
            print ('  Format...') 
                         
            # 10695
            # bin index = 0 format time = 27
            # get format time
            formatTime, = struct.unpack('>I', self.ser.read(4))  # format time = 27
            print ('  Format time: %d' % formatTime)
            # 10703
            
            # Wait bin[0] format time 0/27
            # Wait bin[0] format time 1/27
            while formatTime > 0:
                val = self.ser.read(1)
                formatTime = formatTime - 1
                #print ('formatTime: 0x%x' % formatTime)
            
            # 10757   
            # bin index = 1 format time = 48
            formatTime, = struct.unpack('>I', self.ser.read(4))  # format time = 48
            print ('  Format time: %d' % formatTime)
            # Wait bin[1] format time 0/48
            # Wait bin[1] format time 1/48#
            while formatTime > 0:
                val, = struct.unpack('B', self.ser.read(1))
                formatTime = formatTime - 1
                #print ('formatTime: 0x%x' % formatTime)
        
        val, = struct.unpack('B', self.ser.read(1))
        print ('  Format Val 0x%x' % val)
        
        print ('DA_SetMemBlock done')
        
    def  DA_ERASE_MAUI_INFO(self):   
        print (' ')
        print ('DA_ERASE_MAUI_INFO')
        # SV5_CMD_ERASE_MAUI_INFO...
        # Send DA_ERASE_MAUI_INFO(0x5B).
        # 10862
        # send command
        self.ser.write(b"\x5B")
        # get the target_MauiInfo_addr: 0x0
        target_MauiInfo_addr, = struct.unpack('>I', self.ser.read(4))
        print ('  target_MauiInfo_addr: 0x%x' % target_MauiInfo_addr)
        
        # get the target_rom_addr: 0x0
        target_rom_addr, = struct.unpack('>I', self.ser.read(4))
        print ('  target_rom_addr: 0x%x' % target_rom_addr)
        
        # get the target_blk_addr: 0x20000
        target_blk_addr, = struct.unpack('>I', self.ser.read(4))
        print ('  target_blk_addr: 0x%x' % target_blk_addr)
        
        # Target address is zero. Do nothing!
        if target_rom_addr != 0x0000:
            print ('  Target address is not zero. Fix me ...')
            val, = struct.unpack('>I', self.ser.read(4))
            print("  address 0x{x}".format(val)) 
            val, = struct.unpack('B', self.ser.read(1))
            if val != self.DA_ACK:
                raise Exception('DA_ERASE_MAUI_INFO no ack')
        print ('DA_ERASE_MAUI_INFO done')
         
    def DA_WriteCMD(self, FileName1, FileName2):
        print (' ')
        print ('DA_WriteCMD' )
        # 10888
        # Send DA_WRITE_CMD(0xD5).
        # send command
        self.ser.write(b"\xD5")
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            print ('  val 0%x' % val)
            raise Exception('DA_WRITE_CMD(0xD5) no ack')
    
        # Packet Length: 4096
        self.ser.write(b"\x01")
        self.ser.write(b"\x00\x00\x10\x00")
        
        time.sleep(0.1)
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('no ack')
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('no ack')
              
       
        # download..
        # 10903  438 pakets
        self.DA_LoadImage(FileName1,1)
        print(' ')
        
         # 14415
        self.DA_LoadImage(FileName2,1)
        
        # Sleep(4ms)=UNCHANED_DATA_BLOCKS(2)x32KWx70ns.
        
        wait_time = self.UNCHANED_DATA_BLOCKS * 32.0 * 1024.0 * (7.0 / 100000.0);
        
        print (' ')
        print ('  Wait time %d' % wait_time)
        print ('  program nor flash. Wait 10s....')
        
        # FIX ME  can we calculate this ?
        time.sleep(10)
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            print (val)
            raise Exception('no ack')
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('no ack')
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('no ack') 
        
            
        print(' ')
        crc_FileName1 = self.getFileCrcByte(FileName1)
               
        # 20579
        # verify image checksum. for normal chip
        # Verifying PRIMARY_MAUI
        # Sending ckecksum (0x721E)                     # 20579 
        self.ser.write(struct.pack('>H', crc_FileName1))
        
        # Checksum matched (image index = 0)
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('Checksum (image index = 0) do not match')
                
        crc_image1 = self.getFileCrcByte(FileName2)
               
        # Verifying VIVA
        # Sending ckecksum (0x3243)                     # 20583
        self.ser.write(struct.pack('>H', crc_image1))
        
        # Checksum matched (image index = 1)
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception(' Checksum (image index = 1) do not match')
        print ('DA_WriteCMD done' )
  
    # detect the internal flash
    # using the flash information table from file
    def DA_DetectFlash(self):
        
        print ('DA_DetectFlash')
       
        # 9829
        self.ser.write(b"\xa5")
        # Version
        self.ser.write(b"\x05")
        # Version
        self.ser.write(b"\xfe")
        
        # Chip select
        self.ser.write(b"\x00")
        self.ser.write(b"\x08")
        self.ser.write(b"\x00")
        
        # NFI setting
        self.ser.write(b"\x70\x07\xff\xff")
        
        # 27Mhz Clock
        self.ser.write(b"\x02")
        
        print ('  NOR/SF flash detect')
        print ('  This part could fail if a unknown NOR flash is in place')
        print ('  -------------------------------------------------------')
        # flash table size 259
        self.ser.write(b"\x00\x00\x01\x03")   
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('flash table size. get no ack')
        
        print ('  Read nor flash table from disk %s' % self.FilenameNorTable)
        f = open(self.FilenameNorTable, 'rb') 
        filename_nor_table_size = self.getSize(self.FilenameNorTable)
        
        nor_flash_count = filename_nor_table_size // 36 - 1
        
        found_flag = False
        
        print ('  nor_flash_count %d' % nor_flash_count)
    
        for x in range(0, nor_flash_count):   
            nor_flash_data = f.read(36)
            if nor_flash_data == '':
                break
            #print (nor_flash_data)
            self.ser.write(nor_flash_data)
            val, = struct.unpack('B', self.ser.read(1))
            if val == self.DA_ACK:
               
                print ('  Nor flash found')
                found_flag = True
                break
        
        if(found_flag != True):
            raise Exception('Do not found nor flash inside the device. stop.....')
              
        val = self.ser.read(2)  
        #print (val)
        # 9899
        # 0x5a 0xa5 0x69
        
        print ('  Detect Nand flash')
        # 9904 host
        # 0x00 00 00 00
        # Table size is null. so we have no nand ????
        command = b"\x00\x00\x00\x00"
        self.ser.write(command)
        
        # 9907
        # 10301
        print ('  Get Nor Nand flash report from target. wait ....')
        
        
        var = self.ser.read(394)
        #print (var)
        print ('DA_DetectFlash done')

    def DA_FormatCBR(self):
        print (' ')
        print ('Format CBR')
        # 10302
        # Format CBR
        self.ser.write(b"\x56")
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('DA_FormatCBR. get no ack')
    
    def DA_DownloadBootLoader(self, fileName1, fileName2):
        
        print (' ')
        print ('DA_DownloadBootLoader')
        
        SizeFileName1 = self.getSize(fileName1)
        SizeFileName2 = self.getSize(fileName2)
        print ('  %s size: 0x%x' % (fileName1, SizeFileName1))
        print ('  %s size: 0x%x' % (fileName2, SizeFileName2))
          
        # 10412
        # self.DA_LoadImage(self.FilenameBootloaderExt)          
        # 10309
        # Format HB    set serial flash
        self.ser.write(b"\x55\07")
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('DA_DownloadBootLoader. get no ack')
        
        # Download Bootloader
        self.ser.write(b"\x51")
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('DA_DownloadBootLoader. get no ack')
        
        # da_write_boot_loader:: send PACKET SIZE(0x00001000)
        # 10316
        self.ser.write(b"\x00\x00\x10\x00")
            
        # da_write_boot_loader send Flash Device Type(0x07)
        self.ser.write(b"\x07")
        # da_write_boot_loader  check Flash Device Type... response 0x5a
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception(' check Flash Device Type... no ack')
        # da_write_boot_loader check Header Block response 0x5a
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception(' check Header Block response. no ack')
        
        # da_write_boot_loader:: 1st download ...
        # da_write_boot_loader: send BL Numers(0x00000002).    #10324
        self.ser.write(b"\x00\x00\x00\x02")

        # da_write_boot_loader ask DA to format HB (0x5A)
        self.ser.write(struct.pack('B', self.DA_ACK))
        self.ser.write(struct.pack('B', self.DA_ACK))
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception(' da_write_boot_loader ask DA to format HB. no ack')
  
        # da_write_boot_loader : send BL[0] - BL_EXIST_MAGIC(0x5A).
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - BL_DEV(0x07). 
        self.ser.write(b"\x5A\x07")
        
        # da_write_boot_loader send BL[0] - BL_TYPE(0x0001).
        self.ser.write(b"\x00\x01")
        
        # da_write_boot_loader:: send BL[0] - BL_BEGIN_DEV_ADDR(load addr)(0x70006000).
        # SEEED02A_DEMO_BOOTLOADER_V005_MT2502_MAUI_11CW1418SP5_W15_19.bin
        
        # FIX ME Can this change ???
        startAddress1 = 0x70006000
        print ('  startAddress1 0x%x' % startAddress1)
        self.ser.write(b"\x70\x00\x60\x00")
        # da_write_boot_loader:: send BL[0] - BL_BOUNDARY_DEV_ADDR(load addr+file_len)(0x70007C68).
        endAddress1 = startAddress1 + SizeFileName1 - 1
        print ('  endAddress1 0x%x' % endAddress1)
        self.ser.write(b"\x70\x00\x7C\x68")
        # da_write_boot_loader::: send BL[0] - BL_ATTRIBUTE(0x00000001)
        self.ser.write(b"\x00\x00\x00\x01")
        # da_write_boot_loader:::  send BL[0] - BL_MAX_SIZE(0xFFFFFFFF)
        self.ser.write(b"\xFF\xFF\xFF\xFF")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - AC_C enable (0).
        self.ser.write(b"\x00")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - AC Final Offset (0) = FILE_CONTENT_OFFSET (0x00000140) + AC Offset (0).
        self.ser.write(b"\x00\x00\x01\x40")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - AC Length (0).
        self.ser.write(b"\x00\x00\x00\x00")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - BL_SIZE(0x00001C68).
       
        self.ser.write(b"\x00\x00\x1C\x68")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - BL_RESERVED_BLOCK(0).
        self.ser.write(b"\x00")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - BL_ALIGN_TYPE(0)
        self.ser.write(b"\x00")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[0] - BL_FILEINFO_ATTR(0x01).
        self.ser.write(b"\x00\x00\x00\x01")
        # DA_cmd::CMD_DownloadBootLoader(): wait for ACK.DA_cmd::CMD_DownloadBootLoader(): ACK(0x5A) OK!
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception(' CMD_DownloadBootLoader(): wait for ACK.DA_cmd::CMD_DownloadBootLoader(). no ack')
                
        # da_write_boot_loader : send BL[1] - BL_EXIST_MAGIC(0x5A)
        self.ser.write(struct.pack('B', self.DA_ACK))
                 
        # 10365
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - BL_DEV(0x07).  
        self.ser.write(b"\x07") 
         # da_write_boot_loader send BL[1] - BL_TYPE(0x0002).
        self.ser.write(b"\x00\x02")
        # da_write_boot_loader:: send BL[1] - BL_BEGIN_DEV_ADDR(load addr)(0x10003000).
        # EXT_BOOTLOADER
        # FIX ME Can this change ???
        startAddress2 = 0x10003000
        print ('  startAddress2 0x%x' % startAddress2)
        self.ser.write(b"\x10\x00\x30\x00")
        # da_write_boot_loader:: send BL[1] - BL_BOUNDARY_DEV_ADDR(load addr+file_len)(0x1001CF20)
        endAddress2 = startAddress2 + SizeFileName2 - 1
        print ('  endAddress2 0x%x' % endAddress2)
        self.ser.write(b"\x10\x01\xCF\x20")
        # da_write_boot_loader::: send BL[1] - BL_ATTRIBUTE(0x00000000)
        self.ser.write(b"\x00\x00\x00\x00")
        # da_write_boot_loader:::  send BL[1] - BL_MAX_SIZE(0xFFFFFFFF).
        self.ser.write(b"\xFF\xFF\xFF\xFF")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - AC_C enable (0).
        self.ser.write(b"\x00") 
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - AC Final Offset (0) = FILE_CONTENT_OFFSET (0x00000060) + AC Offset (0).
        self.ser.write(b"\x00\x00\x00\x60")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - AC Length (0).
        self.ser.write(b"\x00\x00\x00\x00")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - BL_SIZE(0x00019F20).
        self.ser.write(b"\x00\x01\x9F\x20")
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - BL_RESERVED_BLOCK(0).
        self.ser.write(b"\x00") 
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - BL_ALIGN_TYPE(0).
        self.ser.write(b"\x00") 
        # DA_cmd::CMD_DownloadBootLoader(): send BL[1] - BL_FILEINFO_ATTR(0x03).
        self.ser.write(b"\x00\x00\x00\x03")
        # DA_cmd::CMD_DownloadBootLoader(): wait for ACK.DA_cmd::CMD_DownloadBootLoader(): ACK(0x5A) OK!
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise(' wait for ACK.DA_cmd::CMD_DownloadBootLoader(). no ack')
        
        # DA_cmd::CMD_DownloadBootLoader(): Set BL Profiles - wait for ACK.
        
        # DA_cmd::CMD_DownloadBootLoader(): Set BL Profiles - ACK(0x5A) OK!
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise(' Set BL Profiles. no ack')
        # DA_cmd::CMD_DownloadBootLoader(): BL Self Update Check - wait for ACK.
        # DA_cmd::CMD_DownloadBootLoader(): BL Self Update Check - ACK(0x5A) OK!
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            raise(' BL Self Update Check. no ack')
        
        #10396  begin upload !!!!!!!!!!!!!!!
        # INT_BOOTLOADER
        print (' ')
        self.DA_LoadImage(fileName1,1)
        
        # 10412
        # EXT_BOOTLOADER
        print (' ')
        self.DA_LoadImage(fileName2,1)
        
        # 10619
        # 0x5a 00 00 00 02
        # da_write_boot_loader download finish response is 0x5a
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            print ('    Error1: %x' % val)
            raise(' download finish response. no ack')
        # da_write_boot_loader download finish response status is 0x0002
        val, = struct.unpack('>I', self.ser.read(4))
        if val != 0x0002:
            print ('    Error1: %x' % val)
            raise Exception('Wrong response status != 0x0002')
        print ('DA_DownloadBootLoader done')
               
            
    def DA_doFATParition(self):
        
        print (' ')
        print ('DA_doFATParition')
        print ('----------------')
        # Fixme
        # DA_FORMAT_FAT_CMD(0xB8)
        self.ser.write(b"\xB8")
        self.ser.write(b"\x01")
        self.ser.write(b"\x00")
        
        # 0x00 00 00 00
        val, = struct.unpack('>I', self.ser.read(4))
        if val != 0x0000:
            print ('Error %d' % val)
            raise Exception('FAT Format fail...stop')
        
        # FAT range : Start address = 0x00500000
        val, = struct.unpack('>I', self.ser.read(4))
        print ('  Nor FAT range : Start address = 0x%x' % val)
        # FAT range : Length        = 0x00afe000.
        val, = struct.unpack('>I', self.ser.read(4))
        print ('  Nor FAT range : Length        = 0x%x' % val)
        # 0x00 00 00 00
        
        # range ????
        val, = struct.unpack('>I', self.ser.read(4)) 
        val, = struct.unpack('>I', self.ser.read(4))
        
        # range ????
        val, = struct.unpack('>I', self.ser.read(4))
        val, = struct.unpack('>I', self.ser.read(4))
        
        
        # Start to Format FAT operation.
        
        # 5a 00
        # Format FAT is Done.         
        # 0b cd 01
        
        # loop ????
        print ('  Wait for ready...')
        while 1:
            val, = struct.unpack('>B', self.ser.read(1))
            if val == self.DA_ACK:
                break
                   
        print ('  Wait for ready2...')
        while 1:
            val, = struct.unpack('>B', self.ser.read(1))
            if val == self.DA_ACK:
                break
        
        print ('  Wait for format done...' )  
        while 1:
            status, = struct.unpack('>I', self.ser.read(4))
            #print (('0x%x') % status)
            if status == 0xBCD or status == 0x00:
                 val, = struct.unpack('B', self.ser.read(1))
            else:
                val, = struct.unpack('>I', self.ser.read(4))
                print ('fail address: 0x%' % val)
                val, = struct.unpack('>I', self.ser.read(1))
                if val == 0x69:
                    continue
                raise Exception('Format fail. Stop...')
            
            self.ser.write(struct.pack('B', self.DA_ACK))
            #print ('Formating.. 0x%x' % status)
            if  status == 0x00:
                break
       
        val, = struct.unpack('>B', self.ser.read(1))
        if val != self.DA_ACK:
            raise Exception('Format fail. no ack...')
             
        
        print ('DA_doFATParition done')
    
    def WaitForBBPUReady(self):
        print ('wait for ready BBPU ...')
        while  1:
            # wait for ready
            val = self.read16_old(0xa0710000)
            # print '0xa0710000 0x%x' % val 
            if val & 0x0040 == 0:
                break
        
    def BL_PowerUpBaseband(self):
        print ('BL_PowerUpBaseband')
        # 9072 0x8
        # 0xa0710000  RTC reg
        # Baseband power up
        val = self.read16_old(0xa0710000)
        #print ('0xa0710000 0x%x' % val)
        # 9086 0x0
        # read power key
        val = self.read16_old(0xa0710050)
        #print ('0xa0710050 0x%x' % val)    
        # 9100 0x0
        # read power key
        val = self.read16_old(0xa0710054)
        #print ('0xa0710054 0x%x' % val)
        # 9114
        # RTC alarm mask
        self.write16(0xa0710010, 0x0)
        
        # 9134
        # RTC IRQ enable
        self.write16(0xa0710008, 0x0)
        
        # 9154
        # Counter increment IRQ enable
        self.write16(0xa071000c, 0x0)
        
        # 9174
        self.write16(0xa0710074, 0x1)
        
        # 9194 0x8
        self.WaitForBBPUReady()
        
        # 9208
        # power key
        self.write16(0xa0710050, 0xa357)
        
        # 9228 
         # power key
        self.write16(0xa0710054, 0x67d2)
        
        # 9248
        self.write16(0xa0710074, 0x01)
        
        # 9268 0x08
        self.WaitForBBPUReady()
        
        # 9285
        self.write16(0xa0710068, 0x586a)
        
        # 9302
        self.write16(0xa0710074, 0x1)
        
        # 9322 0x08    
        self.WaitForBBPUReady()
        
        # 9336
        self.write16(0xa0710068, 0x9136)
        
        # 9356
        self.write16(0xa0710074, 0x1)
        
        # 9376 0x08
        self.WaitForBBPUReady()
        
        # 9390
        self.write16(0xa0710000, 0x430e)
        
        # 9410
        self.write16(0xa0710074, 0x1)
        
        # 9430 0x0e
        self.WaitForBBPUReady()    
    
    def BL_RemapEMI(self):
        print ('BL_RemapEMI')
        
        # 9444 0x0
        # Remap EMI
        val = self.read32(0xa0510000)
        #print ('0xa0510000 0x%x' % val) 
        val &= 0xFFFFFFFC;
        val |= 0x00000002;  # set bits
        # 9462
        self.write32(0xa0510000, val)
        
        # 9482 0x02
      
        val = self.read32(0xa0510000)
        #print ('0xa0510000 0x%x' % val)
        val &= 0xFFFFFFFC;
        val |= 0x00000002;
        # 9500
        self.write32(0xa0510000, val)
        # Why doing this twice ??
    
    
    # Upload the DA-Loader to Internal and PSRAM memory
    # Execute the uploaded DA-Loader
    def BL_UploadAndStartDA_Bootloader(self):
        
        print ('BL_UploadAndStartDA_Bootloader')
        print ('------------------------------')
        # 9520 Upload
    
        # 9538 Upload INT_SYSRAM  to 0x70007000
        # internal TCM Memory
        crc_file = self.getFileCrc(self.FilenameINT_SYSRAM)
        crc_upload = self.writeFile(0x70007000, self.FilenameINT_SYSRAM)
        if crc_file != crc_upload:
            print ('CRC is wrong INT_SYSRAM')
            raise Exception('CRC is wrong INT_SYSRAM')
    
        # 9546 Upload EXT_RAM  to 0x10020000 
        # PSRAM inside the chip
        crc_file = self.getFileCrc(self.FilenameEXT_RAM)
        crc_upload = self.writeFile(0x10020000, self.FilenameEXT_RAM)
        if crc_file != crc_upload:
            print ('CRC is wrong EXT_RAM')
            raise Exception('CRC is wrong EXT_RAM')
        
        # 9564 Upload EXT_RAM
    
        # 9810 Start the DA Bootloader
        self.startProg(0x70007000)   
          
    def DA_disconnect(self):
        print (' ')
        print ('DA_disconnect')
        self.ser.write(b"\xD9")
                       
        self.ser.write(b"\x00\x00\x00\x00")
        
        
        val, = struct.unpack('B', self.ser.read(1))
        if val != self.DA_ACK:
            print ('  val 0%x' % val)
            raise Exception('DA_disconnect. no ack')
        print ('DA_disconnect done')
             
        
    def flushCom(self):
        self.ser.flushInput()
        self.ser.flushOutput()
    

def main():


    parser = argparse.ArgumentParser(description='Firmware uploader for Rephone', prog='uploader')
    parser.add_argument('--port', '-p', help='Serial port device', default='/dev/ttyUSB0')
    parser.add_argument('--firmPath', '-f', help='Firmware path', default='RePhone')
    parser.add_argument('--nobattery', '-nobat',help='Upload without battery', action="store_true")
    parser.add_argument('--native', help='Upload a test binary and execute it', action="store_true")
    parser.add_argument('--nofatformat', help='Do not format the FAT partition', action="store_true")
    
    
    args = parser.parse_args()
     
    FirmwarePath = args.firmPath
   
    
      # This is the Bootloader
    FilenameBootloader = FirmwarePath + '/INT_BOOTLOADER'
    FilenameBootloaderExt = FirmwarePath + '/EXT_BOOTLOADER'
    
    # this is the Firmware
    FilenameROM1 = FirmwarePath + '/ROM'
    FilenameROM2 = FirmwarePath + '/VIVA' 
    
    
    if args.native == False:
        if not os.path.isfile(FilenameBootloader):
                raise Exception('Can not open file: %s') % (FilenameBootloader)
        if not os.path.isfile(FilenameBootloaderExt):
                raise Exception('Can not open file: %s') % (FilenameBootloaderExt)
        if not os.path.isfile(FilenameROM1):
                raise Exception('Can not open file: %s') % (FilenameROM1)
        if not os.path.isfile(FilenameROM2):
                raise Exception('Can not open file: %s') % (FilenameROM2)
            
   
    print(' ')
    print ('Setup the right udev rules.')
    print(' ')
    
    if args.nobattery == True:
        print("==========================================================")
        print("Doing upload in 'without battery' mode.")   
        print("Disconnect the battery and connect device to the USB port.")
        print("==========================================================")
    else:
        print("=====================================================================================")
        print("Connect the battery to the device.") 
        print("Switch off the device and connect it to the USB port.")  
        print("The first connect sometimes fails. Ubuntu mounts a USB Mass Storage device (5.2 MB ).")  
        print("Disconnect and  reconnect the device to the USB port.")
        print("or use the --nobattery option")
        print("=====================================================================================")
     
        
    h = MTKFirmwareUploader()
    
    if args.native == False:
        h.checkFilesExit()
      
    h.open(args.port)
    h.connectBootloader()
         
    print (' ')
    print ('=============================================================')
    print ('If something goes wrong.')
    print ('Restart your device. Disconnect it from USB and battery!!!...')
    print ('=============================================================')
    print (' ')
    
    # 166  WDT block  disable Watchdog
    h.write16(0xa0030000, 0x2200)  
    
    # 186   Config block (chip version, etc.)
    code = h.read16_old(0x80000008)  # read MTK_HARDWARE_CODE
    print ('Hardware Code %x ' % code)
    if code != 0x6261:
        print ('Wrong hardware code. exit.')
        return
    
    # 204 0x26400000  EFuse block
    value = h.read32(0xa01c0108)
    print ('EFuse 0x%x' % value)

    # 218  some Magic
    # Power management block ??
    h.write16(0xa0700f00, 0x41)
    # 238
    h.write16(0xa0700f00, 0x51)
    # 258
    h.write16(0xa0700f00, 0x41)

    # 278
    # GPIO control block
    # GPIO_CTRL_DOUT1 (GPIO_CTRL_ADDR + 0x0310)
    # clear
    h.write32(0xa0020318, 0x2000)
    # 298
    # GPIO_CTRL_PULLEN0 (GPIO_CTRL_ADDR + 0x0100)
    # clear or set ???
    h.write32(0xa0020014, 0x2000)
    # 318
    # GPIO_CTRL_MODE5        (GPIO_CTRL_ADDR + 0x0c50)
    # set 
    h.write32(0xa0020c58, 0x700000)
   

    h.BL_EMI_PSRAM_Calibration()
   
    # 8924 
    # reset engine
    h.write32(0xa0050300, 0x0)
    # 8944
    # EMI_CTRL_DLLV
    h.write32(0xa0050090, 0x300f0000)
    # 8964
    h.write32(0xa0050090, 0x700f0000)
    # 8987
    h.write32(0xa0050090, 0x100f0000)
    
    # 9004 0x10
    # Power management block
    # PMIC_CTRL10 charger
    val = h.read16_old(0xa0700a28)
    #print ('0xa0700a28 0x%x' % val)
      
    
    
    if  args.nobattery == False:
        # 9018
        h.write16(0xa0700a28, 0x4010)
    else:
        # Upload without battery
        # CHR_CON9
        h.write16(0xa0700a24,0x15) 
        #Read16  0xa0700a14 0x6109 
        h.write16(0xa0700a14,0x6009 )
        #h.read16 ( 0xa0700a14, 0x6009) 
        h.write16( 0xa0700a14, 0x6049) 
        #h.read16 ( 0xa0700a08, 0x0f) 
        h.write16 (0xa0700a08, 0x10b) 
       
    # 9038 0xf262
    # PMIC_CTRL0
    val = h.read16_old(0xa0700a00)
    #print ('0xa0700a00 0x%x' % val)
    # 9052 charger control enable charger ?
    
    if args.nobattery == False:
        h.write16(0xa0700a00, 0xf272)
    else:
        # Upload without battery
        
        h.write16( 0xa0700a00 ,0xf27a )
        #h.read16(  0xa0700a28 ,0x00 )
        #Enable USB Download mode (required for no-battery operation) 
        h.write16( 0xa0700a28 ,0x8000)         
    
    h.BL_PowerUpBaseband()
        
    h.BL_RemapEMI()
    
    
    if args.native == True:
        h.writeFile(0x10020000, FirmwarePath)
        h.startProg(0x10020000)   
       
        
    else:
        h.BL_UploadAndStartDA_Bootloader()
       
        # 9819 DA is running
        
        # #Get SYNC_CHAR C0 DA return BB = 142 
        h.DA_WaitForSync()
        print ('DA Loader is running')
        print ('====================')
        print (' ')
        print (' ')
         
        h.DA_DetectFlash()
        
        h.DA_FormatCBR()   
        
        h.DA_DownloadBootLoader(FilenameBootloader, FilenameBootloaderExt)
        
        h.DA_CheckEFuse()
        
        h.DA_CBRInfo()
          
        h.DA_SetMemBlock(FilenameROM1, FilenameROM2)
        
        h.DA_ERASE_MAUI_INFO()
        
        h.DA_WriteCMD(FilenameROM1, FilenameROM2)
        
        
        if args.nofatformat == False:
        
            h.DA_doFATParition()
        
        h.DA_disconnect() 
        
        print (' ')
        print ('===========================')
        print ('Update done !!!!!!!!')
        print ('Disconnect Rephone from USB')
        print ('===========================')
        print (' ')
    
    h.close()

    
if __name__ == '__main__':
    try:
        main()
       
    except Exception:
        #sys.stderr.write('ERROR: %s\n' % str(err))
        traceback.print_exc()
