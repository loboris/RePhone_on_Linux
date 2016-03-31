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

#ifndef VMCHSET_SDK_H
#define VMCHSET_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* The maximum length of character conversion. */
#define VM_CHSET_CONVERT_MAX_LENGTH 4096



/* Character sets */
typedef enum
{
    VM_CHSET_ENCODING_BASE = 0,         /* BASE */
    VM_CHSET_ENCODING_ASCII,            /* ASCII */
    VM_CHSET_ENCODING_UTF16LE = 35,     /* UTF16LE */
    VM_CHSET_ENCODING_UTF16BE,          /* UTF16BE */
    VM_CHSET_ENCODING_UTF8,             /* UTF8 */
    VM_CHSET_ENCODING_UCS2,             /* UCS2 */
    VM_CHSET_ENCODING_MAX = 0X7FFFFFFF  /* Forces the compiler to use 4-byte integer for this enum. */
} VM_CHSET_ENCODING;

/* The return value of vm_chset_convert */
typedef enum
{
    VM_CHSET_CONVERT_RESULT_SUCCESS                     = (0),        /* If successful. */
    VM_CHSET_CONVERT_RESULT_ERROR_PARAMETER             = (-1),       /* Parameter error. */
    VM_CHSET_CONVERT_RESULT_SOURCE_UNSUPPORT_CHSET      = (-2),       /* Character set used by input string is not supported. */
    VM_CHSET_CONVERT_RESULT_DESTINATION_UNSUPPORT_CHSET = (-3),       /* Character set used by destination string is not supported. */
    VM_CHSET_CONVERT_RESULT_FATAL_ERROR                 = (-4),       /* General failure. */
    VM_CHSET_CONVERT_RESULT_MAX                         = 0X7FFFFFFF  /* Forces the compiler to use 4-byte integer for this enum. */
} VM_CHSET_CONVERT_RESULT;


/*****************************************************************************
 * FUNCTION
 *    vm_chset_ucs2_to_ascii
 * DESCRIPTION
 *    Converts the UCS-2 string to an ASCII string. The caller should ensure that
 *    the size of the destination buffer is large enough to contain the resulting
 *    string. If not, the conversion will only convert a part of the source string
 *    up to the size of the destination buffer. In other words, the resulting string
 *    is a truncated string up to the size of the destination buffer.
 * PARAMETERS
 *    destination: [OUT] The resulting string.
 *    size:        [IN]  The size of the destination buffer in bytes.
 *    source:      [IN]  The source string.
 * RETURNS
 *    VM_CHSET_CONVERT_RESULT
 *****************************************************************************/
VM_CHSET_CONVERT_RESULT vm_chset_ucs2_to_ascii(VMSTR destination, VMINT size, VMCWSTR source);

/*****************************************************************************
 * FUNCTION
 *    vm_chset_ascii_to_ucs2
 * DESCRIPTION
 *    Converts the ASCII string to a UCS-2 string. The caller should ensure that
 *    the size of the destination buffer is large enough to contain the resulting
 *    string. If not, the conversion will only convert a part of the source string
 *    up to the size of the destination buffer. In other words, the resulting string
 *    is a truncated string up to the size of the destination buffer.
 * PARAMETERS
 *    destination: [OUT] The resulting string.
 *    size:        [IN]  The size of the destination buffer in bytes.
 *    source:      [IN]  The source string.
 * RETURNS
 *    VM_CHSET_CONVERT_RESULT
 *****************************************************************************/
VM_CHSET_CONVERT_RESULT vm_chset_ascii_to_ucs2(VMWSTR destination, VMINT size, VMCSTR source);


/*****************************************************************************
 * FUNCTION
 *    vm_chset_convert
 * DESCRIPTION
 *    Converts a string between 2 character sets, and add the corresponding
 *    terminate character to the destination string. The caller should ensure that
 *    the size of the destination buffer is large enough to contain the resulting
 *    string. If not, the conversion will only convert a part of the source string
 *    up to the size of the destination buffer. In other words, the resulting string
 *    is a truncated string up to the size of the destination buffer.
 * PARAMETERS
 *    source_type:      [IN]  The character set of the source string.
 *    destination_type: [IN]  The character set of the resulting string.
 *    source:           [IN]  The source string.
 *    destination:      [OUT] The resulting string.
 *    destination_size: [IN]  The size of the destination buffer in bytes.
 * RETURNS
 *    VM_CHSET_CONVERT_RESULT
 *****************************************************************************/
VM_CHSET_CONVERT_RESULT vm_chset_convert(VM_CHSET_ENCODING source_type,
        VM_CHSET_ENCODING destination_type,
        const VMCHAR* source, VMCHAR* destination,
        VMINT destination_size);


#ifdef __cplusplus
}
#endif

#endif /* VMCHSET_SDK_H_ */

