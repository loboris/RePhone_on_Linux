
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_i2c.h"
#include "vmdrv_tp.h"
#include "vmdatetime.h"
#include "vmthread.h"
#include "tp_i2c.h"
#include "vmdatetime.h"
#include "string.h"

VM_DRV_TP_BOOL ctp_i2c_configure_done = VM_DRV_TP_FALSE;

extern const VMUINT8 gpio_ctp_i2c_sda_pin;
extern const VMUINT8 gpio_ctp_i2c_scl_pin;
extern VMUINT32 CTP_DELAY_TIME;

VM_DCL_HANDLE ctp_i2c_handle;

static VM_DCL_HANDLE sda_handle, scl_handle;

#define CTP_I2C_DELAY ctp_i2c_udelay(CTP_DELAY_TIME);

void ctp_i2c_udelay(VMUINT32 delay)
{
    VMUINT32 ust = 0, ust1 = 0; // ust_get_current_time
    VMUINT32 count = 0;
    VMUINT32 break_count = 0;

    if(delay > 4615) // longer than 1 tick
    {
        vm_thread_sleep(delay / 4615);
        delay = delay % 4615;
    }
    ust = vm_time_ust_get_count();
    do {
        ust1 = vm_time_ust_get_count();
        if(ust1 != ust)
            count++;
        else
            break_count++;
        ust = ust1;
    } while((count < delay) && (break_count < 0xFFFFFF));
}

void ctp_i2c_configure(VMUINT32 slave_addr, VMUINT32 speed)
{
    vm_dcl_i2c_control_config_t conf_data;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(!ctp_i2c_configure_done) {
            ctp_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
        }

        conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
        conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
        conf_data.get_handle_wait = VM_DRV_TP_TRUE;
        conf_data.reserved_1 = 0;
        conf_data.delay_length = 0;
        conf_data.slave_address = slave_addr;

        conf_data.fast_mode_speed = speed;
        conf_data.high_mode_speed = 0;
        vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_CONFIG, (void*)&conf_data);
    } else {
        sda_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_i2c_sda_pin);
        scl_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_i2c_scl_pin);
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    }
    ctp_i2c_configure_done = VM_DRV_TP_TRUE;
}

void ctp_i2c_start(void)
{

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {

    } else {
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
}

void ctp_i2c_stop(void)
{
    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {

    } else {
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
    }
}

VM_DRV_TP_BOOL ctp_i2c_send_byte(VMUINT8 ucData)
{
    // VM_DRV_TP_BOOL bRet;
    int i;
    VM_DRV_TP_BOOL ret;
    VMUINT8 ucMask;
    VM_DCL_STATUS status = VM_DCL_STATUS_FAIL;
    vm_dcl_gpio_control_level_status_t sda_read;
    vm_dcl_i2c_control_continue_write_t write;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            write.data_ptr = &ucData;
            write.data_length = 1;
            write.transfer_number = 1;
            status = vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void*)&write);
        }
        ret = (status == VM_DCL_STATUS_OK) ? VM_DRV_TP_TRUE : VM_DRV_TP_FALSE;
    } else {
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

        for(i = 0, ucMask = 0x80; i < 8; i++, ucMask >>= 1) {
            if(ucData & ucMask)
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
    }
    return ret;
}

VMUINT8 ctp_i2c_receive_byte(VM_DRV_TP_BOOL bAck)
{
    VMUINT8 ucRet = 0;
    int i;
    vm_dcl_i2c_control_continue_read_t read;
    vm_dcl_gpio_control_level_status_t sda_read;
    VM_DCL_STATUS status;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            read.data_ptr = &ucRet;
            read.data_length = 1;
            read.transfer_number = 1;
            status = vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, (void*)&read);
            if(status != VM_DCL_STATUS_OK)
                return 0;
        }
    } else {
        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

        for(i = 7; i >= 0; i--) {
            vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_READ, (void*)&sda_read);
            ucRet |= sda_read.level_status << i;

            vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
            CTP_I2C_DELAY;
            vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
            CTP_I2C_DELAY;
        }

        vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);

        if(bAck)
            vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        else
            vm_dcl_control(sda_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        CTP_I2C_DELAY;

        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        CTP_I2C_DELAY;
        vm_dcl_control(scl_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        CTP_I2C_DELAY;
    }
    return ucRet;
}

VM_DRV_TP_BOOL ctp_i2c_send(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VMUINT32 i;
    VMUINT8 write_buf[9];
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    vm_dcl_i2c_control_continue_write_t write;
    VM_DCL_STATUS status;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            write_buf[0] = ucBufferIndex;
            for(i = 0; i < unDataLength; i++) {
                write_buf[i + 1] = *(pucData + i);
            }
            write.data_ptr = write_buf;
            write.data_length = unDataLength + 1;
            write.transfer_number = 1;
            status = vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void*)&write);
            if(status != VM_DCL_STATUS_OK)
                return VM_DRV_TP_FALSE;
        }
    } else {
        ctp_i2c_start();

        if(ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
            if(ctp_i2c_send_byte(ucBufferIndex) == CTP_I2C_ACK) {
                for(i = 0; i < unDataLength; i++) {
                    if(ctp_i2c_send_byte(pucData[i]) == CTP_I2C_NAK) {
                        bRet = VM_DRV_TP_FALSE;
                        break;
                    }
                }
            } else {
                bRet = VM_DRV_TP_FALSE;
            }
        } else {
            bRet = VM_DRV_TP_FALSE;
        }
        ctp_i2c_stop();
    }
    return bRet;
}

VM_DRV_TP_BOOL ctp_i2c_send_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VMUINT32 i;
    VMUINT8 write_buf[10];
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    vm_dcl_i2c_control_continue_write_t write;
    VM_DCL_STATUS status;
    VMUINT8 addr_h = (ucBufferIndex >> 8) & 0xFF;
    VMUINT8 addr_l = ucBufferIndex & 0xFF;
    VMUINT32 offset = 0;
    VMUINT8 pkt_len;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            while(offset <= unDataLength) {
                write_buf[0] = ((ucBufferIndex + offset) >> 8) & 0xFF;
                write_buf[1] = (ucBufferIndex + offset) & 0xFF;
                if(unDataLength - offset > 6) {
                    pkt_len = 6;
                } else {
                    pkt_len = unDataLength - offset;
                }
                memcpy(&write_buf[2], &pucData[offset], pkt_len);
                offset += pkt_len;
                write.data_ptr = write_buf;
                write.data_length = pkt_len + 2;
                write.transfer_number = 1;
                status = vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void*)&write);
                if(status != VM_DCL_STATUS_OK)
                    return VM_DRV_TP_FALSE;

                if(offset == unDataLength)
                    break;
            }
        }
    } else {
        ctp_i2c_start();

        if(ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
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
        } else {
            bRet = VM_DRV_TP_FALSE;
        }
        ctp_i2c_stop();
    }
    return bRet;
}

VM_DRV_TP_BOOL ctp_i2c_receive(VMUINT8 ucDeviceAddr, VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VMUINT32 i;
    VM_DCL_STATUS dcl_i2c_ret;
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    vm_dcl_i2c_control_write_and_read_t write_read;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            write_read.in_data_ptr = pucData;
            write_read.in_data_length = unDataLength;
            write_read.out_data_ptr = &ucBufferIndex;
            write_read.out_data_length = 1;
            dcl_i2c_ret = vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, (void*)&write_read);
            if(dcl_i2c_ret != VM_DCL_STATUS_OK) {
                bRet = VM_DRV_TP_FALSE;
            }
        }
    } else {
        ctp_i2c_start();

        if(ctp_i2c_send_byte(ucDeviceAddr & 0xFE) == CTP_I2C_ACK) {
            if(ctp_i2c_send_byte(ucBufferIndex) == CTP_I2C_ACK) {
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
        } else {
            bRet = VM_DRV_TP_FALSE;
        }
        ctp_i2c_stop();
    }
    return bRet;
}

VM_DRV_TP_BOOL
ctp_i2c_receive_ext(VMUINT8 ucDeviceAddr, VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
    VMUINT32 i;
    VM_DRV_TP_BOOL bRet = VM_DRV_TP_TRUE;
    vm_dcl_i2c_control_write_and_read_t write_read;
    VMUINT8 write_buf[2];
    VMUINT16 reg_addr = ucBufferIndex;
    VMUINT32 offset = 0;
    VMUINT8 pkt_len;
    VMUINT8 addr_h = (ucBufferIndex >> 8) & 0xFF;
    VMUINT8 addr_l = ucBufferIndex & 0xFF;

    if(gpio_ctp_i2c_sda_pin == 0xFF) // HW I2C
    {
        if(ctp_i2c_configure_done) {
            while(offset < unDataLength) {
                write_buf[0] = (reg_addr >> 8) & 0xFF;
                write_buf[1] = reg_addr & 0xFF;
                if(unDataLength - offset > 8) {
                    pkt_len = 8;
                } else {
                    pkt_len = unDataLength - offset;
                }
                write_read.in_data_ptr = pucData + offset;
                write_read.in_data_length = pkt_len;
                write_read.out_data_ptr = write_buf;
                write_read.out_data_length = 2;
                vm_dcl_control(ctp_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, (void*)&write_read);
                offset += pkt_len;
                reg_addr = ucBufferIndex + offset;
            }
        }
    } else {
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
        } else {
            bRet = VM_DRV_TP_FALSE;
        }
        ctp_i2c_stop();
    }
    return bRet;
}
