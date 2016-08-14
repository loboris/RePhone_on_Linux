
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ymodem.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "CheckSumUtils.h"
#include "vmtype.h"
#include "vmfs.h"
#include "vmthread.h"
#include "vmchset.h"
#include "vmstdlib.h"

#include "shell.h"

extern void retarget_putc(char ch);
extern int retarget_getc(int tmo);
extern void retarget_write(const char *str, unsigned int len);

//-----------------------------------------------------------
static uint16_t Cal_CRC16(const uint8_t* data, uint32_t size)
{
  CRC16_Context contex;
  uint16_t ret;
  
  CRC16_Init( &contex );
  CRC16_Update( &contex, data, size );
  CRC16_Final( &contex, &ret );
  return ret;
}

//------------------------------------------------------------------------
static unsigned short crc16(const unsigned char *buf, unsigned long count)
{
  unsigned short crc = 0;
  int i;

  while(count--) {
    crc = crc ^ *buf++ << 8;

    for (i=0; i<8; i++) {
      if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
      else crc = crc << 1;
    }
  }
  return crc;
}

//--------------------------------------------------------------
static int32_t Receive_Byte (unsigned char *c, uint32_t timeout)
{
  int ch = retarget_getc(timeout/1000);
  if (ch < 0) return -1;
  *c = (unsigned char)ch;
  return 0;
}

//-----------------------------------
static uint32_t Send_Byte (uint8_t c)
{
  retarget_putc(c);
  return 0;
}

//----------------------------
static void send_CA ( void ) {
  Send_Byte(CA);
  Send_Byte(CA);
  vm_thread_sleep(100);
}



/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  timeout
  * @param  length
  *     0: end of transmission
  *    -1: abort by sender
  *    >0: packet length
  * @retval 0: normally return
  *        -1: timeout or packet error
  *        -2: crc error
  *         1: abort by user
  */
//--------------------------------------------------------------------------
static int32_t Receive_Packet (uint8_t *data, int *length, uint32_t timeout)
{
  int count, packet_size, i, ch;
  *length = 0;
  
  // receive 1st byte
  ch = retarget_getc(timeout/1000);
  if (ch < 0) return -1;

  switch (ch) {
    case SOH:
      packet_size = PACKET_SIZE;
      break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
      return 0;
    case CA:
   	  ch = retarget_getc(timeout/1000);
   	  if (ch < 0) return -1;
      if (ch == CA) {
        *length = -1;
        return 0;
      }
      else return -1;
    case ABORT1:
    case ABORT2:
      return 1;
    default:
      return -1;
  }

  *data = (uint8_t)ch;
  uint8_t *dptr = data+1;
  count = packet_size + PACKET_OVERHEAD-1;

  for (i=0; i<count; i++) {
   	  ch = retarget_getc(1);
   	  if (ch < 0) return -1;
	  *dptr++ = (uint8_t)ch;;
  }

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) return -1;
  if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) return -1;

  *length = packet_size;
  return 0;
}

/**
  * @brief  Receive a file using the ymodem protocol.
  * @param  buf: Address of the first byte.
  * @retval The size of the file.
  */
//--------------------------------------------------------------------------------
int Ymodem_Receive ( char* FileName, unsigned int maxsize, unsigned char getname )
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr;
  unsigned int i, file_len, write_len, session_done, file_done, packets_received, errors, session_begin, size = 0;
  int packet_length = 0;
  int ffd = -1;
  
  for (session_done = 0, errors = 0, session_begin = 0; ;) {
    for (packets_received = 0, file_done = 0; ;) {
      switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT)) {
        case 0:  // normal return
          switch (packet_length) {
            // Abort by sender
            case -1:
              Send_Byte(ACK);
              size = 0;
              goto exit;
            // End of transmission
            case 0:
              Send_Byte(ACK);
              file_done = 1;
              break;
            // Normal packet
            default:
              if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0x000000ff)) {
                errors ++;
                if (errors > MAX_ERRORS) {
                  send_CA();
                  size = 0;
                  goto exit;
                }
                Send_Byte(NAK);
              }
              else {
                errors = 0;
                if (packets_received == 0) {  // ** First packet, Filename packet
                  if (packet_data[PACKET_HEADER] != 0) {
                    // Filename packet has valid data
                    if (getname == 0) {
                      for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < 64);) {
                        FileName[i++] = *file_ptr++;
                      }
                      FileName[i++] = '\0';
                    }
                    for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < packet_length);) {
                      file_ptr++;
                    }
                    for (i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);) {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';
                    //Str2Int(file_size, &size);
                    size = strtol(file_size, NULL, 10);

                    // Test the size of the file
                    if ((size < 1) || (size > maxsize)) {
                      // End session
                      send_CA();
                      size = -4;
                      goto exit;
                    }

                    // *** Open the file for writing***
                    ffd = file_open(FileName, O_CREAT);
                    if (ffd < 0) {
                      // End session
                      send_CA();
                      size = -2;
                      goto exit;
                    }
                    file_len = 0;
                    Send_Byte(ACK);
                    Send_Byte(CRC16);
                  }
                  // Filename packet is empty, end session
                  else {
                    Send_Byte(ACK);
                    file_done = 1;
                    session_done = 1;
                    break;
                  }
                }
                else {  // Data packet
                  // Write received data to file
                  if (file_len < size) {
                    file_len = file_len + packet_length;  // total bytes received
                    if (file_len > size) write_len = packet_length - (file_len - size);
                    else write_len = packet_length;
                    if (ffd < 0) {
                      // File not opened, End session
                      send_CA();
                      size = -2;
                      goto exit;
                    }
                    VMUINT written_bytes = file_write(ffd, (char*)(packet_data + PACKET_HEADER), write_len);
                    if (written_bytes != write_len) { //failed
                      /* End session */
                      send_CA();
                      size = 1;
                      goto exit;
                    }
                    if (file_flush(ffd) < 0) {
					  /* End session */
					  send_CA();
					  size = 1;
					  goto exit;
                    }
                  }
                  //success
                  Send_Byte(ACK);
                }
                packets_received++;
                session_begin = 1;
              }
          }
          break;
        case 1:  // user abort
          send_CA();
          size = -3;
          goto exit;
        default: // timeout, packet error or crc error
          if (session_begin >= 0) errors ++;
          if (errors > MAX_ERRORS) {
            send_CA();
            size = 0;
            goto exit;
          }
          Send_Byte(CRC16);
          break;
      }
      if (file_done != 0) break;
    }
    if (session_done != 0) break;
  }
exit:
  if (ffd >= 0) file_close(ffd);
  return size;
}

//---------------------------------------------------------------------------------------------
static void Ymodem_PrepareIntialPacket(uint8_t *data, const uint8_t* fileName, uint32_t length)
{
  uint16_t i, j;
  uint8_t file_ptr[10] = {0};
  uint16_t tempCRC;
  
  // Make first three packet
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;
  
  // Filename packet has valid data
  for (i = 0; (fileName[i] != '\0') && (i < 64);i++) {
     data[i + PACKET_HEADER] = fileName[i];
  }

  data[i + PACKET_HEADER] = 0x00;
  
  //Int2Str (file_ptr, *length);
  sprintf(file_ptr, "%d", length);
  for (j =0, i = i + PACKET_HEADER + 1; file_ptr[j] != '\0' ; ) {
     data[i++] = file_ptr[j++];
  }
  data[i++] = 0x20;
  
  for (j = i; j < PACKET_SIZE + PACKET_HEADER; j++) {
    data[j] = 0;
  }
  tempCRC = Cal_CRC16(&data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------
static void Ymodem_PrepareLastPacket(uint8_t *data)
{
  uint16_t i;
  uint16_t tempCRC;
  
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;
  for (i = PACKET_HEADER; i < (PACKET_SIZE + PACKET_HEADER); i++) {
    data[i] = 0x00;
  }
  tempCRC = Cal_CRC16(&data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//---------------------------------------------------------------------------------------
static void Ymodem_PreparePacket(uint8_t *data, uint8_t pktNo, uint32_t sizeBlk, int ffd)
{
  uint16_t i, size;
  uint16_t tempCRC;
  
  data[0] = STX;
  data[1] = (pktNo & 0x000000ff);
  data[2] = (~(pktNo & 0x000000ff));

  size = sizeBlk < PACKET_1K_SIZE ? sizeBlk :PACKET_1K_SIZE;
  // Read block from file
  if (size > 0) {
	  size = file_read(ffd, data + PACKET_HEADER, size);
  }

  if ( size  <= PACKET_1K_SIZE) {
    for (i = size + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
      data[i] = 0x1a; // EOF (0x1A) or 0x00
    }
  }
  tempCRC = Cal_CRC16(&data[PACKET_HEADER], PACKET_1K_SIZE);
  data[PACKET_1K_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_1K_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//--------------------------------------------------------
static uint8_t Ymodem_WaitACK(uint8_t ackchr, uint8_t tmo)
{
  uint8_t receivedC[2];
  uint32_t errors = 0;

  do {
    if (Receive_Byte(&receivedC[0], NAK_TIMEOUT) == 0) {
      if (receivedC[0] == ackchr) {
        return 1;
      }
      else if (receivedC[0] == CA) {
        send_CA();
        return 2; // CA received, Sender abort
      }
      else if (receivedC[0] == NAK) {
        return 3;
      }
      else {
        return 4;
      }
    }
    else {
      errors++;
    }
  }while (errors < tmo);
  return 0;
}


//-----------------------------------------------------------------------------
char Ymodem_Transmit (const char* sendFileName, unsigned int sizeFile, int ffd)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint8_t filename[64];
  uint16_t blkNumber;
  uint8_t receivedC[1], i, err;
  uint32_t size = 0;

  for (i = 0; i < (64 - 1); i++)
  {
    filename[i] = sendFileName[i];
  }
    
  while (retarget_getc(0) >= 0) {};

  // Wait for response from receiver
  err = 0;
  do {
    Send_Byte(CRC16);
  } while (Receive_Byte(&receivedC[0], NAK_TIMEOUT) < 0 && err++ < 45);

  if (err >= 45 || receivedC[0] != CRC16) {
    send_CA();
    return 99;
  }
  
  // === Prepare first block and send it =======================================
  /* When the receiving program receives this block and successfully
   * opened the output file, it shall acknowledge this block with an ACK
   * character and then proceed with a normal YMODEM file transfer
   * beginning with a "C" or NAK tranmsitted by the receiver.
   */
  Ymodem_PrepareIntialPacket(&packet_data[0], filename, sizeFile);
  do 
  {
    // Send Packet
	retarget_write(packet_data, PACKET_SIZE + PACKET_OVERHEAD);
    // Wait for Ack
    err = Ymodem_WaitACK(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return 90;                  // timeout or wrong response
    }
    else if (err == 2) return 98; // abort
  }while (err != 1);

  // After initial block the receiver sends 'C' after ACK
  if (Ymodem_WaitACK(CRC16, 10) != 1) {
    send_CA();
    return 90;
  }
  
  // === Send file blocks ======================================================
  size = sizeFile;
  blkNumber = 0x01;
  
  // Resend packet if NAK  for a count of 10 else end of communication
  while (size)
  {
    // Prepare and send next packet
    Ymodem_PreparePacket(&packet_data[0], blkNumber, size, ffd);
    do
    {
      retarget_write(packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);
      // Wait for Ack
      err = Ymodem_WaitACK(ACK, 10);
      if (err == 1) {
        blkNumber++;
        if (size > PACKET_1K_SIZE) size -= PACKET_1K_SIZE; // Next packet
        else size = 0; // Last packet sent
      }
      else if (err == 0 || err == 4) {
        send_CA();
        return 90;                  // timeout or wrong response
      }
      else if (err == 2) return 98; // abort
    }while(err != 1);
  }
  
  // === Send EOT ==============================================================
  Send_Byte(EOT); // Send (EOT)
  // Wait for Ack
  do 
  {
    // Wait for Ack
    err = Ymodem_WaitACK(ACK, 10);
    if (err == 3) {   // NAK
      Send_Byte(EOT); // Send (EOT)
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return 90;                  // timeout or wrong response
    }
    else if (err == 2) return 98; // abort
  }while (err != 1);
  
  // === Receiver requests next file, prepare and send last packet =============
  if (Ymodem_WaitACK(CRC16, 10) != 1) {
    send_CA();
    return 90;
  }

  Ymodem_PrepareLastPacket(&packet_data[0]);
  do 
  {
	retarget_write(packet_data, PACKET_SIZE + PACKET_OVERHEAD); // Send Packet
    // Wait for Ack
    err = Ymodem_WaitACK(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return 90;                  // timeout or wrong response
    }
    else if (err == 2) return 98; // abort
  }while (err != 1);
  
  return 0; // file transmitted successfully
}

