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

#ifndef VMGSM_TEL_SDK_H
#define VMGSM_TEL_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmaudio.h"


/* Maximum length of the telephone numbers. */
#define VM_GSM_TEL_MAX_NUMBER_LENGTH (41)

/* Maximum length of the error messages. */
#define VM_GSM_TEL_MAX_ERROR_MESSAGE_LENGTH (50)

/* Maximum length of IP number. */
#define VM_GSM_TEL_MAX_IP_NUMBER_LENGTH (10)

/* UCM error cause enum. */
typedef enum
{
    VM_GSM_TEL_OK,                              /* OK. */
    VM_GSM_TEL_ERROR_UNSPECIFIED_ERROR,         /* Unspecified error. */
    VM_GSM_TEL_ERROR_CANCEL,                    /* Cancel. */
    VM_GSM_TEL_ERROR_USER_ABORT,                /* User abort. */
    VM_GSM_TEL_ERROR_ACTION_NOT_ALLOWED,        /* Action not allowed. */
    VM_GSM_TEL_ERROR_NOT_ALLOWED_TO_DIAL,       /* Not allowed to dial. */
    VM_GSM_TEL_ERROR_NOT_ALLOWED_TO_ACCEPT,     /* Not allowed to accept. */
    VM_GSM_TEL_ERROR_NO_ACTIVE_CALL,            /* No active call. */
    VM_GSM_TEL_ERROR_CALLED_PARTY_BUSY,         /* Called party busy. */
    VM_GSM_TEL_ERROR_NETWORK_NOT_AVAILABLE,     /* Network not available. */
    VM_GSM_TEL_ERROR_CALLED_PARTY_NOT_ANSWERED, /* Called party not answered. */
    VM_GSM_TEL_ERROR_SOS_NUMBER_ONLY,           /* SOS number only. */
    VM_GSM_TEL_ERROR_INVALID_NUMBER,            /* Invalid number. */
    VM_GSM_TEL_ERROR_EMPTY_NUMBER,              /* Empty number. */
    VM_GSM_TEL_ERROR_SOS_CALL_EXISTS,           /* Exist SOS call. */
    VM_GSM_TEL_ERROR_PREFERRED_MODE_PROHIBIT,   /* Preferred mode prohibit. */
    VM_GSM_TEL_ERROR_FLIGHT_MODE_PROHIBIT,      /* Flight mode prohibit. */
    VM_GSM_TEL_ERROR_LOW_BATTERY,               /* Low battery. */
    VM_GSM_TEL_ERROR_UCM_BUSY,                  /* Busy. */
    VM_GSM_TEL_ERROR_VT_FALLBACK,               /* VT fallback. */
    VM_GSM_TEL_RESULT_MAX = 0x7FFFFFFF          /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_TEL_RESULT;



/* Call indication enum. */
typedef enum
{
    VM_GSM_TEL_INDICATION_INCOMING_CALL = 0, /* Incoming call. */
    VM_GSM_TEL_INDICATION_OUTGOING_CALL,     /* Outgoing call. */
    VM_GSM_TEL_INDICATION_CONNECTED,         /* Call is connected. */
    VM_GSM_TEL_INDICATION_CALL_ENDED,        /* Call is ended normally. */
    VM_GSM_TEL_INDICATION_MAX = 0x7FFFFFFF   /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_TEL_INDICATION;

/* Call type enum */
typedef enum
{
    VM_GSM_TEL_CALL_SIM_1   = 0x0001,    /* SIM1 voice call type */
    VM_GSM_TEL_CALL_SIM_2   = 0x0020,    /* SIM2 voice call type */
    VM_GSM_TEL_CALL_SIM_3   = 0x0200,    /* SIM3 voice call type */
    VM_GSM_TEL_CALL_SIM_4   = 0x1000,    /* SIM4 voice call type */
} VM_GSM_TEL_CALL_SIM;

/* Call ID struct, should be unique. */
typedef struct
{
    VM_GSM_TEL_CALL_SIM sim; /* Call type. */
    VMUINT16 group_id;       /* Group ID. */
    VMUINT16 call_id;        /* Call ID. */
} vm_gsm_tel_id_info_t;


/* Incoming & outgoing call indication struct. */
typedef struct
{
    vm_gsm_tel_id_info_t uid_info;                     /* Call ID. */
    VMUINT8 num_uri[VM_GSM_TEL_MAX_NUMBER_LENGTH + 1]; /* Phone number in ASCII format. */
} vm_gsm_tel_call_info_t;


/* Answer indication struct. */
typedef struct
{
    vm_gsm_tel_id_info_t uid_info; /* Call ID. */
} vm_gsm_tel_connect_indication_t;

/* Listen call event callback structure. */
typedef struct
{
    VM_GSM_TEL_INDICATION  call_type; /* vm_call_type incoming, outgoing. */
    void*                  data;      /* Event data. */
} vm_gsm_tel_call_listener_data_t;

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_tel_call_listener_callback
 * DESCRIPTION
 *    Callback function of the call indication.
 * PARAMETERS
 *    data: [IN] Refer to vm_gsm_tel_call_listener_data_t.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_gsm_tel_call_listener_callback)(vm_gsm_tel_call_listener_data_t* data);



/* Call actions. */
typedef enum
{
    VM_GSM_TEL_CALL_ACTION_NO = 0,     /* No action. */
    VM_GSM_TEL_CALL_ACTION_DIAL,       /* Dial action (vm_gsm_tel_dial_action_request_t, vm_gsm_tel_action_response_t). */
    VM_GSM_TEL_CALL_ACTION_ACCEPT,     /* Accept action (vm_gsm_tel_single_call_action_request_t, vm_gsm_tel_action_response_t). */
    VM_GSM_TEL_CALL_ACTION_HOLD,       /* Hold action (vm_gsm_tel_single_group_action_request_t, vm_gsm_tel_action_response_t). */
    VM_GSM_TEL_CALL_ACTION_END_SINGLE, /* End single action (vm_gsm_tel_single_call_action_request_t, vm_gsm_tel_action_response_t) */
    VM_GSM_TEL_CALL_ACTION_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_TEL_CALL_ACTION;


/* Action struct for VM_GSM_TEL_CALL_ACTION_DIAL call type. */
typedef struct
{
    VM_GSM_TEL_CALL_SIM sim;                            /* Call type. */
    VMUINT16 num_uri[VM_GSM_TEL_MAX_NUMBER_LENGTH + 1]; /* Number, in UCS-2 encoding. */
    VMUINT8 module_id;                                  /* Initiate module ID. The default is 0. */
    VMBOOL is_ip_dial;                                  /* IP dial, currently only applicable for GSM & 3G. The default is VM_FALSE. */
    void* phonebook_data;                               /* Phonebook data for call service to query caller info. The default is NULL. */
} vm_gsm_tel_dial_action_request_t;

/* Action struct for VM_GSM_TEL_CALL_ACTION_ACCEPT & VM_GSM_TEL_CALL_ACTION_END_SINGLE call types. */
typedef struct
{
    vm_gsm_tel_id_info_t action_id; /* Call ID. */
} vm_gsm_tel_single_call_action_request_t;

/* Action struct for VM_GSM_TEL_CALL_ACTION_HOLD call type. */
typedef struct
{
    VM_GSM_TEL_CALL_SIM sim; /* Call type. */
    VMUINT16 action_group;   /* Group ID. */
} vm_gsm_tel_single_group_action_request_t;


/* Action CALLBACK structure. */
typedef struct
{
    VM_GSM_TEL_RESULT result; /* Result */
    VMUINT16 error_msg[VM_GSM_TEL_MAX_ERROR_MESSAGE_LENGTH + 1]; /* Recommend error message, UCS-2 encoding. */
} vm_gsm_tel_result_info_t;

typedef struct
{
    vm_gsm_tel_result_info_t result_info; /* Result. */
    VMBOOL no_counter; /* Start to count duration or not, only for accept action. */
} vm_gsm_tel_action_response_t;

/* Action callback data struct. */
typedef struct
{
    VM_GSM_TEL_CALL_ACTION type_action;        /* Action type: accept, hold, end. */
    vm_gsm_tel_action_response_t data_act_rsp; /* Data of the action callback struct. */
    void*   user_data;	                       /* user_data of APP. */
} vm_gsm_tel_call_actions_callback_data_t;

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_tel_call_actions_callback
 * DESCRIPTION
 *    The callback for the call actions. The action is specified in the type_action
 *    of data of vm_gsm_tel_call_actions_callback_data_t.
 * PARAMETERS
 *    data: [IN] The data to be used by the callback. Refer to vm_gsm_tel_call_actions_callback_data_t.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_gsm_tel_call_actions_callback)(vm_gsm_tel_call_actions_callback_data_t* data);

/* Action structure */
typedef struct
{
    VM_GSM_TEL_CALL_ACTION action; /* Action type: accept, hold, end. */
    void*   data_action;           /* Data of the action struct. */
    void*   user_data;			   /* user data of APP. */
    vm_gsm_tel_call_actions_callback callback;	/* Callback function of the action. */
} vm_gsm_tel_call_actions_data_t;


/*****************************************************************************
 * FUNCTION
 *    vm_gsm_tel_call_reg_listener
 * DESCRIPTION
 *    APP uses this function to register a callback function to listen to call events.
 *    The user_data for the callback function will be provided by the underneath
 *    native.
 * PARAMETERS
 *    callback: [IN]  A callback function, reference vm_gsm_tel_call_listener_callback
 * RETURNS
 *    It returns 0, if the callback is registered successfully. It returns -2,
 *    if the parameter is invalid. It returns -3, when out of resources. It
 *    returns -4 or less, if there is an internal error.
 *****************************************************************************/
VM_RESULT vm_gsm_tel_call_reg_listener(vm_gsm_tel_call_listener_callback callback);


/*****************************************************************************
 * FUNCTION
 *    vm_gsm_tel_call_actions
 * DESCRIPTION
 *    APP uses this function to control call progress, such as accepting an incoming
 *    call, putting a hold on a call, or ending an ongoing call.
 * PARAMETERS
 *    data: [IN] Action command, refer to vm_gsm_tel_call_actions_data_t.
 * RETURNS
 *    It returns 0, if the function is executed successfully with the specified
 *    action. It returns -2, if the parameter is invalid. It returns -3, when out
 *    of resources. It returns -4 or less, if there is an internal error.
 *****************************************************************************/
VM_RESULT vm_gsm_tel_call_actions(const vm_gsm_tel_call_actions_data_t* data);

/* Telephony output device structure */
typedef enum
{
  VM_GSM_TEL_DEVICE_NORMAL = 0,   /* Earphone, Car-kit  */
  VM_GSM_TEL_DEVICE_HEADSET = 1,  /* Earphone, Car-kit  */
  VM_GSM_TEL_DEVICE_LOUDSPK = 2,  /* Loudspeaker for free sound  */
  VM_GSM_TEL_DEVICE_MAX = 0xFF
}VM_GSM_TEL_DEVICE;

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_tel_keytone_callback
 * DESCRIPTION
 *  Telephony DTMF callback function pointer
 * PARAMETERS
 *  key_ascii:  [IN] The ascii code.
 *  user_data:  [IN] User data.
 *****************************************************************************/
typedef void (*vm_gsm_tel_keytone_callback)(VMINT16 key_ascii, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_tel_keytone_start_detect
 * DESCRIPTION
 *  Start telephony keytone detect. When there is keytone, this callback will be invoked, the interface should be invoke after telephony connected.
 * PARAMETERS
 * callback:    [IN] Callback function pointer.
 * user_data:   [IN] User data.
 * RETURNS
 *  VM_SUCCESS : start detect successfully.
 *  VM_FAIL : start detect failed.
*****************************************************************************/
VM_RESULT vm_gsm_tel_keytone_start_detect(vm_gsm_tel_keytone_callback callback, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_tel_keytone_stop_detect
 * DESCRIPTION
 *  Stop telephony keytone detect.
 * PARAMETERS
 *    void
 * RETURNS
 *    void
*****************************************************************************/
void vm_gsm_tel_keytone_stop_detect(void);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_tel_set_output_device
 * DESCRIPTION
 *  Sets Telephony output device.
 * PARAMETERS
 *  device : [IN] Output device, like earphone or loudspeaker.
 * RETURNS
 *    void
*****************************************************************************/
void vm_gsm_tel_set_output_device(VM_GSM_TEL_DEVICE device);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_tel_set_volume
 * DESCRIPTION
 *  Sets Telephony volume level.
 * PARAMETERS
 *  volume : [IN] Desired volume level, the range spreads from 0-6.
 * RETURNS
 *    void
*****************************************************************************/
void vm_gsm_tel_set_volume(VM_AUDIO_VOLUME volume);

#ifdef __cplusplus
}
#endif

#endif /* VMTEL_SDK_H */
