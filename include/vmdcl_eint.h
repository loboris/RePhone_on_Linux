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

#ifndef VMDCL_EINT_SDK_H
#define VMDCL_EINT_SDK_H

#include "vmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Defines the device for EINT module, used by vm_dcl_open(). To control EINTs,
 * an application should use DCL(Driver Common Layer) APIs.
 * EXAMPLE
 * <code>
 *   #include "vmdcl.h"
 *   VM_DCL_HANDLE eint_handle;                  // Declares a VM_DCL_HANDLE variable.
 *   eint_handle = vm_dcl_open(VM_DCL_EINT, PIN2EINT(VM_PIN_P0)); // Calls vm_dcl_open() to get a handle. Use PIN2EINT to convert from pin name to EINT number.
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_EINT_DEVICE_GROUP_START = VM_DCL_EINT_GROUP_START,   /* EINT device group starts. */
    VM_DCL_EINT,                                                /* DCL EINT device. */
} VM_DCL_EINT_DEVICE;

/******************************************************************************
 * Defines the events for EINT module. When an application receives this event,
 * it means the EINT has triggered an interrupt.
 *****************************************************************************/
typedef enum
{
    VM_DCL_EINT_EVENT_TRIGGER = VM_DCL_EINT_EVENTS_START,   /* EINT trigger event. */
} VM_DCL_EINT_EVENT;


/******************************************************************************
 * Defines the EINT control commands, used by vm_dcl_control() as a parameter.
 *   With different commands, user could control the different function of the EINTs.
 *   To control EINTs, you should use DCL(Driver Common Layer) APIs.
 * EXAMPLE
 * <code>
 *   #include "vmdcl.h"
 *   VM_DCL_HANDLE eint_handle;                    // Declares a VM_DCL_HANDLE variable.
 *   vm_eint_control_config_t eint_config;         // Declares a vm_eint_control_config_t variable.
 *   vm_eint_control_sensitivity_t sens_data;      // Declares a vm_eint_control_sensitivity_t variable.
 *   vm_eint_control_hw_debounce_t debounce_time;  // Declares a vm_eint_control_hw_debounce_t variable.
 *   eint_handle = vm_dcl_open(VM_DCL_EINT, PIN2EINT(VM_PIN_P0));   // Calls vm_dcl_open() to get a handle. Use PIN2EINT to convert from pin name to EINT number.
 *   vm_dcl_control(eint_handle, VM_EINT_COMMAND_MASK, NULL);    // Before configuring EINT, we mask it firstly.
 *   vm_dcl_registercallback(eint_handle, EVENT_EINT_TRIGGER, eint_callback, (void*)NULL ); // Registers the callback. Note: the last parameter is NULL.
 *   sens_data.sensitivity = 1;         // 1 means the Level Sensitive. 0 means the Edge Sensitive.
 *   vm_dcl_control(eint_handle, VM_EINT_COMMAND_SET_SENSITIVITY, (void *)&sens_data);  // Sets EINT sensitivity.
 *   debounce_time.debounce_time = 10;  // Debounce time 10ms
 *   vm_dcl_control(eint_handle,VM_EINT_COMMAND_SET_HW_DEBOUNCE,(void *)&debounce_time); // Set debounce time
 *   eint_config.act_polarity = 0;  // 1 means positive activated. 0 means negative activated.
 *   eint_config.debounce_en  = 1;  // 1 means to enable the HW debounce. 0 means to disable the HW debounce.
 *   eint_config.auto_unmask  = 1;  // 1 means to unmask after callback. 0 means not to unmask and the callers should unmask themselves.
 *   vm_dcl_control(eint_handle, VM_EINT_COMMAND_CONFIG, (void*)&eint_config);  // Calls this API prior to unmasking the EINT next.
 *   vm_dcl_control(eint_handle, VM_EINT_COMMAND_UNMASK, NULL);  // Calls this function to unmask the EINT.
 *   vm_dcl_close(eint_handle); // Finally, calls vm_dcl_close().
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_EINT_COMMAND_GROUP_START = 0x1E00,       /* EINT command group starts. */
    VM_DCL_EINT_COMMAND_CONFIG,                     /* EINT configure command. */
    VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE,            /* EINT set hardware debounce command. */
    VM_DCL_EINT_COMMAND_SET_POLARITY,               /* EINT set polarity command. */
    VM_DCL_EINT_COMMAND_SET_SENSITIVITY,            /* EINT set sensitivity command. */
    VM_DCL_EINT_COMMAND_MASK,                       /* EINT mask command. */
    VM_DCL_EINT_COMMAND_UNMASK,                     /* EINT unmask command. */
    VM_DCL_EINT_COMMAND_RESERVED,                   /* Reserved. Do not use it. */
    VM_DCL_EINT_COMMAND_SET_AUTO_CHANGE_POLARITY,   /* EINT set auto change polarity command. */
} VM_DCL_EINT_COMMAND;


/******************************************************************************
 * This struct is for VM_DCL_EINT_COMMAND_CONFIG command and used by vm_dcl_control().
 * With this command, a caller can configure the attributes of a specific EINT.
 * The sample code is in the description of VM_DCL_EINT_COMMAND. The usage of all
 * the commands are similar.
 *****************************************************************************/
typedef struct
{
    VMUINT8 debounce_enable;    /* The value is 1 or 0. 1 means to enable the HW debounce; 0 means to disable. */
    VMUINT8 act_polarity;       /* The value is 1 0r 0. 1 means the positive active; 0 means the negative active. */
    VMUINT8 auto_unmask;        /* The value is 1 or 0. 1 means unmask after callback;
                                 * 0 means not to unmask and the users must unmask themselves. */
} vm_dcl_eint_control_config_t;


/******************************************************************************
 * This struct is for VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE command, used by vm_dcl_control().
 * With this command, the user can set the HW debounce time of a specific EINT.
 * The sample code is in the description of VM_DCL_EINT_COMMAND. The usage of all
 * the commands are similar.
 *****************************************************************************/
typedef struct
{
    VMUINT32   debounce_time;  /* The HW debounce time in milliseconds. */
} vm_dcl_eint_control_hw_debounce_t;


/******************************************************************************
 * This struct is for VM_DCL_EINT_COMMAND_SET_POLARITY command, used by vm_dcl_control().
 * With this command, you can set the polarity of a specific EINT.
 * The sample code is in the description of VM_DCL_EINT_COMMAND. The usage of all
 * the commands are similar.
 *****************************************************************************/
typedef struct
{
    VMUINT8 polarity;  /* The value is 1 0r 0. 1 means the positive active;
                         * 0 means the negative active. */
} vm_dcl_eint_control_polarity_t;

/******************************************************************************
 * This struct is for VM_DCL_EINT_COMMAND_SET_SENSITIVITY command, used in vm_dcl_control().
 * With this command, the user can set the sensitivity of a specific EINT.
 * The sample code is in the description of VM_DCL_EINT_COMMAND. The usage of all
 * the commands are similar.
 *****************************************************************************/
typedef struct
{
    VMUINT8 sensitivity;   /* The value is 1 or 0. 1 means the Level  Sensitive;
                             * 0 means the Edge Sensitive. */
} vm_dcl_eint_control_sensitivity_t;

/******************************************************************************
 * This struct is for VM_DCL_EINT_COMMAND_SET_AUTO_CHANGE_POLARITY command,
 * used in vm_dcl_control as parameter.
 * The sample code is in the description of VM_DCL_EINT_COMMAND. The usage of all
 * the commands are similar.
 *****************************************************************************/
typedef struct
{
    VMUINT8 auto_change_polarity;  /* The value is 1 or 0. 1 means enable auto
                                     * change polarity, 0 means disable. note
                                     * that the default value is 0 */
} vm_dcl_eint_control_auto_change_polarity_t;

#ifdef __cplusplus
}
#endif

#endif /* VMDCL_EINT_SDK_H */
