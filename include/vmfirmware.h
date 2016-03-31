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

#ifndef VMFIRMWARE_SDK_H
#define VMFIRMWARE_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Information that can be queried by vm_firmware_get_info. */
typedef enum
{

    VM_FIRMWARE_RESERVED_1 = 1,     /* Reserved, do not use. */
    VM_FIRMWARE_RESERVED_2 = 2,     /* Reserved, do not use. */
    VM_FIRMWARE_RESERVED_3 = 3,     /* Reserved, do not use. */
    VM_FIRMWARE_HOST_VERSION = 4,   /* Firmware version, a string with max
                                       length of 30 bytes. */
    VM_FIRMWARE_HOST_MAX_MEM = 5,   /* Maximum memory in KB that a LinkIt
                                       application can get. A string with
                                       max length of 6 bytes. Use atoi function
                                       to convert to integer value. */
    VM_FIRMWARE_RESERVED_4 = 6,     /* Reserved, do not use. */
    VM_FIRMWARE_BUILD_DATE_TIME = 7,/* Fimware build date and time, a string
                                       with max length of 30 bytes, e.g.
                                       "2012/02/09 11:47" */
    VM_FIRMWARE_RELEASE_BRANCH = 8, /* Firmware branch info.
                                       e.g. "11B NEUTRON52_12864" */
    VM_FIRMWARE_KEY_INFO_MAX = 0x7F
} VM_FIRMWARE_KEY_INFO;

/* Use this enum to query if a functional module is supported by firmware. */
typedef enum
{
    VM_FIRMWARE_WIFI,       /* Wi-Fi support. */
    VM_FIRMWARE_GPS,        /* GNSS support. */
    VM_FIRMWARE_BLUETOOTH,  /* Bluetooth support, both 2.1 and 4.0. */
    VM_FIRMWARE_GSM,        /* GSM support, including SIM, SMS, telephony,
                               and cell information. */
    VM_FIRMWARE_GPRS,       /* GPRS support. */
    VM_FIRMWARE_LCM_ENGINE, /* MTK Serial Interface display support. */

    VM_FIRMWARE_MODULE_NAME_MAX = 0x7F
} VM_FIRMWARE_MODULE_NAME;

/* The result code store in the error_code field of
   vm_firmware_update_status_t */
typedef enum
{
    VM_FIRMWARE_UPDATE_OK = 0,

    /* The update file is too big for the device. */
    VM_FIRMWARE_UPDATE_ERROR_COMPRESSED_BINARY_TOO_LARGE = -1107,

    /* The update file is too big for the RAM on the device. */
    VM_FIRMWARE_UPDATE_ERROR_COPY_TO_RAM_FAIL = -1106,

    /* The update file is too big for the file system on the device. */
    VM_FIRMWARE_UPDATE_ERROR_DECOMPRESSED_BINARY_TOO_LARGE = -1108,

    /* The update file is too big for the C:\ partition on the device. */
    VM_FIRMWARE_UPDATE_ERROR_OVERLAP_WITH_TARGET_FS = -321,

    /* Incorrect header signature.
       The content of update package is corrupted. */
    VM_FIRMWARE_UPDATE_ERROR_INVALIDE_HEADER_SIG = -303,

    /* Incorrect body signature.
       The content of update package is corrupted. */
    VM_FIRMWARE_UPDATE_ERROR_INVALIDE_BODY_SIG = -304,

    /* Cannot mount the drive.
       Make sure the system drive is functional
       and the file is located in "C:\image.bin". */
    VM_FIRMWARE_UPDATE_ERROR_NO_CARD_FOUND = -701,

    /* Cannot mount the drive.
       Make sure the update package is located in "C:\image.bin". */
    VM_FIRMWARE_UPDATE_ERROR_NO_DL_PACKAGE_FOUND = -702,

    /* Ensure the compiler treat this enum as 32-bit interger. */
    VM_FIRMWARE_UPDATE_RESULT_MAX = 0x7FFFFFFF
} VM_FIRMWARE_UPDATE_RESULT;

/* After vm_firmware_trigger_update is called and the system reboots,
   open "C:\update_info" in binary mode and read the content to
   this structure to know the result of update process. */
typedef struct
{
    VMINT32 version;     /* Reserved. */
    VMINT32 marker;      /* This marker must be 0x54454E46 for
                            the entry to be valid. */
    VMINT32 error_code;  /* The resulting code of update procedure.
                            0 if successfully updated,
                            negative values if failed.
                            Refer to VM_FIRMWARE_UPDATE_RESULT for details. */
    VMINT32 reserved[5]; /* Reserved fields */
} vm_firmware_update_status_t;

/*****************************************************************************
 * FUNCTION
 *  vm_firmware_get_info
 * DESCRIPTION
 *  Get firmware information, e.g. firmware version, build date, and maximum
 *  memory that LinkIt application can use.
 * PARAMETERS
 *  value : [OUT] A buffer to store the firmware information in ASCII string.
*                 Pass NULL to get the required buffer length from return value.
 *  length : [IN] Size of the buffer in bytes, it should be 1 byte larger than
 *                the required buffer length to accomodate the terminating NULL.
 *                e.g. if length is 12 and the information string is 15 bytes
 *                long, only the first 12 character will be written to the
 *                buffer without the terminating NULL.
 *  key : [IN] Firmware information to retrieve. See VM_FIRMWARE_KEY_INFO.
 * RETURNS
 *  The actual written length in bytes, not including the terminating NULL.
 * EXAMPLE
 *  <code>
 *	VMCHAR value[30] = {0};
 *  VMUINT written = vm_firmware_get_info(value,
 *                                        sizeof(value),
 *                                        VM_FIRMWARE_HOST_VERSION);
 *  </code>
 *****************************************************************************/
VMUINT vm_firmware_get_info(VMSTR value,
                            VMUINT length,
                            VM_FIRMWARE_KEY_INFO key);

/*****************************************************************************
 * FUNCTION
 *  vm_firmware_is_module_supported
 * DESCRIPTION
 *  Query if the specified module is supported or not.
 * PARAMETERS
 *  module: [IN] The module to check. Refer to VM_FIRMWARE_MODULE_NAME
 * RETURNS:
 * VM_TRUE if supported by firmware, VM_FALSE if not.
 *****************************************************************************/
VMBOOL vm_firmware_is_module_supported(VM_FIRMWARE_MODULE_NAME module);

/*****************************************************************************
 * FUNCTION
 *	vm_fota_trigger
 * DESCRIPTION
 *  Calling this function causes the system to reboot immediately and then
 *  trigger the firmware update process in bootloader to update full firmware
 *  image. The path to the image must be "C:\image.bin"
 *  This function always returns VM_SUCCESS because the LinkIt application
 *  will be terminated before the actual updating process take action.
 *  The result of the update process is written to a file named
 *  "C:\update_info" on disk. The file is a binary file with structure
 *  vm_firmware_update_status_t as the content. Use file system APIs
 *  to read and parse the update_info file to check if the firmware update
 *  procedure has performed successfully or not. Refer to
 *  vm_firmware_update_status_t and VM_FIRMWARE_UPDATE_RESULT for parsing
 *  the result of update and possible reasons for failture.
 * PARAMETERS
 *  void
 * RETURNS
 *	VM_RESULT, this function always return VM_SUCCESS
 *****************************************************************************/
VM_RESULT vm_firmware_trigger_update(void);

#ifdef __cplusplus
}
#endif

#endif/*VMFIRMWARE_SDK_H*/





