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

#ifndef VMBT_GATT_SDK_H
#define VMBT_GATT_SDK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "vmtype.h"

/* GATT attribute handle */
#define VM_BT_GATT_ATTRIBUTE_HANDLE VMUINT16
typedef VMUINT16 VM_BT_GATT_SERVICE_HANDLE;
typedef VMUINT16 VM_BT_GATT_CHARACTERISTIC_HANDLE;
typedef VMUINT16 VM_BT_GATT_DESCRIPTOR_HANDLE;

/* GATT attribute max value length */
#define VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH 512

/* GATT attribute handle */
#define VM_BT_GATT_ADDRESS_SIZE 6

/* GATT characteristic properties. These are bit flags */
typedef VMUINT32 VM_BT_GATT_CHAR_PROPERTIES;
#define VM_BT_GATT_CHAR_PROPERTY_BROADCAST         0x01
#define VM_BT_GATT_CHAR_PROPERTY_READ              0x02
#define VM_BT_GATT_CHAR_PROPERTY_WRITE_WO_RESPONSE 0x04
#define VM_BT_GATT_CHAR_PROPERTY_WRITE             0x08
#define VM_BT_GATT_CHAR_PROPERTY_NOTIFY            0x10
#define VM_BT_GATT_CHAR_PROPERTY_INDICATE          0x20
#define VM_BT_GATT_CHAR_PROPERTY_SIGNED_WRITE      0x40
#define VM_BT_GATT_EXT_PROPERTY                    0x80

/* GATT characteristic permission */
typedef VMUINT32 VM_BT_GATT_PERMISSION;
#define VM_BT_GATT_PERMISSION_READ                   (1 << 0) /* bit 0 */
#define VM_BT_GATT_PERMISSION_READ_ENCRYPTED         (1 << 1) /* bit 1 */
#define VM_BT_GATT_PERMISSION_READ_ENC_MITM          (1 << 2) /* bit 2 */
#define VM_BT_GATT_PERMISSION_WRITE                  (1 << 4) /* bit 4 */
#define VM_BT_GATT_PERMISSION_WRITE_ENCRYPTED        (1 << 5) /* bit 5 */
#define VM_BT_GATT_PERMISSION_WRITE_ENC_MITM         (1 << 6) /* bit 6 */
#define VM_BT_GATT_PERMISSION_WRITE_SIGNED           (1 << 7) /* bit 7 */
#define VM_BT_GATT_PERMISSION_WRITE_SIGNED_MITM      (1 << 8) /* bit 8 */


/* GATT context handle */
typedef void* VM_BT_GATT_CONTEXT_HANDLE;

/* GATT connection handle */
typedef void* VM_BT_GATT_CONNECTION_HANDLE;


/* GATT address */
typedef struct
{
    VMUINT8 data[VM_BT_GATT_ADDRESS_SIZE]; /* BLE bd_addr, little-endian */
} vm_bt_gatt_address_t;

/* GATT attribute value  */
typedef struct
{
    VMUINT16 length; /* length of attribute value */
    VMUINT8 data[VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH]; /* attribute value buffer */
} vm_bt_gatt_attribute_value_t;

/* Bluetooth UUID */
typedef struct
{
    VMUINT8 length;       /* Set to 2 for 16-bit UUID;
                             Set to 16 for 128-bit UUID. */
    VMUINT8 uuid[16];     /* UUID, starting from most significant byte.
                             uuid[0...1] is valid when length is 2;
                             uuid[0...15] is valid when length is 16. */
} vm_bt_uuid_with_length_t;

/* GATT attribute UUID */
typedef struct
{
    vm_bt_uuid_with_length_t uuid;
    VMUINT8 inst;       /* Attribute handle. This field is used when
                           vm_bt_gatt_attribute_uuid_t is returned from
                           callbacks.
                           This field can be ignored when adding services,
                           characteristics and descriptors. */
} vm_bt_gatt_attribute_uuid_t;

/* GATT Service */
typedef struct
{
    vm_bt_gatt_attribute_uuid_t uuid;
    VMUINT8 is_primary;
} vm_bt_gatt_service_info_t; 


/*GATT connection handle structure  */
typedef struct
{
    VM_BT_GATT_CONTEXT_HANDLE context_handle;
    VM_BT_GATT_CONNECTION_HANDLE connection_handle;
} vm_bt_gatt_connection_t;


/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_register_server_callback
 * DESCRIPTION
 *    The callback for vm_bt_gatt_server_register.
 * PARAMETERS
 *    context:   [IN] Registered context, the profile must save it because it can be used in other APIs
 *    has_error: [IN] Non-zero indicates an error otherwise it's a success.
 *    app_uuid:  [IN] Profile UUID specify in register
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_register_server_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error, VMUINT8 app_uuid[16]);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_connection_callback
 * DESCRIPTION
 *    Connect confirm callback
 * PARAMETERS
 *    context:      [IN] Register context from the connect request
 *    connected:    [IN] TRUE: successful, FALSE: Unsuccessful
 *    gatt_address: [IN] Bluetooth address
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_connection_callback)(const vm_bt_gatt_connection_t* connection, VMBOOL connected, const vm_bt_gatt_address_t* gatt_address);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_register_server_callback
 * DESCRIPTION
 *    Listen confirm callback
 * PARAMETERS
 *    context:      [IN] Registered context from the listen request.
 *    has_error:    [IN] Non-zero indicates an error otherwise it's a success.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_listen_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error);

/** Callback invoked in response to create_service */
typedef void (*vm_bt_gatt_service_added_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        vm_bt_gatt_service_info_t* service_id, VM_BT_GATT_SERVICE_HANDLE service);

/** Callback indicating that an included service has been added to a service */
typedef void (*vm_bt_gatt_included_service_added_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service, VM_BT_GATT_SERVICE_HANDLE include_service);

/** Callback invoked when a characteristic has been added to a service */
typedef void (*vm_bt_gatt_characteristic_added_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        vm_bt_gatt_attribute_uuid_t* uuid, VM_BT_GATT_SERVICE_HANDLE service, VM_BT_GATT_CHARACTERISTIC_HANDLE characteristic);

/** Callback invoked when a descriptor has been added to a characteristic */
typedef void (*vm_bt_gatt_descriptor_added_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        vm_bt_gatt_attribute_uuid_t* uuid, VM_BT_GATT_SERVICE_HANDLE service, VM_BT_GATT_DESCRIPTOR_HANDLE descriptor);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_service_started_callback
 * DESCRIPTION
 *    Start service callback
 * PARAMETERS
 *    has_error:    [IN] Non-zero indicates an error otherwise it's a success.
 *    context:      [IN] Registered context from the listen request.
 *    service:      [IN] Service handle.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_service_started_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service);

/** Callback invoked in response to stop_service */
typedef void (*vm_bt_gatt_service_stopped_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service);

/** Callback triggered when a service has been deleted */
typedef void (*vm_bt_gatt_service_deleted_callback)(VMBOOL has_error, VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service);


/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_request_read_callback
 * DESCRIPTION
 *    Client read characteristic request indication
 * PARAMETERS
 *    connection :      [IN] Connection context from vm_bt_gatt_connection_callback.
 *    transmit_id :     [IN] Transaction ID (used when sending a response).
 *    gatt_address :    [IN] Bluetooth address.
 *    attr_handle :     [IN] Attribute handle.
 *    offset:           [IN] Read offset.
 *    is_long:          [IN] True: Read long(characteristic/descriptor), False: not long
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_request_read_callback)(vm_bt_gatt_connection_t* connection, VMUINT16 transmit_id, vm_bt_gatt_address_t* gatt_address,
        VM_BT_GATT_CHARACTERISTIC_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_request_write_callback
 * DESCRIPTION
 *    Client write characteristic request indication
 * PARAMETERS
 *    connection :      [IN] Connection context from vm_bt_gatt_connection_callback.
 *    transmit_id :     [IN] Transaction ID(used when send response).
 *    gatt_address :    [IN] BT Address.
 *    attr_handle :     [IN] Attribute handle.
 *    value :           [IN] attribute value.
 *    offset:           [IN] Write offset.
 *    need_response:    [IN] True: needs a response(write request), False: doesn't need response(write command)
 *    is_prep:          [IN] Set to true in order to write long characteristic/reliable writes
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_request_write_callback)(vm_bt_gatt_connection_t* connection, VMUINT16 transmit_id, vm_bt_gatt_address_t* gatt_address,
        VM_BT_GATT_CHARACTERISTIC_HANDLE attr_handle, vm_bt_gatt_attribute_value_t* value, VMUINT16 offset,
        VMBOOL need_response, VMBOOL is_prep);

/** Callback invoked when a previously prepared write is to be executed */
typedef void (*vm_bt_gatt_request_exec_write_callback)(vm_bt_gatt_connection_t* connection, VMUINT16 transmit_id,
        vm_bt_gatt_address_t* gatt_address, VMBOOL cancel);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_response_confirmation_callback
 * DESCRIPTION
 *    Send response confirm callback
 * PARAMETERS
 *    has_error:    [IN] Non-zero indicates an error otherwise it's a success.
 *    handle:       [IN] Attribute handle.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_response_confirmation_callback)(VMBOOL has_error, VM_BT_GATT_CHARACTERISTIC_HANDLE handle);

/** Callback triggered in response to read_tx_power */
typedef void (*vm_bt_gatt_read_tx_power_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error, vm_bt_gatt_address_t* gatt_address, VMUINT8 tx_power);

/* GATT service callback function structure */
typedef struct
{
    vm_bt_gatt_register_server_callback        register_server;         /* Register server callback. */
    vm_bt_gatt_connection_callback             connection;              /* Connection status callback */
    vm_bt_gatt_listen_callback                 listen;                  /* Listen callback */
    vm_bt_gatt_service_added_callback          service_added;           /* Service has/hasn't been added callback */
    vm_bt_gatt_included_service_added_callback included_service_added;  /* Service has/hasn't been included callback */
    vm_bt_gatt_characteristic_added_callback   characteristic_added;    /* Characteristic has/hasn't been added callback */
    vm_bt_gatt_descriptor_added_callback       descriptor_added;        /* Descriptor has/hasn't been added callback */
    vm_bt_gatt_service_started_callback        service_started;         /* Service started status callback */
    vm_bt_gatt_service_stopped_callback        service_stopped;         /* Service stopped callback */
    vm_bt_gatt_service_deleted_callback        service_deleted;         /* Service has been deleted callback */
    vm_bt_gatt_request_read_callback           request_read;            /* Service requests read callback */
    vm_bt_gatt_request_write_callback          request_write;           /* Service requests write callback */
    vm_bt_gatt_request_exec_write_callback     request_exec_write;      /* Request execute write callback */
    vm_bt_gatt_response_confirmation_callback  response_confirmation;   /* Response confirm callback */
    vm_bt_gatt_read_tx_power_callback          read_tx_power;           /* Tx power callback */
} vm_bt_gatt_server_callback_t;

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_register
 * DESCRIPTION
 *  Registers a GATT server application with the stack.
 * PARAMETERS
 *  uuid:       [IN] BT UUID.
 *  callback:   [IN] The pointer of callback structure.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_register(const VMUINT8 app_uuid[16], vm_bt_gatt_server_callback_t* callback);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_deregister
 * DESCRIPTION
 *  De-register profile from GATT
 * PARAMETERS
 *  context  :[IN] Registered context, profile can get it within the register callback.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_deregister(VM_BT_GATT_CONTEXT_HANDLE context);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_connect
 * DESCRIPTION
 *  Connect Request
 * PARAMETERS
 *  context  :      [IN] Registered context, profile can get it within the register callback.
 *  gatt_address  : [IN] Bluetooth address.
 *  direct  :       [IN] TRUE: direct connect, FALSE: background connect.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_connect(VM_BT_GATT_CONTEXT_HANDLE context, const vm_bt_gatt_address_t* gatt_address, VMBOOL direct);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_disconnect
 * DESCRIPTION
 *  Disconnect request
 * PARAMETERS
 *  connection :    [IN] Connection context from vm_bt_gatt_connection_callback.
 *  gatt_address  : [IN] Bluetooth Address
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_disconnect(vm_bt_gatt_connection_t* connection, const vm_bt_gatt_address_t* gatt_address);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_listen
 * DESCRIPTION
 *  Send listen request to listen connect indicate during disconnect state.
 * PARAMETERS
 *  register_context :  [IN] The context of the GATT server.
 *  start :             [IN] Indicates if it should start or stop listening.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_listen(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL start);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_add_service
 * DESCRIPTION
 *  Creates a new service. The result and corresponding service handle
 *  is passed to vm_bt_gatt_service_added_callback for each services created.
 *  Therefore, vm_bt_gatt_service_added_callback will be invoked 5 times
 *  if num_handles is set to 5.
 * PARAMETERS
 *  context  :      [IN] Server context from vm_bt_gatt_register_server_callback.
 *  service_id :    [IN] Array of service UUIDs for the services to be created.
 *  num_handles :   [IN] Number of elements in array service_id. The maximum
 *                       number of services can be created depends on the
 *                       available memory resource.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_add_service(VM_BT_GATT_CONTEXT_HANDLE context,
                                        const vm_bt_gatt_service_info_t* service_id,
                                        VMUINT16 num_handles);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_add_included_service
 * DESCRIPTION
 *  Assign an included service to it's parent service.
 *  To include multiple services to a parent service,
 *  call this function multiple times.
 * PARAMETERS
 *  context:                [IN] The context of the GATT server.
 *  service:                [IN] Handle to the parent service.
 *  service_to_be_included: [IN] Handle to the service to be included.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_add_included_service(VM_BT_GATT_CONTEXT_HANDLE context, VM_BT_GATT_SERVICE_HANDLE service, VM_BT_GATT_ATTRIBUTE_HANDLE service_to_be_included);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_add_characteristic
 * DESCRIPTION
 *  Adds a characteristic to a service.
 * PARAMETERS
 *  context:                [IN] The context of the GATT server.
 *  service:                [IN] Service handle from vm_bt_gatt_service_added_callback.
 *  characteristic_uuid:    [IN] The UUID of the characteristic to add.
 *  properties:             [IN] Characteristic properties, use the bit flags
 *                               VM_BT_GATT_CHAR_PROPERTIES.
 *  permission:             [IN] Read and write permission of the characteristic.
 *                               Use the bit flags VM_BT_GATT_PERMISSION.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_add_characteristic(VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service,
        const vm_bt_uuid_with_length_t* characteristic_uuid,
        VM_BT_GATT_CHAR_PROPERTIES properties, VM_BT_GATT_PERMISSION permission);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_add_descriptor
 * DESCRIPTION
 *  Adds a descriptor to a given service.
 * PARAMETERS
 *  context:            [IN] The context of the GATT server.
 *  service:            [IN] Service handle from vm_bt_gatt_service_added_callback.
 *  descriptor_uuid:    [IN] UUID of the descriptor to add. The inst field can be ignored.
 *  permission:         [IN] Permission of the descriptor. Use bit flags defined in VM_BT_GATT_PERMISSION.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_add_descriptor(VM_BT_GATT_CONTEXT_HANDLE context, VM_BT_GATT_SERVICE_HANDLE service,
        const vm_bt_gatt_attribute_uuid_t* descriptor_uuid, VM_BT_GATT_PERMISSION permission);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_start_service
 * DESCRIPTION
 *  Start service(add service to ATT database)
 * PARAMETERS
 *  context:    [IN] Register context, profile can get it in register callback
 *  service:    [IN] Service handle from vm_bt_gatt_service_added_callback.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_start_service(VM_BT_GATT_CONTEXT_HANDLE context,
        VM_BT_GATT_SERVICE_HANDLE service);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_stop_service
 * DESCRIPTION
 *  Stops a local service.
 * PARAMETERS
 *  context: [IN] The context of the GATT server.
 *  service: [IN] The service.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_stop_service(VM_BT_GATT_CONTEXT_HANDLE context, VM_BT_GATT_SERVICE_HANDLE service);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_delete_service
 * DESCRIPTION
 *  Deletes a local service.
 * PARAMETERS
 *  context: [IN] The context of the GATT server.
 *  service: [IN] The service.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_delete_service(VM_BT_GATT_CONTEXT_HANDLE context, VM_BT_GATT_SERVICE_HANDLE service);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_send_indication
 * DESCRIPTION
 *  Notify/Indicate data to Client
 * PARAMETERS
 *  connection:         [IN] Connection context from vm_bt_gatt_connection_callback.
 *  attribute_handle:   [IN] Characteristic attribute handle.
 *  need_confirm:       [IN] TRUE: Indication, FALSE: Notification.
 *  value:              [IN] Attribute value.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_send_indication(vm_bt_gatt_connection_t* connection, VM_BT_GATT_CHARACTERISTIC_HANDLE attribute_handle,
        VMBOOL need_confirm, vm_bt_gatt_attribute_value_t* value);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_send_response
 * DESCRIPTION
 *  Send response to client for write request
 * PARAMETERS
 *  connection:  [IN] Connection context from vm_bt_gatt_connection_callback.
 *  transmit_id: [IN] Transaction ID (the same with write/read request).
 *  has_error:   [IN] Non-zero indicates an error otherwise it's a success.
 *  handle:      [IN] Attribute handle (the same with write/read request).
 *  value:       [IN] Attribute value (the same with write request, ACK data of read request)
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_send_response(vm_bt_gatt_connection_t* connection, VMUINT16 transmit_id,
        VMBOOL has_error, VM_BT_GATT_CHARACTERISTIC_HANDLE handle, vm_bt_gatt_attribute_value_t* value);


/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_read_tx_power
 * DESCRIPTION
 *  Queries tx power of local device. For Proximity profile. (Songmin Liao)
 * PARAMETERS
 *  context:        [IN] The context of the GATT server.
 *  gatt_address:   [IN] Bluetooth address. 
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_read_tx_power(VM_BT_GATT_CONTEXT_HANDLE context, vm_bt_gatt_address_t* gatt_address);



/* authentication requirement */
typedef VMUINT8 VM_BT_GATT_CLIENT_AUTH_REQ;
#define VM_BT_GATT_CLIENT_AUTH_REQ_NONE              0 /* Nothing. */
#define VM_BT_GATT_CLIENT_AUTH_REQ_NO_MITM           1 /* Unauthenticated encryption. */
#define VM_BT_GATT_CLIENT_AUTH_REQ_MITM              2 /* Authenticated encryption. */
#define VM_BT_GATT_CLIENT_AUTH_REQ_SIGNED_NO_MITM    3 /* Signed no MITM protection. */
#define VM_BT_GATT_CLIENT_AUTH_REQ_SIGNED_MITM       4 /* Singed with MITM proection. */

/* GATT client write types */
typedef VMUINT8 VM_BT_GATT_CLIENT_WRITE_TYPE;
#define VM_BT_GATT_CLIENT_WRITE_TYPE_NO_RSP          1 /* Write with no response. */
#define VM_BT_GATT_CLIENT_WRITE_TYPE_REQUEST         2 /* Write request. */
#define VM_BT_GATT_CLIENT_WRITE_TYPE_PREPARE         3 /* Write prepare. */

/* GATT client device types */
typedef VMUINT8 VM_BT_GATT_CLIENT_DEV_TYPE;
#define VM_BT_GATT_CLIENT_DEV_TYPE_UNKNOWN           0 /* Unknown device. */
#define VM_BT_GATT_CLIENT_DEV_TYPE_LE                1 /* Low energy device. */
#define VM_BT_GATT_CLIENT_DEV_TYPE_BR_EDR            2 /* Br edr device. */
#define VM_BT_GATT_CLIENT_DEV_TYPE_BR_EDR_LE         3 /* Br edr low energy device. */


/* GATT client characteristic structure */
typedef struct
{
    vm_bt_gatt_service_info_t* svc_uuid; /* Service UUID. */
    vm_bt_gatt_attribute_uuid_t* ch_uuid; /* Characteristic UUID. */
} vm_bt_gatt_client_characteristic_t;

/* GATT client descriptor structure */
typedef struct
{
    vm_bt_gatt_service_info_t* svc_uuid; /* Service UUID */
    vm_bt_gatt_attribute_uuid_t* ch_uuid; /* Characteristic UUID */
    vm_bt_gatt_attribute_uuid_t* descriptor_uuid; /* Descriptor UUID */
} vm_bt_gatt_client_descriptor_t;


/** Callback invoked in response to register. */
typedef void (*vm_bt_gatt_register_client_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error, VMUINT8 app_uuid[16]);

/** Callback for scan results. */
typedef void (*vm_bt_gatt_scan_result_callback)(VM_BT_GATT_CONTEXT_HANDLE context, vm_bt_gatt_address_t* gatt_address, VMINT32 rssi, VMUINT8 eir_length, VMUINT8* eir);

/** Callback triggered in response to set_advertisement_data. */
typedef void (*vm_bt_gatt_set_advertisement_data_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_search_complete_callback
 * DESCRIPTION
 *    Search complete
 * PARAMETERS
 *    connection:[IN] Connection context from vm_bt_gatt_connection_callback.
 *    has_error: [IN] Non-zero indicates an error, otherwise its a success.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_search_complete_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_search_result_callback
 * DESCRIPTION
 *    Indicates the profile when finding one primary service, the profile can save it in its own field
 * PARAMETERS
 *    connection:   [IN] Connection context from vm_bt_gatt_connection_callback.
 *    uuid:         [IN] Service UUID
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_search_result_callback)(vm_bt_gatt_connection_t* connection, vm_bt_gatt_service_info_t* uuid);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_get_characteristic_callback
 * DESCRIPTION
 *    Get characteristic confirm callback
 * PARAMETERS
 *    connection:   [IN] Connection context from vm_bt_gatt_connection_callback.
 *    has_error:    [IN] Non-zero indicates an error, otherwise its a success.
 *    ch:           [IN] Characteristic information.
 *    properties:   [IN] Characteristic property.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_get_characteristic_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_client_characteristic_t* ch, VM_BT_GATT_CHAR_PROPERTIES properties);

/** GATT descriptor enumeration result callback. */
typedef void (*vm_bt_gatt_get_descriptor_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error, vm_bt_gatt_client_descriptor_t* descr);

/** GATT included service enumeration result callback. */
typedef void (*vm_bt_gatt_get_included_service_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_service_info_t* svc_uuid, vm_bt_gatt_service_info_t* include_svc_uuid);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_register_for_notification_callback
 * DESCRIPTION
 *    Register notification callback
 * PARAMETERS
 *    context:      [IN] Register context, profile can get it within register callback
 *    has_error:    [IN] Non-zero indicates an error, otherwise its a success.
 *    gatt_address: [IN] Bluetooth Address.
 *    ch:           [IN] Characteristic information.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_register_for_notification_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error,
        vm_bt_gatt_address_t* gatt_address, vm_bt_gatt_client_characteristic_t* ch);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_notify_callback
 * DESCRIPTION
 *    Remote device notification callback, invoked when a remote device sends
 *    a notification or indication that a client has registered.
 * PARAMETERS
 *    connection:    [IN] Connection context from vm_bt_gatt_connection_callback.
 *    gatt_address:  [IN] Address of the remote device.
 *    ch:            [IN] Characteristic information.
 *    value:         [IN] Characteristic value.
 *    is_notify:     [IN] VM_TRUE if it is a notification; VM_FALSE if
 *                        it is an indication.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_notify_callback)(vm_bt_gatt_connection_t* connection,
        vm_bt_gatt_address_t* gatt_address,
        vm_bt_gatt_client_characteristic_t* ch,
        vm_bt_gatt_attribute_value_t* value,
        VMBOOL is_notify);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_read_characteristic_callback
 * DESCRIPTION
 *    Read characteristic value confirm callback
 * PARAMETERS
 *    context:      [IN] Connection context, profile can get it within connect callback.
 *    has_error:    [IN] Non-zero indicates an error, otherwise its a success.
 *    ch:           [IN] Characteristic information.
 *    value:        [IN] Characteristic value.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_read_characteristic_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_client_characteristic_t* ch, vm_bt_gatt_attribute_value_t* value);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_write_characteristic_callback
 * DESCRIPTION
 *    Write characteristic value confirm callback
 * PARAMETERS
 *    context:      [IN] Connection context, profile can get it within connect callback.
 *    has_error:    [IN] Non-zero indicates an error, otherwise its a success.
 *    ch:           [IN] Characteristic information.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_write_characteristic_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_client_characteristic_t* ch);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_read_descriptor_callback
 * DESCRIPTION
 *    Read characteristic descriptor value confirm callback
 * PARAMETERS
 *    connection:   [IN] Connection context from vm_bt_gatt_connection_callback.
 *    has_error:    [IN] Non-zero indicates an error, otherwise its a success.
 *    descr:        [IN] Characteristic  descriptor information.
 *    value:        [IN] Characteristic descriptor value.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_read_descriptor_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_client_descriptor_t* descr, vm_bt_gatt_attribute_value_t* value);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_gatt_write_descriptor_callback
 * DESCRIPTION
 *    Read characteristic descriptor value confirm callback
 * PARAMETERS
 *    connection:[IN] Connection context from vm_bt_gatt_connection_callback.
 *    has_error: [IN] Non-zero indicates an error, otherwise its a success.
 *    descr:     [IN] Characteristic  descriptor information.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_gatt_write_descriptor_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error,
        vm_bt_gatt_client_descriptor_t* descr);

/** GATT execute prepared write callback. */
typedef void (*vm_bt_gatt_execute_write_callback)(vm_bt_gatt_connection_t* connection, VMBOOL has_error);

/** Callback invoked in response to read_remote_rssi. */
typedef void (*vm_bt_gatt_read_remote_rssi_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error, vm_bt_gatt_address_t* gatt_address, VMINT32 rssi);

/** Callback invoked in response to get_device_type. */
typedef void (*vm_bt_gatt_get_device_type_callback)(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL has_error, vm_bt_gatt_address_t* gatt_address, VM_BT_GATT_CLIENT_DEV_TYPE dev_type);


/* GATT service callback function structure */
typedef struct
{
    vm_bt_gatt_register_client_callback            register_client; /* Register client callback. */
    vm_bt_gatt_scan_result_callback                scan_result;/* Scan Bluetooth 4.0 device callback */
    vm_bt_gatt_connection_callback                 connection;/* Connection status callback */
    vm_bt_gatt_listen_callback                     listen;/* Listen callback */
    vm_bt_gatt_set_advertisement_data_callback     set_advertisement_data;/* Set the advertisement info callback. */
    vm_bt_gatt_search_complete_callback            search_complete;/* Search services has ended callback. */
    vm_bt_gatt_search_result_callback              search_result;/* Search services on going callback. */
    vm_bt_gatt_get_characteristic_callback         get_characteristic;/* Get service's characteristic callback. */
    vm_bt_gatt_get_descriptor_callback             get_descriptor;/* Get service's descriptor callback. */
    vm_bt_gatt_get_included_service_callback       get_included_service;/* Get included services callback. */
    vm_bt_gatt_register_for_notification_callback  register_for_notification;/* Register notification callback. */
    vm_bt_gatt_notify_callback                     notify;/* Remote notify callback. */
    vm_bt_gatt_read_characteristic_callback        read_characteristic;/* Read remote characteristic callback. */
    vm_bt_gatt_write_characteristic_callback       write_characteristic;/* Write remote characteristic callback. */
    vm_bt_gatt_read_descriptor_callback            read_descriptor;/* Read characteristic's descriptor callback. */
    vm_bt_gatt_write_descriptor_callback           write_descriptor;/* Write characteristic's descriptor callback. */
    vm_bt_gatt_execute_write_callback              execute_write;/* Execute write callback. */
    vm_bt_gatt_read_remote_rssi_callback           read_remote_rssi;/* Read remote Bluetooth 4.0 RSSI callback */
    vm_bt_gatt_get_device_type_callback            get_device_type;/* Get device type callback */
} vm_bt_gatt_client_callback_t;

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_register
 * DESCRIPTION
 *  Registers a GATT client application with the stack.
 *  The resulting client context is passed from vm_bt_gatt_register_client_callback.
 * PARAMETERS
 *  app_uuid :[IN] Bluetooth UUID.
 *  callback :[IN] The pointer of the callback structure.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_register(const VMUINT8 app_uuid[16], vm_bt_gatt_client_callback_t* callback);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_deregister
 * DESCRIPTION
 *  De-register confirm callback
 * PARAMETERS
 *  context: [IN] Client context from vm_bt_gatt_register_client_callback.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_deregister(VM_BT_GATT_CONTEXT_HANDLE context);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_scan
 * DESCRIPTION
 *  Starts or stops LE device scanning.
 * PARAMETERS
 *  context:    [IN] Client context from vm_bt_gatt_register_client_callback.
 *  start:      [IN] Indicates if it should start or stop
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_scan(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL start);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_connect
 * DESCRIPTION
 *  Send connect remote device to GATT
 * PARAMETERS
 *  context:        [IN] Client context from vm_bt_gatt_register_client_callback.
 *  gatt_address:   [IN] Bluetooth Address
 *  direct:         [IN] TRUE: Direct connect (try connect once, if failed, callback will return in 20 seconds.
 *                       FALSE: Background connect (try connect in background. If the device appears, callback will be invoked.)
 *                       Note that once the device is connected, background connect stops.
 *                       Therefore, to continuously searching for a certain device, call vm_bt_gatt_client_connect
 *                       vm_bt_gatt_client_connect with direct set to FALSE when receiving
 *
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_connect(VM_BT_GATT_CONTEXT_HANDLE context, vm_bt_gatt_address_t* gatt_address, VMBOOL direct);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_disconnect
 * DESCRIPTION
 *  Send disconnect remote device to GATT
 * PARAMETERS
 *  connection:     [IN] Connection context from vm_bt_gatt_connection_callback.
 *  gatt_address:   [IN] Bluetooth Address
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_disconnect(vm_bt_gatt_connection_t* connection, vm_bt_gatt_address_t* gatt_address);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_listen
 * DESCRIPTION
 *  Sends listen request to listen connect indicate during disconnected state
 * PARAMETERS
 *  context:  [IN] Client context from vm_bt_gatt_register_client_callback.
 *  start:    [IN] TRUE: listen; FALSE: not listen.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_listen(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL start);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_server_set_advertisement_data
 * DESCRIPTION
 *  Configures server advertisement behavior and packet data.
 * PARAMETERS
 *  context:            [IN] Client context from vm_bt_gatt_register_client_callback.
 *  set_scan_response:  [IN] Reserved, must be 0.
 *  include_name:       [IN] Set to VM_TRUE to include device name in the
 *                           advertisement packet. Set to VM_FALSE to exclude.
 *  include_txpower:    [IN] Set to VM_TRUE to include tx power in the
 *                           advertisement packet. Set to VM_FALSE to exclude.
 *  min_interval:       [IN] Set the minimum advertisement interval.
 *  max_interval:       [IN] Set the maximum advertisement interval.
 *  appearance:         [IN] The appearance category as in Bluetoot 4.0 spec.
 *  manufacturer_length:[IN] Length of the manufacturer_data.
 *  manufacturer_data:  [IN] Manufacturer data in the advertisement packet.
 *  service_data_length:[IN] Length of the service_data.
 *  service_data:       [IN] Service data in the advertisement packet.
 *  service_uuid_length:[IN] Optional, number of services in service_uuid array.
 *  service_uuid:       [IN] Optional, an array of service UUIDs to be included
 *                           in the advertisement packet. Currently,
 *                           only 16-bit UUIDs are supported.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_server_set_advertisement_data(VM_BT_GATT_CONTEXT_HANDLE context, VMBOOL set_scan_response, VMBOOL include_name, VMBOOL include_txpower,
        VMUINT16 min_interval, VMUINT16 max_interval, VMUINT16 appearance,
        VMUINT16 manufacturer_length, VMCHAR* manufacturer_data,
        VMUINT16 service_data_length, VMCHAR* service_data,
        vm_bt_uuid_with_length_t* service_uuid);


/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_search_service
 * DESCRIPTION
 *  Enumerates all GATT services on a connected device.
 *  Optionally, the results can be filtered for a given UUID.
 * PARAMETERS
 *  connection  :  [IN] Connection context from vm_bt_gatt_connection_callback.
 *  service_uuid:  [IN] Search the specify UUID. Support both 16-bit and 128-bit UUID.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_search_service(const vm_bt_gatt_connection_t* connection,
        const vm_bt_uuid_with_length_t* service_uuid
        //       append BT Base UUID for 16-bit UUID because underlying implementation supports only 128-bit entries.
        //       The base UUID is :00000000-0000-1000-8000-00805F9B34FB
        //       (https://www.bluetooth.org/en-us/specification/assigned-numbers/service-discovery)
                                          );


/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_get_included_service
 * DESCRIPTION
 *  Enumerates included services for a given parent service.
 *  Set start_include_service_id to NULL to get the first included service.
 *  The result is passed back from callback vm_bt_gatt_get_included_service_callback.
 *  After receiving the included service UUID, call vm_bt_gatt_client_get_included_service
 *  again and set include_svc_uuid to the service UUID to query the next
 *  included service in the parent service.
 * PARAMETERS
 *  connection:       [IN] Connection context from vm_bt_gatt_connection_callback.
 *  svc_uuid:         [IN] UUID of the parent service.
 *  include_svc_uuid: [IN] Set to NULL for the 1st included service.
 *                         Set to a included service UUID for the next
 *                         included service UUID in the same parent service.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_get_included_service(const vm_bt_gatt_connection_t* connection,
        const vm_bt_gatt_service_info_t* svc_uuid,
        const vm_bt_gatt_service_info_t* include_svc_uuid);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_get_characteristic
 * DESCRIPTION
 *  Enumerates characteristics for a given service.
 *  Set start_characteristic_uuid to NULL to get the first characteristic.
 * PARAMETERS
 *  connection:                 [IN] Connection context from vm_bt_gatt_connection_callback.
 *  svc_uuid:                   [IN] UUID of the service.
 *  start_characteristic_uuid:  [IN] Set to NULL for the 1st characteristic.
 *                                   Set to a included characteristic UUID for the next
 *                                   characteristic in the same service.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_get_characteristic(const vm_bt_gatt_connection_t* connection, const vm_bt_gatt_service_info_t* svc_uuid, const vm_bt_gatt_attribute_uuid_t* start_characteristic_uuid);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_get_descriptor
 * DESCRIPTION
 *  Enumerates descriptors for a given characteristic.
 *  Set start_descriptor_id to NULL to get the first descriptor.
 * PARAMETERS
 *  connection:             [IN] Connection context from vm_bt_gatt_connection_callback.
 *  characteristic_info:    [IN] UUID of the characteristic to query.
 *  start_descriptor_uuid:  [INOUT] Set to NULL for the 1st descriptor.
 *                                   Set to a descriptor UUID for the next
 *                                   descriptor in the same characteristic.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_get_descriptor(const vm_bt_gatt_connection_t* connection,
        const vm_bt_gatt_client_characteristic_t* characteristic_info,
        const vm_bt_gatt_attribute_uuid_t* start_descriptor_uuid);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_read_characteristic
 * DESCRIPTION
 *  Read characteristic value, can be larger than MTU.
 * PARAMETERS
 *  connection: [IN] Connection context from vm_bt_gatt_connection_callback.
 *  ch:         [IN] Characteristic information
 *  auth_req:   [IN] Authentication request type
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_read_characteristic(const vm_bt_gatt_connection_t* connection, const vm_bt_gatt_client_characteristic_t* ch, VM_BT_GATT_CLIENT_AUTH_REQ auth_req);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_write_characteristic
 * DESCRIPTION
 *  Write characteristic value, can be larger than MTU.
 * PARAMETERS
 *  connection: [IN] Connection context from vm_bt_gatt_connection_callback.
 *  ch:         [IN] Characteristic information.
 *  value:      [IN] Attribute value.
 *  write_type: [IN] Write type.
 *  auth_req:   [IN] Authentication request type.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_write_characteristic(const vm_bt_gatt_connection_t* connection, const vm_bt_gatt_client_characteristic_t* ch,
        const vm_bt_gatt_attribute_value_t* value, VM_BT_GATT_CLIENT_WRITE_TYPE write_type, VM_BT_GATT_CLIENT_AUTH_REQ auth_req);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_read_descriptor
 * DESCRIPTION
 *  Read characteristic descriptor value.
 * PARAMETERS
 *  connection: [IN] Connection context from vm_bt_gatt_connection_callback.
 *  descr:      [IN] Characteristic descriptor information.
 *  auth_req:   [IN] Authentication request type.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_read_descriptor(const vm_bt_gatt_connection_t* connection, const vm_bt_gatt_client_descriptor_t* descr, VM_BT_GATT_CLIENT_AUTH_REQ auth_req);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_write_descriptor
 * DESCRIPTION
 *  Write characteristic descriptor value.
 * PARAMETERS
 *  connection: [IN] Connection context from vm_bt_gatt_connection_callback.
 *  descr:      [IN] Characteristic descriptor information.
 *  value:      [IN] Attribute value.
 *  write_type: [IN] Write type.
 *  auth_req:   [IN] Authentication request type.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_write_descriptor(const vm_bt_gatt_connection_t* connection,
        const vm_bt_gatt_client_descriptor_t* descr,
        const vm_bt_gatt_attribute_value_t* value,
        VM_BT_GATT_CLIENT_WRITE_TYPE write_type,
        VM_BT_GATT_CLIENT_AUTH_REQ auth_req);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_execute_write
 * DESCRIPTION
 *  Send a command to server to execute a prepared write operation.
 * PARAMETERS
 *  connection: [IN] Connection context from vm_bt_gatt_connection_callback.
 *  execute:    [IN] Set to 1 to make server execute the prepared write operation;
 *                   Set to 0 to cancel the prepared write operation.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_execute_write(const vm_bt_gatt_connection_t* connection, VMUINT8 execute);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_register_for_notification
 * DESCRIPTION
 *  Register to listen the notification or indication of a certain characteristic
 *  from a connected server
 * PARAMETERS
 *  context:        [IN] Client context, profile can get it in Register Callback
 *  gatt_address:   [IN] BT address. This must be an address of a connected device.
 *  ch:             [IN] The characteristic to be registered.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_register_for_notification(
    const VM_BT_GATT_CONTEXT_HANDLE context,
    const vm_bt_gatt_address_t* gatt_address, 
    const vm_bt_gatt_client_characteristic_t* ch);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_deregister_for_notification
 * DESCRIPTION
 *  De-registers a previous request for notifications/indications.
 * PARAMETERS
 *  context: [IN] Client context from vm_bt_gatt_register_client_callback.
 *  gatt_address:   [IN] BT address. This must be an address of a connected device.
 *  ch:             [IN] The characteristic to be deregistered.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_deregister_for_notification(const VM_BT_GATT_CONTEXT_HANDLE context, const vm_bt_gatt_address_t* gatt_address, const vm_bt_gatt_client_characteristic_t* ch);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_read_remote_rssi
 * DESCRIPTION
 *  Requests the RSSI for a given remote device.
 *  The result is passed from vm_bt_gatt_read_remote_rssi_callback.
 * PARAMETERS
 *  context: [IN] Client context from vm_bt_gatt_register_client_callback.
 *  gatt_address: [IN] Address of a connected device to read the RSSI.
 * RETURNS
 *  failure if less than 0, else successful
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_read_remote_rssi(
    const VM_BT_GATT_CONTEXT_HANDLE context,
    const vm_bt_gatt_address_t* gatt_address);

/*****************************************************************************
 * FUNCTION
 *  vm_bt_gatt_client_get_device_type.
 * DESCRIPTION
 *  Determines the type of the remote device (LE, BR/EDR, Dual-mode)
 * PARAMETERS
 *  context: [IN] Client context from vm_bt_gatt_register_client_callback.
 *  gatt_address:   [IN] Address of a connected device to read the RSSI.
 * RETURNS
 *  failure if less than 0, else successful
 *****************************************************************************/
VM_RESULT vm_bt_gatt_client_get_device_type(
    const VM_BT_GATT_CONTEXT_HANDLE context,
    const vm_bt_gatt_address_t* gatt_address);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* VM_BTCM_SDK_H */


