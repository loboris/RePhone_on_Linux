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

#ifndef VMBT_NS_SDK_H
#define VMBT_NS_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmbt_cm.h"

/* Notification category enum. */
typedef enum
{
    VM_BT_NS_CATEGORY_NONE,             /* None category. */
    VM_BT_NS_CATEGORY_NOTIFICATION,     /* Notification category. */
    VM_BT_NS_CATEGORY_CALL,             /* Call category. */
    VM_BT_NS_CATEGORY_MAP,              /* Map category. */
} VM_BT_NS_CATEGORY;

/* Notification subtype enum. */
typedef enum
{
    VM_BT_NS_SUBTYPE_TEXT,              /* Text subtype in notification category. */
    VM_BT_NS_SUBTYPE_SMS,               /* SMS subtype in notification category. */
    VM_BT_NS_SUBTYPE_BLOCKED_SENDER,    /* Blocked sender subtype in notification category. */
    VM_BT_NS_SUBTYPE_MISSED_CALL,       /* Missed call subtype in call category. */
} VM_BT_NS_SUBTYPE;

/* Notification action enum. */
typedef enum
{
    VM_BT_NS_ACTION_ADD,                /* Add action. */
    VM_BT_NS_ACTION_DELETE,             /* Delete action. */
    VM_BT_NS_ACTION_DELETE_ALL,         /* Delete all action. */
    VM_BT_NS_ACTION_UPDATE,             /* Update action. */
} VM_BT_NS_ACTION;

/* Notification source. */
typedef enum
{
    VM_BT_NS_DATA_SOURCE_UNKNOWN = 0,       /* Unknown source. */
    VM_BT_NS_DATA_SOURCE_SPP,               /* SPP source. */
    VM_BT_NS_DATA_SOURCE_DOGP,              /* DOGP source. */
    VM_BT_NS_DATA_SOURCE_ANCS,              /* ANCS source. */
} VM_BT_NS_DATA_SOURCE;

/* Bluetooth notification result. */
typedef enum
{
    VM_BT_NS_SUCCESS           = 0,     /* Success. */
    VM_BT_NS_INVALD_PARAMETER  = -1,    /* Invalid parameter. */
    VM_BT_NS_REGISTERED        = -2,    /* Register again. */
    VM_BT_NS_UNKNOWN_ERROR     = -3,    /* Unknown error. */
    VM_BT_NS_ERROR             = -4,    /* Common error. */
    VM_BT_NS_ERROR_OF_MEMORY   = -5,    /* Memory full. */
    VM_BT_NS_ERROR_OF_RESOURCE = -6,    /* Resource error. */
    VM_BT_NS_ERROR_INVALID_ID  = -7,    /* Invalid ID. */
    VM_BT_NS_ERROR_OF_PROCESS  = -8,    /* Process error. */
    VM_BT_NS_RESULT_MAX = 0x7FFFFFFF    /* Forces the compiler to use 4-byte integer for enum. */
} VM_BT_NS_RESULT;


/* Notification header struct. */
typedef struct
{
    VM_BT_NS_CATEGORY category;     /* Notification category. */
    VM_BT_NS_SUBTYPE subtype;       /* Notification subtype. */
    VM_BT_NS_ACTION action;         /* Notification action. */
    VMUINT32 message_id;            /* Notification message ID. */
} vm_bt_ns_header_t;

/* The length of the sender name. */
#define VM_BT_NS_SENDER_LENGTH              50

/* The length of the sender number. */
#define VM_BT_NS_NUMBER_LENGTH              20

/* The length of the content. */
#define VM_BT_NS_CONTENT_LENGTH             300

/* The length of the tittle. */
#define VM_BT_NS_TITLE_LENGTH               150

/* The length of the ticker text. */
#define VM_BT_NS_TICKER_TEXT_LENGTH         150

/* The length of the icon buffer. */
#define VM_BT_NS_ICON_LENGTH                2560

/* Bluetooth notification message struct. */
typedef struct
{
    vm_bt_ns_header_t header;                           /* Notification header. */
    VMWCHAR  sender[VM_BT_NS_SENDER_LENGTH];            /* It is the App name for notification. It is contact name or number for SMS. */
    VMWCHAR  sender_number[VM_BT_NS_NUMBER_LENGTH];     /* It is the contact number for SMS or for missed call. */
    VMWCHAR  content[VM_BT_NS_CONTENT_LENGTH];          /* Notification content. */
    VMWCHAR  title[VM_BT_NS_TITLE_LENGTH];              /* The content of the notification. */
    VMWCHAR  ticker_text[VM_BT_NS_TICKER_TEXT_LENGTH];  /* The ticker text of the notification. */
    VMUINT32 missed_call_count;                         /* The count of missed call. It is only applicable for missed call subtype. */
    VMUINT32 time_stamp;                                /* The timestamp of notification. */
    VMUINT32 app_id;                                    /* The App ID. */
    VMUINT8  icon_buffer[VM_BT_NS_ICON_LENGTH];         /* The icon buffer for JEPG format. */
    VM_BT_NS_DATA_SOURCE source;                        /* The source of the message. */
    vm_bt_cm_bt_address_t bt_address;                   /* Reserved. */
} vm_bt_ns_message_t;


/* Bluetooth notification data. */
typedef struct
{
    VMUINT16 event_data_size;           /* The size of the event data. Reserved. */
    void* event_data;                   /* The event data. Reserved. */
    vm_bt_cm_bt_address_t bt_address;   /* The Bluetooth address. */
    vm_bt_ns_message_t* message;        /* Notification message. */
} vm_bt_ns_notification_data_t;


/* Bluetooth notification connection status change */
typedef struct
{
    VMUINT16 event_size;                /* The size of the event data. Reserved. */
    void* event_data;                   /* The event data. Reserved. */
    vm_bt_cm_bt_address_t bt_address;   /* The Bluetooth address. */
    VMBOOL is_connected;                /* The connection status. */
} vm_bt_ns_connection_status_t;

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_notification_callback
 * DESCRIPTION
 *    The Bluetooth notification callback.
 * PARAMETERS
 *    parameter_ptr: [IN] The notification data to be passed to this callback by
 *                        the LinkIt Engine.
 *    user_data:     [IN] The additional data that can be passed to this callback.
 * RETURNS
 *    VMINT. Reserved.
 *****************************************************************************/
typedef VMINT (*vm_bt_ns_notification_callback)(vm_bt_ns_notification_data_t* parameter_ptr, void* user_data);

/* Bluetooth connection status change callback. */
/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_connection_status_change_callback
 * DESCRIPTION
 *    The Bluetooth connection status change callback.
 * PARAMETERS
 *    parameter_ptr: [IN] The data of status change to be passed to this callback by
 *                        the LinkIt Engine.
 *    user_data:     [IN] The additional data that can be passed to this callback.
 * RETURNS
 *    VMINT. Reserved.
 *****************************************************************************/
typedef VMINT (*vm_bt_ns_connection_status_change_callback)(vm_bt_ns_connection_status_t* parameter_ptr, void* user_data);

/* Bluetooth notification handle. */
typedef VMINT VM_BT_NS_NOTIFICATION_HANDLE;

/* Bluetooth connection status handle. */
typedef VMINT VM_BT_NS_CONNECTION_STATUS_HANDLE;

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_register_notification_callback
 * DESCRIPTION
 *    Registers a callback function for the notification of the Bluetooth.
 * PARAMETERS
 *    callback:  [IN] The callback will be called when the notification is received.
 *    user_data: [IN] The user data, which will be used by the callback.
 * RETURNS
 *    VM_BT_NS_NOTIFICATION_HANDLE, a positive number, when the registration is
 *    successful. If it is a negative number, the registration failed. Refer to
 *    VM_BT_NS_RESULT for error code.
 *****************************************************************************/
VM_BT_NS_NOTIFICATION_HANDLE vm_bt_ns_register_notification_callback(vm_bt_ns_notification_callback callback, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_deregister_notification_callback
 * DESCRIPTION
 *    De-registers the callback for the notification as specified by the handle.
 * PARAMETERS
 *    handle: [IN] The notification handle, returned by vm_bt_ns_register_notification_callback.
 * RETURNS
 *    It returns 0, if is succeeds. If it returns a negative number, refer to
 *    VM_BT_NS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_bt_ns_deregister_notification_callback(VM_BT_NS_NOTIFICATION_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_register_connection_status_change_callback
 * DESCRIPTION
 *    Registers a connection status change callback.
 * PARAMETERS
 *    callback:  [IN] The callback will be called when the Bluetooth connection status changes.
 *    user_data: [IN] The user data, which will be used by the callback.
 * RETURNS
 *    VM_BT_NS_CONNECTION_STATUS_HANDLE, a positive number, when the registration is
 *    successful. If it is a negative number, the registration failed. Refer to
 *    VM_BT_NS_RESULT for error code.
 *****************************************************************************/
VM_BT_NS_CONNECTION_STATUS_HANDLE vm_bt_ns_register_connection_status_change_callback(vm_bt_ns_connection_status_change_callback callback, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_deregister_connection_status_change_callback
 * DESCRIPTION
 *    De-registers the callback for the connection status change, as specified by the handle.
 * PARAMETERS
 *    handle: [IN] The connection status change handle, returned by
 *                 vm_bt_ns_register_connection_status_change_callback.
 * RETURNS
 *    It returns 0, if is succeeds. If it returns a negative number, refer to
 *    VM_BT_NS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_bt_ns_deregister_connection_status_change_callback(VM_BT_NS_CONNECTION_STATUS_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_bt_ns_data_block_remote_notification
 * DESCRIPTION
 *    Sends data to the remote device through SPP to notify that it will block
 *    the specified notification by App ID.
 * PARAMETERS
 *    bt_address: [IN] The Bluetooth address of the remote device.
 *    app_id:     [IN] The App ID of the notification.
 * RETURNS
 *    It returns VM_TRUE, if the operation is successful. Otherwise, it returns VM_FALSE.
 *****************************************************************************/
VMBOOL vm_bt_ns_block_remote_notification(vm_bt_cm_bt_address_t* bt_address, VMUINT32 app_id);


#ifdef __cplusplus
}
#endif

#endif /* VMBT_NS_SDK_H */