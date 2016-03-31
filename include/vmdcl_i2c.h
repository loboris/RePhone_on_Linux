/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005-2015
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef VMDCL_I2C_SDK_H
#define VMDCL_I2C_SDK_H

#ifdef __cplusplus
extern "C" {
#endif


/* DCL(Driver Common Layer)I2C(Inter-Integrated Circuit) */

/* VM_DCL_I2C_OWNER */
typedef enum
{
    VM_DCL_I2C_OWNER_DUMMY = 0xD    /* Indicates that this is the DEFAULT user when no users use I2Cdriver */
} VM_DCL_I2C_OWNER;


/*****************************************************************************
 * DESCRIPTION
 *  VM_DCL_I2C_DEVICE indicates the device that uses the I2C module. This enumerator
 *  defines the device for the I2C module. It is used with vm_dcl_open.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *  i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * //Do something with the handle.
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_I2C_START = VM_DCL_I2C_GROUP_START,       /* The I2C device starts. */
    VM_DCL_I2C                                       /* The parameter of a device for vm_dcl_open.  */
} VM_DCL_I2C_DEVICE;


/*****************************************************************************
 * DESCRIPTION
 *  VM_DCL_I2C_TRANSACTION_MODE  indicates the I2C Transaction Mode, you
 *  can select FAST or HIGH speed if needed.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 *conf_data.reserved_0 = 0;
 *conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 *conf_data.get_handle_wait = 0;
 *conf_data.reserved_1 = 0;
 *conf_data.delay_length = 0;
 *conf_data.slave_address = 0x98;
 *conf_data.fast_mode_speed = 400;
 *conf_data.high_mode_speed = 0;
 *vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * //To do something with the handle.
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_I2C_TRANSACTION_FAST_MODE,        /* The Fast Mode: < 400kbps. */
    VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE   /* The High Speed Mode: > 400kbps. */
} VM_DCL_I2C_TRANSACTION_MODE;


/*****************************************************************************
 * DESCRIPTION
 *  VM_DCL_I2C_CTRL_CMD_T  indicates the I2C control commands, you
 *  can select CMDS to control the driver.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 *	vm_i2c_ctrl_single_write_t write_data;
 *  VM_DCL_BUFF_LEN len;
 *	VMUINT8 i2c_write_data[2] = {0x56,0x78};
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * write_data.data_ptr = i2c_write_data;
 * write_data.data_length = 2;
 * vm_dcl_write(i2c_handle,(VM_DCL_BUFF*)i2c_write_data,2,&len,0);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_I2C_CMD_GET_TRANSACTION_MODE = 0x901,     /* This gets the transaction mode. */
    VM_DCL_I2C_CMD_SET_TRANSACTION_SPEED,            /* This sets the transaction speed. */
    VM_DCL_I2C_CMD_SINGLE_WRITE,                     /* The single write of a blocking mode. */
    VM_DCL_I2C_CMD_SINGLE_READ,                      /* The single read of a blocking mode. */
    VM_DCL_I2C_CMD_CONT_WRITE,                       /* This continues the write of a blocking mode. */
    VM_DCL_I2C_CMD_CONT_READ,                        /* This continues the read of a blocking mode. */
    VM_DCL_I2C_CMD_WRITE_AND_READ,                   /* This is the write and the read of a blocking mode. */
    VM_DCL_I2C_CMD_RESERVED_0,                       /* This is a reserved command. Do not use. */
    VM_DCL_I2C_CMD_RESERVED_1,                       /* This is a reserved command. Do not use. */
    VM_DCL_I2C_CMD_RESERVED_2,                       /* This is a reserved command. Do not use. */
    VM_DCL_I2C_CMD_RESERVED_3,                       /* This is a reserved command. Do not use. */
    VM_DCL_I2C_CMD_SET_SLAVE_ADDRESS,                /* This sets the slave address of a sensor. */
    VM_DCL_I2C_CMD_CONFIG                            /* This configures the I2C for read or write. */
} VM_DCL_I2C_CONTROL_COMMAND;

/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_get_transaction_mode_t  indicates I2C_CMD_GET_TRANSACTION_MODE command
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 * vm_i2c_ctrl_get_transaction_mode_t mode;
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * vm_dcl_control(i2c_handle,I2C_CMD_GET_TRANSACTION_MODE,(void *)&mode);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VM_DCL_I2C_TRANSACTION_MODE transaction_mode; /* This is the transaction mode. */
} vm_dcl_i2c_control_get_transaction_mode_t;



/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_set_transaction_speed_t  indicates I2C_CMD_SET_TRANSACTION_SPEED command
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 * vm_i2c_ctrl_set_transaction_speed_t speed;
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * speed.transaction_mode = VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE;
 * speed.fast_mode_speed = 800;
 * speed.high_mode_speed = 1;
 * vm_dcl_control(i2c_handle,I2C_CMD_SET_TRANSACTION_SPEED,(void *)&speed);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VM_DCL_I2C_TRANSACTION_MODE transaction_mode;    /* The transaction mode. */
    VMUINT32 fast_mode_speed;                        /* The fast mode speed. */
    VMUINT32 high_mode_speed;                        /* The high speed mode. */
} vm_dcl_i2c_control_set_transaction_speed_t;

/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_single_write_t and vm_dcl_i2c_control_single_read_t indicate
 *  I2C_CMD_SINGLE_WRITE and I2C_CMD_SINGLE_READ command, respectively.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 *	vm_dcl_i2c_control_single_write_t write_data;
 *  vm_dcl_i2c_control_single_read_t  read_data;
 *  VM_DCL_BUFF_LEN len;
 *	VMUINT8 i2c_write_data[2] = {0x56,0x78};
  *  VMUINT8 i2c_read_data[2] = {0x0,0x0};
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * write_data.data_ptr = i2c_write_data;
 * write_data.data_length = 2;
 * vm_dcl_control(i2c_handle,I2C_CMD_SINGLE_WRITE,(void *)&write_data);
 * read_data.data_ptr=i2c_read_data;
 * read_data.data_length = 2;
 * vm_dcl_control(i2c_handle,I2C_CMD_SINGLE_READ,(void *)&read_data);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VMUINT8* data_ptr;           /* The pointer to the data buffer. */
    VMUINT32 data_length;        /* The data length. Please note: this length should not exceed 8.*/
} vm_dcl_i2c_control_single_write_t,vm_dcl_i2c_control_single_read_t;

/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_continue_write_t and vm_dcl_i2c_control_continue_read_t indicate
 *   I2C_CMD_CONT_WRITE and I2C_CMD_CONT_READ command, respectively.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 *	vm_dcl_i2c_control_continue_write_t write_data;
 *  vm_dcl_i2c_control_continue_read_t  read_data;
 *  VM_DCL_BUFF_LEN len;
 *	VMUINT8 i2c_write_data[20] = {0x56,0x78,0x00};
 *  VMUINT8 i2c_read_data[20] = {0x0};
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * write_data.data_ptr = i2c_write_data;
 * write_data.data_length = 20;
 * write_data.transfer_number = 1;
 * vm_dcl_control(i2c_handle,I2C_CMD_SINGLE_WRITE,(void *)&write_data);
 *  read_data.data_ptr=i2c_read_data;
 * read_data.data_length = 20;
  * read_data.transfer_number = 1;
 * vm_dcl_control(i2c_handle,I2C_CMD_SINGLE_READ,(void *)&read_data);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VMUINT8* data_ptr;           /* The pointer to the data buffer.*/
    VMUINT32 data_length;        /* The data length of each transfer. Please note: this length should not exceed 8.*/
    VMUINT32 transfer_number;    /* The transfer number.*/
} vm_dcl_i2c_control_continue_write_t,vm_dcl_i2c_control_continue_read_t;


/*****************************************************************************
 * DESCRIPTION
 * vm_dcl_i2c_control_write_and_read_t indicates the I2C_CMD_WRITE_AND_READ command.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 *	vm_dcl_i2c_control_write_and_read_t write_and_read_data;
 *  vm_i2c_ctrl_cont_read_t  read_data;
 *  VM_DCL_BUFF_LEN len;
 *	VMUINT8 i2c_write_data[20] = {0x56,0x78,0x00};
 *  VMUINT8 i2c_read_data[20] = {0x0};
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * write_and_read_data.out_data_ptr = i2c_write_data;
 *  write_and_read_data.out_data_length = 20;
 * write_and_read_data.in_data_ptr = i2c_read_data;
 *  write_and_read_data.in_data_length = 20;
 * vm_dcl_control(i2c_handle,I2C_CMD_WRITE_AND_READ,(void *)&write_and_read_data);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VMUINT8* in_data_ptr;            /* The pointer to the read data. */
    VMUINT32 in_data_length;         /* The read data length. Please note: this length should not exceed 8.*/
    VMUINT8* out_data_ptr;           /* The pointer to the write data. */
    VMUINT32 out_data_length;        /* The write data length. Please note: this length should not exceed 8.*/
} vm_dcl_i2c_control_write_and_read_t;


/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_set_slave_address_t indicates the I2C_CMD_SET_SLAVE_ADDRESS command.
  * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 * vm_dcl_i2c_control_set_slave_address_t address_setting;
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 * conf_data.reserved_0 = 0;
 * conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 * conf_data.get_handle_wait = 0;
 * conf_data.reserved_1 = 0;
 * conf_data.delay_length = 0;
 * conf_data.slave_address = 0x98;
 * conf_data.fast_mode_speed = 400;
 * conf_data.high_mode_speed = 0;
 * vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * address_setting.slave_address=0x98;
 * vm_dcl_control(i2c_handle,I2C_CMD_GET_TRANSACTION_MODE,(void *)&address_setting);
 *  //To do something with the handle.
 *  </code>
 *****************************************************************************/
typedef struct
{
    VMUINT8 slave_address;       /* The slave address. */
} vm_dcl_i2c_control_set_slave_address_t;


/*****************************************************************************
 * DESCRIPTION
 *  vm_dcl_i2c_control_config_t indicates the I2C_CMD_CONFIG command.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE i2c_handle;
 *	vm_dcl_i2c_control_config_t conf_data;
 * i2c_handle = vm_dcl_open(VM_DCL_I2C,0);
 * if(VM_DCL_HANDLE_INVALID==i2c_handle) return;
 *conf_data.reserved_0 = 0;
 *conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
 *conf_data.get_handle_wait = 0;
 *conf_data.reserved_1 = 0;
 *conf_data.delay_length = 0;
 *conf_data.slave_address = 0x98;
 *conf_data.fast_mode_speed = 400;
 *conf_data.high_mode_speed = 0;
 *vm_dcl_control(i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);
 * //To do something with the handle.
 * </code>
 *****************************************************************************/
typedef struct
{
    VM_DCL_I2C_OWNER reserved_0;         /* This is reserved. Please use 0. */
    VMBOOL get_handle_wait;              /* If KAL_TRUE: wait until I2C is idle; If KAL_FALSE: do not wait. */
    VMUINT8 slave_address;               /* The slave address. */
    VMUINT8 delay_length;                /* The delay length between consecutive transfers (in half pulse width unit). */
    VM_DCL_I2C_TRANSACTION_MODE transaction_mode; /* The fast mode or high speed mode. */
    VMUINT32 fast_mode_speed;            /* The transfer speed under fast mode. You should configure this parameter even when using high speed mode. */
    VMUINT32 high_mode_speed;            /* The transfer speed under high speed mode. */
    VMBOOL reserved_1;                    /* This is reserved. Please always use 0 to fill this member. */
} vm_dcl_i2c_control_config_t;



#ifdef __cplusplus
}
#endif


#endif /* VMDCL_I2C_SDK_H */


