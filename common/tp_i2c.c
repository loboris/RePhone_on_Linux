
#include <stdio.h>
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_i2c.h"
#include "vmdrv_tp.h"
#include "vmdatetime.h"
#include "vmthread.h"
#include "tp_i2c.h"
#include "vmdatetime.h"
#include "vmthread.h"
#include "string.h"


#define REG_OUTPUT			(uint32_t *)0xA0020004
#define REG_INPUT			(uint32_t *)0xA0020008
#define REG_OUT_HIGH		(uint32_t *)0xA0020304
#define REG_OUT_LOW			(uint32_t *)0xA0020308
#define REG_PULL_ENABLE		(uint32_t *)0xA0020104
#define REG_PULL_DISABLE	(uint32_t *)0xA0020108
#define REG_PULL_UP			(uint32_t *)0xA0020504
#define REG_PULL_DOWN		(uint32_t *)0xA0020508
#define REG_ST_ON			(uint32_t *)0xA0020604
#define REG_ST_OFF			(uint32_t *)0xA0020608
#define REG_GPIO_ID_DATA	(uint32_t *)0xA0020400

VM_DRV_TP_BOOL ctp_i2c_configure_done = VM_DRV_TP_FALSE;

extern const VMUINT8 gpio_ctp_i2c_sda_pin;
extern const VMUINT8 gpio_ctp_i2c_scl_pin;
//extern VMUINT32 CTP_DELAY_TIME;

static VM_DCL_HANDLE sda_handle;
static VM_DCL_HANDLE scl_handle;

#define CTP_I2C_DELAY ctp_i2c_udelay();


//----------------------------------------
static void ctp_i2c_udelay(void)
{
    VMUINT32 ust = 0;
    VMUINT32 count = 0;

    ust = vm_time_ust_get_count() + 2;
    while (vm_time_ust_get_count() < ust) {
        //count++;
        if (++count > 10) break;
    }
}

//---------------------------------------------------------
int ctp_i2c_configure(VMUINT32 slave_addr, VMUINT32 speed)
{
    vm_dcl_i2c_control_config_t conf_data;
    ctp_i2c_configure_done = VM_DRV_TP_FALSE;

	sda_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_i2c_sda_pin);
	scl_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_i2c_scl_pin);

	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	//vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);

	ctp_i2c_configure_done = VM_DRV_TP_TRUE;

	return ctp_i2c_configure_done;
}

//-----------------------------
static void ctp_i2c_start(void)
{
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	CTP_I2C_DELAY;
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	CTP_I2C_DELAY;

	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	CTP_I2C_DELAY;
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	CTP_I2C_DELAY;

	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	CTP_I2C_DELAY;
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	CTP_I2C_DELAY;
}

//----------------------------
static void ctp_i2c_stop(void)
{
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	CTP_I2C_DELAY;
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	CTP_I2C_DELAY;

	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	CTP_I2C_DELAY;
	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	CTP_I2C_DELAY;

	vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
}

//-----------------------------------------------------
static VM_DRV_TP_BOOL ctp_i2c_send_byte(VMUINT8 ucData)
{
    VM_DRV_TP_BOOL ret = VM_DRV_TP_FALSE;;
	VMUINT8 ucMask;
	int i;
	vm_dcl_gpio_control_level_status_t sda_read;

    vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

    for (i = 0, ucMask = 0x80; i < 8; i++, ucMask >>= 1) {
        if (ucData & ucMask)
            vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        else
            vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        CTP_I2C_DELAY;
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        CTP_I2C_DELAY;
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        CTP_I2C_DELAY;
    }
    vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);

    vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_READ, (void*)&sda_read);

    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    CTP_I2C_DELAY;
    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    CTP_I2C_DELAY;

    ret = (sda_read.level_status == VM_DCL_GPIO_IO_LOW) ? (VM_DRV_TP_BOOL)CTP_I2C_ACK : (VM_DRV_TP_BOOL)CTP_I2C_NAK;

    return ret;
}

//------------------------------------------------------
static VMUINT8 ctp_i2c_receive_byte(VM_DRV_TP_BOOL bAck)
{
    VMUINT8 ucRet = 0;

	vm_dcl_gpio_control_level_status_t sda_read;
	int i;

    vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

    for (i = 7; i >= 0; i--) {
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_READ, (void*)&sda_read);
        ucRet |= sda_read.level_status << i;

        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        CTP_I2C_DELAY;
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        CTP_I2C_DELAY;
    }

    vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

    if (bAck)
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    else
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    CTP_I2C_DELAY;

    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    CTP_I2C_DELAY;
    vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    CTP_I2C_DELAY;

    return ucRet;
}

//---------------------------------------------------------------------------------------------------------------
VM_DRV_TP_BOOL ctp_i2c_send(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VMUINT32 i=0;
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    vm_dcl_i2c_control_continue_write_t write;

	ctp_i2c_start();

	if (ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
		if (ctp_i2c_send_byte(ucBufferIndex) == CTP_I2C_ACK) {
			for(i = 0; i < unDataLength; i++) {
				if (ctp_i2c_send_byte(pucData[i]) == CTP_I2C_NAK) {
					bRet = VM_DRV_TP_FALSE;
					break;
				}
			}
		} else {
			bRet = VM_DRV_TP_FALSE;
		}
	}
	else bRet = VM_DRV_TP_FALSE;

	ctp_i2c_stop();

    return bRet;
}

//--------------------------------------------------------------------------------------------------------------------
VM_DRV_TP_BOOL ctp_i2c_send_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    VMUINT8 addr_h = (ucBufferIndex >> 8) & 0xFF;
    VMUINT8 addr_l = ucBufferIndex & 0xFF;
	VMUINT32 i=0;

	ctp_i2c_start();

	if (ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
		if((ctp_i2c_send_byte(addr_h) == CTP_I2C_ACK) && (ctp_i2c_send_byte(addr_l) == CTP_I2C_ACK)) {
			for(i = 0; i < unDataLength; i++) {
				if(ctp_i2c_send_byte(pucData[i]) == CTP_I2C_NAK) {
					bRet = VM_DRV_TP_FALSE;
					break;
				}
			}
		} else {
			bRet = VM_DRV_TP_FALSE;
		}
	}
	else bRet = VM_DRV_TP_FALSE;
	ctp_i2c_stop();

	return bRet;
}

//------------------------------------------------------------------------------------------------------------------
VM_DRV_TP_BOOL ctp_i2c_receive(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
	VMUINT32 i = 0;

    ctp_i2c_start();

	if (ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
		if (ctp_i2c_send_byte(ucBufferIndex) == CTP_I2C_ACK) {
			ctp_i2c_start();

			if(ctp_i2c_send_byte(ucDeviceAddr | 0x01) == CTP_I2C_ACK) {
				for(i = 0; i < unDataLength - 1; i++) {
					pucData[i] = ctp_i2c_receive_byte((VM_DRV_TP_BOOL)CTP_I2C_ACK);
				}
				pucData[unDataLength - 1] = ctp_i2c_receive_byte((VM_DRV_TP_BOOL)CTP_I2C_NAK);
			} else {
				bRet = VM_DRV_TP_FALSE;
			}
		} else {
			bRet = VM_DRV_TP_FALSE;
		}
	}
	else bRet = VM_DRV_TP_FALSE;
	ctp_i2c_stop();

    return bRet;
}

//-----------------------------------------------------------------------------------------------------------------------
VM_DRV_TP_BOOL ctp_i2c_receive_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    VMUINT8 addr_h = (ucBufferIndex >> 8) & 0xFF;
    VMUINT8 addr_l = ucBufferIndex & 0xFF;
	VMUINT32 i = 0;

	ctp_i2c_start();
	if(ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
		if((ctp_i2c_send_byte(addr_h) == CTP_I2C_ACK) && (ctp_i2c_send_byte(addr_l) == CTP_I2C_ACK)) {
			ctp_i2c_start();

			if(ctp_i2c_send_byte(ucDeviceAddr | 0x01) == CTP_I2C_ACK) {
				for(i = 0; i < unDataLength - 1; i++) {
					pucData[i] = ctp_i2c_receive_byte((VM_DRV_TP_BOOL)CTP_I2C_ACK);
				}
				pucData[unDataLength - 1] = ctp_i2c_receive_byte((VM_DRV_TP_BOOL)CTP_I2C_NAK);
			} else {
				bRet = VM_DRV_TP_FALSE;
			}
		} else {
			bRet = VM_DRV_TP_FALSE;
		}
	}
	else bRet = VM_DRV_TP_FALSE;
	ctp_i2c_stop();

    return bRet;
}
