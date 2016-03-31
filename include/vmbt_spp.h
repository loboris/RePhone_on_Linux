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

#ifndef VMBT_SPP_SDK_H
#define VMBT_SPP_SDK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "vmtype.h"
#include "vmbt_cm.h"


/* bt_spp return error type*/
typedef enum
{
    VM_BT_SPP_ERROR_SUCCESS = 0,    /* no error */
    VM_BT_SPP_ERROR_LOGICAL = -5,   /* error in logical, SPP handle open is invalid handle*/
    VM_BT_SPP_ERROR_BAD_PARAMETER,  /* error in bad param, transfer invliad parameter into api*/
    VM_BT_SPP_ERROR_NO_RESOURCE,    /* error in resource, malloc memory fail */
    VM_BT_SPP_ERROR_NOT_SUPPORT,    /* error in support, not support request */
    VM_BT_SPP_ERROR_NO_AUTHORIZATION,/* error in authorization*/
    VM_BT_SPP_MAX = 0x7FFFFFFF      /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_BT_SPP_RESULT;


/*bt_spp security level*/
typedef enum
{
    VM_BT_SPP_SECURITY_NONE = 0x00,             /* no security */
    VM_BT_SPP_SECURITY_AUTHENTICATION = 0x01,   /* security level authentication */
    VM_BT_SPP_SECURITY_AUTHORIZATION = 0x04,    /* security level */
    VM_BT_SPP_SECURITY_ENCRYPTION = 0x10,       /* security level */
    VM_BT_SPP_SECURITY_ALL = 0x7FFFFFFF         /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_BT_SPP_SECURITY_LEVEL;


/* bt_spp srv for app event enum*/
typedef enum
{
    VM_BT_SPP_EVENT_START = 0x0001,         /* SPP event start */
    VM_BT_SPP_EVENT_BIND_FAIL = 0x0002,     /* SPP event bind fail */
    VM_BT_SPP_EVENT_AUTHORIZE = 0x0004,     /* SPP event authorize */
    VM_BT_SPP_EVENT_CONNECT = 0x0008,       /* SPP event connect */
    VM_BT_SPP_EVENT_SCO_CONNECT = 0x0010,   /* SPP event soc connect */
    VM_BT_SPP_EVENT_READY_TO_WRITE = 0x0020,/* SPP event ready to write */
    VM_BT_SPP_EVENT_READY_TO_READ = 0x0040, /* SPP event ready to read */
    VM_BT_SPP_EVENT_DISCONNECT = 0x0080,    /* SPP event disconnect */
    VM_BT_SPP_EVENT_SCO_DISCONNECT = 0x0100,/* SPP event soc disconnect */
    VM_BT_SPP_EVENT_END
} VM_BT_SPP_EVENT;

/* bt_spp open handle */
typedef VMINT VM_BT_SPP_HANDLE;

/* bt SPP connection id*/
typedef VMINT VM_BT_SPP_CONNECTION_ID;

/*spp srv for app event structure */
typedef struct
{
    VM_BT_SPP_HANDLE handle;       /* res handle*/
    void* user_data;              /* user data */
    VMINT result;                 /* result (bool) */
    VM_BT_SPP_CONNECTION_ID connection_id;          /* connection id */
} vm_bt_spp_event_cntx_t;

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_event_callback
 * DESCRIPTION
 *  Callback function of bt_spp.
 * PARAMETERS
 *  event       : [IN] Refer to VM_BT_SPP_EVENT.
 *  parameter   : [IN] Event structure.
 *  user_data   : [IN] User data.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void(*vm_bt_spp_event_callback)(VM_BT_SPP_EVENT event_id, vm_bt_spp_event_cntx_t* event, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_get_min_buffer_size
 * DESCRIPTION
 *  Gets the bt_spp minimum buffer, returns the minimum size for tx and rx buffer in vm_bt_spp_connect()
 *  It is recommended to malloc tx/rx buffer for SPP data transmission, the size is determined by the device
 * PARAMETERS
 * void
 * RETURNS
 *  VMINT : Buffer size, failure if less than 0
 * see also vm_bt_spp_accept()/vm_bt_spp_connect()
 *****************************************************************************/
VMINT vm_bt_spp_get_min_buffer_size(void);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_open
 * DESCRIPTION
 *  Opens bt_spp and returns the resource handle if it is successful
 * PARAMETERS
 * void
 * event_mask:  [IN] VM_BT_SPP_EVENT
 * callback:    [IN] Callback
 * user_data:   [IN] user_data for callback
 * RETURNS
 *  VM_BT_SPP_HANDLE
 *****************************************************************************/
VM_BT_SPP_HANDLE vm_bt_spp_open(VMUINT event_mask, vm_bt_spp_event_callback callback, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_close
 * DESCRIPTION
 *  This function is for destroying an SPP resource
 * PARAMETERS
 *  handle:[IN] Resource handler
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_close(VM_BT_SPP_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_disconnect
 * DESCRIPTION
 * Disconnects an already opened SPP connection
 * PARAMETERS
 *  connection_id :[IN] Connection id, available from vm_bt_spp_event_cntx_t
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_disconnect(VM_BT_SPP_CONNECTION_ID connection_id);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_write
 * DESCRIPTION
 *  Write data over an established connection.
 * PARAMETERS
 *  connection_id   :[IN] Connection ID.
 *  buffer          :[IN] Data buffer of content which will be transfered
 *  size            :[IN] The size of data which will be transfered, maximum size
                    is 5939 byte. If it is required to transfer more than 5939 byte,
                    transfer the first 5939 byte first and then transfer the rest in
                    a separate call.
 * RETURNS
 *  VM_RESULT : If successful it will return the size of data transferred,
 Please view VM_BT_SPP_RESULT for error messages

 *****************************************************************************/
VM_RESULT vm_bt_spp_write(VM_BT_SPP_CONNECTION_ID connection_id, void* buffer, VMUINT size);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_read
 * DESCRIPTION
 *  Read data over an established connection.
 * PARAMETERS
 *  connection_id   :[IN] Connection ID.
 *  buffer          :[IN] Data buffer for incoming data.
 *  size            :[IN] The size of the data that is expected to be received, the maximum size is 5939 byte at once.
 * RETURNS
 *  VM_RESULT : Returns the size of data received successfully,
in case of a failure please view VM_BT_SPP_RESULT for error messages.
 * else
 *****************************************************************************/
VM_RESULT vm_bt_spp_read(VM_BT_SPP_CONNECTION_ID connection_id, void* buffer, VMUINT size);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_get_dev_addr
 * DESCRIPTION
 *  Get the Bluetooth address with the connection ID
 * PARAMETERS
 *  connection_id   :[IN] Connection ID.
 *  bt_address      :[IN] Bluetooth address.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_get_device_address(VM_BT_SPP_CONNECTION_ID connection_id, vm_bt_cm_bt_address_t* bt_address);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_set_security_level
 * DESCRIPTION
 *  Set the security level for the handle
 * PARAMETERS
 *  handle  :[IN] Res handler.
 *  level   :[IN] SPP instance security level, VM_BT_SPP_SECURITY_LEVEL.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_set_security_level(VM_BT_SPP_HANDLE handle, VM_BT_SPP_SECURITY_LEVEL level);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_set_service_name
 * DESCRIPTION
 *  Sets an SPP resource name.
 * PARAMETERS
 *  handle  :[IN] Res handler.
 *  name    :[IN] SPP resource name UCS2.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_set_service_name(VM_BT_SPP_HANDLE handle, VMWCHAR* name);


/* SPP SERVER USING START*/

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_bind
 * DESCRIPTION
 *  Creates an SPP instance.
 * PARAMETERS
 *  handle  :[IN] Handler
 *  uuid    :[IN] Profile ID, only 16bit such as 0x1101
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_bind(VM_BT_SPP_HANDLE handle, VMUINT16 uuid);


/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_accept
 * DESCRIPTION
 *  A server can use this function in order to accept a connection when it is
    notified by the  VM_BT_SPP_EVENT_AUTHORIZE event.
 * PARAMETERS
 *  connection_id :[IN] Connection ID
 *  buffer :[IN] tx rx buffer
 *  tx_size :[IN] tx buffer size
 *  rx_size :[IN] rx buffer size
 * For example:
 *  buffer[0][1][2]...[tx_size -1][tx_size][tx_size+1]....[tx_size+rx_size-1]
 *  ------[-----tx_buffer--------][----------rx_buffer----------------------]
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_accept(VM_BT_SPP_CONNECTION_ID connection_id,
                           void* buffer,
                           VMUINT tx_size,
                           VMUINT rx_size
                          );


/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_reject
 * DESCRIPTION
 *  The server can use this function to reject when it is notified by the VM_BT_SPP_EVENT_AUTHORIZE event.
 * PARAMETERS
 *  connection_id : Connection ID
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_reject(VM_BT_SPP_CONNECTION_ID connection_id);

/* SPP SERVER USING END */


/* SPP CLIENT USING START */

/*****************************************************************************
 * FUNCTION
 *  vm_bt_spp_connect
 * DESCRIPTION
 *  Connects to a server
 * PARAMETERS
 *  handle :[IN] Res handler
 *  bt_address :[IN] Server's Bluetooth address
 *  buffer :[IN] tx rx buffer
 *  tx_size :[IN] tx buffer size
 *  rx_size :[IN] rx buffer size
 *  uuid :[IN] Profile ID, this should be set same as the server's UUID, only 16bit, such as 0x1101
 * for example
 *  buffer[0][1][2]...[tx_size -1][tx_size][tx_size+1]....[tx_size+rx_size-1]
 *  ------[-----tx_buffer--------][----------rx_buffer----------------------]
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_spp_connect(VM_BT_SPP_HANDLE handle,
                            const vm_bt_cm_bt_address_t* bt_address,
                            void* buffer,
                            VMUINT tx_size,
                            VMUINT rx_size,
                            VMUINT16 uuid);


/* SPP CLIENT USING END */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* VMBT_SPP_SDK_H */
