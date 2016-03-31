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

#ifndef VMKEYPAD_SDK_H
#define VMKEYPAD_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* Keypad events passed to vm_keypad_event_callback. */
typedef enum
{
    VM_KEYPAD_EVENT_UP = 1,         /* Key up event. */
    VM_KEYPAD_EVENT_DOWN = 2,       /* Key down event. */
    VM_KEYPAD_EVENT_LONG_PRESS = 3, /* Key long press event. */
    VM_KEYPAD_EVENT_REPEAT = 4,     /* Key repeat event. */

    VM_KEYPAD_EVENT_MAX = 0x7FFFFFFF
} VM_KEYPAD_EVENT;

/*****************************************************************************
 * FUNCTION
 *  vm_keypad_event_callback
 * DESCRIPTION
 *  Callback function of keypad event handler. Register this callback with
 *  vm_keypad_register_event_callback to receive key events processed by
 *  underlying framework.
 *  The underlying system framework may process special key codes such as
 *  power key. Developers may choose to intercept the key code
 *  by returning non-zero values in this callback function, or bypassing
 *  the key code to system framework by returning 0.
 * PARAMETERS
 *  event:  [IN] Refer to VM_KEYPAD_EVENT.
 *  code:   [IN] The key code associated with the event.
 * RETURNS
 *  This event handler should return non-zero values if application has
 *  processed the key event and don't want to pass the key event to the
 *  system framework.
 *  Return 0 to bypass this key event to underlying system framework.
 *****************************************************************************/
typedef VMINT (*vm_keypad_event_callback)(VM_KEYPAD_EVENT event, VMINT code);


/*****************************************************************************
 * FUNCTION
 *  vm_reg_keyboard_callback
 * DESCRIPTION
 *  Registers system keypad event callback.
 *  The keypad events are passed to the registered callback.
 *  There can only have one handler registered at the same time.
 * PARAMETERS
 *  vm_keypad_event_callback: [IN] Callback function that processes the keypad
 *                                 event. Pass NULL to de-register the callback.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_keypad_register_event_callback(vm_keypad_event_callback callback);


#ifdef __cplusplus
}
#endif

#endif /* VMKEYPAD_SDK_H */

