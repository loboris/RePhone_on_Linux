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

#ifndef VMBT_CM_SDK_H
#define VMBT_CM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "vmtype.h"

#define VM_BT_CM_DEVICE_NAME_LENGTH (56)
#define VM_BT_CM_MAX_HOST_SUPPORTED_PROFILE (10)


/* Bluetooth Connection Manager resource handle */
typedef VMINT VM_BT_CM_HANDLE;

/* Error code of the Bluetooth */
typedef enum
{
    VM_BT_CM_ERROR_SUCCESS = 0,
    VM_BT_CM_ERROR_LOGICAL = -5,
    VM_BT_CM_ERROR_BAD_PARAMETER,
    VM_BT_CM_ERROR_NO_RESOURCE,
    VM_BT_CM_ERROR_NOT_SUPPORT,
    VM_BT_CM_ERROR_NO_AUTHORIZATION,
    VM_BT_CM_ERROR_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_ERROR;

typedef enum
{
    VM_BT_CM_RESULT_SUCCESS                = 0,  /* Success. */
    VM_BT_CM_RESULT_FAILED                 = -1, /* Failed */
    VM_BT_CM_RESULT_INVALID_HANDLE         = -2, /* Invalid instance handler. */
    VM_BT_CM_RESULT_DEV_NOT_FOUND          = -3, /* Device not found. */
    VM_BT_CM_RESULT_DEV_LIST_FULL          = -4, /* Device list full. */
    VM_BT_CM_RESULT_NO_CONNECTION          = -5, /* There is no connection. */
    VM_BT_CM_RESULT_ONE_HEADSET_CONNECTED  = -6, /* There is one headset connected. */
    VM_BT_CM_RESULT_ONE_A2DP_CONNECTED     = -7, /* There is one A2DP connection. */
    VM_BT_CM_RESULT_SAME_HEADSET_CONNECTED = -8, /* The same headset is connected. */
    VM_BT_CM_RESULT_PROFILE_NOT_SUPPORT    = -9, /* The profile is not supported. */
    VM_BT_CM_RESULT_MAX = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_RESULT;


/* Event definitions of the Bluetooth*/
typedef enum
{
    VM_BT_CM_EVENT_ACTIVATE                    = 0x00000001, /* Activate. */
    VM_BT_CM_EVENT_DEACTIVATE                  = 0x00000002, /* Deactivate. */
    VM_BT_CM_EVENT_INQUIRY_IND                 = 0x00000004, /* Inquiry indication. */
    VM_BT_CM_EVENT_INQUIRY_COMPLETE            = 0x00000008, /* Inquiry complete. */
    VM_BT_CM_EVENT_BEGIN_ACTIVATE              = 0x00000010, /* Begin activation. */
    VM_BT_CM_EVENT_BEGIN_DEACTIVATE            = 0x00800000, /* Begin deactivation. */
    VM_BT_CM_EVENT_BOND_RESULT                 = 0x00000020, /* Bond result. */
    VM_BT_CM_EVENT_PAIR_INDICATION             = 0x00000040, /* Pair indication. */
    VM_BT_CM_EVENT_PAIR_RESULT                 = 0x00000080, /* Pair result. */
    VM_BT_CM_EVENT_SECURITY_USER_CONFIRM       = 0x00000100, /* Security user confirm. */
    VM_BT_CM_EVENT_SECURITY_PASSKEY_NOTIFY     = 0x00000200, /* Security passkey notify. */
    VM_BT_CM_EVENT_SECURITY_KEYPRESS_NOTIFY    = 0x00000400, /* Security key press notify. */
    VM_BT_CM_EVENT_CONNECT_REQ                 = 0x00000800, /* Connection request. */
    VM_BT_CM_EVENT_CONNECT_RESULT              = 0x00001000, /* Connection result. */
    VM_BT_CM_EVENT_DISCONNECT_IND              = 0x00002000, /* Disconnect indication. */
    VM_BT_CM_EVENT_RELEASE_ALL_CONN            = 0x00004000, /* Release all connections. */
    VM_BT_CM_EVENT_REFRESH_PROFILE             = 0x00008000, /* Refresh profile. */
    VM_BT_CM_EVENT_SET_AUTHORIZE               = 0x00010000, /* Set authorization. */
    VM_BT_CM_EVENT_UNBLOCK                     = 0x00020000, /* Unblock device. */
    VM_BT_CM_EVENT_MYDEV_DEL                   = 0x00040000, /* Delete MyDev. */
    VM_BT_CM_EVENT_BLE_ACTIVATE                = 0x00080000, /* Activate BLE. */
    VM_BT_CM_EVENT_SET_VISIBILITY              = 0x00100000, /* Set visibility. */
    VM_BT_CM_EVENT_SET_NAME                    = 0x00200000, /* Set name. */
    VM_BT_CM_EVENT_SET_AUTHENTICATION          = 0x00400000, /* Set authentication. */
    VM_BT_CM_EVENT_SET_SIMAP                   = 0x01000000, /* Set SIMAP. */
    VM_BT_CM_EVENT_SCO_IND                     = 0x02000000, /* SCO indication. */
    VM_BT_CM_EVENT_PANIC_IND                   = 0x04000000, /* Panic indication. */
    VM_BT_CM_EVENT_READ_REMOTE_DEV_NAME        = 0x08000000, /* Read remote device. */
    VM_BT_CM_EVENT_SET_LIMIT_DISCOVERABLE_MODE = 0x10000000, /* Set limited discoverable mode. */
    VM_BT_CM_EVENT_CHIP_DETECT                 = 0x20000000, /* Chip auto detect. */
    VM_BT_CM_EVENT_BOND_CANCEL                 = 0x40000000, /* Bond cancel. */
    VM_BT_CM_EVENT_FAIL_CONN_TO                = 0x80000000, /* Failed to connect. */
    VM_BT_CM_EVENT_MAX                         = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_EVENT;

/* The power status of the Bluetooth. */
typedef enum
{
    VM_BT_CM_POWER_ON,                      /* Power on. */
    VM_BT_CM_POWER_OFF,                     /* Power off. */
    VM_BT_CM_POWER_SWITCHING_ON,            /* Power switching on. */
    VM_BT_CM_POWER_SWITCHING_OFF,           /* Power switching off. */
    VM_BT_CM_POWER_STATUS_MAX = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_POWER_STATUS;

/* Bluetooth visibility type enum */
typedef enum
{
    VM_BT_CM_VISIBILITY_ON = 1,                 /* The visibility is on. It can be discovered by other Bluetooth devices. */
    VM_BT_CM_VISIBILITY_TEMPORARY_ON,           /* The visibility is temporarily on. It can be discovered by other Bluetooth devices for a period of time. */
    VM_BT_CM_VISIBILITY_OFF,                    /* The visibility is off. It can not be discovered by other Bluetooth devices. */
    VM_BT_CM_VISIBILITY_TYPE_MAX = 0x7FFFFFFF   /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_VISIBILITY_TYPE;

/* Bluetooth device type */
typedef enum
{
    VM_BT_CM_DEVICE_DISCOVERED,         /* Discovered device. */
    VM_BT_CM_DEVICE_RECENT_USED,        /* Recent used device. */
    VM_BT_CM_DEVICE_PAIRED,             /* Paired device. */
    VM_BT_CM_DEV_TYPE_MAX = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_DEV_TYPE;

/* Bluetooth address. */
typedef struct
{
    VMUINT   lap;   /* Lower Address Part 00..23 */
    VMUINT8  uap;   /* Upper Address Part 24..31 */
    VMUINT16 nap;   /* Non-significant    32..47 */
} vm_bt_cm_bt_address_t;

/* Pairing indication struct. */
typedef struct
{
    VMINT32  service_handle;                /* Service handle. */
    void*    user_data;                     /* User data. */
    VMUINT8* device_name;                   /* Device name. */
    VMUINT8  enable_16digits_pin_code;      /* Whether enable 16 digits pin code */
    vm_bt_cm_bt_address_t device_address;   /* Device address. */
} vm_bt_cm_pair_indication_t;

/* Device info structure. */
typedef struct
{
    VMUINT  class_of_device;                                 /* Class of device. */
    VMUINT8 name[VM_BT_CM_DEVICE_NAME_LENGTH];               /* Device name. */
    vm_bt_cm_bt_address_t device_address;                    /* Device address. */
    VMUINT8  attribute;                                      /* Authorized or blocked */
    VMUINT8  service_list_number;                            /* Supported service number */
    VMINT service_list[VM_BT_CM_MAX_HOST_SUPPORTED_PROFILE]; /* Supported service list */
} vm_bt_cm_device_info_t;



/* Event struct for VM_BT_CM_EVENT_ACTIVATE & VM_BT_CM_EVENT_BLE_ACTIVATE. */
typedef struct
{
    VM_BT_CM_HANDLE handle; /* Resource handle. */
    void*  user_data;       /* User data. */
    VMBOOL result;          /* Activate result */
} vm_bt_cm_activate_t;


/* Event struct for VM_BT_CM_EVENT_DEACTIVATE. */
typedef struct
{
    VM_BT_CM_HANDLE handle; /* Resource handle. */
    void*  user_data;       /* User data. */
    VMBOOL result;          /* Deactivate result. */
} vm_bt_cm_deactivate_t;


/* Event struct for VM_BT_CM_EVENT_INQUIRY_IND. */
typedef struct
{
    VM_BT_CM_HANDLE handle;             /* Resource handle. */
    void* user_data;                    /* User data. */
    VMUINT discovered_device_number;    /* Discovered device number. */
} vm_bt_cm_inquiry_indication_t;

/* Event struct for VM_BT_CM_EVENT_INQUIRY_COMPLETE */
typedef struct
{
    VM_BT_CM_HANDLE handle; /* Resource handle. */
    void* user_data;        /* User data. */
    VMINT result;           /* Inquiry complete result. */
    VMINT is_cancelled;     /* Non-zero: cancelled by user.
                             * Zero: not cancelled by user. */
} vm_bt_cm_inquiry_complete_t;

/* Event struct for VM_BT_CM_EVENT_SET_VISIBILITY. */
typedef struct
{
    VM_BT_CM_HANDLE handle; /* Resource handle. */
    void* user_data;        /* User data. */
    VMBOOL result;          /* Set visibility result. */
    VMINT reserved;         /* Reserved, do not use. */
} vm_bt_cm_set_visibility_t;

/* Event struct for VM_BT_CM_EVENT_SET_NAME. */
typedef struct
{
    VM_BT_CM_HANDLE handle; /* Resource handle. */
    void* user_data;        /* User data. */
    VM_BT_CM_RESULT result; /* Set name result in VM_BT_CM_RESULT. */
} vm_bt_cm_set_name_t;

/* Pairing method. */
typedef enum
{
    VM_BT_CM_PIN_AND_SSP = 0,              /* Pairing needs UI and user should confirm.
                                             * It is the normal case for a product, such
                                             * as phone/mid. We support both PIN and SSP. */
    VM_BT_CM_FIXED_PIN_ONLY,               /* Pairing does not need UI. It auto-accepts
                                             * the pairing. Adopts FIXED PIN CODE method only. */
    VM_BT_CM_FIXED_PIN_AND_SSP_JUST_WORK,  /* Pairing does not need UI. It auto-accepts
                                             * the pairing. Adopts FIXED PIN CODE and JUST WORK of SSP. */
    VM_BT_CM_PAIRING_TYPE_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_CM_PAIRING_TYPE;


/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_callback
 * DESCRIPTION
 *    The prototype of the callback function for event notification from the
 *    Bluetooth Connection Manager.
 * PARAMETERS
 *    event:     [IN] The event type that triggers this callback.
 *    parameter: [IN] The event structure, such as vm_bt_cm_inquiry_indication_t, for example.
 *    user_data: [IN] The user data.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_bt_cm_callback)(VM_BT_CM_EVENT event, void* parameter, void* user_data);

/******************************************************************************
 * FUNCTION
 *    vm_bt_cm_init
 * DESCRIPTION
 *    Initializes the Bluetooth Connection Manager and registers the callback.
 *    The callback will be triggered when the events occur, as specified by the
 *    event_mask.
 * PARAMETERS
 *    callback:   [IN] The callback to be invoked after an event occurs.
 *    event_mask: [IN] The event mask, a bitwise OR of the desired values in VM_BT_CM_EVENT.
 *    user_data:  [IN] The user data.
 *
 * RETURNS
 *    The resource handle, a positive integer. A negative integer is returned when
 *    a failure occurs. Check VM_BT_CM_ERROR for error code.
 ******************************************************************************/
VM_BT_CM_HANDLE vm_bt_cm_init(vm_bt_cm_callback callback, VMUINT event_mask, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_exit
 * DESCRIPTION
 *    Exits the Bluetooth Connection Manager and releases the resources.
 * PARAMETERS
 *    handle: [IN] The resource handle
 * RETURNS
 *    Returns 0, if it is successful. A negative integer is returned when
 *    a failure occurs. Check VM_BT_CM_ERROR for error code.
 *****************************************************************************/
VM_RESULT vm_bt_cm_exit(VM_BT_CM_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_get_power_status
 * DESCRIPTION
 *    Gets the current power status of the Bluetooth.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_BT_CM_POWER_STATUS
 *****************************************************************************/
VM_BT_CM_POWER_STATUS vm_bt_cm_get_power_status(void);


/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_switch_on
 * DESCRIPTION
 *    Switches on the Bluetooth chipset. It also activates the Bluetooth
 *    profiles that are registered to the Connection Manager.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_switch_on(void);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_switch_off
 * DESCRIPTION
 *    Switches off the Bluetooth chipset. It also deactivates the Bluetooth
 *    profiles. If there are any active connections, it will disconnect the
 *    connections before switching off the Bluetooth chipset.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_switch_off(void);

/******************************************************************************
 * FUNCTION
 *    vm_bt_cm_set_visibility
 * DESCRIPTION
 *    Sets the Bluetooth visibility, so that other devices can detect it.
 * PARAMETERS
 *    type: [IN] Specifies the type of visibility to be set.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 ******************************************************************************/
VM_RESULT vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_TYPE type);

/******************************************************************************
 * FUNCTION
 *  vm_bt_cm_get_visibility
 * DESCRIPTION
 *  This function is to get the BT visibility
 * PARAMETERS
 *  void
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 ******************************************************************************/
VM_RESULT vm_bt_cm_get_visibility(void);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_search
 * DESCRIPTION
 *    Searches for the Bluetooth devices.The search process could trigger two
 *    events, the VM_BT_CM_EVENT_INQUIRY_IND and VM_BT_CM_EVENT_INQUIRY_COMPLETE,
 *    which in turn triggers the callback.
 * PARAMETERS
 *    max_response:      [IN] The maximum number of devices that will be detected.
 *    timeout:           [IN] The timeout for the search. It will stop the search
 *                            after the timeout.
 *    class_of_device:   [IN] Specifies the type of Bluetooth devices to search for.
 *                            The 0xFFFFFFFF is for searching all devices. Refer to
 *                            General- and Device-Specific Inquiry Access Codes
 *                            (DIACs) for details.
 *    is_name_discovery: [IN] Specifies whether to do name discovery.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_search(VMUINT8 max_response, VMUINT16 timeout,
                          VMUINT class_of_device, VMBOOL is_name_discovery);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_search_abort
 * DESCRIPTION
 *    Aborts the search process. It invokes srv_bt_cm_cancel_discovery_req_hdler.
 *    A callback will be invoked by the VM_BT_CM_EVENT_INQUIRY_COMPLETE event,
 *    after the cancelling of the search procedure is complete.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_search_abort(void);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_set_host_name
 * DESCRIPTION
 *    Sets the host device name. It will trigger the VM_BT_CM_EVENT_SET_NAME
 *    event, after the host device name procedure is completed.
 * PARAMETERS
 *    name: [IN] The host device name, in UTF-8 encoding, to be set.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_set_host_name(VMUINT8* name);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_get_host_name
 * DESCRIPTION
 *    Gets the host device information.
 * PARAMETERS
 *    info: [OUT] The host device information, retrieved by calling this function.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_get_host_device_info(vm_bt_cm_device_info_t* info);

/******************************************************************************
 * FUNCTION
 *    vm_bt_cm_get_device_number
 * DESCRIPTION
 *    Gets the device number according to device type
 * PARAMETERS
 *    device_type: [IN] Specifies the device type as defined in VM_BT_CM_DEV_TYPE.
 * RETURNS
 *    VMINT, the device number of specified device type.
 ******************************************************************************/
VMINT vm_bt_cm_get_device_number(VM_BT_CM_DEV_TYPE device_type);

/******************************************************************************
 * FUNCTION
 *    vm_btspp_get_device_info_by_index
 * DESCRIPTION
 *    Gets the device info
 * PARAMETERS
 *    index:       [IN]  Specifies the device index.
 *    device_type: [IN]  Specifies the device type as defined in VM_BT_CM_DEV_TYPE.
 *    info:        [OUT] The device information, retrieved by calling this function.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 ******************************************************************************/
VM_RESULT vm_bt_cm_get_device_info_by_index(VMUINT index, VM_BT_CM_DEV_TYPE device_type,
        vm_bt_cm_device_info_t* info);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_release_all_connection
 * DESCRIPTION
 *    Releases all active Bluetooth connections. When it completes, the callback
 *    is invoked through the VM_BT_CM_EVENT_RELEASE_ALL_CONN event.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail
 *****************************************************************************/
VM_RESULT vm_bt_cm_release_all_connection(void);


/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_send_passkey
 * DESCRIPTION
 *    Processes the pairing request and sends the passkey when the VM_BT_CM_PAIR_IND
 *    event occurs.
 * PARAMETERS
 *    device_address: [IN] The device address of the pairing Bluetooth device.
 *    device_passkey: [IN] The passkey to be sent to the pairing Bluetooth device.
 *    result:         [IN] Specifies whether to send a valid passkey or reject the
 *                         pairing request. VM_TURE indicates to send the valid
 *                         passkey. VM_FALSE indicates to reject this pairing request.
 * RETURNS
 *    VM_RESULT. Refer to VM_BT_CM_RESULT for detail.
 *****************************************************************************/
VM_RESULT vm_bt_cm_send_passkey(const vm_bt_cm_bt_address_t* device_address,
                                const VMUINT8* device_passkey, VMBOOL result);


/*****************************************************************************
 * FUNCTION
 *    vm_bt_cm_set_bt_pairing_method
 * DESCRIPTION
 *    Sets the Bluetooth pairing type. This function should be called prior to the
 *    VM_BT_CM_PAIR_IND event occurs, to properly configure the expected pairing method.
 * PARAMETERS
 *    pairing_type: [IN] Specifies the pairing type
 * RETURNS
 *    void
 *****************************************************************************/
void vm_bt_cm_set_bt_pairing_type(VM_BT_CM_PAIRING_TYPE config_type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* VMBT_CM_SDK_H */

