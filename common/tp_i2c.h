#ifndef __TP_I2C_H__
#define __TP_I2C_H__

#include "vmtype.h"
#include "vmdrv_tp.h"

#define CTP_I2C_LOW				0
#define CTP_I2C_HIGH			1
#define CTP_I2C_ACK				CTP_I2C_LOW
#define CTP_I2C_NAK				CTP_I2C_HIGH

//delay
void ctp_i2c_udelay(VMUINT32 delay);

//power on I2C
void ctp_i2c_power_on(VM_DRV_TP_BOOL ON, VMUINT32 ldo, VMUINT32 ldo_volt);

//configure SW I2C or HW I2C parameters
void ctp_i2c_configure(VMUINT32 slave_addr, VMUINT32 speed);

// Start bit of I2C waveform
extern void ctp_i2c_start(void);

// Stop bit of I2C waveform
extern void ctp_i2c_stop(void);

// Send one byte from host to client
extern VM_DRV_TP_BOOL ctp_i2c_send_byte(VMUINT8 ucData);

// Receive one byte form client to host
extern VMUINT8 ctp_i2c_receive_byte(VM_DRV_TP_BOOL bAck);

// I2C send data fuction
extern VM_DRV_TP_BOOL ctp_i2c_send(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C receive data function
extern VM_DRV_TP_BOOL ctp_i2c_receive(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C send data for 16 bits address fuction
extern VM_DRV_TP_BOOL ctp_i2c_send_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C receive data for 16 bits address function
extern VM_DRV_TP_BOOL ctp_i2c_receive_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

#endif
