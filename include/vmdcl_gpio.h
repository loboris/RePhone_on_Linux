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

#ifndef VMDCL_GPIO_SDK_H
#define VMDCL_GPIO_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************
 * DESCRIPTION
 *   These enum defines the device for GPIO module that is passed
 *   to vm_dcl_open as a parameter. Before open the device,
 *   make sure the pin mode is configured to VM_DCL_PIN_MODE_GPIO with
 *   vm_dcl_config_pin_mode. There are two different device enums
 *   that can be opened on a GPIO pin. Use the VM_DCL_GPIO_CLOCK enum
 *   to send clock-related commands to the GPIO pin.
 *
 * EXAMPLE
 * <code>
 *   #include "vm_dcl.h"
 *   VM_DCL_HANDLE gpio_handle;
 *   gpio_handle = vm_dcl_open(VM_DCL_GPIO, 55); // Open GPIO pin number 55,
 *                                               // denoted as GPIO55 in HDK
 *                                               // documentation.
 * </code>
 *******************************************************************/
typedef enum
{
    VM_DCL_GPIO_START = VM_DCL_GPIO_GROUP_START,
    VM_DCL_GPIO,      /* Open the device for common GPIO commands. */
    VM_DCL_GPIO_CLOCK /* Open the device for clock-related commands. */
} VM_DCL_GPIO_DEVICE;


/******************************************************************
 * DESCRIPTION
 *   This enum defines the clock_mode of parameter
 *   vm_dcl_gpio_control_set_clock_out_t for command
 *   VM_DCL_GPIO_COMMAND_SET_CLOCK_OUT.
 *   Please refer to vm_dcl_gpio_control_set_clock_out_t for details.
 *******************************************************************/
typedef enum
{
    VM_DCL_GPIO_MODE_DEFAULT   = 0,    /* Set the clock to default clock.
                                         The actual clock values are different
                                         between GPIO pins.
                                         Refer to the GPIO datasheet
                                         for the default clock of each pin. */
    VM_DCL_GPIO_CLOCK_32K      = 0x82, /* Set to 32K clock out in a faster way. */
    VM_DCL_GPIO_MODE_MAX               /* This value defines the max number of
                                         mode, do not exceed this value.*/
} VM_DCL_GPIO_CLOCK_MODE;

/******************************************************************
 * DESCRIPTION
 *   This enum is for VM_DCL_GPIO_COMMAND_SET_CLOCK_DIVIDE, used in vm_dcl_control as parameter.
 *   Please refer the introduction of vm_dcl_gpio_control_set_clock_division_t for more details.
 *******************************************************************/
typedef enum
{
    VM_DCL_GPIO_DIVIDE_NUM_MAX = 16  /* This value defines the max number
                                         of division, do not exceed this
                                         value. */
} VM_DCL_GPIO_CLOCK_DIVIDE;


/******************************************************************
 * DESCRIPTION
 *   GPIO commands that can be passed to GPIO devices with vm_dcl_control.
 *   Refer to each command for the related GPIO functionality.
 *   Note that for clock-related commands VM_DCL_GPIO_COMMAND_SET_CLOCK_DIVIDE
 *   and VM_DCL_GPIO_COMMAND_SET_CLOCK_OUT, open a VM_DCL_GPIO_CLOCK device
 *   instead of VM_DCL_GPIO.
 *
 * EXAMPLE
 * <code>
 *   #include "vm_dcl.h"
 *   VM_DCL_HANDLE gpio_handle;  // device handle
 *   vm_dcl_gpio_control_level_status_t gpio_input_data;  // parameter for VM_DCL_GPIO_COMMAND_READ.
 *   gpio_handle = vm_dcl_open(VM_DCL_GPIO, 55);          // First, we call vm_dcl_open to get a handle. 55 means gpio55
 *   vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_3, NULL);                 // we call vm_dcl_control to set gpio55 as mode3.
 *   vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_READ, (void *)&gpio_input_data);   // We call vm_dcl_control to read gpio55.
 *   vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, 0);                    // we call vm_dcl_control to write gpio55.
 *   vm_dcl_close(gpio_handle);    // close the device handle.
 * </code>
 *******************************************************************/
typedef enum
{
    VM_DCL_GPIO_COMMAND_READ = 1,   /* Read level status from GPIO Pin. Uses
                                      vm_dcl_gpio_control_level_status_t
                                      as parameter. This is only valid
                                      when the direction is set to input with
                                      VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN. */
    VM_DCL_GPIO_COMMAND_WRITE_LOW,  /* Write LOW to GPIO pin. The pin must be
                                      set with
                                      VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT. */
    VM_DCL_GPIO_COMMAND_WRITE_HIGH, /* Write HIGH to GPIO pin. The pin must be
                                      set with
                                      VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT. */
    VM_DCL_GPIO_COMMAND_SET_MODE_0, /* Set pin mode to mode 0. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_1, /* Set pin mode to mode 1. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_2, /* Set pin mode to mode 2. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_3, /* Set pin mode to mode 3. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_4, /* Set pin mode to mode 4. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_5, /* Set pin mode to mode 5. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_6, /* Set pin mode to mode 6. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_7, /* Set pin mode to mode 7. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_8, /* Set pin mode to mode 8. Refer to the
                                      pin mapping table for the corresponding
                                      mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT,/* Set GPIO direction as output. */
    VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, /* Set GPIO direction as input. */
    VM_DCL_GPIO_COMMAND_RETURN_MODE,      /* Get current mode of the GPIO pin.
                                            Use
                                            vm_dcl_gpio_control_mode_t
                                            as output parameter. */
    VM_DCL_GPIO_COMMAND_RETURN_DIRECTION, /* Get current GPIO direction.
                                            Use
                                            vm_dcl_gpio_control_direction_t
                                            as output parameter. */
    VM_DCL_GPIO_COMMAND_RETURN_OUT,  /* Get current GPIO output level.
                                       Uses
                                       vm_dcl_gpio_control_level_status_t
                                       as parameter. This is only valid
                                       when the direction is set to input
                                       with
                                       VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN. */
    VM_DCL_GPIO_COMMAND_ENABLE_PULL,      /* Enable GPIO pull-up/down
                                            resistors. No parameter needed. */
    VM_DCL_GPIO_COMMAND_DISABLE_PULL,     /* Disable GPIO pull-up/pull-down
                                            resistors. No parameter needed. */
    VM_DCL_GPIO_COMMAND_SET_PULL_HIGH,    /* Set pull-up resistor. The pin
                                            must be set with
                                            VM_DCL_GPIO_COMMAND_ENABLE_PULL
                                            first. No parameter needed. */
    VM_DCL_GPIO_COMMAND_SET_PULL_LOW,     /* Set pull-down resistor. The pin
                                            must be set with
                                            VM_DCL_GPIO_COMMAND_ENABLE_PULL
                                            first. No parameter needed. */
    VM_DCL_GPIO_COMMAND_SET_INVERSION,    /* Enable or disable GPIO data
                                            inversion. Uses
                                            vm_dcl_gpio_control_set_division_t
                                            as parameter. This command is
                                            valid only when the direction
                                            is set to input with
                                            VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN.
                                            */
    VM_DCL_GPIO_COMMAND_RESERVED_0,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_SET_CLOCK_OUT,    /* Select the clock output source of GPIO. */
    VM_DCL_GPIO_COMMAND_SET_CLOCK_DIVIDE, /* Set the clock division of the clock source. */
    VM_DCL_GPIO_COMMAND_RESERVED_1,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_2,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_3,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_4,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_5,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_6,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_7,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_8,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_9,       /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_10,      /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_11,      /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_12,      /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_13,      /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_RESERVED_14,      /* Reserved, do not use. */
    VM_DCL_GPIO_COMMAND_SET_REGISTER_STATUS_R0_R1, /* set the special gpio's pull up/down register, user should refer to the GPIO datasheet. */
    VM_DCL_GPIO_COMMAND_SET_MODE_9,    /* Set pin mode to mode 9. Refer to the
                                         pin mapping table for the corresponding
                                         mode of each pin. */
    VM_DCL_GPIO_COMMAND_SET_MODE_10    /* Set pin mode to mode 10. Refer to the
                                         pin mapping table for the corresponding
                                         mode of each pin. */
} VM_DCL_GPIO_CONTROL_COMMAND;


/******************************************************************
 * DESCRIPTION
 *   Defines the HIGH and LOW state in level_status field of
 *   parameter vm_dcl_gpio_control_level_status_t.
 *******************************************************************/
typedef enum
{
    VM_DCL_GPIO_IO_LOW = 0,   /* This value is GPIO status high. */
    VM_DCL_GPIO_IO_HIGH = 1   /* This value is GPIO status low. */
} VM_DCL_GPIO_IO_LEVEL_STATUS;


/******************************************************************
 * DESCRIPTION
 *   The output parameter for VM_DCL_GPIO_COMMAND_READ and
 *   VM_DCL_GPIO_COMMAND_RETURN_OUT commands.
 *******************************************************************/
typedef struct
{
    VM_DCL_GPIO_IO_LEVEL_STATUS level_status; /* The HIGH and LOW status. */
} vm_dcl_gpio_control_level_status_t;


/******************************************************************
 * DESCRIPTION
 *   The output parameter for GPIO_COMMAND_RETURN_MODE control command.
 *   With this command, you can get the current mode of a specified GPIOs.
 *   Please refer to the sample code in the description of VM_DCL_GPIO_CONTROL_COMMAND.
 *******************************************************************/
typedef struct
{
#define VM_DCL_GPIO_MODE_0    0   /* GPIO mode 0. */
#define VM_DCL_GPIO_MODE_1    1   /* GPIO mode 1. */
#define VM_DCL_GPIO_MODE_2    2   /* GPIO mode 2. */
#define VM_DCL_GPIO_MODE_3    3   /* GPIO mode 3. */
#define VM_DCL_GPIO_MODE_4    4   /* GPIO mode 4. */
#define VM_DCL_GPIO_MODE_5    5   /* GPIO mode 5. */
#define VM_DCL_GPIO_MODE_6    6   /* GPIO mode 6. */
#define VM_DCL_GPIO_MODE_7    7   /* GPIO mode 7. */
#define VM_DCL_GPIO_MODE_8    8   /* GPIO mode 8. */
    VMUINT8   mode_status;            /* The mode number. Refer to HDK pin table
                                         for the corresponding mode of the
                                         number, which is different for each
                                         pin. */
} vm_dcl_gpio_control_mode_t;


/******************************************************************
 * DESCRIPTION
 *   Output parameter for VM_DCL_GPIO_COMMAND_RETURN_DIRECTION command.
 *******************************************************************/
typedef struct
{
#define VM_DCL_GPIO_DIRECTION_IN  0  /* GPIO mode: Input. Refer to 
                                            vm_dcl_gpio_control_direction_t. */
#define VM_DCL_GPIO_DIRECTION_OUT 1  /* GPIO mode: Output. Refer to
                                            vm_dcl_gpio_control_direction_t. */
    VMUINT8   direction_status;          /* 0: direction is input;
                                            1: direction is output. */
} vm_dcl_gpio_control_direction_t;


/******************************************************************
 * DESCRIPTION
 *   Input parameter for command VM_DCL_GPIO_COMMAND_SET_INVERSION.
 *   When inversion is set to VM_TRUE, the definition of HIGH and LOW
 *   is reversed.
 *******************************************************************/
typedef struct
{
    VMBOOL  inversion;  /* VM_TRUE: enable inversion.
                          VM_FALSE: disable inversion. */
} vm_dcl_gpio_control_set_division_t;

/******************************************************************
 * DESCRIPTION
 *   Input parameter for command VM_DCL_GPIO_COMMAND_SET_REGISTER_STATUS_R0_R1.
 *******************************************************************/
typedef struct
{
    VMBOOL    register_status; /* This function set the pull up/down resisters of
                                 some special GPIO pins. Different
                                 set_register_status, r0, r1 means different
                                 pull up/down resisters.
                                 Please get more details in datasheet. */
    VMBOOL    r0;              /* This function set the pull up/down resisters
                                 of some special GPIO pins.
                                 Different set_register_status, r0, r1 means
                                 different pull up/down resisters.
                                 Please get more details in datasheet.*/
    VMBOOL    r1;              /* This function set the pull up/down resisters
                                 of some special GPIO pins.
                                 Different set_register_status, r0, r1 means
                                 different pull up/down resisters.
                                 Please get more details in datasheet. */
} vm_dcl_gpio_control_set_register_status_r0_r1_t;


/******************************************************************
 * DESCRIPTION
 *   Input parameter for command VM_DCL_GPIO_COMMAND_SET_CLOCK_OUT.
 *   Certain GPIO pins can generate different clock waves by request.
 *   There are several clock modes supported. Refer to the chipset
 *   datasheet for the available clock modes and numbers available
 *   to each pin.
 * EXAMPLE
 *   <code>
 *   #include "vm_dcl.h"
 *   VM_DCL_HANDLE gpio_handle; // Declare a VM_DCL_HANDLE variable.
 *   vm_dcl_gpio_control_set_clock_out_t clock_data; //Declare a vm_dcl_gpio_control_set_clock_out_t variable.
 *   gpio_handle = vm_dcl_open(VM_DCL_GPIO_CLOCK,55);// Step1, we call vm_dcl_open to get a handle. 55 means gpio55. Notice the device parameter is VM_DCL_GPIO_CLOCK.
 *   clock_data.clock_number = 2;// More details, please refer to the GPIO datasheet.
 *   clock_data.clock_mode = 4; //More details, please refer to the GPIO datasheet.
 *   vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_CLOCK_OUT,(void *)&clock_data); // Step2, we call vm_dcl_control to set register status of gpio55 .
 *   vm_dcl_close(gpio_handle); // Finally, we call vm_dcl_close
 *   </code>
 *******************************************************************/
typedef struct
{
    VMUINT16  clock_number;               /* The clock register number in datasheet. */
    VM_DCL_GPIO_CLOCK_MODE  clock_mode;   /* The clock mode in datasheet, different mode means different source clock selected. */
} vm_dcl_gpio_control_set_clock_out_t;


/******************************************************************
 * DESCRIPTION
 *   Input parameter for command VM_DCL_GPIO_COMMAND_SET_CLOCK_DIVIDE.
 *   Certain GPIO pins can generate different clock waves by request.
 *   There are several clock numbers and divison values supported. Refer to the
 *   chipset datasheet for the available combinations of clock and division
 *   number.
 * EXAMPLE
 *   <code>
 *   #include "vm_dcl.h"
 *   VM_DCL_HANDLE gpio_handle; // Declare a VM_DCL_HANDLE variable.
 *   vm_dcl_gpio_control_set_clock_division_t clock_data; //Declare a vm_dcl_gpio_control_set_clock_out_t variable.
 *   gpio_handle = vm_dcl_open(VM_DCL_GPIO_CLOCK,55);// Step1, we call vm_dcl_open to get a handle. 55 means gpio55. Notice the device parameter is VM_DCL_GPIO_CLOCK.
 *   clock_data.clock_number = 3;// More details, please refer to the GPIO datasheet.
 *   clock_data.division = 2; //More details, please refer to the GPIO datasheet.
 *   vm_dcl_control(gpio_handle,GPIO_COMMAND_SET_CLOCK_DIV,(void *)&clock_data); // Step2, we call vm_dcl_control to set register status of gpio55 .
 *   vm_dcl_close(gpio_handle); // Finally, we call vm_dcl_close
 *   </code>
 *******************************************************************/
typedef struct
{
    VMUINT16  clock_number;               /* The clock register number in datasheet. */
    VM_DCL_GPIO_CLOCK_DIVIDE  division;   /* The division value in datasheet.Please get more details in datasheet. */
} vm_dcl_gpio_control_set_clock_division_t;

#ifdef __cplusplus
}
#endif

#endif /* VMDCL_GPIO_SDK_H */


