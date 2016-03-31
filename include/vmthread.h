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

#ifndef VMTHREAD_SDK_H
#define VMTHREAD_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

typedef VMUINT32 VM_THREAD_HANDLE;      /* Thread handle, returned by vm_thread_create(). */
typedef VMUINT32 VM_SIGNAL_ID;          /* Signal id, returned by vm_signal_init(). */
typedef VMUINT8  VM_THREAD_PRIORITY;    /* Thread priority. */

/* Signal result. */
typedef enum
{
    VM_SIGNAL_SUCCESS = 0,  /* Denotes the calling thread is waken up by the signal post. */
    VM_SIGNAL_TIMEOUT = 1,  /* Denotes the calling thread is waken up by the time out. */
    VM_SIGNAL_CANCEL  = 2,  /* Denotes the calling thread is waken up forcibly. */
    VM_SIGNAL_RESULT_MAX = 0x7FFFFFFF   /* Forces compiler to use 4-byte integer for this enum. */
} VM_SIGNAL_RESULT;

/* Thread message. */
typedef struct
{
    VMUINT16 message_id;    /* Message ID. */
    void* user_data;        /* The user data of the message. */
} vm_thread_message_t;

/* A mutex node. Used by system. */
typedef struct _vm_mutex_list_note
{
    VMUINT8 unused; /* Used by system. */
} vm_mutex_list_node_t;

/* mutex */
typedef struct
{
    VMUINT32 guard;                     /* Used by system. Initialized when invoking vm_mutex_int(). */
    vm_mutex_list_node_t* mutex_info;   /* Used by system. Initialized when invoking vm_mutex_int(). */
} vm_mutex_t;


/*****************************************************************************
 * FUNCTION
 *    vm_thread_callback
 * DESCRIPTION
 *    Thread main function type, the thread will be closed when it returned.
 * PARAMETERS
 *    handle:    [IN] The thread handle, used for identifying a thread.
 *    user_data: [IN] The user data to be consumed by the callback function.
 * RETURNS
 *    An integer. The value is reserved.
 *****************************************************************************/
typedef VMINT32 (*vm_thread_callback)(VM_THREAD_HANDLE handle, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_thread_create
 * DESCRIPTION
 *    Creates a thread. Limited by the system, vm_thread_create() can only create
 *    up to 10 threads. The caller of this function needs to pass the expected priority
 *    to the create. The value 1 indicates the highest priority, while the 255 is
 *    the lowest. The actual thread priority will be assigned by the system according
 *    to the following rule.
 *
 *    1. If the expected priority is equal or less than 128, the actual priority
 *    assigned by the system will be lower than the main thread.
 *    2. If the expected priority is equal or greater than 129, the actual thread
 *    priority assigned by the system will be higher than the main thread.
 *    3. If the expected priority is 0, the system will assign the default priority
 *    to the thread, which is lower than priority of the main thread.
 * PARAMETERS
 *   callback:  [IN] The callback function to be executed by the newly created thread.
 *   user_data: [IN] The data to be consumed by the callback function, such as parameters.
 *   priority:  [IN] The expected thread priority for the newly created thread.
 * RETURNS
 *   The handle of the newly created thread.
 *****************************************************************************/
VM_THREAD_HANDLE vm_thread_create(vm_thread_callback callback, void* user_data,
                                  VM_THREAD_PRIORITY priority);

/*****************************************************************************
 * FUNCTION
 *    vm_thread_get_current_handle
 * DESCRIPTION
 *    Gets the handle of the current thread. Note, this function is not supported
 *    in the main thread.
 * PARAMETERS
 *    void
 * RETURNS
 *    The handle of current thread.
 *****************************************************************************/
VM_THREAD_HANDLE vm_thread_get_current_handle(void);

/*****************************************************************************
 * FUNCTION
 *    vm_thread_get_main_handle
 * DESCRIPTION
 *    Get the handle of the main.
 * PARAMETERS
 *    void
 * RETURNS
 *    The handle of the main thread.
 *****************************************************************************/
VM_THREAD_HANDLE vm_thread_get_main_handle(void);

/*****************************************************************************
 * FUNCTION
 *    vm_thread_send_msg
 * DESCRIPTION
 *    Sends a message to a thread.
 * PARAMETERS
 *    receiver_handle: [IN] The thread handle of receiving thread.
 *    message:         [IN] The message to be sent. It contains the message ID and the user_data.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_thread_send_message(VM_THREAD_HANDLE receiver_handle, vm_thread_message_t* message);

/*****************************************************************************
 * FUNCTION
 *    vm_thread_get_message
 * DESCRIPTION
 *    Reads the message from a message queue. This function will be blocked if the
 *    message queue is empty. It can only be used in a thread spawned from vm_thread_create().
 * PARAMETERS
 *    message: [OUT] The message content contains the message ID and the user_data.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_thread_get_message(vm_thread_message_t* message);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_create
 * DESCRIPTION
 *    Creates a signal for thread communication.
 * PARAMETERS
 *    void
 * RETURNS
 *    The signal ID of the newly created signal.
 *****************************************************************************/
VM_SIGNAL_ID vm_signal_create(void);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_clean
 * DESCRIPTION
 *    The ID of the signal which was clean.
 * PARAMETERS
 *    signal_id: [IN] The ID of he signal to be cleared.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_signal_clear(VM_SIGNAL_ID signal_id);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_post
 * DESCRIPTION
 *    Posts a signal then all the threads, which are waiting for it, will be waken up.
 * PARAMETERS
 *  signal_id : [IN] The id of the signal which was post.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_signal_post(VM_SIGNAL_ID signal_id);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_wait
 * DESCRIPTION
 *    Waits on a signal. If the signal is not set, the thread will be blocked.
 *    When the other thread posts the signal, the waiting threads will be waken up.
 *    This API should only be used in the threads spawned from vm_thread_create().
 * PARAMETERS
 *    signal_id: [IN] The ID of the signal the calling thread will be waiting on.
 * RETURNS
 *    VM_SIGNAL_RESULT
 *****************************************************************************/
VM_SIGNAL_RESULT vm_signal_wait(VM_SIGNAL_ID signal_id);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_timed_wait
 * DESCRIPTION
 *    Waits on a signal. If the signal is not set, the thread will be blocked.
 *    When the other thread posts the signal, the waiting threads will be waken up.
 *    This API should be used in the threads spawned from vm_thread_create().
 *    If the time out has been reached and the signal is posted yet, the waiting
 *    thread will resume the execution.
 * PARAMETERS
 *    signal_id: [IN] The ID of the signal the calling thread will be waiting on.
 *    time_out:  [IN] The maximum wait time in microsecond.
 * RETURNS
 *    VM_SIGNAL_RESULT
 *****************************************************************************/
VM_SIGNAL_RESULT vm_signal_timed_wait(VM_SIGNAL_ID signal_id, VMUINT32 time_out);

/*****************************************************************************
 * FUNCTION
 *    vm_signal_destroy
 * DESCRIPTION
 *    Destroys a signal when it is no longer needed, so the resource is released.
 * PARAMETERS
 *    signal_id: [IN] The ID of the signal to be destroyed.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_signal_destroy(VM_SIGNAL_ID signal_id);

/*****************************************************************************
 * FUNCTION
 *	  vm_mutex_init
 * DESCRIPTION
 *	  Initializes a mutex.
 * PARAMETERS
 *	  mutex: [IN|OUT] The mutex that needs to be initialized.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_mutex_init(vm_mutex_t* mutex);

/*****************************************************************************
 * FUNCTION
 *	  vm_mutex_lock
 * DESCRIPTION
 *	  Locks a mutex, to block the access to the data (objects or variables)
 *    protected by the mutex.
 * PARAMETERS
 *	  mutex: [IN] The mutex to be locked.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_mutex_lock(const vm_mutex_t* mutex);

/*****************************************************************************
 * FUNCTION
 *	  vm_mutex_unlock
 * DESCRIPTION
 *	  Releases the mutex. The thread that is blocked by the mutex will have the
 *    access to the protected objects by the mutex when the mutex is unlocked.
 * PARAMETERS
 *	  mutex: [IN] The mutex to be unlocked.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_mutex_unlock(const vm_mutex_t* mutex);

/*****************************************************************************
 * FUNCTION
 *	  vm_thread_sleep
 * DESCRIPTION
 *	  Makes a thread sleep for a number of milliseconds. It should only be used
 *    in the threads spawned from vm_thread_create(). Note, putting a thread into
 *    the sleep for too long could potentially cause unexpected behavior. If the
 *    thread, which is responsible for reading from a message queue, was put into
 *    sleep for too long, for example, the message queue might overflow because
 *    of too many unprocessed messages.
 * PARAMETERS
 *	  time_out: [IN] The duration of the sleep, in milliseconds.
 * RETURNS
 *    void
 * EXAMPLE
 * <code>
 *
 * VMINT vm_sub_thread_proc(VM_THREAD_HANDLE thread_handle, void* user_data)
 * {
 *      // Do something here.
 *      vm_thread_sleep(3000); // Sleeps for 3,000 milliseconds.
 *  }
 *
 * </code>
 *****************************************************************************/
void vm_thread_sleep(VMUINT32 time_out);

#ifdef __cplusplus
}
#endif

#endif /* VMXML_SDK_H */

