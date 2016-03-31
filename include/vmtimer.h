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

#ifndef VMTIMER_SDK_H
#define VMTIMER_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* timer error code type */
typedef enum
{
    VM_TIMER_OK = 0,                 /* Success */

    VM_TIMER_ERROR_OF_NO_FREE = -5,  /* Error of not free timer handle. */
    VM_TIMER_ERROR_OF_PROCESS,       /* Error related to process or handle. */
    VM_TIMER_ERROR_OF_RESOURCE,      /* Error related to saving or getting resource data. */
    VM_TIMER_ERROR_OF_MEMORY,        /* Error related to allocating or freeing memory. */
    VM_TIMER_ERROR,                  /* Error that is not specific. */

    VM_TIMER_RESULT_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_TIMER_RESULT;


/* Precise timer id. */
typedef VMINT VM_TIMER_ID_PRECISE;

/* Non-precise timer id. */
typedef VMINT VM_TIMER_ID_NON_PRECISE;

/* HISR (High-level Interrupt Service Routine) timer id. */
typedef void* VM_TIMER_ID_HISR;

/* Precise timer callback function. */
typedef void (*vm_timer_precise_callback)(VM_TIMER_ID_PRECISE timer_id, void* user_data);

/* Non-precise timer callback function. */
typedef void (*vm_timer_non_precise_callback)(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data);

/* HISR timer callback function. */
typedef void (*vm_timer_hisr_callback)(void* user_data);



/*****************************************************************************
 * FUNCTION
 *    vm_timer_create_precise
 * DESCRIPTION
 *    Creates a precise timer. Once the timer is created, it will fire repeatedly
 *    at the interval until the timer is deleted. A precise timer will be stopped
 *    when the system gets into the power saving mode. An application is allowed
 *    to have up to 10 precise timers.
 *
 *    The callback functions of both precise and non-precise timers are executed
 *    by the same thread. A callback will not be executed until the execution of
 *    the previous callback completes.
 *
 * PARAMETERS
 *    milliseconds:    [IN] The timer interval in millisecond.
 *    timer_procedure: [IN] The callback function that is to be invoked when the timer fires.
 *    user_data:       [IN] The data to be consumed by the callback function.
 * RETURNS
 *    A timer handle. If the timer handle is a positive integer, it means the
 *    timer is created successfully. Otherwise, a negative integer is returned.
 *    Refer to VM_TIMER_RESULT for error code, if it fails.
 * EXAMPLE
 * <code>
 * void customer_precise_timer_example_proc(VM_TIMER_ID_PRECISE tid, void* user_data)
 * {
 *     static VMINT s_restore_num = 0;
 *     s_restore_num++;
 *     if(s_restore_num > 10)
 *     {
 *         vm_timer_delete_precise(tid);
 *     }
 * }
 *
 * void customer_run_precise_timer(void)
 * {
 *      VM_TIMER_ID_PRECISE timer_id = 0;
 *      timer_id = vm_timer_create_precise(1000, (vm_timer_precise_callback)customer_precise_timer_example_proc, NULL);
 *      if(timer_id < 0)
 *      {
 *          vm_log_debug("customer_run_timer create timer fail");
 *      }
 * }
 * </code>
 *****************************************************************************/
VM_TIMER_ID_PRECISE vm_timer_create_precise(VMUINT32 milliseconds, vm_timer_precise_callback timer_procedure, void* user_data);


/*****************************************************************************
 * FUNCTION
 *    vm_timer_delete_precise
 * DESCRIPTION
 *    Deletes a precise timer that is created by vm_timer_create_precise(). It
 *    cannot be used to delete other types of timers.
 * PARAMETERS
 *    timer_id: [IN] The timer ID of the precise timer to be deleted.
 * RETURNS
 *    It returns 0, if it deletes the timer successfully. Otherwise, refer to
 *    VM_TIMER_RESULT for error code, if it fails.
 * EXAMPLE
 * <code>
 * Refer to vm_timer_create_precise code example.
 * </code>
 *****************************************************************************/
VM_RESULT vm_timer_delete_precise(VM_TIMER_ID_PRECISE timer_id);


/*****************************************************************************
 * FUNCTION
 *    vm_timer_create_non_precise
 * DESCRIPTION
 *    Creates a non-precise timer. Once the timer is created, it will fire repeatedly
 *    at the interval until the timer is deleted. A non-precise timer will not stop
 *    when the system gets into the power saving mode. The non-precise timer can
 *    be used to wake the system up. An application is allowed to have up to 10
 *    non-precise timers.
 *
 *    The callback functions of both precise and non-precise timers are executed
 *    by the same thread. A callback will not be executed until the execution of
 *    the previous callback completes.
 * PARAMETERS
 *    milliseconds:    [IN] The timer interval in millisecond.
 *    timer_procedure: [IN] The callback function that is to be invoked when the timer fires.
 *    user_data:       [IN] The data to be consumed by the callback function.
 * RETURNS
 *    A timer handle. If the timer handle is a positive integer, it means the
 *    timer is created successfully. Otherwise, a negative integer is returned.
 *    Refer to VM_TIMER_RESULT for error code, if fails.
 * EXAMPLE
 * <code>
 * void customer_non_precise_timer_example_proc(VM_TIMER_ID_NON_PRECISE tid, void* user_data)
 * {
 *     static VMINT s_restore_num = 0;
 *     s_restore_num++;
 *     if(s_restore_num > 10)
 *     {
 *         vm_timer_delete_non_precise(tid);
 *     }
 * }
 *
 * void customer_run_non_precise_timer(void)
 * {
 *      VM_TIMER_ID_NON_PRECISE timer_id = 0;
 *      timer_id = vm_timer_create_non_precise(1000, (vm_timer_non_precise_callback)customer_non_precise_timer_example_proc, NULL);
 *      if(timer_id < 0)
 *      {
 *         vm_log_debug("customer_run_time create non-precise timer fail!");
 *      }
 * }
 * </code>
 *****************************************************************************/
VM_TIMER_ID_NON_PRECISE vm_timer_create_non_precise(VMUINT32 milliseconds, vm_timer_non_precise_callback timer_procedure, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_timer_delete_non_precise
 * DESCRIPTION
 *    Deletes a non-precise timer that is created by vm_timer_create_non_precise().
 *    It cannot be used to delete other types of timers.
 * PARAMETERS
 *    timer_id: [IN] The timer ID of the non-precise timer to be deleted.
 * RETURNS
 *    It returns 0, if succeeds. Otherwise, a negative value is returned. Refer
 *    to VM_TIMER_RESULT for error code.
 * EXAMPLE
 * <code>
 * Refer to vm_timer_create_non_precise code example.
 * </code>
 *****************************************************************************/
VM_RESULT vm_timer_delete_non_precise(VM_TIMER_ID_NON_PRECISE timer_id);

/*****************************************************************************
 * FUNCTION
 *    vm_timer_create_hisr
 * DESCRIPTION
 *    Creates a HISR timer. Once the timer is created, it will fire repeatedly
 *    at the interval until the timer is deleted. The thread priority of a HISR
 *    timer is higher than the main thread. The callback of a HISR timer is invoked
 *    in the interrupt context. Therefore, the callback of a HISR timer should not
 *    be used to access any LinkIt SDK APIs. The callback should not use signal,
 *    mutex or thread API. But it can call the DCL API. It should only be used to
 *    perform fast operations, such as value updates.
 * PARAMETERS
 *    timer_name: [IN] The timer name. It is used for debugging purpose. The length
 *                     of the name cannot be more than 8 characters. Otherwise, it
 *                     will be truncated.
 * RETURNS
 *    A timer handle. If the timer handle is a positive integer, it means the
 *    timer is created successfully. Otherwise, a negative integer is returned.
 *    Refer to VM_TIMER_RESULT for error code.
 * EXAMPLE
 * <code>
 * VM_TIMER_ID_HISR hisr_id = NULL;
 * static VMINT num_flag = 0;
 * static void cuatomer_hisr_timer_proc(void* para)
 * {
 *     num_flag++;
 *    //cant use LinkIT API
 * }
 * static void customer_delete_timer_proc(VM_TIMER_ID_PRECISE tid, void* user_data)
 * {
 *     vm_log_debug("customer_delete_timer_proc delete timer num_flag = %d", num_flag);
 *     if(num_flag > 5)
 *     {
 *         vm_log_debug("customer_delete_timer_proc cancel timer");
 *         vm_timer_delete_hisr(hisr_id);
 *         vm_timer_delete_precise(tid);
 *      }
 * }
 *
 * void customer_run_hisr_timer(void)
 * {
 *     hisr_id = vm_timer_create_hisr("HISR_TIMER");
 *     if(hisr_id != NULL)
 *     {
 *         vm_timer_set_hisr(hisr_id, cuatomer_hisr_timer_proc, 0, 1000, 1000);
 *         if(vm_timer_create_precise(1000, (vm_timer_precise_callback)customer_delete_timer_proc)<0)
 *         {
 *             vm_log_debug("create precise timer fail");
 *         }
 *     }
 *     else
 *     {
 *          vm_log_debug("create hisr timer fail");
 *      }
 * }
 * </code>
 *****************************************************************************/
VM_TIMER_ID_HISR vm_timer_create_hisr(VMINT8* timer_name);

/*****************************************************************************
 * FUNCTION
 *    vm_timer_set_hisr
 * DESCRIPTION
 *    Starts a HISR timer. Calling this function will cancel the timer event set
 *    previously, and start a new HISR timer event.
 * PARAMETERS
 *    timer_id:        [IN] The HISR timer ID, returned by vm_timer_create_hisr().
 *    timer_procedure: [IN] The callback that is to be invoked when the timer fires.
 *    user_data:       [IN] The data to be consumed by the callback function.
 *    ticks:           [IN] The timer interval in ticks. The maximum value is 0xFFFFFFFF.
 *    reload_ticks:    [IN] The new timer interval to be reloaded, in ticks, after the
 *                          previous timer interval elapses. If the value of reload_ticks
 *                          is zero, the HISR timer is a one-shot timer. Otherwise,
 *                          it is a repetitive timer at the interval of reload_ticks.
 * RETURNS
 *    void
 * EXAMPLE
 * <code>
 * Refer to vm_timer_create_hisr code example.
 * </code>
 *****************************************************************************/
void vm_timer_set_hisr(VM_TIMER_ID_HISR timer_id, vm_timer_hisr_callback timer_procedure,
                       void* user_data, VMUINT32 ticks, VMUINT32 reload_ticks);

/*****************************************************************************
 * FUNCTION
 *    vm_timer_delete_hisr
 * DESCRIPTION
 *    Delete a HISR timer. Deleting a HISR timer inside the HISR callback execution
 *    may cancel a timer that is in expired state but the callback function has
 *    not yet been serviced. It cannot be used to delete other types of timers.
 * PARAMETERS
 *    timer_id: [IN] The HISR timer ID, which is the return value of vm_timer_create_hisr().
 * RETURNS
 *    void
 * EXAMPLE
 * <code>
 * Refer to vm_timer_create_hisr code example.
 * </code>
 *****************************************************************************/
void vm_timer_delete_hisr(VM_TIMER_ID_HISR timer_id);

#ifdef __cplusplus
}
#endif

#endif
