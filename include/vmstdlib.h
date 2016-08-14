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

#ifndef VMSTDLIB_SDK_H
#define VMSTDLIB_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include "vmtype.h"

/*
#if defined(__COMPILER_RVCT__) || defined(__COMPILER_GCC__)
#undef vsprintf
#undef sprintf
#undef sscanf

VMINT vm_vsprintf(VMSTR buffer, VMCSTR format, va_list args);
VMINT vm_sprintf( VMSTR  buffer, VMCSTR  format, ... );
VMINT vm_sscanf(VMCSTR str, VMCSTR format, ...);

#define vsprintf vm_vsprintf
#define sprintf vm_sprintf
#define sscanf vm_sscanf
#endif
*/

/*****************************************************************************
 * FUNCTION
 *    vm_str_strtoi
 * DESCRIPTION
 *    Converts a number-formatted string to an integer. It returns -1, if the
 *    number formatted string is a negative string, such as "-12345".
 * PARAMETERS
 *    string: [IN] The number-formatted string to be converted, such as "12345".
 * RETURNS
 *    The integer of the converted number-formatted string, or -1 if conversion fails.
 *****************************************************************************/
VMINT vm_str_strtoi(VMCSTR string);

/*****************************************************************************
 * FUNCTION
 *    vm_string_equals_ignore_case
 * DESCRIPTION
 *    Compares two strings without case sensitive.
 * PARAMETERS
 *    string1: [IN] The first string.
 *    string2: [IN] The second string.
 * RETURNS
 *    0, if the two strings are the same. -1, if they are different.
 *****************************************************************************/
VMINT vm_str_equals_ignore_case(VMCSTR string1, VMCSTR string2);


/*****************************************************************************
 * FUNCTION
 *    vm_ends_with
 * DESCRIPTION
 *    Checks if a string is ended with a specified string.
 * PARAMETERS
 *    string1: [IN] The string to be checked against.
 *    string2: [IN] The ending string.
 * RETURNS
 *    0, if string1 is ended with string2. Non-zero, if otherwise.
 *****************************************************************************/
VMINT vm_str_ends_with(VMCSTR string1, VMCSTR string2);

/*****************************************************************************
 * FUNCTION
 *    vm_str_upper_case
 * DESCRIPTION
 *    Converts the characters in the string pointed by source to all upper case
 *    in the string pointed by destination.
 * PARAMETERS
 *    destination: [OUT] The converted string with all upper case.
 *    source:      [IN]  The string to be converted.
 *****************************************************************************/
void vm_str_upper_case(VMSTR destination, VMCSTR source);

/*****************************************************************************
 * FUNCTION
 *    vm_str_lower_case
 * DESCRIPTION
 *    Converts the characters in the string pointed by source to all lower case
 *    in the string pointed by destination.
 * PARAMETERS
 *    destination: [OUT] The converted string with all lower case.
 *    source:      [IN]  The string to be converted.
 *****************************************************************************/
void vm_str_lower_case(VMSTR destination, VMCSTR source);

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_safe_wstrcpy
 * DESCRIPTION
 *    A UCS-2 string safe copy operation. It copies the string pointed by source
 *    to the string pointed by destination, up to the length of the destination
 *    buffer. The extra characters will not be copied.
 * PARAMETERS
 *    destination: [OUT] The resulting string.
 *    size:        [IN]  The length of the destination buffer.
 *    source:      [IN]  The source string.
 * RETURNS
 *    The actual number of UCS-2 characters copied.
 *****************************************************************************/
VMINT vm_wstr_safe_wstrcpy(VMWSTR destination, VMINT size, VMWSTR source);

#define wstrlen			vm_wstr_string_length
/*****************************************************************************
 * FUNCTION
 *    vm_wstr_string_length
 * DESCRIPTION
 *    Gets the length of a UCS-2 string.
 * PARAMETERS
 *    string: [IN] The '\0' terminated string.
 * RETURNS
 *    The length of string.
 *****************************************************************************/
VMINT vm_wstr_string_length(VMWSTR string);

#define wstrcpy			vm_wstr_copy
/*****************************************************************************
 * FUNCTION
 *    vm_wstr_copy
 * DESCRIPTION
 *    Copies the UCS-2 string pointed by source to the UCS-2 string pointed by
 *    destination. The caller should ensure that the destination buffer is large
 *    enough to contain the copied resulting string.
 * PARAMETERS
 *    destination: [OUT] The destination string.
 *    source:      [IN]  The source string.
 * RETURNS
 *    The actual length of the string copied.
 *****************************************************************************/
VMINT vm_wstr_copy(VMWSTR destination, VMCWSTR source);

#define wstrcat			vm_wstr_concatenate
/*****************************************************************************
 * FUNCTION
 *    vm_wstr_concatenate
 * DESCRIPTION
 *    Appends the UCS-2 string pointed by source to the UCS-2 string pointed by
 *    destination. The caller should ensure the destination buffer is large enough
 *    to contain the concatenated resulting string.
 * PARAMETERS
 *    destination: [OUT] The destination string.
 *    source:      [IN]  The source string.
 *****************************************************************************/
VMINT vm_wstr_concatenate(VMWSTR destination, VMCWSTR source);

#define wstrncpy		vm_wstr_copy_with_length
/*****************************************************************************
 * FUNCTION
 *    vm_wstr_copy_with_length
 * DESCRIPTION
 *    Copies specified number of characters in a UCS-2 string pointed by source to
 *    the UCS-2 string pointed by destination. The caller should ensure that the
 *    destination buffer is large enough to contain the copied resulting string.
 *    This function does not insert null terminated '\0' at the end of the resulting
 *    string.
 * PARAMETERS
 *    destination : [OUT] The resulting string.
 *    source :      [IN]  The source string.
 *    length :      [IN]  The length of the source string.
 * RETURNS
 *    The actual number of UCS-2 characters copied.
 *****************************************************************************/
VMINT vm_wstr_copy_with_length(VMWSTR destination, VMCWSTR source, VMINT length);

#define wstrcmp			vm_wstr_compare

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_compare
 * DESCRIPTION
 *    Compares two UCS-2 strings.
 * PARAMETERS
 *    string1: [IN] The string to be compared.
 *    string2: [IN] The string to be compared.
 * RETURNS
 *    0, if the two strings are the same. Non-zero, if they are different.
 *****************************************************************************/
VMINT vm_wstr_compare(VMWSTR wide_string1, VMWSTR wide_string2);

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_starts_with
 * DESCRIPTION
 *    Checks if the UCS-2 string starts with a specified UCS-2 string.
 * PARAMETERS
 *    string1: [IN] The string to be checked.
 *    string2: [IN] The starting string to check for.
 * RETURNS
 *    0, if string1 starts with string2. Non-zero, if not.
 *****************************************************************************/
VM_RESULT vm_wstr_starts_with(VMWSTR string1, VMWSTR string2);

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_sprintf
 * DESCRIPTION
 *    output the wchar string to buffer with the format input
 * PARAMETERS
 *    buffer: [IN] the wchar string buffer for output
 *    size:   [IN] the number of bytes of the buffer
 *    format: [IN] the format pattern,like  L"test is %d,%d"
 * RETURNS
 *    the number of bytes output
 *****************************************************************************/
VMINT vm_wstr_sprintf(VMWSTR buffer, VMINT size, VMCWSTR format, ...);


/*****************************************************************************
 * FUNCTION
 *    vm_wstr_get_filename
 * DESCRIPTION
 *    Gets the filename from the full path, in UCS-2 encoding.
 * PARAMETERS
 *    path:     [IN]  The UCS-2 string of the full path.
 *    filename: [OUT] The UCS-2 string of the filename.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_wstr_get_filename(VMCWSTR path, VMWSTR filename);

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_get_path
 * DESCRIPTION
 *    Gets file path from full path, in UCS-2 encoding.
 * PARAMETERS
 *    full_path: [IN]  The UCS-2 string of the full path.
 *    path:      [OUT] The UCS-2 string of the file path.
 * RETURNS
 *    VM_RESULT
 *****************************************************************************/
VM_RESULT vm_wstr_get_path(VMCWSTR full_path, VMWSTR path);

/*****************************************************************************
 * FUNCTION
 *    vm_wstr_ends_with
 * DESCRIPTION
 *    Checks if a UCS-2 string is ended with a specified UCS-2 string.
 * PARAMETERS
 *    string1: [IN] The string to be checked.
 *    string2: [IN] The string to be checked against.
 * RETURNS
 *    0, if string1 is ended with string2. Otherwise, returns non-zero.
 *****************************************************************************/
VM_RESULT vm_wstr_ends_with(VMCWSTR string1, VMCWSTR string2);


#ifdef __cplusplus
}
#endif

#endif /* VMSTDLIB_SDK_H_ */
