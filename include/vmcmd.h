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

#ifndef VMCMD_SDK_H
#define VMCMD_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* This function represents the incoming command from the Monitor tool. */
typedef struct
{
    VMUINT32 command_type;        /* Reserved type. */
    VMUINT32 command_buffer_size; /* The size of the incoming command buffer. */
    VMSTR command_buffer;		      /* The buffer that stores the command. */
} vm_cmd_command_t;

/*****************************************************************************
 * FUNCTION
 *  vm_cmd_callback
 * DESCRIPTION
 *  This represents the callback function that handles incoming commands from the Monitor tool.
 *  This callback is invoked in the main thread.
 *
 * PARAMETERS
 *  command   : [IN] The command that is passed from the Monitor tool.
 *                   The content of the buffer is only valid within the
 *                   callback. Once the callback returns, buffer pointed by
 *                   command.command_buffer becomes invalid. 
 *  user_data : [IN] The user_data passed to vm_cmd_open_port.
 * RETURNS
 *  void.
 *****************************************************************************/
typedef void (*vm_cmd_callback)(vm_cmd_command_t* command, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_cmd_open_port
 * DESCRIPTION
 *  This function is called to start listening for commands from the Monitor tool.
 *  Commands are passed from the port, which ranges from 500 to 65535.
 *  
 *  For example, if the command is passed into port 5000, then
 *  only the callback registered with port number 5000 will be invoked.
 *  
 *  This API fails when invalid port number is assigned.
 *
 * PARAMETERS
 *  port      : [IN] The port number, which ranges from 500 to 65535.
 *  callback  : [IN] The callback that is invoked when it receives command
 *                   from the assigned port.
 *  user_data : [IN] User data for the callback; Pass NULL if unused.
 * RETURNS
 *  VM_RESULT.
 *****************************************************************************/
VM_RESULT vm_cmd_open_port(VMUINT16 port,
                           vm_cmd_callback callback,
                           void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_cmd_close_port
 * DESCRIPTION
 *  This function closes the command port. The callback will no longer be invoked even when
 *  Monitor tool passes command.
 * PARAMETERS
 *  port : [IN] The port number that is opened by vm_open_command_port().
 *****************************************************************************/
void vm_cmd_close_port(VMUINT16 port);


#ifdef __cplusplus
}
#endif

#endif /* VMCMD_SDK_H */

