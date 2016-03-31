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

#ifndef VMDCL_PMU_SDK_H
#define VMDCL_PMU_SDK_H

#include "vmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Identifider for PMU devices for vm_dcl_open. */
typedef enum
{
    VM_DCL_PMU_START = VM_DCL_PMU_GROUP_START,  /* Reserved. */
    VM_DCL_PMU    /* Device identifier for PMU devices.
                     Pass this to vm_dcl_open to control PMU. */
} VM_DCL_PMU_DEVICE;

/* Enum for controlling certain LDO or Buck power.
   This is a parameter of the
   VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE command.
   Note that not all LDO/Buck is available on every chipset or breakout.
   For example, LinkIt ONE does not have the vibrator breakout.
   Refer to HDK pin-out diagram for details. */
typedef enum
{
    VM_DCL_PMU_VIBR = 17,   /* LDO output for vibrator. */
    VM_DCL_PMU_LDO_BUCK_MAX
} VM_DCL_PMU_LDO_BUCK;

/* Enum for controlling GPIO pin power. Note that not all GPIO pin numbers
   are available for configuration on all HDK plaform. Refer to HDK pin-out
   diagram for the available GPIO pin number. For example, use
   VM_DCL_PMU_VGPIO10 for GPIO10 on LinkIt ONE, which maps to VM_PIN_D0 pin. */
typedef enum
{
    VM_DCL_PMU_VGPIO0,   /* Pin number for GPIO0 */
    VM_DCL_PMU_VGPIO1,   /* Pin number for GPIO1 */
    VM_DCL_PMU_VGPIO2,   /* Pin number for GPIO2 */
    VM_DCL_PMU_VGPIO3,   /* Pin number for GPIO3 */
    VM_DCL_PMU_VGPIO4,   /* Pin number for GPIO4 */
    VM_DCL_PMU_VGPIO5,   /* Pin number for GPIO5 */
    VM_DCL_PMU_VGPIO6,   /* Pin number for GPIO6 */
    VM_DCL_PMU_VGPIO7,   /* Pin number for GPIO7 */
    VM_DCL_PMU_VGPIO8,   /* Pin number for GPIO8 */
    VM_DCL_PMU_VGPIO9,   /* Pin number for GPIO9 */
    VM_DCL_PMU_VGPIO10,  /* Pin number for GPIO10 */
    VM_DCL_PMU_VGPIO11,  /* Pin number for GPIO11 */
    VM_DCL_PMU_VGPIO12,  /* Pin number for GPIO12 */
    VM_DCL_PMU_VGPIO13,  /* Pin number for GPIO13 */
    VM_DCL_PMU_VGPIO14,  /* Pin number for GPIO14 */
    VM_DCL_PMU_VGPIO15,  /* Pin number for GPIO15 */
    VM_DCL_PMU_VGPIO16,  /* Pin number for GPIO16 */
    VM_DCL_PMU_VGPIO17,  /* Pin number for GPIO17 */
    VM_DCL_PMU_VGPIO18,  /* Pin number for GPIO18 */
    VM_DCL_PMU_VGPIO19,  /* Pin number for GPIO19 */
    VM_DCL_PMU_VGPIO20,  /* Pin number for GPIO20 */
    VM_DCL_PMU_VGPIO21,  /* Pin number for GPIO21 */
    VM_DCL_PMU_VGPIO22,  /* Pin number for GPIO22 */
    VM_DCL_PMU_VGPIO23,  /* Pin number for GPIO23 */
    VM_DCL_PMU_VGPIO24,  /* Pin number for GPIO24 */
    VM_DCL_PMU_VGPIO25,  /* Pin number for GPIO25 */
    VM_DCL_PMU_VGPIO26,  /* Pin number for GPIO26 */
    VM_DCL_PMU_VGPIO27,  /* Pin number for GPIO27 */
    VM_DCL_PMU_VGPIO28,  /* Pin number for GPIO28 */
    VM_DCL_PMU_VGPIO29,  /* Pin number for GPIO29 */
    VM_DCL_PMU_VGPIO30,  /* Pin number for GPIO30 */
    VM_DCL_PMU_VGPIO31,  /* Pin number for GPIO31 */
    VM_DCL_PMU_VGPIO32,  /* Pin number for GPIO32 */
    VM_DCL_PMU_VGPIO33,  /* Pin number for GPIO33 */
    VM_DCL_PMU_VGPIO34,  /* Pin number for GPIO34 */
    VM_DCL_PMU_VGPIO35,  /* Pin number for GPIO35 */
    VM_DCL_PMU_VGPIO36,  /* Pin number for GPIO36 */
    VM_DCL_PMU_VGPIO37,  /* Pin number for GPIO37 */
    VM_DCL_PMU_VGPIO38,  /* Pin number for GPIO38 */
    VM_DCL_PMU_VGPIO39,  /* Pin number for GPIO39 */
    VM_DCL_PMU_VGPIO40,  /* Pin number for GPIO40 */
    VM_DCL_PMU_VGPIO41,  /* Pin number for GPIO41 */
    VM_DCL_PMU_VGPIO42,  /* Pin number for GPIO42 */
    VM_DCL_PMU_VGPIO43,  /* Pin number for GPIO43 */
    VM_DCL_PMU_VGPIO44,  /* Pin number for GPIO44 */
    VM_DCL_PMU_VGPIO45,  /* Pin number for GPIO45 */
    VM_DCL_PMU_VGPIO46,  /* Pin number for GPIO46 */
    VM_DCL_PMU_VGPIO47,  /* Pin number for GPIO47 */
    VM_DCL_PMU_VGPIO48,  /* Pin number for GPIO48 */
    VM_DCL_PMU_VGPIO49,  /* Pin number for GPIO49 */
    VM_DCL_PMU_VGPIO50,  /* Pin number for GPIO50 */
    VM_DCL_PMU_VGPIO51,  /* Pin number for GPIO51 */
    VM_DCL_PMU_VGPIO52,  /* Pin number for GPIO52 */
    VM_DCL_PMU_VGPIO53,  /* Pin number for GPIO53 */
    VM_DCL_PMU_VGPIO54,  /* Pin number for GPIO54 */
    VM_DCL_PMU_VGPIO55,  /* Pin number for GPIO55 */
    VM_DCL_PMU_VGPIO56,  /* Pin number for GPIO56 */
    VM_DCL_PMU_VGPIO57,  /* Pin number for GPIO57 */
    VM_DCL_PMU_VGPIO58,  /* Pin number for GPIO58 */
    VM_DCL_PMU_VGPIO59,  /* Pin number for GPIO59 */
    VM_DCL_PMU_VGPIO60,  /* Pin number for GPIO60 */
    VM_DCL_PMU_VGPIO61,  /* Pin number for GPIO61 */
    VM_DCL_PMU_VGPIO62,  /* Pin number for GPIO62 */
    VM_DCL_PMU_VGPIO63,  /* Pin number for GPIO63 */
    VM_DCL_PMU_VGPIO64,  /* Pin number for GPIO64 */
    VM_DCL_PMU_VGPIO65,  /* Pin number for GPIO65 */
    VM_DCL_PMU_VGPIO66,  /* Pin number for GPIO66 */
    VM_DCL_PMU_VGPIO67,  /* Pin number for GPIO67 */
    VM_DCL_PMU_VGPIO68,  /* Pin number for GPIO68 */
    VM_DCL_PMU_VGPIO69,  /* Pin number for GPIO69 */
    VM_DCL_PMU_VGPIO70,  /* Pin number for GPIO70 */
    VM_DCL_PMU_VGPIO71,  /* Pin number for GPIO71 */
    VM_DCL_PMU_VGPIO72,  /* Pin number for GPIO72 */
    VM_DCL_PMU_VGPIO73,  /* Pin number for GPIO73 */
    VM_DCL_PMU_VGPIO74,  /* Pin number for GPIO74 */
    VM_DCL_PMU_VGPIO75,  /* Pin number for GPIO75 */
    VM_DCL_PMU_VGPIO76,  /* Pin number for GPIO76 */
    VM_DCL_PMU_VGPIO77,  /* Pin number for GPIO77 */
    VM_DCL_PMU_VGPIO78,  /* Pin number for GPIO78 */
    VM_DCL_PMU_VGPIO79,  /* Pin number for GPIO79 */
    VM_DCL_PMU_VGPIO80,  /* Pin number for GPIO80 */
    VM_DCL_PMU_VGPIO81,  /* Pin number for GPIO81 */
    VM_DCL_PMU_VGPIO82,  /* Pin number for GPIO82 */
    VM_DCL_PMU_VGPIO83,  /* Pin number for GPIO83 */
    VM_DCL_PMU_VGPIO84,  /* Pin number for GPIO84 */
    VM_DCL_PMU_VGPIO85,  /* Pin number for GPIO85 */
    VM_DCL_PMU_VGPIO86,  /* Pin number for GPIO86 */
    VM_DCL_PMU_VGPIO87,  /* Pin number for GPIO87 */
    VM_DCL_PMU_VGPIO88,  /* Pin number for GPIO88 */
    VM_DCL_PMU_VGPIO89,  /* Pin number for GPIO89 */
    VM_DCL_PMU_VGPIO90,  /* Pin number for GPIO90 */
    VM_DCL_PMU_VGPIO91,  /* Pin number for GPIO91 */
    VM_DCL_PMU_VGPIO92,  /* Pin number for GPIO92 */
    VM_DCL_PMU_VGPIO93,  /* Pin number for GPIO93 */
    VM_DCL_PMU_VGPIO94,  /* Pin number for GPIO94 */
    VM_DCL_PMU_VGPIO95,  /* Pin number for GPIO95 */
    VM_DCL_PMU_VGPIO96,  /* Pin number for GPIO96 */
    VM_DCL_PMU_VGPIO97,  /* Pin number for GPIO97 */
    VM_DCL_PMU_VGPIO98,  /* Pin number for GPIO98 */
    VM_DCL_PMU_VGPIO99,  /* Pin number for GPIO99 */
    VM_DCL_PMU_VGPIO100, /* Pin number for GPIO100 */
    VM_DCL_PMU_LDO_BUCK_GPIO_POWER_MAX
} VM_DCL_PMU_LDO_BUCK_GPIO_POWER;

/* PMU driver command list */
typedef enum
{
    /* Command to enable and disable LDO/Buck,
       use structure vm_dcl_pmu_ld0_buck_enable_t with
       enum VM_DCL_PMU_LDO_BUCK. */
    VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE = 0x2,

    /* Command to enable and disable gpio power.
       Use structure vm_dcl_pmu_control_ld0_buck_gpio_power_enable_t.*/
    VM_DCL_PMI_CONTROL_LDO_BUCK_SET_GPIO_POWER_ENABLE = 0x1006,

    /* Command to enable keypad LED(kpled).
       Use structure vm_dcl_pmu_control_kpled_enable_t. */
    VM_DCL_PMU_CONTROL_KPLED_SET_ENABLE = 3000
} VM_DCL_PMU_CONTROL_COMMAND;

/*****************************************************************************
* Description
*      This struct is for VM_PMU_CTRL_CMD_LDO_BUCK_SET_EN command
* EXAMPLE
* <code>
*    // This example explain how to power on the vibrator with the PMU API
*    // with the VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE command.
*    VM_DCL_HANDLE handle;
*    vm_dcl_pmu_ld0_buck_enable_t val;
*
*    handle = vm_dcl_open(VM_DCL_PMU, 0);
*
*    val.enable = VM_TRUE;  // enable ,  VM_FALSE : disable
*    val.module = VM_DCL_PMU_VIBR;   // LDO
*
*    vm_dcl_control(handle,
*                   VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE,
*                   (void *)&val);
*    vm_dcl_close(handle);
*
* </code>
*****************************************************************************/
typedef struct
{
    VM_DCL_PMU_LDO_BUCK module;  /* The LDO/Buck to control.
                                    Please refer to VM_DCL_PMU_LDO_BUCK. */
    VMBOOL  enable;              /* VM_TRUE: Enable, VM_FALSE: Disable. */
} vm_dcl_pmu_ld0_buck_enable_t;


/*****************************************************************************
 * Description
 *    This struct is for VM_DCL_PMI_CONTROL_LDO_BUCK_SET_GPIO_POWER_ENABLE command
 * EXAMPLE
 * <code>
 *    // This example explain how to enable gpio power of
 *    // pin GPIO40 (VM_PIN_D4 on LinkIt ONE).
 *    VM_DCL_HANDLE handle;
 *    vm_dcl_pmu_control_ld0_buck_gpio_power_enable_t val;
 *
 *    handle = vm_dcl_open(VM_DCL_PMU, 0);
 *
 *    val.enable = VM_TRUE;   // enable ,  VM_FALSE : disable
 *    val.gpio_number = 40;   // gpio number
 *
 *    vm_dcl_control(handle, VM_DCL_PMI_CONTROL_LDO_BUCK_SET_GPIO_POWER_ENABLE, (void *)&val);
 *    vm_dcl_close(handle);
 *
 * </code>
 *****************************************************************************/
typedef struct
{
    /* The GPIO pin number to enable or to disable.
       Refer to the pin-out diagram of your HDK for the exact GPIO pin number,
       e.g. set this value to VM_DCL_PMU_VGPIO10 for "GPIO10" on
       pin-out diagram.
       Developers can also include vmboard.h and use the supplied macro,
       e.g. D0. */
    VM_DCL_PMU_LDO_BUCK_GPIO_POWER gpio_number;

    /* VM_TRUE: Enable, VM_FALSE: Disable. */
    VMBOOL  enable;
} vm_dcl_pmu_control_ld0_buck_gpio_power_enable_t;


/*****************************************************************************
 * Description
 *     This struct is for VM_DCL_PMU_CONTROL_KPLED_SET_ENABLE command.
 * EXAMPLE
 * <code>
 *    // This example explain how to turn on keypad LED(KPLED) power.
 *    VM_DCL_HANDLE handle;
 *    vm_dcl_pmu_control_kpled_set_enable_t val;
 *
 *    handle = vm_dcl_open(VM_DCL_PMU, 0);
 *
 *    val.enable = VM_TRUE;    // enable ,  VM_FALSE : disable
 *
 *    vm_dcl_control(handle, VM_PMU_CTRL_KPLED_SET_ENABLE, (void *)&val);
 *    vm_dcl_close(handle);
 *
 * </code>
 *****************************************************************************/
typedef struct
{
    VMBOOL  enable;    /* VM_TRUE: enable, VM_FALSE: disable. */
} vm_dcl_pmu_control_kpled_enable_t;

#ifdef __cplusplus
}
#endif

#endif /* VMDCL_PMU_SDK_H */

