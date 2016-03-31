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

#ifndef VMTYPE_SDK_H
#define VMTYPE_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE (1)      /* 1 */
#endif

#ifndef FALSE
#define FALSE (0)     /* 0 */
#endif

#ifndef NULL
#define NULL (0)      /* 0 */
#endif


typedef unsigned char  VMUINT8;             /* unsigned char,  8-bit,  range: 0 - 2^8-1 */
typedef unsigned short VMUINT16;            /* unsigned short, 16-bit, range: 0 - 2^16-1 */
typedef unsigned int   VMUINT;              /* unsigned int,   32-bit, range: 0 - 2^32-1 */
typedef unsigned long  VMUINT32;            /* unsigned long,  32-bit, range: 0 - 2^32-1 */


typedef  unsigned long long VMUINT64;       /* unsigned long long, 64-bit, range:     0 - 2^64-1 */
typedef  long long          VMINT64;        /* long long,          64-bit, range: -2^63 - 2^63-1 */


typedef signed char VMINT8;                 /* char,  8-bit,  range: -2^7 - 2^7-1 */
typedef short       VMINT16;                /* short, 16-bit, range: -2^15 - 2^15-1 */
typedef int         VMINT;                  /* int,   32-bit, range: -2^31 - 2^31-1 */
typedef long        VMINT32;                /* long,  32-bit, range: -2^31 - 2^31-1 */

typedef unsigned char  VMUCHAR;             /* unsigned char,  8-bit,  range: 0 - 2^8-1 */
typedef unsigned short VMUWCHAR;            /* unsigned short, 16-bit, range: 0 - 2^16-1 */

typedef signed char  VMCHAR;                /* char,  8-bit,  range: -2^7 - 2^7-1 */
typedef short        VMWCHAR;               /* short, 16-bit, range: -2^15 - 2^15-1 */
typedef signed char* VMSTR;                 /* signed char*, a NULL-terminated single-byte string. */
typedef short*       VMWSTR;                /* short*,       a NULL-terminated two-byte string. */

typedef const VMWCHAR* VMCWSTR;             /* const short *, a const NULL-terminated two-bye string */
typedef const VMCHAR* VMCSTR;               /* const signed char *, a const NULL-terminated single-byte string. */

typedef unsigned char  VMBYTE;              /* unsigned char,  8-bit,  range:     0 - 2^8-1 */
typedef unsigned short VMUSHORT;            /* unsigned short, 16-bit, range:     0 - 2^16-1 */
typedef short          VMSHORT;             /* short,          16-bit, range: -2^15 - 2^15-1 */


typedef int VMBOOL;                         /* int, 32-bit, range: -2^31 - 2^31-1 */


typedef float         VMFLOAT;              /* float,  32-bit, single precision. */
typedef double        VMDOUBLE;             /* double, 64-bit, double precision. */


#define VM_TRUE       TRUE                  /* Boolean value true. */
#define VM_FALSE      FALSE                 /* Boolean value false. */
#define VM_RESULT     VMINT                 /* General result type for LinkIt SDK API. Must be used with VM_IS_SUCCEEDED() and VM_IS_FAILED() to confirm sucess or not. Do not compare with VM_SUCESS and VM_FAIL directly. */
#define VM_SUCCESS    (1)                   /* Do not compare it with VM_RESULT directly, i.e. DO NOT USE if(VM_SUCCESS==xxxx()) to determine a success or not. Instead, use if(VM_IS_SUCCEEDED(xxx()). */
#define VM_OK         (0)                   /* Do not compare it with VM_RESULT directly. */
#define VM_FAIL       (-1)                  /* Do not compare it with VM_RESULT directly, i.e. DO NOT USE if(VM_FAIL==xxxx()) to determine a failure or not. Instead, use if(VM_IS_FAILED(xxx()). */


#define VM_IS_SUCCEEDED(x) (((x)>=0)?VM_TRUE:VM_FALSE)    /* Use this macro to determine if a VM_RESULT is a success or not. */
#define VM_IS_FAILED(x) (((x)<0)?VM_TRUE:VM_FALSE)        /* Use this macro to determine if a VM_RESULT is a failure or not. */


#define VM_SYMBOL_NOT_FOUND  (-103)         /* The error code (-103) means a symbol cannot be found in the firmware. */


#ifdef __cplusplus
}
#endif

#endif  /* VMTYPE_SDK_H */
