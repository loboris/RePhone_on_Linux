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

#ifndef VMTOUCH_SDK_H
#define VMTOUCH_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

typedef enum
{
    VM_TOUCH_EVENT_TAP = 1,         /* Touch down event. */
    VM_TOUCH_EVENT_RELEASE = 2,     /* Touch release event. */
    VM_TOUCH_EVENT_MOVE = 3,        /* Touch move event. */
    VM_TOUCH_EVENT_LONG_TAP = 4,    /* Touch long tap event. Note:
                                       1. VM_TOUCH_EVENT_LONG_TAP event is
                                          invoked when the Touch is tapped for
                                          a period of time and stays at the
                                          same place where it is tapped down.
                                       2. If Touch move happen after Touch down,
                                          there will be no
                                          VM_TOUCH_EVENT_LONG_TAP event.
                                       3. VM_TOUCH_EVENT_LONG_TAP can only
                                          happen one time at most before
                                          VM_TOUCH_EVENT_RELEASE. */
    VM_TOUCH_EVENT_DOUBLE_CLICK = 5,/* Touch double click event. */
    VM_TOUCH_EVENT_REPEAT = 6,      /* Touch repeat event. Note:
                                       1. VM_TOUCH_EVENT_REPEAT can happen after
                                          VM_TOUCH_EVENT_LONG_TAP or
                                          VM_TOUCH_EVENT_MOVE or another
                                          VM_TOUCH_EVENT_REPEAT.
                                       2. The coordinate follows
                                          previous VM_TOUCH_EVENT_REPEAT
                                          events.*/
    VM_TOUCH_EVENT_ABORT = 7,       /* Touch abort event. */

    VM_TOUCH_EVENT_MAX = 0x7FFFFFFF
} VM_TOUCH_EVENT;

/*****************************************************************************
 * FUNCTION
 *  vm_touch_event_callback
 * DESCRIPTION
 *  Callback of touch events.
 * PARAMETERS
 *  event: [IN] touch event,see VM_TOUCH_EVENT
 *  x: [IN] x coordinate, starting from upper-left of the display, in pixels.
 *  y: [IN] y coordinate, starting from upper-left of the display, in pixels.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_touch_event_callback)(VM_TOUCH_EVENT event, VMINT x, VMINT y);

/*****************************************************************************
 * FUNCTION
 *  vm_touch_register_event_callback
 * DESCRIPTION
 *  Registers touch event callback.
 * PARAMETERS
 *  callback : [IN] touch event callback.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_touch_register_event_callback(vm_touch_event_callback callback);

#ifdef __cplusplus
}
#endif

#endif /* VMTOUCH_SDK_H */

