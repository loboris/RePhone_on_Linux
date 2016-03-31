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

#ifndef VMDCL_SIO_SDK_H
#define VMDCL_SIO_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmdcl.h"


/*******************************************************************************
 * DESCRIPTION
 *   Device identifiers for serial ports. Pass this to vm_dcl_open to select
 *   one of the serial port devices to use. Currently, only the hardware
 *   UART port (TX/RX pins) is available on 2502A. The USB COM ports are
 *   used by SDK for debugging and SDK functions, therefore not available to
 *   the application.
 *
 * EXAMPLE
 * <code>
 *     VM_DCL_HANDLE uart_handle;
 *     VM_DCL_MODULE_TYPE usr_id;
 *     usr_id = vm_dcl_get_ownerid();
 *     uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, usr_id);
 *     if(VM_DCL_HANDLE_INVALID==uart_handle) return;
 *     //Do something with the handle.
 * </code>
 ******************************************************************************/
typedef enum
{

    VM_DCL_SIO_UART_PORT1 = VM_DCL_SIO_GROUP_START, /* Hardware UART1 device.
                                                       This connects to the
                                                       TX/RX pins on the HDK
                                                       breakout. */
    VM_DCL_SIO_UART_PORT2,                          /* Reserved, UART port 2. */
    VM_DCL_SIO_UART_PORT3,                          /* Reserved, UART port 3. */
    VM_DCL_SIO_UART_PORT_RESERVED,                  /* Reserved.*/
    VM_DCL_SIO_USB_PORT1,                           /* Reserved,
                                                       USB COM port 1. */
    VM_DCL_SIO_USB_PORT2                            /* Reserved,
                                                       USB COM port 2. */
} VM_DCL_SIO_DEVICE_SERIAL_PORT;

/*******************************************************************************
 * DESCRIPTION
 *   VM_DCL_SIO_EVENT are serial port hardware events that can be registered
 *   with vm_dcl_registercallback.
 * EXAMPLE
 * <code>
 *     VM_DCL_HANDLE uart_handle;
 *     VM_DCL_MODULE_TYPE usr_id;
 *     usr_id = vm_dcl_get_ownerid();
 *     uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1,usr_id);
 *     vm_dcl_register_callback(uart_handle,
 *                             VM_DCL_SIO_UART_READY_TO_WRITE,
 *                             uart_callback,  // Callback defined by user.
 *                             NULL);
 *
 * </code>
 ******************************************************************************/
typedef enum
{
    /* UART read event. Call vm_dcl_read after receiving this event
       to read the data. */
    VM_DCL_SIO_UART_READY_TO_READ = VM_DCL_SERIPORT_EVENTS_START,

    /* UART write event. This is a notification, the application
       does not have to wait for this event to call vm_dcl_write.
       vm_dcl_write can be called at any time,
       and the outputs will be buffered. */
    VM_DCL_SIO_UART_READY_TO_WRITE
} VM_DCL_SIO_EVENT;


/*******************************************************************************
 * DESCRIPTION
 *  Commands for serial port hardware. DCB stands for Device Control Block.
 *  Call vm_dcl_control with VM_DCL_SIO_COMMAND_SET_DCB_CONFIG
 *  to config uart driver.
 *
 * EXAMPLE
 * <code>
 * VM_DCL_HANDLE uart_handle;
 * VM_DCL_MODULE_TYPE usr_id;
 * vm_dcl_sio_control_dcb_t config;
 * usr_id = vm_dcl_get_ownerid();
 * uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, usr_id);
 * if(VM_DCL_HANDLE_INVALID==uart_handle) return;
 *  config.owner_id = usr_id;
 *  config.config.dsr_check = 0;
 *  config.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
 *  config.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_SW;
 *  config.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
 *  config.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
 *  config.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
 *  config.config.sw_xoff_char = 0x13;
 *  config.config.sw_xon_char = 0x11;
 *  vm_dcl_control(uart_handle,VM_DCL_SIO_COMMAND_SET_DCB_CONFIG,&config);
 * </code>
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_COMMAND_SET_DCB_CONFIG = 0xd      /*uart setting*/
} VM_DCL_SIO_COMMAND;


/*******************************************************************************
 * DESCRIPTION
 * VM_DCL_SIO_UART_BAUDRATE defines values for the baud_rate field
 * in the serial port configuration structure vm_dcl_sio_config_t.
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_UART_BAUDRATE_AUTO = 0,         /* Set the baudrate
                                                  automatically. */
    VM_DCL_SIO_UART_BAUDRATE_75  = 75 ,        /* Set to 75 baud. */
    VM_DCL_SIO_UART_BAUDRATE_150 = 150,        /* Baudrate 150. */
    VM_DCL_SIO_UART_BAUDRATE_300 = 300,        /* Baudrate 300. */
    VM_DCL_SIO_UART_BAUDRATE_600 = 600,        /* Baudrate 600. */
    VM_DCL_SIO_UART_BAUDRATE_1200 = 1200,      /* Baudrate 1200. */
    VM_DCL_SIO_UART_BAUDRATE_2400 = 2400,      /* Baudrate 2400. */
    VM_DCL_SIO_UART_BAUDRATE_4800 = 4800,      /* Baudrate 4800. */
    VM_DCL_SIO_UART_BAUDRATE_7200 = 7200,      /* Baudrate 7200. */
    VM_DCL_SIO_UART_BAUDRATE_9600 = 9600,      /* Baudrate 9600. */
    VM_DCL_SIO_UART_BAUDRATE_14400 = 14400,    /* Baudrate 14400. */
    VM_DCL_SIO_UART_BAUDRATE_19200 = 19200,    /* Baudrate 19200. */
    VM_DCL_SIO_UART_BAUDRATE_28800 = 28800,    /* Baudrate 28800. */
    VM_DCL_SIO_UART_BAUDRATE_33900 = 33900,    /* Baudrate 33900. */
    VM_DCL_SIO_UART_BAUDRATE_38400 = 38400,    /* Baudrate 38400. */
    VM_DCL_SIO_UART_BAUDRATE_57600 = 57600,    /* Baudrate 57600. */
    VM_DCL_SIO_UART_BAUDRATE_115200 = 115200,  /* Baudrate 115200. */
    VM_DCL_SIO_UART_BAUDRATE_230400 = 230400,  /* Baudrate 230400. */
    VM_DCL_SIO_UART_BAUDRATE_460800 = 460800,  /* Baudrate 460800. */
    VM_DCL_SIO_UART_BAUDRATE_921600 = 921600,  /* Baudrate 921600. */
    VM_DCL_SIO_UART_BAUDRATE_1500000 = 1500000 /* Baudrate 1500000. */
} VM_DCL_SIO_UART_BAUDRATE;


/*******************************************************************************
 * DESCRIPTION
 *  Defines values for the data_bits_per_char_length field in
 *  vm_dcl_sio_config_t. This configures the length of data bits during
 *  UART transmission.
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_5=5,    /* 5 bits data length. */
    VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_6,      /* 6 bits data length. */
    VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_7,      /* 7 bits data length. */
    VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8       /* 8 bits data length. */
} VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH;


/*******************************************************************************
 * DESCRIPTION
 *  Defines values for the stop_bits field in
 *  vm_dcl_sio_config_t. This configures the stop bits during
 *  UART transmission.
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_UART_STOP_BITS_1=1,  /* 1 stop bit. */
    VM_DCL_SIO_UART_STOP_BITS_2,    /* 2 stop bits. */
    VM_DCL_SIO_UART_STOP_BITS_1_5   /* 1.5 stop bits. */
} VM_DCL_SIO_UART_STOP_BITS;


/*******************************************************************************
 * DESCRIPTION
 *  Defines values for the parity field in
 *  vm_dcl_sio_config_t. This configures the behavior of parity bit during
 *  UART transmission.
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_UART_PARITY_NONE=0,  /* No parity bit. */
    VM_DCL_SIO_UART_PARITY_ODD,     /* Odd parity bit. */
    VM_DCL_SIO_UART_PARITY_EVEN,    /* Even parity bit. */
    VM_DCL_SIO_UART_PARITY_MARK,    /* Mark parity bit (always 1). */
    VM_DCL_SIO_UART_PARITY_SPACE    /* Space parity bit (always 0). */
} VM_DCL_SIO_UART_PARITY;


/*******************************************************************************
 * DESCRIPTION
 *  Defines values for the flow_control field in
 *  vm_dcl_sio_config_t. This configures hardware or software flow control
 *  is used during UART transmission.
 ******************************************************************************/
typedef enum
{
    VM_DCL_SIO_UART_FLOW_CONTROL_NONE=1, /*none flow control*/
    VM_DCL_SIO_UART_FLOW_CONTROL_HW,     /*hw flow control*/
    VM_DCL_SIO_UART_FLOW_CONTROL_SW      /*sw flow control*/
} VM_DCL_SIO_UART_FLOW_CONTROL_MODE;


/*******************************************************************************
 * DESCRIPTION
 *  This structure is part of the parameter structure
 *  vm_dcl_sio_control_dcb_t struct for command
 *  VM_DCL_SIO_COMMAND_SET_DCB_CONFIG. This structure allows developer
 *  to configure UART parameters such as baud rate and parity bits.
 *  Refer to vm_dcl_sio_config_t for usage example.
 ******************************************************************************/
typedef struct
{

    VM_DCL_SIO_UART_BAUDRATE baud_rate; /* Baud rate for the serial port,
                                           e.g. VM_DCL_SIO_UART_BAUDRATE_115200.
                                           Refer to VM_DCL_SIO_UART_BAUDRATE for
                                           other possible values. */
    VMUINT8 data_bits_per_char_length;  /* Length of data bit, refer to
                                           VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH
                                           for possible values. */
    VMUINT8 stop_bits;                  /* Stop bit(s), refer to
                                           VM_DCL_SIO_UART_STOP_BITS for
                                           possible values. */
    VMUINT8 parity;                     /* Parity bit setting. Refer to
                                           VM_DCL_SIO_UART_PARITY for possible
                                           values. */
    VMUINT8 flow_control;               /* Flow control by hardware or software.
                                           Refer to
                                           VM_DCL_SIO_UART_FLOW_CONTROL_MODE
                                           for enumeration to use. */
    VMUCHAR sw_xon_char;                /* Software flow control character for
                                           XON(transmit on). If the hardware
                                           does not support software
                                           flow control, assign 0x0 to this
                                           field. */
    VMUCHAR sw_xoff_char;               /* Software flow control character for
                                           XFF(transmit off). If the hardware
                                           does not support software
                                           flow control, assign 0x0 to this
                                           field.  */
    VMBOOL dsr_check;                   /* Reserved, always assign 0x0 to
                                           this field. */
} vm_dcl_sio_config_t;


/*******************************************************************************
 * DESCRIPTION
 *  Parameter structure for command VM_DCL_SIO_COMMAND_SET_DCB_CONFIG,
 *  which configures the parameter for the serial port device.
 * EXAMPLE
 * <code>
 * VM_DCL_HANDLE uart_handle;
 * VM_DCL_MODULE_TYPE usr_id;
 * vm_dcl_sio_control_dcb_t config;
 * usr_id = vm_dcl_get_ownerid();
 * uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, usr_id);
 * if(VM_DCL_HANDLE_INVALID==uart_handle) return;
 *  config.owner_id = usr_id;
 *  config.config.dsr_check = 0;
 *  config.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
 *  config.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
 *  config.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
 *  config.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
 *  config.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
 *  config.config.sw_xoff_char = 0x13;
 *  config.config.sw_xon_char = 0x11;
 *  vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG,&config);
 * </code>
 ******************************************************************************/
typedef struct
{
    VMUINT32 owner_id;            /* Owner id of the device. Require owner id
                                     by calling vm_dcl_get_ownerid. */
    vm_dcl_sio_config_t  config;  /* Configuration parameters. Refer to
                                     vm_dcl_sio_config_t for each field. */
} vm_dcl_sio_control_dcb_t;

#ifdef __cplusplus
}
#endif

#endif /* VMDCL_SIO_SDK_H */




