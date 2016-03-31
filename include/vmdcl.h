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

#ifndef VMDCL_SDK_H
#define VMDCL_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmboard.h"


/*****************************************************************************
 *   VM_DCL_DEVICE is the type used to represent a specific device such as UART,
 *   I2C, GPIO and so on. Each device belongs to a specific device group.
 *   The value for a device group is defined in VM_DCL_DEVICE_GROUP.
 *   The value for a specific device is defined in the header file of that device.
 *****************************************************************************/
typedef VMUINT16 VM_DCL_DEVICE;

/*****************************************************************************
 *   VM_DCL_FLAGS is the type used to represent the flags passed to the vm_dcl_open.
 *   The value of the flag depends on each specific device.
 *****************************************************************************/
typedef VMUINT32 VM_DCL_FLAGS;

/*****************************************************************************
 *   VM_DCL_OPTIONS is the type used to specify options to the read/write API.
 *   The value of the options depends on each specific device.
 *****************************************************************************/
typedef VMUINT32 VM_DCL_OPTIONS;

/*****************************************************************************
 *   VM_DCL_HANDLE is the type used to represent a specific device that is opened
 *   by vm_dcl_open. It is the return value of vm_dcl_open. The value will be
 *   passed to other API to tell the driver which device will respond to the
 *   corresponding operation.
 *****************************************************************************/
typedef VMINT32 VM_DCL_HANDLE;

/* Invalid handle, returned by vm_dcl_open when there is an error. */
#define VM_DCL_HANDLE_INVALID    (-1)

/* A buffer type. */
typedef VMUCHAR   VM_DCL_BUFFER;

/* Length of a buffer of VM_DCL_BUFFER type. */
typedef VMUINT32  VM_DCL_BUFFER_LENGTH;

/* Owner ID. */
typedef VMINT16 VM_DCL_OWNER_ID;

/*****************************************************************************
 *   This is the event type to represent the change of a specific hardware condition,
 *   such as data transfer finished. An event is used to communicate asynchronously
 *   among the device operations. The value of each event is defined in VM_DCL_EVENT_VALUE.
 *****************************************************************************/
typedef VMUINT32  VM_DCL_EVENT;

/*****************************************************************************
 * FUNCTION
 *    vm_dcl_callback
 * DESCRIPTION
 *    The definition of the callback function used in DCL module.
 * PARAMETERS
 *    user_data:     [IN] The additional data to be passed to this callback function.
 *                        The real type of this value should be vm_dcl_callback_data_t.
 *                        The detail of this structure is given by the driver.
 *    event:         [IN] Identifies in what condition that this callback will be triggered by.
 *    device_handle: [IN] Represents the device that the triggering events are from.
 *****************************************************************************/
typedef void (*vm_dcl_callback)(void* user_data, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle);

/************************************************************************
  * VM_DCL_CONTROL_COMMAND is the type used to represent the operations a
  * driver needs to perform. The value of the variable of this type
  * variable depends on specific device driver. Refer to the command enum
  * of specific driver for detail.
  *   SIO:  VM_DCL_SIO_CONTROL_COMMAND,
  *   I2C:  VM_DCL_I2C_CONTROL_COMMAND,
  *   SPI:  VM_DCL_SPI_CONTROL_COMMAND,
  *   ADC:  VM_DCL_ADC_CONTROL_COMMAND,
  *   GPIO: VM_DCL_GPIO_CONTROL_COMMAND,
  *   PMU:  VM_DCL_PMU_CONTROL_COMMAND,
  *   EINT: VM_DCL_EINT_CONTROL_COMMAND.
  ************************************************************************/
typedef VMUINT32  VM_DCL_CONTROL_COMMAND;

/*****************************************************************************
 * VM_DCL_STATUS is the type used to represent the status the driver returns.
 *****************************************************************************/
typedef enum
{
    VM_DCL_STATUS_OK                     = 0,    /* The operation succeeded. */
    VM_DCL_STATUS_FAIL                   = -1,   /* The operation failed. */
    VM_DCL_STATUS_INVALID_CMD            = -2,   /* Invalid command sent to device. */
    VM_DCL_STATUS_UNSUPPORTED            = -3,   /* The operation is not supported. */
    VM_DCL_STATUS_NOT_OPENED             = -4,   /* The device is not opened. */
    VM_DCL_STATUS_INVALID_EVENT          = -5,   /* Invalid event. */
    VM_DCL_STATUS_INVALID_DCL_HANDLE     = -6,   /* Invalid device handle. */
    VM_DCL_STATUS_INVALID_CONTROL_DATA   = -7,   /* Invalid control data. */
    VM_DCL_STATUS_INVALID_ARGUMENT       = -9,   /* Invalid argument. */
    VM_DCL_STATUS_ERROR_TIMEOUT          = -10,  /* The operation timed out. */
    VM_DCL_STATUS_ERROR_CRCERROR         = -11,  /* CRC error. */
    VM_DCL_STATUS_ERROR_READONLY         = -12,  /* The device is read only. */
    VM_DCL_STATUS_ERROR_WRONG_STATE      = -13,  /* Incorrect state. */
    VM_DCL_STATUS_INVALID_DEVICE         = -14,  /* Invalid device. */
    VM_DCL_STATUS_ALREADY_OPENED         = -15,  /* The device is already opened.*/
    VM_DCL_STATUS_INVALID_OPERATION      = -17,  /* Invalid operation. */
    VM_DCL_STATUS_DEVICE_NOT_EXIST       = -18,  /* The device does not exist. */
    VM_DCL_STATUS_DEVICE_IS_BUSY         = -20,  /* The device is busy. */
    VM_DCL_STATUS_MAX                    = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_DCL_STATUS;

/*****************************************************************************
 * The type is used for callback to transfer the data from the driver.
 * It is the parameter type of the callback. Usually, the data sent back by the
 * driver is wrapped in the local_parameters, and sometimes in peer_buffer.
 * Different device driver has different specific types for these two parameters.
 *****************************************************************************/
typedef struct
{
    void* local_parameters; /* local_para pointer. */
    void* peer_buffer;      /* peer_buff pointer. */
} vm_dcl_callback_data_t;

/*****************************************************************************
 * This is the device number group enum.
 * Each group device may have several devices.
 * The specific device number please reference to the device driver.
 *****************************************************************************/
typedef enum
{
    VM_DCL_SIO_GROUP_START  = 0,         /* SIO(UART, USB) device number starts. Refer to VM_DCL_SIO_DEVICE. */
    VM_DCL_I2C_GROUP_START  = 0x100,     /* I2C device number starts. Refer to VM_DCL_I2C_DEVICE. */
    VM_DCL_SPI_GROUP_START  = 0x200,     /* SPI device number starts. Refer to VM_DCL_SPI_DEVICE. */
    VM_DCL_ADC_GROUP_START  = 0x300,     /* ADC device number starts. Refer to VM_DCL_ADC_DEVICE. */
    VM_DCL_GPIO_GROUP_START = 0x400,     /* GPIO device number starts. Refer to VM_DCL_GPIO_DEVICE. */
    VM_DCL_PWM_GROUP_START  = 0x500,     /* PWM device number starts. Refer to VM_DCL_PWM_DEVICE. */
    VM_DCL_PMU_GROUP_START  = 0xB00,     /* PMU device number starts. Refer to VM_DCL_PMU_DEVICE. */
    VM_DCL_KBD_GROUP_START  = 0xF00,      /* KBD device number starts. Refer to VM_DCL_KBD_DEVICE */
    VM_DCL_EINT_GROUP_START = 0x1C00,    /* EINT device number starts. Refer to VM_DCL_EINT_DEVICE. */
} VM_DCL_DEVICE_GROUP;

/*****************************************************************************
 * This is the event number group enum.
 * Each device group may have several events. Some device may not generate the events.
 * Refer to the device driver for the specific event number and the meaning.
 *****************************************************************************/
typedef enum
{
    VM_DCL_EVENT_NULL            = 0,
    VM_DCL_GPIO_EVENTS_START     = 0x100,        /* For GPIO event. */
    VM_DCL_ADC_EVENTS_START      = 0x600,        /* For ADC events. Refer to VM_DCL_ADC_EVENT. */
    VM_DCL_I2C_EVENTS_START      = 0x700,        /* For I2C events. */
    VM_DCL_SERIPORT_EVENTS_START = 0x1400,       /* For SIO(UART, USB) events. Refer to VM_DCL_SIO_EVENT. */
    VM_DCL_SPI_EVENTS_START      = 0x1500,       /* For SPI events. */
    VM_DCL_EINT_EVENTS_START     = 0x1600,       /* For EINT events. Refer to VM_DCL_EINT_EVENT. */
} VM_DCL_EVENT_VALUE;

/* DOM-NOT_FOR_SDK-BEGIN */
/* Use the macro PIN2PWM, PIN2CHANNEL, PIN2EINT instead. */
VMUINT vm_dcl_pin_to_pwm(VMUINT pin);
VMUINT vm_dcl_pin_to_channel(VMUINT pin);
VMUINT vm_dcl_pin_to_eint(VMUINT pin);
/* DOM-NOT_FOR_SDK-END */

/*****************************************************************************
 * FUNCTION
 *    vm_dcl_open
 * DESCRIPTION
 *    Opens the device. Every device must be opened with this function before it
 *    can be used. The device number passed to this function is defined in the
 *    header file of the device driver. The returned handle is the identifier of the
 *    device. The device handle is used to access the device by the other functions.
 * PARAMETERS
 *    device: [IN]  The device number, a 16-bit integer given by device driver.
 *    flags:  [IN]  A 32-bit unsigned integer. Some device driver may not need this.
 * RETURNS
 *    It returns VM_DCL_HANDLE, if it succeeded. If it failed, it returns the
 *    VM_DCL_HANDLE_INVALID, which is -1.
 *****************************************************************************/
VM_DCL_HANDLE vm_dcl_open(VM_DCL_DEVICE device, VM_DCL_FLAGS flags);

/*****************************************************************************
 * FUNCTION
 *    vm_dcl_control
 * DESCRIPTION
 *    Sends command to control the device, after the device is opened successfully
 *    by vm_dcl_open(). The command specifies what to do and the argument specifies
 *    the information needed for controlling the device by the device driver.
 *    The specific command is defined in the header file of the device driver.
 *
 *    The device_handle, command and argument should correlate, so that the desired
 *    operation command is sent to the desired device. The execution of vm_dcl_control()
 *    can be either synchronously or asynchronously, depending on the command and
 *    the targeted device. Refer to the command description in the specific driver
 *    documentation.
 * PARAMETERS
 *    device_handle: [IN] The handle of the targeted device, which is returned by vm_dcl_open().
 *    command:       [IN] The command to control the device.  Refer to the document of the device driver.
 *    argument:      [IN] The argument corresponding to the command. Refer to the document of the device driver.
 * RETURNS
 *    VM_DCL_STATUS.
 *****************************************************************************/
VM_DCL_STATUS vm_dcl_control(VM_DCL_HANDLE device_handle, VM_DCL_CONTROL_COMMAND command, void* argument);

/*****************************************************************************
 * FUNCTION
 *    vm_dcl_close
 * DESCRIPTION
 *    Closes the device, as specified by the device_handle. After the device
 *    operation completes, this function should be invoked to close the device.
 *    If the device operation involves a callback, the device should not be
 *    closed before the callback finishes.
 * PARAMETERS
 *    device_handle: [IN] The handle of the targeted device, which is returned by vm_dcl_open().
 * RETURNS
 *    VM_DCL_STATUS.
 *****************************************************************************/
VM_DCL_STATUS vm_dcl_close(VM_DCL_HANDLE device_handle);

/*****************************************************************************
* FUNCTION
*    vm_dcl_read
* DESCRIPTION
*    Reads data from the device. This function may not applicable to the devices
*    that do not transfer data.
*    Some devices may also use vm_dcl_control() to transfer data, such as SPI.
* PARAMETERS
*    device_handle: [IN]  The handle of the targeted device, which is returned by vm_dcl_open().
*    buffer:        [IN]  The buffer used to store the data from this read operation.
*    buffer_length: [IN]  The size of the buffer.
*    read_length:   [OUT] The actual number of bytes read from the device after the function returns.
*    options:       [IN]  The specific options for the device, defined in the device document.
* RETURNS
*    VM_DCL_STATUS.
*****************************************************************************/
VM_DCL_STATUS vm_dcl_read(VM_DCL_HANDLE device_handle, VM_DCL_BUFFER* buffer,
                          VM_DCL_BUFFER_LENGTH buffer_length,
                          VM_DCL_BUFFER_LENGTH* read_length, VM_DCL_OPTIONS options);


/*****************************************************************************
 * FUNCTION
 *    vm_dcl_write
 * DESCRIPTION
 *    Writes data to the device. This function may not applicable to the devices
 *    that do not transfer data. Refer to the specific device driver document
 *    for details.
 *    Some devices may also use vm_dcl_control() to transfer data, such as SPI.
 * PARAMETERS
 *    device_handle:  [IN]  The handle of the targeted device, which is returned by vm_dcl_open().
 *    buffer:         [IN]  The buffer containing the data to be written to the device.
 *    buffer_length:  [IN]  The size of the buffer in bytes.
 *    written_length: [OUT] The actual number of bytes written to the device after the function returns.
 *    options:        [IN]  The specific options for the device, defined in the device document.
 * RETURNS
 *    VM_DCL_STATUS.
*****************************************************************************/
VM_DCL_STATUS vm_dcl_write(VM_DCL_HANDLE device_handle,VM_DCL_BUFFER* buffer,
                           VM_DCL_BUFFER_LENGTH buffer_length,
                           VM_DCL_BUFFER_LENGTH* written_length,VM_DCL_OPTIONS options);


/*****************************************************************************
 * FUNCTION
 *    vm_dcl_register_callback
 * DESCRIPTION
 *    Registers a callback to listen to the events from a device specified by
 *    the device_handle. The callback enables an asynchronous operation between
 *    the application and the device.
 * PARAMETERS
 *    device_handle: [IN] The handle of the targeted device, which is returned by vm_dcl_open().
 *    event:         [IN] Identifies in what condition that the callback is triggered by.
 *    callback:      [IN] The callback function to be registered.
 *    user_data:     [IN] The additional data to be passed to this callback function.
 *                        The real type of this value should be vm_dcl_callback_data_t.
 *                        The detail of this structure is given by the driver.
 * RETURNS
 *    VM_DCL_STATUS.
 *****************************************************************************/
VM_DCL_STATUS vm_dcl_register_callback(VM_DCL_HANDLE device_handle, VM_DCL_EVENT event,
                                       vm_dcl_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *    vm_dcl_add_event
 * DESCRIPTION
 *    If want to listen to multiple events, then use vm_dcl_register_callback()
 *    first to register the callback and use vm_dcl_add_event() to add additional
 *    events you want the callback to listen to.
 * PARAMETERS
 *    device_handle: [IN] The handle of the targeted device, which is returned by vm_dcl_open().
 *    event:         [IN] Identifies in what condition that the callback is triggered by.
 *    user_data:     [IN] The additional data to be passed to this callback function.
 *                        The real type of this value should be vm_dcl_callback_data_t.
 *                        The detail of this structure is given by the driver.
 * RETURNS
 *    VM_DCL_STATUS.
 *****************************************************************************/
VM_DCL_STATUS vm_dcl_add_event(VM_DCL_HANDLE device_handle, VM_DCL_EVENT event, void* user_data);


/*****************************************************************************
 * FUNCTION
 *    vm_dcl_get_owner_id
 * DESCRIPTION
 *    Gets the identifier of the user. Some device may need this.
 *    Most of the time the return value of this function needs to be passed to
 *    the driver when the device do asynchronous operations.
 * PARAMETERS
 *    void.
 * RETURNS
 *    VM_DCL_OWNER_ID.
 *****************************************************************************/
VM_DCL_OWNER_ID vm_dcl_get_owner_id(void);


/*****************************************************************************
 * FUNCTION
 *    vm_dcl_config_pin_mode
 * DESCRIPTION
 *    Sets the pin mode.
 * PARAMETERS
 *    pin:  [IN] The PIN ID, like VM_PIN_P0/VM_PIN_P1...
 *    mode: [IN] Specifies the mode that is to be set for the PIN, like
 *                 GPIO/PWM/EINT. Refer to VM_DCL_PIN_MODE for detail.
 * RETURNS
 *    VM_DCL_STATUS.
 *****************************************************************************/
VM_DCL_STATUS vm_dcl_config_pin_mode(VMUINT pin, VM_DCL_PIN_MODE mode);

/*****************************************************************************
 * FUNCTION
 *    PIN2PWM
 * DESCRIPTION
 *    Converts PIN to PWM ID.
 * PARAMETERS
 *    pin: [IN] The PIN ID, like VM_PIN_P0/VM_PIN_P1...
 * RETURNS
 *    VMINT. The PWM ID of this PIN.  If failed, it returns 0.
 *****************************************************************************/
#define PIN2PWM(pin) vm_dcl_pin_to_pwm(pin)

/*****************************************************************************
 * FUNCTION
 *    PIN2CHANNEL
 * DESCRIPTION
 *    Converts PIN to ADC channel ID.
 * PARAMETERS
 *    pin: [IN] The PIN ID, like VM_PIN_P0/VM_PIN_P1...
 * RETURNS
 *    VMINT. The ADC channel ID of this PIN.  If failed, it returns 0.
 *****************************************************************************/
#define PIN2CHANNEL(pin) vm_dcl_pin_to_channel(pin)

/*****************************************************************************
 * FUNCTION
 *    PIN2EINT
 * DESCRIPTION
 *    Converts PIN to EINT ID, e.g.
 *    <code>
 *        VM_DCL_HANDLE eint_handle = vm_dcl_open(VM_DCL_EINT,PIN2EINT(VM_PIN_P2));
 *    </code>
 * PARAMETERS
 *    pin: [IN] The PIN ID, like VM_PIN_P0/VM_PIN_P1...
 * RETURNS
 *    VMINT. The EINT ID of this PIN. If failed, it returns 0.
 *****************************************************************************/
#define PIN2EINT(pin) vm_dcl_pin_to_eint(pin)

/*****************************************************************************
 * FUNCTION
 *    vm_irq_mask
 * DESCRIPTION
 *    Used in AP mode, it masks the I bit of the processor. This function must
 *    be used in pair with the vm_irq_restore(). The I bit mask time should not exceed 5ms.
 * PARAMETERS
 *    void.
 * RETURNS
 *    The mask status of the current I bit.
 * EXAMPLE
 * <code>
 * VMUIN32 savedMask;
 * savedMask = vm_irq_mask();
 *      ... // your code need to protect
 * vm_irq_restore(savedMask);
 *
 * </code>
 *****************************************************************************/
VMUINT32 vm_irq_mask(void);


/*****************************************************************************
 * FUNCTION
 *    vm_irq_restore
 * DESCRIPTION
 *    Used in AP mode, it restores the I bit of the processor. This function must
 *    be used in pair with vm_irq_mask().
 * PARAMETERS
 *    mask: [IN] The value returned vm_irq_mask().
 * RETURNS
 *    void.
 * EXAMPLE
 * <code>
 * VMUIN32 savedMask;
 * savedMask = vm_irq_mask();
 *      ... // your code need to protect
 * vm_irq_restore(savedMask);
 *
 * </code>
 *****************************************************************************/
void vm_irq_restore(VMUINT32 mask);


#ifdef __cplusplus
}
#endif

#endif  /* VMDCL_SDK_H */