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

#ifndef	VMSYSTEM_SDK_H
#define VMSYSTEM_SDK_H

#include "vmtype.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Process handle can be used to manipulate and retrieve process information. */
typedef VMINT VM_PROCESS_HANDLE;


/* Status of an application process. */
typedef enum
{
    VM_PROCESS_STATUS_UNLOAD = 0,  /* The application has loaded to memory,
                                      but not started yet. */
    VM_PROCESS_STATUS_FOREGROUND,  /* Reserved. */
    VM_PROCESS_STATUS_BACKGROUND,  /* Reserved. */
    VM_PROCESS_STATUS_STARTING,    /* The application is running. */
    VM_PROCESS_STATUS_CLOSING,     /* The application is about to exit. */
    VM_PROCESS_STATUS_INACTIVE,    /* Reserved. */
    VM_PROCESS_STATUS_MAX = 0x7FFFFFFF
} VM_PROCESS_STATUS;

/* Process properties */
typedef struct
{
    VM_PROCESS_HANDLE   handle;         /* Process handle. */
    VMINT               reserved_1;     /* Reserved. */
    VM_PROCESS_STATUS   status;         /* Process status,
                                           refer to VM_PROCESS_STATUS.  */
    VM_PROCESS_HANDLE   reserved_2;     /* Reserved. */
    VMINT               reserved_3;     /* Reserved. */
    VMINT               reserved_4[5];  /* Reserved. */
    VMWCHAR             file_path[260]; /* Full path to the executable file,
                                           e.g. "C:\helloword.vxp" */
} vm_process_property_t;


/* System events for LinkIt applications.
   Use vm_pmng_register_system_event_callback to register. */
typedef enum
{
    /* The graphics system is ready for application to use. Application can
       use graphics APIs, e.g. vm_graphic_draw_line, after receiving this
       event. The application can ignore this event if it does not invoke
       graphics APIs. */
    VM_EVENT_PAINT = 1,

    /* This is the first message sent to the application. The application
       can perform initialization and setup resources. */
    VM_EVENT_CREATE = 4,

    /* This is the last message sent to the application, denoting the
       application is about to terminate. */
    VM_EVENT_QUIT = 5,

    /* This events notifies application that memory card has been plug out.
       This is only available on hardwares with SD card slots. */
    VM_EVENT_CARD_PLUG_OUT = 7,

    /* This event notifices that GSM cell information has updated. Refer to
       vm_gsm_cell_open for details. */
    VM_EVENT_CELL_INFO_CHANGE = 15,

    /* This event alerts application that the battery on the devices
       is running low. The application should preserve critical data
       accordingly. */
    VM_EVENT_LOW_BATTERY = 16,

    /* This event is sent when vm_pmng_exit_and_update_app fails. The framework
       re-launches the application, and send this event to system event handler.
       The error code VM_PMNG_UPDATE_RESULT will be sent as event parameter in
       handler callback. */
    VM_EVENT_UPDATE_FAIL = 20,

    VM_SYSTEM_EVENT_MAX = 0x7FFFFFFF

} VM_SYSTEM_EVENT;

/* Encryption method used by the package when updating VXP executables with
   vm_pmng_exit_and_update_application. */
typedef enum
{
    VM_PMNG_ENCRYPTION_NONE,    /* no encryption */
    VM_PMNG_ENCRYPTION_DES,     /* DES encryption */

    VM_PMNG_ENCRYPTION_MAX = 0x7F
} VM_PMNG_ENCRYPTION;

/* Error code for vm_pmng_exit_and_update_application. Some of the erro code
   are returned immediatly, some are passed to application after re-launch
   as the event parameter of system event VM_EVENT_UPDATE_FAIL. */
typedef enum
{

    /* Insufficient space on the SD card drive. */
    VM_PMNG_UPDATE_ERROR_NOT_ENOUGH_MOVEABLE_DRIVE_SPACE = -1,

    /* Insufficient space on the internal system drive. */
    VM_PMNG_UPDATE_ERROR_NOT_ENOUGH_SYSTEM_DRIVE_SPACE = -2,

    /* The encryption method is missing or invalid. */
    VM_PMNG_UPDATE_ERROR_MISSING_ENCRYTION_METHOD = -3,

    /* The encrytion key is missing. */
    VM_PMNG_UPDATE_ERROR_MISSING_ENCRYTION_KEY = -4,

    /* The length of the encryption key is missing or invalid. */
    VM_PMNG_UPDATE_ERROR_MISSING_ENCRYTION_KEY_LENGTH = -5,

    /* The execution context of vm_pmng_exit_and_update_application is
       incorrect. This function must be called in the main thread of
       a LinkIt application of VXP executable format. */
    VM_PMNG_UPDATE_ERROR_NOT_CALL_IN_APPLICATION = -6,

    /* Not enough system memory. */
    VM_PMNG_UPDATE_ERROR_NOT_ENOUGH_KERNEL_MEMORY = -7,

    /* The path to update package is invalid. */
    VM_PMNG_UPDATE_ERROR_NOT_FOUND_APPLICATION = -8,

    /* File system error when creating folders and files. */
    VM_PMNG_UPDATE_ERROR_INTERNEL_ERROR = -9,

    VM_PMNG_UDPATE_RESULT_MAX = 0x7FFFFFFF
} VM_PMNG_UPDATE_RESULT;


/*****************************************************************************
 * DESCRIPTION
 *  Prototype of the system event handler.
 * PARAMETERS
 *  system_event: [IN] The system event, see VM_SYSTEM_EVENT for details.
 *  parameter: [IN] The actual value depends on the event, see VM_SYSTEM_EVENT
 *                  for details. If the event does not have a parameter, the
 *                  value is default to 0.
 *****************************************************************************/
typedef void (*vm_pmng_system_event_callback)(VMINT system_event,
        VMINT parameter);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_register_system_event_callback
 * DESCRIPTION
 *  Registers an application-defined system event callback function that
 *  processes vital system events like VM_EVENT_CREATE and VM_EVENT_PAINT.
 *  This function must be called in vm_main() of the LinkIt application.
 * PARAMETERS
 *  callback: [IN] The event handler callback.
 *****************************************************************************/
void vm_pmng_register_system_event_callback(vm_pmng_system_event_callback callback);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_restart_application
 * DESCRIPTION
 *  Quit the current running application and launch it again.
 * PARAMETERS
 *  N/A
 *****************************************************************************/
void vm_pmng_restart_application(void);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_exit_and_update_application
 * DESCRIPTION
 *  For application update.
 *  A LinkIt application may call this function to quit and update itself by
 *  assiging path to the new VXP executable. The framework would verify the
 *  integrity of the new VXP executable, and update the executable.
 *
 *  While the SDK does not provide facility to encrypt the VXP binary,
 *  3rd party software may be used to encrypt the binary with
 *  supported encryption method. This method is defined by the VM_PMNG_ENCRYPTION.
 *  In this version of SDK only DES encryption is supported.
 *
 *  If update succeeded, the new VXP executable replaces the original executable
 *  and launches.
 *
 *  If update failed, the original VXP executable is launched again,
 *  and a system message VM_MSG_UPDATE_FAIL is sent with the parameter
 *  VM_PMNG_UPDATE_RESULT.
 *
 * PARAMETERS
 *  filename:   [IN] The full path to the new application executable.
 *  key:        [IN] The key to decrypt the executable.
 *                   Pass NULL if the new app have no encryption.
 *  key_length: [IN] Length of key, Pass 0 if there is no ecryption.
 *  encryption: [IN] encryption method, see VM_PMNG_ENCRYPTION.
 *                   Note that key must be NULL and key_length must be 0 if
 *                   encryption is set to VM_PMNG_ENCRYPTION_NONE, otherwise
 *                   VM_PMNG_UPDATE_ERROR_MISSING_ENCRYTION_METHOD may return.
 *                   Currently, only DES encryption is supported, and developers
 *                   should avoid padding during DES encryption. The file size
 *                   of the encrypted executable must be the same as the
 *                   un-encrypted executable, otherwise the application may
 *                   fail to update.
 * RETURNS
 * VM_RESULT. See VM_PMNG_UPDATE_RESULT for details.
 *****************************************************************************/
VM_RESULT vm_pmng_exit_and_update_application(VMCWSTR filename,
        const VMCHAR* key,
        VMINT key_length,
        VM_PMNG_ENCRYPTION encryption);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_running_application_path
 * DESCRIPTION
 *  Get full path to the VXP executable file of current application.
 * PARAMETERS
 *  path: [OUT] Output buffer for full path string with UCS2 encoding.
 *              The length of the buffer must be greater than
 *              (VM_FS_MAX_PATH_LENGTH+1), otherwise it may result in
 *              buffer overrun and corrupted system.
 * RETURNS
 *  0 if succeeded, else means there is no application running.
 *****************************************************************************/
VM_RESULT vm_pmng_get_running_application_path(VMWSTR path);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_current_process_handle
 * DESCRIPTION
 *  Get the process handle of the current process.
 * RETURNS
 *  A non-zero process handle if succeeded.
 *  0 is returned when the current process is not running or invalid.
 *****************************************************************************/
VM_PROCESS_HANDLE vm_pmng_get_current_process_handle(void);


/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_process_handle
 * DESCRIPTION
 *  Get the process handle of the specified executable.
 * PARAMETERS
 *  path: [IN] Full path to the executable to be checked.
 * RETURNS
 *  A non-zero process handle if succeeded.
 *  0 is returned if the path is invalid or the executable is not running.
 *****************************************************************************/
VM_PROCESS_HANDLE vm_pmng_get_process_handle(VMCWSTR path);


/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_current_application_handle
 * DESCRIPTION
 *  Get process handle of current application. For LinkIt SDK 2.0,
 *  this function is the same as vm_pmng_get_current_process_handle.
 * RETURNS
 *  A non-zero process handle of current running application if succedded.
 *  0 means no the current application is not running.
 *****************************************************************************/
VM_PROCESS_HANDLE vm_pmng_get_current_application_handle(void);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_process_list
 * DESCRIPTION
 *  Get a list all process.
 * PARAMETERS
 *  handle_list: [IN/OUT] Process handle list. Pass NULL to get the number of
 *                       process.
 *  number:     [IN/OUT] Size of the handle list. After the function returns,
 *                       this paramter is updated to reflect the number of
 *                       process.
 * RETURNS
 *  >=0 means number of process, negative numbers when failed.
 *****************************************************************************/
VMINT vm_pmng_get_process_list(VM_PROCESS_HANDLE* handle_list, VMUINT* number);

/*****************************************************************************
 * FUNCTION
 *    vm_pmng_get_process_property
 * DESCRIPTION
 *  Get property of a process, e.g. status and executable file path.
 * PARAMETERS
 *  handle:     [IN] process handle
 *  property:   [OUT] process property
 * RETURNS
 *  VM_OK if success, <0 otherwise.
 *****************************************************************************/
VM_RESULT vm_pmng_get_process_property(VM_PROCESS_HANDLE handle,
                                       vm_process_property_t* property);

#ifdef __cplusplus
}
#endif
#endif /* VMSYSTEM_SDK_H */
