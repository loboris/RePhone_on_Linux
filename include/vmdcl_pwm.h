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

#ifndef VMDCL_PWM_SDK_H
#define VMDCL_PWM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * This enum type represents the device of PWM (Pulse Width Modulation) module.
 * The vm_dcl_open() uses the enum values to create a handle to access the device.
 * Applications should use the DCL (Driver Common Layer) APIs to control the PWM.
 * EXAMPLE
 * <code>
 *   #include "vmdcl.h"
 *   VM_DCL_HANDLE pwm_handle;                      // Declares a VM_DCL_HANDLE.
 *   pwm_handle = vm_dcl_open(VM_DCL_PWM_KPLED, 0); // Calls vm_dcl_open() to get a handle.
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_PWM_START = VM_DCL_PWM_GROUP_START, /* PWM devices start from here. */
    VM_DCL_PWM_KPLED,                          /* The keypad LED PWM. */

    VM_DCL_HW_PWM_GROUP_START = (VM_DCL_PWM_GROUP_START+0x20), /* PWM HW Group start from here. */
    VM_DCL_PWM_1,                       /* The PWM1. When configuring the GPIO as PWM, this would be PWM0. */
    VM_DCL_PWM_2_RESERVED,              /* Reserved and do not use. */
    VM_DCL_PWM_3_RESERVED,              /* Reserved and do not use. */
    VM_DCL_PWM_4,                       /* The PWM4. When configuring the GPIO as PWM, this would be PWM1. */
} VM_DCL_PWM_DEVICE;

/******************************************************************************
 * This enum defines the control commands for PWM module. The vm_dcl_control()
 * uses these commands to control different functions of the PWM device.
 * The applications should use the DCL (Driver Common Layer) APIs to control PWM.
 * EXAMPLE
 * <code>
 *   #include "vmdcl.h"
 *   VM_DCL_HANDLE pwm_handle;                      // Declares a VM_DCL_HANDLE variable.
 *   VM_PWM_LEVEL_T pwm_level_var;
 *   pwm_handle = vm_dcl_open(VM_DCL_PWM_KPLED, 0); // Calls vm_dcl_open() to get a handle. The second parameter is always 0.
 *   pwm_level_var.pwm_level = 2;
 *   vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_LEVEL, (void *)(&pwm_level_var)); //  Calls vm_dcl_control() to set the KPLED full brightness.
 *   vm_dcl_close(pwm_handle); // Calls vm_dcl_close() to close the device.
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_PWM_CMD_START                     = 0xD06,   /* Starts HW PWM control. */
    VM_PWM_CMD_STOP                      = 0xD07,   /* Stops HW PWM control. */
    VM_PWM_CMD_SET_LEVEL                 = 0xD0C,   /* Controls VM_DCL_PWM_KPLED. The keypad LED
                                                     * only supports 3 levels: 0,1,2.  0 means
                                                     * stop PWM; 1 means 50% of brightness,
                                                     * 2 means 100% of brightness. */
    VM_PWM_CMD_CONFIG                    = 0xD14,   /* Configures HW PWM. */
    VM_PWM_CMD_SET_CLOCK                 = 0xD1B,   /* Sets PWM clock. */
    VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD = 0xD1C,   /* Sets PWM counter and threshold. */
} VM_DCL_PWM_CONTROL_COMMAND;


/******************************************************************************
 * This is used in vm_dcl_control() for VM_PWM_CMD_SET_LEVEL control command.
 * Using this command, an application can set the brightness of the keypad LED PWM.
 * The sample code is in the description of VM_DCL_PWM_CONTROL_COMMAND.
 *****************************************************************************/
typedef struct
{
    VMUINT8  pwm_level;  /* The keypad LED supports 3 levels (0, 1, 2). 0 means
                         * to stop PWM. 1 means 50% of brightness. 2 means 100% of brightness. */
} vm_dcl_pwm_level_t;

/* Used for VM_PWM_CMD_CONFIG command. */
typedef struct
{
    VMUINT32 frequency; /* The output frequency of the PWM channel. */
    VMUINT8  duty;      /* The duty cycle of the high state of the periodical output. */
} vm_dcl_pwm_config_t;

/* Used for VM_PWM_CMD_SET_CLOCK command. */
typedef struct
{
    VMUINT8 source_clock;           /* The source clock frequency. 0 means 13MHz. 1 means 32KHz. */
    VMUINT8 source_clock_division;  /* The source clock division. 0 means no division.
                                     * 1 means division by 2. 2 means division by 4.
                                     * 3 means division by 8. */
} vm_dcl_pwm_set_clock_t;

/* Used for VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD command. */
typedef struct
{
    VMUINT16 counter;   /* The cycle, the value of which is between 0 and 8191. */
    VMUINT16 threshold; /* The duty cycle, the value of which is between 0 and 8191,
                         * but must be smaller than the value of the cycle.
                         * The threshold/counter is the duty. */
} vm_dcl_pwm_set_counter_threshold_t;


#ifdef __cplusplus
}
#endif

#endif /* VMDCL_PWM_SDK_H */


