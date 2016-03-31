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

#ifndef VMUDP_SDK_H
#define VMUDP_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmbearer.h"
#include "vmsock.h"
#include "vmgsm_gprs.h"

typedef VMINT VM_UDP_HANDLE;

/* UDP events */
typedef enum
{
    VM_UDP_EVENT_WRITE = 2,         /* The write status. */
    VM_UDP_EVENT_READ = 3,          /* The read status. */
    VM_UDP_EVENT_PIPE_BROKEN = 4,   /* The broken status. */
    VM_UDP_EVENT_PIPE_CLOSED = 6,   /* The UDP pipe is closed. */
    VM_UDP_EVENT_MAX = 0x7FFFFFFF   /* To ensure the compiler treats this enum as 32-bit integer. */
} VM_UDP_EVENT;

/* TCP results */
typedef enum
{
    VM_UDP_SUCCESS = 0,                     /* The success. */
    VM_UDP_ERROR = -1,                      /* The general error. */
    VM_UDP_ERROR_NO_ENOUGH_RESOURCE	= -2,   /* There's not enough resource to create an UDP connection. */
    VM_UDP_ERROR_CREATE_FAILED	    = -3,   /* To create UDP connection has failed. */
    VM_UDP_RESULT_MAX = 0x7FFFFFFF          /* To ensure the compiler treats this enum as 32-bit integer. */
} VM_UDP_RESULT;

/*****************************************************************************
 * FUNCTION
 *    vm_udp_callback
 * DESCRIPTION
 *  UDP callback function type.
 * PARAMETERS
 *  handle : [IN] The UDP handle.
 *  event : [IN] The UDP events.
 * RETURNS
 * void
*****************************************************************************/
typedef void (*vm_udp_callback)(VM_UDP_HANDLE handle, VM_UDP_EVENT event);

/*****************************************************************************
 * FUNCTION
 *    vm_udp_create
 * DESCRIPTION
 *  This function creates UDP socket.
 * PARAMETERS
 *  port : [IN] The port number for binding.
 *  data_account_type : [IN] Is a Type that defines how to connect, please see VM_BEARER_DATA_ACCOUNT_TYPE for details.
 *  callback : [IN] The Callback function for UDP notifications.
 *  loopback : [IN] The loopback setting, VM_TRUE: loopback; VM_FALSE: no loopback.
 * RETURNS
 *  The create UDP is successful or not.
 * RETURN VALUES
 *  >=0: The UDP handle.
 *  < 0: An error code in VM_UDP_RESULT.
 * EXAMPLE
 * <code>
 *  static VMINT g_hdll;
 *  void hl_udp_create(void)
 *  {
 *      VMINT port = 0x1234;
 *      g_hdll = vm_udp_create(port, VM_BEARER_DATA_ACCOUNT_TYPE_WLAN, udp_callback, VM_TRUE);
 *  }
 * </code>
*****************************************************************************/
VM_UDP_HANDLE vm_udp_create(VMINT port, VM_BEARER_DATA_ACCOUNT_TYPE data_account_type, vm_udp_callback callback, VMBOOL loopback);

/*****************************************************************************
 * FUNCTION
 *    vm_udp_send
 * DESCRIPTION
 *  This function sends data to the specified destination.
 * PARAMETERS
 *  handle :    [IN] The UDP handle.
 *  buffer :    [IN] The buffer.
 *  length :    [IN] The buffer length.
 *  address :   [IN] The destination address.
 * RETURNS
 *  The size of sent data.
 * RETURN VALUES
 *  VM_UDP_RESULT
 * EXAMPLE
 * <code>
 *  static void udp_callback(VMINT hdl, VMINT evt)
 *  {
 *      vm_ip_address_t sendto = {0};
 *      sendto.address[0] = 192;
 *      sendto.address[1] = 168;
 *      sendto.address[2] = 1;
 *      sendto.address[3] = 33;
 *      sendto.address_length = 4;
 *      sendto.port = 2500;
 *
 *      void* buffer = "Hello";
 *
 *      switch (evt)
 *      {
 *      case VM_UDP_EVENT_WRITE:
 *          {
 *              VMINT ret = 0;
 *              ret = vm_udp_send(hdl, buffer, 6, &sendto);
 *          }break;
 *      default:
 *          {
 *
 *          }break;
 *      }
 * }
 *
 * </code>
*****************************************************************************/
VMINT vm_udp_send(VM_UDP_HANDLE handle, const void* buffer, VMUINT32 length, const vm_soc_address_t* address);

/*****************************************************************************
 * FUNCTION
 *  vm_udp_receive_from
 * DESCRIPTION
 *  The Receive data from a source.
 * PARAMETERS
 *  handle :    [IN] The UDP handle.
 *  buffer :    [OUT] The Buffer.
 *  length :    [IN] The Buffer length.
 *  address :   [OUT] The source address.
 * RETURNS
 * >= 0: The number of bytes received.
 * < 0: An error code in VM_UDP_RESULT.
 * EXAMPLE
 * <code>
 *  static void udp_callback(VMINT hdl, VMINT evt)
 *  {
 *      VMCHAR buf[1024] = {0};
 *      switch (evt)
 *      {
 *      case VM_UDP_EVENT_READ:
 *          {
 *              VMINT ret = 0;
 *              vm_soc_address_t recvfrom = {0};
 *              ret = vm_udp_receive(hdl, buf, 1024, &recvfrom);
 *          }break;
 *      default:
 *          {
 *
 *          }break;
 *      }
 *  }
 * </code>
*****************************************************************************/
VMINT vm_udp_receive(VM_UDP_HANDLE handle, void* buffer, VMINT32 length, vm_soc_address_t* address);

/*****************************************************************************
 * FUNCTION
 *    vm_udp_close
 * DESCRIPTION
 *  This function closes the UDP socket.
 * PARAMETERS
 *  handle : [IN] The UDP handle.
 * RETURNS
 *  VM_UDP_RESULT which indicates if it was closed successfully or not.
 * EXAMPLE
 * <code>
 *  static VMINT g_hdll;
 *  void hl_udp_create(void)
 *  {
 *      VMINT port = 0x1234;
 *      g_hdl1 = vm_udp_create(port, VM_BEARER_DATA_ACCOUNT_TYPE_WLAN, udp_callback, 1);
 *      vm_udp_close(g_hdll);
 *  }
 * </code>
*****************************************************************************/
VM_RESULT vm_udp_close(VM_UDP_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* VMUDP_SDK_H */

