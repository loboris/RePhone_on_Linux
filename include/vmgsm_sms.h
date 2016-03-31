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

#ifndef VMGSM_SMS_SDK_H
#define VMGSM_SMS_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmdatetime.h"

/* The maximum length of SMS message content. */
#define VM_GSM_SMS_MESSAGE_CONTENT_LENGTH        (500)

/* The maximum length of SMS message address. */
#define VM_GSM_SMS_MAX_ADDRESS_LENGTH            (21)


/* SMS request results. */
typedef enum
{
    VM_GSM_SMS_RESULT_OK                       = 0,     /* SMS operation request succeeds. */
    VM_GSM_SMS_SUPPORT_SYNC_SMS_CENTER_ADDRESS = 1,     /* Supports synchronous SMS center address. */
    VM_GSM_SMS_CALLBACK_REGISTERED             = 1,     /* Sets interrupt again if registered. */
    VM_GSM_SMS_ERROR_IDENTIFY                  = -4,    /* SMS identify error. */
    VM_GSM_SMS_ERROR_MIBR_NOT_IN_PROCESS_SPACE = -3,    /* SMS MIBR error. */
    VM_GSM_SMS_ERROR_MIBR                      = -2,    /* SMS request error. */
    VM_GSM_SMS_ERROR_REQUEST                   = -1,    /* SMS operation request failed. */
    VM_GSM_SMS_RESULT_MAX = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_SMS_RESULT;

/* The results of SMS send. */
typedef enum
{
    VM_GSM_SMS_SEND_RESULT_OK    = 1,   /* Sends SMS successfully. */
    VM_GSM_SMS_SEND_RESULT_ERROR = 2,   /* Failed to send SMS. */
    VM_GSM_SMS_SEND_RESULT_ABORT = 3    /* Sending SMS is cancelled. */
} VM_GSM_SMS_SEND_RESULT;


/* SMS Box type. */
typedef enum
{
    VM_GSM_SMS_BOX_INBOX    = 0x01,         /* Inbox. */
    VM_GSM_SMS_BOX_OUTBOX   = 0x02,         /* Outbox. */
    VM_GSM_SMS_BOX_DRAFTS   = 0x04,         /* Daft box. */
    VM_GSM_SMS_BOX_UNSENT   = 0x08,         /* Unsent box. Messages to be sent. */
    VM_GSM_SMS_BOX_SIM      = 0x10,         /* SIM card. */
    VM_GSM_SMS_BOX_ARCHIVE  = 0x20,         /* Archive box. */
    VM_GSM_SMS_BOX_RESERVED = 0x40          /* Reserved. Do not use. */
} VM_GSM_SMS_BOX;

/* Message Folder type. */
typedef enum
{
    VM_GSM_SMS_FOLDER_DEFAULT,          /* Default folder. */
    VM_GSM_SMS_FOLDER_ARCHIVE           /* Archive folder. */
} VM_GSM_SMS_FOLDER;

/* Message Status. */
typedef enum
{
    VM_GSM_SMS_STATUS_UNREAD      = 0x01,   /* Unread. */
    VM_GSM_SMS_STATUS_READ        = 0x02,   /* Read. */
    VM_GSM_SMS_STATUS_SENT        = 0x04,   /* Sent. */
    VM_GSM_SMS_STATUS_UNSENT      = 0x08,   /* Unsent (to be sent). */
    VM_GSM_SMS_STATUS_DRAFT       = 0x10,   /* Draft. */
    VM_GSM_SMS_STATUS_UNSUPPORTED = 0x20    /* Unsupported. */
} VM_GSM_SMS_STATUS;


/* Message Type Indication. */
typedef enum
{
    VM_GSM_SMS_MTI_DELIVER          = 0x00,         /* Deliver type. */
    VM_GSM_SMS_MTI_DELIVER_REPORT   = 0x00,         /* Deliver report type. */
    VM_GSM_SMS_MTI_SUBMIT           = 0x01,         /* Submit type. */
    VM_GSM_SMS_MTI_SUBMIT_REPORT    = 0x01,         /* Submit report type. */
    VM_GSM_SMS_MTI_STATUS_REPORT    = 0x02,         /* Status report type. */
    VM_GSM_SMS_MTI_COMMAND          = 0x02,         /* Command type. */
    VM_GSM_SMS_MTI_RESERVED         = 0x03          /* Reserved. */
} VM_GSM_SMS_MTI;


/* Message Storage Type */
typedef enum
{
    VM_GSM_SMS_STORAGE_INTERNAL,        /* Internal storage. */
    VM_GSM_SMS_STORAGE_SIM,             /* SIM card storage. */
    VM_GSM_SMS_STORAGE_UNSPECIFIC       /* Unspecified storage. */
} VM_GSM_SMS_STORAGE;


/* Network type. */
typedef enum
{
    VM_GSM_SMS_NETWORK_GSM      = 0x0100     /* GSM network. */
} VM_GSM_SMS_NETWORK_FLAG;


/* SIM Card Type */
typedef enum
{
    VM_GSM_SMS_SIM_1 = VM_GSM_SMS_NETWORK_GSM | 0x0001,  /* SIM Card 1. */
    VM_GSM_SMS_SIM_2 = VM_GSM_SMS_NETWORK_GSM | 0x0002,  /* SIM Card 2. */
    VM_GSM_SMS_SIM_3 = VM_GSM_SMS_NETWORK_GSM | 0x0004,  /* SIM Card 3, the SIM card inserted in the slot 3. */
    VM_GSM_SMS_SIM_4 = VM_GSM_SMS_NETWORK_GSM | 0x0008   /* SIM Card 4, the SIM card inserted in the slot 4. */
} VM_GSM_SMS_SIM;


/* Message Class Type. */
typedef enum
{
    VM_GSM_SMS_CLASS_0   = 0x00,      /* Class 0. */
    VM_GSM_SMS_CLASS_1   = 0x01,      /* Class 1. */
    VM_GSM_SMS_CLASS_2   = 0x02,      /* Class 2. */
    VM_GSM_SMS_CLASS_3   = 0x03       /* Class 3. */
} VM_GSM_SMS_CLASS;


/* Validity period. */
typedef enum
{
    VM_GSM_SMS_VALIDITY_PERIOD_1_HR     = 11,        /* Valid for 1 hour. */
    VM_GSM_SMS_VALIDITY_PERIOD_6_HR     = 71,        /* Valid for 6 hours. */
    VM_GSM_SMS_VALIDITY_PERIOD_12_HR    = 143,       /* Valid for 12 hours. */
    VM_GSM_SMS_VALIDITY_PERIOD_24_HR    = 167,       /* Valid for 24 hours. */
    VM_GSM_SMS_VALIDITY_PERIOD_72_HR    = 169,       /* Valid for 72 hours. */
    VM_GSM_SMS_VALIDITY_PERIOD_1_WEEK   = 173        /* valid for 1 week. */
} VM_GSM_SMS_VALIDITY_PERIOD;


/* SMS DCS(Data Coding Scheme). */
typedef enum
{
    VM_GSM_SMS_DCS_7BIT     = 0x00,         /* GSM 7 bit default alphabet. */
    VM_GSM_SMS_DCS_8BIT     = 0x04,         /* 8 bit data. */
    VM_GSM_SMS_DCS_UCS2     = 0x08,         /* UCS2 (16bit). */
    VM_GSM_SMS_DCS_RESERVED = 0x0c          /* Reserved. Do not use. */
} VM_GSM_SMS_DCS;


/* Protocol Identifier */
typedef enum
{
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_DEFAULT             = 0x00,  /* Text SMS. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_EMAIL               = 0x32,  /* Internet Electronic Mail. */

    VM_GSM_SMS_PROTOCOL_IDENTIFIER_TYPE_0              = 0x40,  /* Short Message Type 0. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_1      = 0x41,  /* Replace Short Message Type 1. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_2      = 0x42,  /* Replace Short Message Type 2. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_3      = 0x43,  /* Replace Short Message Type 3. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_4      = 0x44,  /* Replace Short Message Type 4. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_5      = 0x45,  /* Replace Short Message Type 5. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_6      = 0x46,  /* Replace Short Message Type 6. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_REPLACE_TYPE_7      = 0x47,  /* Replace Short Message Type 7. */

    VM_GSM_SMS_PROTOCOL_IDENTIFIER_RETURN_CALL_message = 0x5F,  /* Return Call Message. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_ANSI_136_RDATA      = 0x7C,  /* ANSI-136 R-DATA. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_INTERNAL_DOWNLOAD   = 0x7D,  /* ME Data Download. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_INTERNAL_DE_PERSONAL= 0x7E,  /* ME De-personalization Short Message. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER_SIM_DOWNLOAD        = 0x7F,  /* (U)SIM Data Download. */

    VM_GSM_SMS_PROTOCOL_IDENTIFIER_RESERVED                     /* Reserved. Do not use. */
} VM_GSM_SMS_PROTOCOL_IDENTIFIER;


/* SMS Action Type. */
typedef enum
{
    VM_GSM_SMS_ACTION_NONE,                 /* No action. */
    VM_GSM_SMS_ACTION_SEND,                 /* Send action. */
    VM_GSM_SMS_ACTION_READ,                 /* Read action. */
    VM_GSM_SMS_ACTION_DELETE,               /* Delete action. */
    VM_GSM_SMS_ACTION_SAVE,                 /* Save action. */
    VM_GSM_SMS_ACTION_UPDATE,               /* Update action. */
    VM_GSM_SMS_ACTION_COPY,                 /* Copy action. */
    VM_GSM_SMS_ACTION_MOVE,                 /* Move action. */
    VM_GSM_SMS_ACTION_QUERY,                /* Query action. */
    VM_GSM_SMS_ACTION_CHANGE_STATUS,        /* Change status action. */
    VM_GSM_SMS_ACTION_MOVE_TO_ARCHIVE,      /* Move to archive action. */
    VM_GSM_SMS_ACTION_GET_CONTENT,          /* Get content action. */
    VM_GSM_SMS_ACTION_GET_SIM_NUMBER,       /* Get SIM number action. */
    VM_GSM_SMS_ACTION_GET_CENTER_ADDRESS    /* Get SMS center address action. */
} VM_GSM_SMS_ACTION;


/* SMS Cause. */
typedef enum
{
    VM_GSM_SMS_CAUSE_NO_ERROR = 0,


    VM_GSM_SMS_CAUSE_INTERNAL_FAILURE           = 300,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_OPERATION_NOT_ALLOWED      = 302,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_OPERATION_NOT_SUPPORTED    = 303,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_INVALID_PDU_PARA           = 304,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_INVALID_TEXT_PARA          = 305,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_SIM_NOT_INSERTED           = 310,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_SIM_FAILURE                = 313,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_MEMORY_FAILURE             = 320,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_INVALID_MEMORY_INDEX       = 321,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_MEMORY_FULL                = 322,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */
    VM_GSM_SMS_CAUSE_SMS_CENTER_ADDRESS_UNKNOWN = 330,  /* Refer to TS 27.005 Clause 3.2.5, for AT Command and MMI. */

    VM_GSM_SMS_CAUSE_NOT_READY                  = 341,  /* SMS not ready. */
    VM_GSM_SMS_CAUSE_SEND_BUSY,                         /* Busy in sending. */
    VM_GSM_SMS_CAUSE_SEND_ABORT,                        /* Send aborted. */
    VM_GSM_SMS_CAUSE_MEMORY_INSUFFICIENT,               /* Memory insufficient. */
    VM_GSM_SMS_CAUSE_DATA_INVALID,                      /* Invalid data. */
    VM_GSM_SMS_CAUSE_DATA_EXCEED,                       /* Data exceed. */
    VM_GSM_SMS_CAUSE_NUMBER_EMPTY,                      /* Empty destination number. */
    VM_GSM_SMS_CAUSE_NUMBER_INVALID,                    /* Invalid destination number. */
    VM_GSM_SMS_CAUSE_CENTER_EMPTY,                      /* Empty SMS center number. */
    VM_GSM_SMS_CAUSE_CENTER_INVALID,                    /* Invalid SMS center number. */
    VM_GSM_SMS_CAUSE_FDL_FAIL,                          /* FDL fail. */
    VM_GSM_SMS_CAUSE_AT_CONFLICT,                       /* AT conflict. */
    VM_GSM_SMS_CAUSE_OPERATION_CONFLICT,                /* Operation conflict. */
    VM_GSM_SMS_CAUSE_EMAIL_NO_SPACE,                    /* No space for email. */
    VM_GSM_SMS_CAUSE_FS_ERROR,                          /* FS error. */

    VM_GSM_SMS_CAUSE_UNKNOWN_ERROR              = 500   /* Unknown error. */
} VM_GSM_SMS_CAUSE;


/* SMS Parameter Flag. */
typedef enum
{
    VM_GSM_SMS_PARAMETER_NONE                   = 0x00000000,    /* No parameter. */

    VM_GSM_SMS_PARAMETER_NUMBER                 = 0x00000001,    /* Parameter of number. */
    VM_GSM_SMS_PARAMETER_SMS_CENTER_ADDRESS     = 0x00000002,    /* Parameter of SMS center address. */
    VM_GSM_SMS_PARAMETER_PROTOCOL_IDENTIFIER    = 0x00000004,    /* Parameter of PID. */
    VM_GSM_SMS_PARAMETER_DCS                    = 0x00000008,    /* Parameter of DCS. */
    VM_GSM_SMS_PARAMETER_VALIDITY_PERIOD        = 0x00000010,    /* Parameter of VP. */
    VM_GSM_SMS_PARAMETER_STATUS_REPORT          = 0x00000020,    /* Parameter of status report. */
    VM_GSM_SMS_PARAMETER_REPLY_PATH             = 0x00000040,    /* Parameter of reply path. */
    VM_GSM_SMS_PARAMETER_PORT                   = 0x00000080,    /* Parameter of port. */
    VM_GSM_SMS_PARAMETER_CLASS                  = 0x00000100,    /* Parameter of class. */
    VM_GSM_SMS_PARAMETER_MMS                    = 0x00000200,    /* Parameter of MMS. */
    VM_GSM_SMS_PARAMETER_FIRST_OCTET            = 0x00000400,    /* Parameter of FO. */

    VM_GSM_SMS_PARAMETER_STATUS                 = 0x00000800,    /* Parameter of status. */
    VM_GSM_SMS_PARAMETER_SIM_ID                 = 0x00001000,    /* Parameter of SIM ID. */
    VM_GSM_SMS_PARAMETER_STORAGE_TYPE           = 0x00002000,    /* Parameter of storage type. */
    VM_GSM_SMS_PARAMETER_TIMESTAMP              = 0x00004000,    /* Parameter of time stamp. */

    VM_GSM_SMS_PARAMETER_CONTENT_EMS            = 0x00008000,    /* Parameter of EMS. */
    VM_GSM_SMS_PARAMETER_CONTENT_BUFFER         = 0x00010000,    /* Parameter of buffer. */
    VM_GSM_SMS_PARAMETER_CONTENT_FILE           = 0x00020000,    /* Parameter of file. */
    VM_GSM_SMS_PARAMETER_MAX                    = 0x7FFFFFFF      /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_SMS_PARAMETER;


/* SMS Sort Type. */
typedef enum
{
    VM_GSM_SMS_SORT_NONE,                   /* No sort. */
    VM_GSM_SMS_SORT_TIMESTAMP,              /* Sort by timestamp. */
    VM_GSM_SMS_SORT_MESSAGE_TYPE,           /* Sort by message type.*/
    VM_GSM_SMS_SORT_STORAGE_TYPE,           /* Sort by storage type.*/
    VM_GSM_SMS_SORT_NUMBER,                 /* Sort by number.*/
    VM_GSM_SMS_SORT_CONTENT,                /* Sort by content.*/
    VM_GSM_SMS_SORT_PROTOCOL_IDENTIFIER,    /* Sort by PID.*/
    VM_GSM_SMS_SORT_SIM_ID                  /* Sort by SIM.*/
} VM_GSM_SMS_SORT;


/* SMS Sorting Order. */
typedef enum
{
    VM_GSM_SMS_ORDER_DEFAULT,       /* Default order. */
    VM_GSM_SMS_ORDER_ASCEND,        /* Ascending order. */
    VM_GSM_SMS_ORDER_DESCEND        /* Descending order. */
} VM_GSM_SMS_ORDER;


/* SMS new message event type. */
typedef enum
{
    VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE,        /* SMS event of new message. */
    VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE_EX,      /* SMS event of new message EX. */
    VM_GSM_SMS_EVENT_ID_SMS_READY_MESSAGE /* sms event of sms ready*/
} VM_GSM_SMS_EVENT_ID;

/* SMS ready message struct */
typedef struct
{
    VMBOOL is_sms_ready;  /* VM_TRUE, ready; VM_FALSE, not ready*/
} vm_gsm_sms_event_ready_t;

/* SMS Operation Callback Data Structure. */
typedef struct
{
    VMINT  result;              /* 1 means the operation succeeds; 0 means the operation failed. */
    VM_GSM_SMS_CAUSE  cause;    /* Cause description, refer to vm_gsm_sms_cause_enum */
    VM_GSM_SMS_ACTION action;   /* Action type, refer to VM_GSM_SMS_ACTION. */
    void* action_data;          /* Action data. The data type depends on action type. */
    void* user_data;            /* Additional user data for the callback. */
} vm_gsm_sms_callback_t;


/* SMS Event Structure. */
typedef struct
{
    VMINT event_id;    /* Event ID. */
    VMINT size;        /* The size of event information. */
    void* user_data;   /* Additional user data for the callback. */
    void* event_info;  /* The pointer to the event information. */
} vm_gsm_sms_event_t;


/* Message Data Structure. */
typedef struct
{
    VMWCHAR number[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1]; /* SMS number. */
    VM_GSM_SMS_STATUS status;    /* SMS status. */
    VM_GSM_SMS_MTI mti;          /* MTI, Message Type Indication. */

    VMUINT8 tp_status;           /* TP-Status, for Status Report. */
    VM_GSM_SMS_FOLDER folder_id; /* Folder ID. */

    VM_GSM_SMS_PARAMETER parameter_flag; /* Parameter flag. */

    VM_GSM_SMS_STORAGE storage_type; /* Sets the storage to read SMS from. */
    VM_GSM_SMS_SIM sim_id;           /* SIM ID. */
    vm_date_time_t  timestamp;       /* Timestamp. */

    VMWCHAR center_address[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1]; /* SMS center address. */
    VM_GSM_SMS_PROTOCOL_IDENTIFIER protocol_identifier;        /* Protocol identifier. */
    VM_GSM_SMS_VALIDITY_PERIOD validity_period;                /* validity period. */

    VMINT status_report; /* Status report. */
    VMINT reply_path;    /* Reply path. */

    VMUINT16 source_port;           /* Source port. */
    VMUINT16 destination_port;      /* Destination port. */
    VM_GSM_SMS_DCS dcs;             /* DCS, Data Coding Scheme. */
    VM_GSM_SMS_CLASS message_class; /* Message class. */
    VMUINT16 content_buffer_size;   /* Content buffer size. */
    VMWCHAR* content_buffer;        /* Content buffer. */
    VMWCHAR* content_file_path;     /* Content file path. */

    void* content_ems;              /* Content EMS */
} vm_gsm_sms_read_message_data_t;


/* Add Message Data Structure. */
typedef struct
{
    VM_GSM_SMS_STATUS status;        /* Status. */
    VM_GSM_SMS_SIM sim_id;           /* SIM ID. */
    VM_GSM_SMS_STORAGE storage_type; /* Storage type. */
    vm_date_time_t  time_stamp;      /* Timestamp. */
    VMUINT16 content_size;           /* Content buffer size. */
    VMWCHAR* content;                /* Content buffer. */
    VMWCHAR number[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1]; /* Destination number. */
} vm_gsm_sms_add_message_t;


/* Query Data Structure. */
typedef struct
{
    VM_GSM_SMS_SORT   sort_flag;    /* Sort flag. */
    VM_GSM_SMS_ORDER  order_flag;   /* Order flag. */
    VM_GSM_SMS_STATUS status;       /* Status. */
} vm_gsm_sms_query_t;


/* New Message Data Structure. */
typedef struct
{
    VMUINT16 message_id;            /* Message ID. */
    VMUINT16 destination_port;      /* Destination port. */
    VMUINT16 source_port;           /* Source port. */
    VMUINT16 message_length;        /* Message length. */
    VMUINT16 reference;             /* Concatenate message reference. */
    VMUINT8 number[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1];             /* Source number. */
    VMUINT8 sms_center_number[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1];  /* SMS center number. */
    vm_date_time_t time_stamp;          /* Timestamp of service center. */
    VMUINT8 total_segment;              /* Total segments. */
    VMUINT8 segment_number;             /* Segment number. */
    VMUINT8 dcs;                        /* Encoding format. */
    VMUINT8 mti;                        /* Message type identification. Reserved. */
    VMUINT8 protocol_id;                /* Protocol ID. */
    VMUINT8 first_octet;                /* The first octet. */
    VMUINT8 data[1];                    /* Data. */
    VM_GSM_SMS_SIM sim_id;              /* SIM ID. */
    VM_GSM_SMS_STORAGE storage_type;    /* Storage type. */
} vm_gsm_sms_new_message_t;


/* Read Message - Callback Data Structure. */
typedef struct
{
    vm_gsm_sms_read_message_data_t* message_data;   /* Message data */
} vm_gsm_sms_read_message_data_callback_t;


/* Add Message - Callback Data Structure. */
typedef struct
{
    VMUINT16 message_id; /* Message ID. */
} vm_gsm_sms_save_message_callback_t;


/* Delete Message - Callback Data Structure. */
typedef struct
{
    void* dummy; /* Dummy. */
} vm_gsm_sms_delete_message_callback_t;


/* Delete Message List - Callback Data Structure. */
typedef struct
{
    VMUINT16 deleted_message_number; /* The number of deleted message. */
} vm_gsm_sms_delete_message_list_callback_t;


/* Query Message - Callback Data Structure. */
typedef struct
{
    VMUINT16  message_number;   /* Message number. */
    VMUINT16* message_id_list;  /* Message ID list. */
} vm_gsm_sms_query_message_callback_t;


/* Message New SMS Interrupt Event - Callback Data Structure. */
typedef struct
{
    VMUINT16 message_id;    /* Message ID. */
    VMUINT8* content;       /* Message content. */
    void*    message_data;  /* New message structure (vm_gsm_sms_new_message_t). */
} vm_gsm_sms_event_new_sms_t;

/* Gets SMS center number - Callback Data Structure */
typedef struct
{
    VMINT8 sms_center_address[VM_GSM_SMS_MAX_ADDRESS_LENGTH + 1]; /* SMS center number. */
} vm_gsm_sms_get_center_address_callback_t;


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_callback
 * DESCRIPTION
 *  SMS operation callback function.
 * PARAMETERS
 *  callback_data: [IN] Refer to vm_gsm_sms_callback_t
 * RETURNS
 *  void
*****************************************************************************/
typedef void(*vm_gsm_sms_callback)(vm_gsm_sms_callback_t* callback_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_event_callback
 * DESCRIPTION
 *  SMS event callback function.
 * PARAMETERS
 *  event_data: [IN] Refer to vm_gsm_sms_event_t
 * RETURNS
 *  void
*****************************************************************************/
typedef VMINT(*vm_gsm_sms_event_callback)(vm_gsm_sms_event_t* event_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_read_message
 * DESCRIPTION
 *  Reads message data, including message status, number, content and other attributes
 *  (Storage Type, SIM Card, Profile element, etc.) specified by the flag.
 *  The buffer to save the content should be provided by the caller.
 *  The Callback Action Data Structure is vm_gsm_sms_read_message_data_callback_t;
 * PARAMETERS
 *  message_id:    [IN]     Used to specify the message to be read.
 *  change_status: [IN]     Whether to change the UNREAD message to READ.
 *  message_data:  [IN/OUT] Message data for filling back the read data.
 *  callback:      [IN]     Callback function.
 *  user_data:     [IN]     User data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return result and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_read_message_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     vm_gsm_sms_read_message_data_callback_t* read_message;
 *     if(callback_data->action == VM_GSM_SMS_ACTION_READ)
 *     {
 *         if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
 *         {
 *             if(!callback_data->action_data)
 *             {
 *                  vm_log_debug("action_data is NULL.");
 *                  return;
 *             }
 *             read_message = (vm_gsm_sms_read_message_data_callback_t*)callback_data->action_data;
 *             vm_log_debug("message content is %s.",
 *                          vm_gb2312_string((VMWSTR)(read_message->message_data->content_buffer)));
 *             //maybe copy content_buff to other place
 *             vm_free(read_message->message_data->content_buffer);
 *             vm_free(read_message->message_data);
 *         }
 *         else
 *         {
 *             vm_log_debug("read message failed.");
 *         }
 *     }
 * }
 *
 * void custom_sms_read_message(void)
 * {
 *     VMINT16 message_id;
 *     vm_gsm_sms_read_message_data_t * message_data = NULL;
 *     VMWCHAR * content_buff;
 *     VMINT res;
 *
 *     message_id = vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX_INBOX, 0);
 *     if(message_id == -1)
 *     {
 *         vm_log_debug("0 message-index does not exist.");
 *     }
 *     message_data = vm_calloc(sizeof(vm_gsm_sms_read_message_data_t));
 *     if(message_data == NULL)
 *     {
 *         vm_log_debug("message_data vm_calloc failed.");
 *     }
 *     content_buff = vm_calloc((500+1)*sizeof(VMWCHAR));
 *     if(content_buff == NULL)
 *     {
 *         vm_free(message_data);
 *         vm_log_debug("content_buff vm_calloc failed.");
 *     }
 *     message_data->content_buffer = content_buff;
 *     message_data->content_buffer_size = 500;
 *     res = vm_gsm_sms_read_message(message_id, VM_TRUE, message_data,
 *                                   custom_sms_read_message_callback, NULL);
 *     if(res != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug("read message failed.");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_read_message(VMUINT16 message_id, VMBOOL change_status,
                                  vm_gsm_sms_read_message_data_t* message_data,
                                  vm_gsm_sms_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_add_message
 * DESCRIPTION
 *  Saves a new message to SMS Storage. If the caller specifies those parameters
 *  (Status, Address, Content, SIM Card, Storage Type, Profile element), they
 *  will be used. Otherwise, it will use the default values (If the parameters
 *  are in the SMS setting, it will use them as default).
 *  The Callback Action Data Struct is vm_gsm_sms_save_message_callback_t.
 * PARAMETERS
 *  message_data: [IN] Message data to be added.
 *  callback:     [IN] Callback function.
 *  user_data:    [IN] User data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return result and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_add_message_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     vm_gsm_sms_save_message_callback_t* add_message;
 *     if(callback_data->action == VM_GSM_SMS_ACTION_SAVE)
 *     {
 *         if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
 *         {
 *             if(!callback_data->action_data)
 *             {
 *                  vm_log_debug("action_data is NULL.");
 *                  return;
 *             }
 *             add_message = (vm_gsm_sms_save_message_callback_t*)callback_data->action_data;
 *             vm_log_debug("add message callback message cause=%d, result=%d, message_id = %d",
 *                          callback_data->cause, callback_data->result, add_message->message_id);
 *         }
 *         else
 *         {
 *             vm_log_debug("add message failed.");
 *         }
 *     }
 * }
 *
 * void cutomer_sms_add_message(void)
 * {
 *     VMINT res;
 *     VMWCHAR content[100];
 *     vm_gsm_sms_add_message_t message_data;
 *
 *     vm_ascii_to_ucs2(content, 100*2, "what is this,draft");
 *     vm_ascii_to_ucs2(message_data.number, 21*2, "10086");
 *     message_data.content = content;
 *     message_data.content_size = vm_wstrlen(content);
 *     res = vm_gsm_sms_add_message(&message_data, custom_sms_add_message_callback, NULL);
 *     if(res != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug(" cutomer_sms_add_message failed to save.");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_add_message(vm_gsm_sms_add_message_t* message_data,
                                 vm_gsm_sms_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_delete_message
 * DESCRIPTION
 *  Deletes an existing message as specified by message ID. The Callback Action
 *  Data Structure is vm_gsm_sms_delete_message_callback_t;
 * PARAMETERS
 *  message_id: [IN] Specifies the message to be deleted.
 *  callback:   [IN] Callback function.
 *  user_data:  [IN] User data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return result and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_delete_message_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     if(callback_data->action == VM_GSM_SMS_ACTION_DELETE)
 *     {
 *         if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
 *         {
 *             if(!callback_data->action_data)
 *             {
 *                  vm_log_debug("action_data is NULL.");
 *                  return;
 *             }
 *             vm_log_debug("delete message success, result = %d, cause = %d",
 *                          callback_data->result, callback_data->cause);
 *         }
 *         else
 *         {
 *             vm_log_debug("delete message failed.");
 *         }
 *     }
 * }
 *
 * void custom_sms_delete_message(void)
 * {
 *     VMINT16 message_id;
 *     VMINT res;
 *
 *     message_id = vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX_INBOX, 0);
 *     if(message_id == -1)
 *     {
 *         vm_log_debug("custom_sms_delete_message 0 message-index does not exist.");
 *     }
 *     res = vm_gsm_sms_delete_message(message_id, custom_sms_delete_message_callback, NULL);
 *     if(res != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug("custom_sms_delete_message delete message failed.");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_delete_message(VMUINT16 message_id,
                                    vm_gsm_sms_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_delete_message_list
 * DESCRIPTION
 *  Deletes all the existing messages in the specified message box, e.g. Inbox,
 *  Outbox, etc. The Callback Action Data Structure is vm_gsm_sms_delete_message_list_callback_t;
 * PARAMETERS
 *  box_type:  [IN] Message box, in which the messages to be deleted. Refer to
 *                  VM_GSM_SMS_BOX. Multiple message boxes can be combination by
 *                  bitwise OR.
 *  sim_id:    [IN] Specifies which SIM card the message belongs to. Refer to VM_GSM_SIM_ID.
 *  callback:  [IN] Callback function.
 *  user_data: [IN] User data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return result and error code.
 *****************************************************************************/
VM_RESULT vm_gsm_sms_delete_message_list(VM_GSM_SMS_BOX box_type, VM_GSM_SMS_SIM sim_id,
        vm_gsm_sms_callback callback,  void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_get_message_id_list
 * DESCRIPTION
 *  Gets the message ID list of the messages on the device through the query.
 *  The query attributes are specified in query_data. The message ID list is
 *  obtained through the callback, vm_gsm_sms_callback(), which takes
 *  vm_gsm_sms_callback_t with vm_gsm_sms_query_message_callback_t as action_data
 *  that contains the result of the query.
 *
 * PARAMETERS
 *  query_data: [IN] specifies the query attributes, such as sort flag, order
 *                   flag and message status.
 *  callback:   [IN] The callback that will put the query result in the action_data.
 *  user_data:  [IN] Additional user data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return value and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_get_message_id_list_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     vm_gsm_sms_query_message_callback_t* query_result;
 *     if(callback_data->action == VM_GSM_SMS_ACTION_QUERY)
 *     {
 *         if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
 *         {
 *             if(!callback_data->action_data)
 *             {
 *                  vm_log_debug("SMS gets message id list action_data is NULL.");
 *                  return;
 *             }
 *             query_result = (vm_gsm_sms_query_message_callback_t*)callback_data->action_data;
 *             if(!query_result->message_id_list)
 *             {
 *                 vm_log_debug("message_id_list == NULL.");
 *                 return;
 *             }
 *             if(query_result->message_num <=0)
 *             {
 *                vm_log_debug("not find the message_id_list.");
 *             }
 *             vm_log_debug("message id list number is %d", query_result->message_num);
 *
 *             // Uses the message_id_list to get message or delete message.
 *             vm_free(query_result->message_id_list);
 *         }
 *         else
 *         {
 *             vm_log_debug("Query message failed.");
 *         }
 *     }
 * }
 *
 * void custom_sms_get_message_id_list_sample(void)
 * {
 *     vm_gsm_sms_query_t query_data_t;
 *     VMINT res = 0;
 *     query_data_t.sort_flag  = VM_GSM_SMS_SORT_TIMESTAMP;
 *     query_data_t.order_flag = VM_GSM_SMS_ORDER_ASCEND;
 *     query_data_t.status     = VM_GSM_SMS_STATUS_READ;
 *     res = vm_gsm_sms_get_message_id_list(&query_data_t,
 *                               custom_sms_get_message_id_list_callback, NULL);
 *     if(res != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug("custom_sms_get_message_id_list_sample failed");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_get_message_id_list(vm_gsm_sms_query_t* query_data,
        vm_gsm_sms_callback callback,  void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_get_message_id
 * DESCRIPTION
 *  Gets the message ID of the message in the box, as specified by message_index.
 * PARAMETERS
 *  box_type:      [IN] Message box.
 *  message_index: [IN] The position of the message in the message box. The following
 *                      example illustrates the add/delete message.
 * For example: msg_id_list[message_index] = message_id;
 * msg_id_list[0]=0-------------------msg_id_list[0]=0-------------msg_id_list[0]=0
 * msg_id_list[1]=1--delete msg_id=2--msg_id_list[1]=1--add a msg--msg_id_list[1]=1
 * msg_id_list[2]=2------------------>msg_id_list[2]=3------------>msg_id_list[2]=2
 * msg_id_list[3]=3------------------------------------------------msg_id_list[3]=3
 * RETURNS
 *  The message ID, if it is a non-negative number. It returns -1, if the message
 *  specified by the message_index does not exist.
 * EXAMPLE
 * <code>
 * void custom_sms_get_inbox_message_id(void)
 * {
 *     VMINT16 message_id = 0;
 *     message_id = vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX_INBOX, 0);
 *     if(message_id < 0)
 *     {
 *         vm_log_debug("The inbox message id of the message_index 0 does not exist");
 *     }
 * }
 * </code>
 *****************************************************************************/
VMINT16 vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX box_type, VMUINT16 message_index);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_get_box_size
 * DESCRIPTION
 *  Gets number of the messages in a box list
 * PARAMETERS
 *  box_type: [IN] Specifies the message box type.
 * RETURNS
 *  The number of messages, if non-negative. It returns negative value, if there
 *  is an error.
 * EXAMPLE
 * <code>
 * void custom_sms_get_draft_box_size(void)
 * {
 *     VMINT16 size = 0;
 *     size = vm_gsm_sms_get_box_size(VM_GSM_SMS_BOX_DRAFTS);
 *     if(size < 0)
 *     {
 *         vm_log_debug("get draft box size error");
 *     }
 * }
 * </code>
 *****************************************************************************/
VMINT16 vm_gsm_sms_get_box_size(VM_GSM_SMS_BOX box_type);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_get_center_address
 * DESCRIPTION
 *  Asynchronously gets center address of the SIM for SMS, which is a phone number.
 * PARAMETERS
 *  sim_id:    [IN] Specifies the SIM card to get the center address for.
 *  callback:  [IN] Callback function.
 *  user_data: [IN] Additional user data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return value and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_get_sc_address_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     vm_gsm_sms_get_center_address_callback_t* sc_address;
 *     if(callback_data->action == VM_GSM_SMS_ACTION_GET_CENTER_ADDRESS)
 *     {
 *         if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
 *         {
 *             if(!callback_data->action_data)
 *             {
 *                 vm_log_debug("action data is NULL");
 *                 return;
 *              }
 *              sc_address = (vm_gsm_sms_get_center_address_callback_t*)callback_data->action_data;
 *              vm_log_debug("sc_address is :%s.",(char*)sc_address->sms_center_address);
 *         }
 *         else
 *         {
 *              vm_log_debug("get sc failed,res != VM_GSM_SMS_CAUSE_NO_ERROR.");
 *         }
 *     }
 * }
 *
 * void custom_sms_get_sc_address(void)
 * {
 *     VMINT res = 0;
 *     res = vm_gsm_sms_get_center_address(VM_GSM_SMS_SIM_1,
 *                                   custom_sms_get_sc_address_callback, NULL);
 *     if(res != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug("custom_sms_get_sc_address: Gets SC address failed");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_get_center_address(VM_GSM_SMS_SIM sim_id,
                                        vm_gsm_sms_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_get_center_address_sync
 * DESCRIPTION
 *  Synchronously gets center address of the SIM for SMS, which is a phone number.
 * PARAMETERS
 *  center_address: [OUT] Contains the center address after this function returns.
 *  sim_id:         [IN]  Specifies the SIM card to get the center address for.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return value and error code.
 *****************************************************************************/
VM_RESULT vm_gsm_sms_get_center_address_sync(
    vm_gsm_sms_get_center_address_callback_t* center_address,
    VM_GSM_SMS_SIM sim_id);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_set_interrupt_event_handler
 * DESCRIPTION
 *  Sets the interrupt handler for the specified event type.
 * PARAMETERS
 *  event_id:       [IN] Specifies the event type that the callback will listen to.
 *  event_callback: [IN] The callback that listens to the events as specified by the event_id.
 *  user_data:      [IN] Additional user data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return value and error code.
 * EXAMPLE
 * <code>
 * VMINT custom_sms_new_message_interrupt_proc(vm_gsm_sms_event_t* event_data)
 * {
 *     vm_gsm_sms_event_new_sms_t* event_new_message_ptr;
 *     vm_gsm_sms_new_message_t* new_message_ptr = NULL;
 *     VMINT res = 0;
 *
 *     if(event_data->event_id == VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE)
 *     {
 *         event_new_message_ptr = (vm_gsm_sms_event_new_sms_t *)event_data->event_info; // Gets the event info.
 *         new_message_ptr = event_new_message_ptr->message_data; // Gets the message data.
 *         res = vm_gsm_sms_clear_interrupt_event_handler(event_data->event_id,
 *                                 custom_sms_new_message_interrupt_proc, NULL);
 *         if(res != VM_GSM_SMS_RESULT_OK)
 *         {
 *              vm_log_debug("custom_sms_new_message_interrupt_proc have got new message, but cannot delete interrupt");
 *         }
 *     }
 *     return res;
 * }
 *
 * void custom_sms_set_interrupt_event_handler_sample(void)
 * {
 *     VMINT return = 0;
 *     return = vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE,
 *                                              custom_sms_new_message_interrupt_proc, NULL);
 *     if(return != VM_GSM_SMS_RESULT_OK)
 *     {
 *         vm_log_debug("Cannot set interrupt!");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID event_id,
        vm_gsm_sms_event_callback event_callback,  void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_clear_interrupt_event_handler
 * DESCRIPTION
 *  Clears the interrupt handler for the specified event type.
 * PARAMETERS
 *  event_id:       [IN] Specifies the event type that the callback was registered for.
 *  event_callback: [IN] The callback to be cleared.
 *  user_data:      [IN] Additional user data.
 * RETURNS
 *  Refer to VM_GSM_SMS_RESULT for return value and error code.
 * EXAMPLE
 * <code>
 * Refer to the example for vm_gsm_sms_set_interrupt_event_handler.
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_clear_interrupt_event_handler(VM_GSM_SMS_EVENT_ID event_id,
        vm_gsm_sms_event_callback event_callback,  void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_is_sms_ready
 * DESCRIPTION
 *  Checks if the SMS message is ready.
 * PARAMETERS
 *  void
 * RETURNS
 *  VM_TRUE, if the SMS message is ready. It returns VM_FALSE, if otherwise.
 *****************************************************************************/
VMBOOL vm_gsm_sms_is_sms_ready(void);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_send
 * DESCRIPTION
 *  Sends SMS
 * PARAMETERS
 *  phone_number: [IN] Destination phone number.
 *  content:      [IN] SMS content to be sent.
 *  callback:     [IN] The callback to be invoked after the SMS is sent. The actual
 *                     result of the SMS send operation is captured by the result
 *                     field of vm_gsm_sms_callback_t.
 *                     Refer to VM_GSM_SMS_SEND_RESULT for detail.
 *  user_data:    [IN] Additional user data.
 * RETURNS
 *  Refer to VM_GSM_SMS_SEND_RESULT for return value and error code.
 * EXAMPLE
 * <code>
 * void custom_sms_send_callback(vm_gsm_sms_callback_t* callback_data)
 * {
 *     if(callback_data->action == VM_GSM_SMS_ACTION_SEND)
 *     {
 *         vm_log_debug("Send SMS callback, result = %d, callback_data->result);
 *     }
 * }
 *
 * void custom_sms_send(void)
 * {
 *     VMWCHAR content[100];
 *     VMWCHAR num[100];
 *     VMINT res = 0;
 *     vm_ascii_to_ucs2(content, 100*2, "Hello, LINKIT SMS!");
 *     vm_ascii_to_ucs2(num, 100*2, "10086");
 *     res = vm_gsm_sms_send(num, content, custom_sms_send_callback, NULL);
 *     if(res == 0)
 *     {
 *         vm_log_debug("Send SMS failed!");
 *     }
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_sms_send(VMWSTR phone_number, VMWSTR content, vm_gsm_sms_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_gsm_sms_cancel_send
 * DESCRIPTION
 *  Cancels sending SMS. The function can be used only after vm_gsm_sms_send()
 *  has been invoked and it returns TRUE, and before the corresponding callback
 *  is invoked. Otherwise, calling this function is invalid.
 * PARAMETERS
 *  void.
 * RETURNS
 *  void.
 *****************************************************************************/
void vm_gsm_sms_cancel_send(void);

#ifdef __cplusplus
}
#endif

#endif /* VMGSM_SMS_SDK_H */
