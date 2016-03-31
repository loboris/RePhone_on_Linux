/*****************************************************************************
*  Copyright Statement:
*  ____________________
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2006
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS_IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
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

#ifndef VMDCL_KBD_SDK_H
#define VMDCL_KBD_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************
* DESCRIPTION
*   This enum defines the device for kbd module,used in vm_dcl_open as a parameter.
*   To control KBD, you should use DCL(Driver Common Layer) APIs. 
* EXAMPLE
* <code>
*   #include "vm_dcl.h"
*   VM_DCL_HANDLE kbd_handle;                   	// Declare a VM_DCL_HANDLE variable.
*   kbd_handle = vm_dcl_open(VM_DCL_KBD,0);         // Call vm_dcl_open to get a handle. 
* </code>
*******************************************************************/
typedef enum
{
    VM_DCL_KBD_START = VM_DCL_KBD_GROUP_START, /*KBD device start*/
    VM_DCL_KBD,                                /*This device is used to control keypad.*/
}VM_DCL_KBD_DEVICE;

/******************************************************************
* DESCRIPTION
*   This enum  define the control command for KBD  module,used in vm_dcl_control as parameter.
*   With different commands, user could control the different function of the KBD.
*   To control KBD, you should use DCL(Driver Common Layer) APIs. 
* EXAMPLE
* <code>
*   #include "vm_dcl.h"
*   VM_DCL_HANDLE       kbd_handle;						// Declare a VM_DCL_HANDLE variable. 
*   vm_dcl_kbd_control_pin_t kbdmap;
*   kbd_handle       = vm_dcl_open(VM_DCL_KBD,0);     // First, we call vm_dcl_open to get a handle. the second parameter always fill 0.
*   kbdmap.colmap  = 0x01;
*   kbdmap.rowmap = 0x01;
*   vm_dcl_control(kbd_handle,VM_DCL_KBD_COMMAND_CONFIG_PIN,(void *)(&kbdmap)); //  we call vm_dcl_control to set the col and row.
*   vm_dcl_close(kbd_handle);                           			// Finally, we call vm_dcl_close 
* </code>
*******************************************************************/

typedef enum
{
    VM_DCL_KBD_COMMAND_CONFIG_PIN = 0xC0D, //control keypad row and col
}VM_DCL_KBD_COMMAND;


/******************************************************************
* DESCRIPTION
*   This struct is for VM_DCL_KBD_COMMAND_CONFIG_PIN control command,used in vm_dcl_control as parameter.
*   With this command, you can set the HW keypad col and row,and enable it.
*   You can find the sample code in the description of VM_DCL_KBD_COMMAND.
*   All commands have similar usage.
*******************************************************************/

/*the control command data for configure col and row pin enalbe*/
typedef struct
{
   VMUINT16  col_map;      /*col map,lowest is col0*/
   VMUINT16  row_map;      /*row map,lowest is row0*/
}vm_dcl_kbd_control_pin_t;

#ifdef __cplusplus
}
#endif

#endif

