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

#ifndef VMLOG_SDK_H
#define VMLOG_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern void lua_log_printf(int type, const char *file, int line, const char *msg, ...);

// !!If you don't need log messages, comment this define!!
//#define __LINKIT_RELEASE__

/*
void _vm_log_debug(const char* fmt, ...);
void _vm_log_info(const char* fmt, ...);
void _vm_log_warn(const char* fmt, ...);
void _vm_log_error(const char* fmt, ...);
void _vm_log_fatal(const char* fmt, ...);
int _vm_log_module(const char* __file__, const int __line__);
*/


#if defined(__LINKIT_RELEASE__)
#define vm_log_fatal
#define vm_log_error
#define vm_log_warn
#define vm_log_info
#define vm_log_debug
#else
/*****************************************************************************
 * FUNCTION
 *    vm_log_fatal
 * DESCRIPTION
 *  Generates a log with fatal level of severity.
 *  This is the most serious error, the result of which is the direct
 *  termination of the program. For example, the application fails
 *  to allocate memory upon program start-up. This will result in the
 *  system being unable to generate the user interface screen and
 *  having to terminate the application.
 * PARAMETERS
 *  format_string : [IN] Format string, same as vm_sprintf, followed by argements for the format string.
*****************************************************************************/
//#define vm_log_fatal(...)		if(_vm_log_module(__FILE__, __LINE__)) _vm_log_fatal(__VA_ARGS__)
#define vm_log_fatal(...)		lua_log_printf(1,__FILE__, __LINE__,__VA_ARGS__)

/*****************************************************************************
 * FUNCTION
 *    vm_log_error
 * DESCRIPTION
 *  Generates a log with error level severity.
 *  This indicates that an error has occurred, with the result that
 *  one of the program's functions fails to work,
 *  although the program itself will not terminate because of it.
 *  For example, on an English learning program the human voice
 *  pronunciation feature fails to produce any sound.
 *  However, all other functions in the application operate normally.
 * PARAMETERS
 *  format_string : [IN] Format string, same as vm_sprintf, followed by argements for the format string.
*****************************************************************************/
//#define vm_log_error(...)		if(_vm_log_module(__FILE__, __LINE__)) _vm_log_error(__VA_ARGS__)
#define vm_log_error(...)		lua_log_printf(2,__FILE__, __LINE__,__VA_ARGS__)

/*****************************************************************************
 * FUNCTION
 *    vm_log_warn
 * DESCRIPTION
 *  Generates a log with warning level of severity.
 *  Use this function for logs with warning purposes.
 *  The cause of the warning is within the expectation of the program and
 *  the situation may restore to normal eventually. For example,
 *  the program may fail to connect to the Internet,
 *  the reason for which may be delinquent phone bills, or bad signal coverage.
 *  After recharging the account or moving the phone to a location with a
 *  stronger signal, the problem will no longer exist.
 * PARAMETERS
 *  format_string : [IN] Format string, same as vm_sprintf, followed by argements for the format string.
*****************************************************************************/
//#define vm_log_warn(...)		if(_vm_log_module(__FILE__, __LINE__)) _vm_log_warn(__VA_ARGS__)
#define vm_log_warn(...)		lua_log_printf(3,__FILE__, __LINE__,__VA_ARGS__)

/*****************************************************************************
 * FUNCTION
 *    vm_log_info
 * DESCRIPTION
 *  Generate logs for informational purposes, e.g.
 *  when execution of app enters into  a function,
 *  a log is written to indicate execution entered in that particular function.
 * PARAMETERS
 *  format_string : [IN] Format string, same as vm_sprintf, followed by argements for the format string.
*****************************************************************************/
//#define vm_log_info(...)		if(_vm_log_module(__FILE__, __LINE__)) _vm_log_info( __VA_ARGS__)
#define vm_log_info(...)		lua_log_printf(4,__FILE__, __LINE__,__VA_ARGS__)

/*****************************************************************************
 * FUNCTION
 *    vm_log_debug
 * DESCRIPTION
 *  Use this function to generate log that serve as debugging information.
 * PARAMETERS
 *  format_string : [IN] Format string, same as vm_sprintf, followed by argements for the format string.
*****************************************************************************/
//#define vm_log_debug(...)		if(_vm_log_module(__FILE__, __LINE__)) _vm_log_debug(__VA_ARGS__)
#define vm_log_debug(...)		lua_log_printf(5,__FILE__, __LINE__,__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* VMLOG_SDK_H */
