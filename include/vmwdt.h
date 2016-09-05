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

#ifndef VMWDT_SDK_H
#define VMWDT_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* Watchdog handle */
typedef VMINT VM_WDT_HANDLE;


/*****************************************************************************
 * FUNCTION
 *    vm_wdt_start
 * DESCRIPTION
 *    Starts a watchdog.
 * PARAMETERS
 *    tick: [IN] The value of the watchdog timer, in ticks.
 * RETURNS
 *    If it returns a positive integer, it is the handle of the watchdog.
 *    Otherwise, it returns a negative integer.
 * EXAMPLE
 * <code>
 * VM_WDT_HANDLE handle = vm_wdt_start(1000);
 *
 * if (handle >= 0) {
 *     // Do something.
 *     vm_wdt_reset(handle);
 * } else {
 *     // Do error logging.
 * }
 *
 * // Done with doing something.
 * vm_wdt_stop(handle);
 * </code>
 *****************************************************************************/
VM_WDT_HANDLE vm_wdt_start(VMUINT tick);

/*****************************************************************************
 * FUNCTION
 *    vm_wdt_reset
 * DESCRIPTION
 *    Resets the watchdog timer. The timer will start again immediately and will
 *    fire after the number of ticks set in the vm_wdt_start() elapses.
 * PARAMETERS
 *    handle: [IN] The watchdog handle returned from vm_wdt_start().
 * EXAMPLE
 * <code>
 * // Time out is set to 1,000 ticks.
 * VM_WDT_HANDLE handle = vm_wdt_start(1000);
 *
 * // Check the return value.
 * if (handle >= 0) {
 *     // Do something.
 *     vm_wdt_reset(handle);
 * } else {
 *     // Do error logging.
 * }
 *
 * // Do something.
 * vm_wdt_reset(handle);
 *
 * // Done with the watchdog timer.
 * vm_wdt_stop(handle);
 *****************************************************************************/
void vm_wdt_reset(VM_WDT_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_wdt_stop
 * DESCRIPTION
 *    Stops the watchdog timer.
 * PARAMETERS
 *    handle: [IN] The watchdog handle returned from vm_wdt_start().
 * RETURNS
 *    void
 * EXAMPLE
 * <code>
 * VM_WDT_HANDLE handle = vm_wdt_start(1000);
 *
 * // Do something.
 * vm_wdt_reset(handle);
 *
 * // Done with the watchdog timer.
 * vm_wdt_stop(handle);
 *****************************************************************************/
void vm_wdt_stop(VM_WDT_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *  vm_wdt_feed
 * DESCRIPTION
 *  feed watch
 * PARAMETERS
 *  handle  : [IN]   watch dog handle that vm_wdt_start returned.
 * RETURN
 *  void
 * EXAMPLE
 * <code>
 * VMINT handle = vm_wdt_start(1000);
 * // do somting
 * vm_wdt_feed(handle);
 * // finish doing
 * vm_wdt_stop(handle);
 *****************************************************************************/
void vm_wdt_feed(VM_WDT_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* VMWDT_SDK_H */

