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

#ifndef VMTCP_SDK_H
#define VMTCP_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmbearer.h"
#include "vmsock.h"
#include "vmgsm_gprs.h"

typedef VMINT VM_TCP_HANDLE;
typedef VMINT VM_TCP_SERVER_HANDLE;

/* TCP events */
typedef enum
{
    VM_TCP_EVENT_CONNECTED =	1,  /* The TCP is connected. */
    VM_TCP_EVENT_CAN_WRITE = 2,     /* The write status. */
    VM_TCP_EVENT_CAN_READ = 3,      /* The read status. */
    VM_TCP_EVENT_PIPE_BROKEN = 4,   /* The TCP pipe is broken. */
    VM_TCP_EVENT_HOST_NOT_FOUND = 5,/* Unable to find host. */
    VM_TCP_EVENT_PIPE_CLOSED	= 6,/* The TCP pipe is closed. */
    VM_TCP_EVENT_MAX = 0x7FFFFFFF   /* To ensure the compiler treats this enum as 32-bit integer. */
} VM_TCP_EVENT;

/* TCP results */
typedef enum
{
    VM_TCP_SUCCESS = 0,
    VM_TCP_ERROR = -1,
    VM_TCP_WOULDBLOCK = -2,
    VM_TCP_ERROR_NO_ENOUGH_RESOURCE	= -3,  /* There isn't enough resources to create TCP connection. */
    VM_TCP_ERROR_CREATE_FAILED	    = -4,  /* To create TCP connection has failed. */
    VM_TCP_RESULT_MAX = 0x7FFFFFFF         /* To ensure the compiler treats this enum as 32-bit integer. */
} VM_TCP_RESULT;

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_connect_callback
 * DESCRIPTION
 *  This function represents TCP connection callback function type.
 * PARAMETERS
 *  handle :    [IN] The TCP handle.
 *  event :     [IN] The connection events.
 *  user_data : [IN] The user_data.
 * RETURNS
 * void
 ****************************************************************************/
typedef void (*vm_tcp_connect_callback)(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_connect
 * DESCRIPTION
 *  This function creates a TCP connection to the host.
 * PARAMETERS
 *  host :              [IN] The destination host IP.
 *  port :              [IN] The destination host port.
 *  data_account_type : [IN] The data account to use.
 *  user_data :         [IN] The user data.
 *  callback :          [IN] The callback function for TCP connection.
 * RETURNS
 *  If successful, it returns the TCP handle;
 *  If Fails, it returns a VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  static VMINT g_hl_tcp_hd;
 *  void hl_tcp_conn_cb_ex(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
 *  {
 *      char* my_data = (char*)user_data;
 *      switch (event)
 *      {
 *          case VM_TCP_EVENT_CONNECTED:
 *           //do something here
 *              vm_tcp_close(handle);
 *              return;
 *          default:
 *              return;
 *      }
 *  }
 *  void hl_tcp_connect(void)
 *  {
 *      char* my_data;
 *      my_data = (char*)vm_malloc(10);
 *      strcpy(my_data, "aa");
 *      vm_enable_proxy(FALSE);
 *      g_hl_tcp_hd = vm_tcp_connect("www.mediatek.com", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN, (void )my_data, hl_tcp_conn_cb_ex);

 *  }
 * </code>
*****************************************************************************/
VM_TCP_HANDLE vm_tcp_connect(const char* host, const VMINT port, VM_BEARER_DATA_ACCOUNT_TYPE data_account_type, void* user_data,
                         vm_tcp_connect_callback connect_callback);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_close
 * DESCRIPTION
 *  This function closes a TCP connection.
 * PARAMETERS
 *  handle : [IN] The TCP handle.
 * RETURNS
 * void
*****************************************************************************/
void vm_tcp_close(VM_TCP_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_read
 * DESCRIPTION
 *  This function reads data from a TCP connection.
 * PARAMETERS
 *  handle : [IN] The TCP handle.
 *  buffer : [OUT] The data buffer.
 *  length : [OUT] The data length.
 * RETURNS
 *  > 0: reads data size.
 *  = 0: receives the FIN from the server.
 *  < 0: error code, please check VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_conn_cb(VM_TCP_HANDLE handle, VM_TCP_EVENT event)
 *  {
 *      char buf[100];
 *      VMINT nread, nwrite;
 *      switch (event)
 *      {
 *          case VM_TCP_EVENT_CONNECTED:
 *              nwrite = vm_tcp_write(
 *                          handle,
 *                          "GET www.mediatek.com HTTP/1.1\r\nHost: www.mediatek.com\r\n\r\n",
 *                          strlen("GET www.mediatek.com HTTP/1.1\r\nHost: www.mediatek.com\r\n\r\n"));
 *              break;
 *          case VM_TCP_EVENT_CAN_READ:
 *              memset(buf, 0, 100);
 *              nread = vm_tcp_read(handle, buf, 100);
 *              vm_tcp_close(handle);
 *              return;
 *          default:
 *              return;
 *      }
 *      return;
 *  }
 * </code>
*****************************************************************************/
VMINT vm_tcp_read(VM_TCP_HANDLE handle, void* buffer, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_write
 * DESCRIPTION
 *  This function writes data to TCP connection.
 * PARAMETERS
 *  handle :[IN] The TCP handle.
 *  buf :   [IN] The data to write.
 *  len :   [IN] The data length.
 * RETURNS
 *  If successful, it returns the write data size.
 *  If fails, the error code can be found in VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_conn_cb(VM_TCP_HANDLE handle, VM_TCP_EVENT event)
 *  {
 *      VMINT nwrite;
 *      switch (event)
 *      {
 *          case VM_TCP_EVENT_CONNECTED:
 *              nwrite = vm_tcp_write(
 *                          handle,
 *                          "GET www.mediatek.com HTTP/1.1\r\nHost: www.mediatek.com\r\n\r\n",
 *                          strlen("GET www.mediatek.com HTTP/1.1\r\nHost: www.mediatek.com\r\n\r\n"));
 *              break;
 *          default:
 *              return;
 *      }
 *      return;
 *  }
 * </code>
*****************************************************************************/
VMINT vm_tcp_write(VM_TCP_HANDLE handle, void* buffer, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_connect_sync
 * DESCRIPTION
 *  This function creates a TCP connection synchronously and it will be blocked until it is connected.
 * PARAMETERS
 *  ip:             [IN] The destination host IP with format "123.123.123.123".
 *  port :          [IN] The destination host port.
 *  data account :  [IN] The data account to use.
 * RETURNS
 *  On success - the TCP handle.
 *  On failure - an error code, please check VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  static VMINT g_tcp_sync_hd;
 *  void hl_tcp_sync(void)
 *  {
 *      g_tcp_sync_hd = vm_tcp_connect_sync("123.123.123.123", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NON_PROXY);
 *  }
 * </code>
*****************************************************************************/
VM_TCP_HANDLE vm_tcp_connect_sync(const VMCHAR* ip, const VMINT port, VM_BEARER_DATA_ACCOUNT_TYPE data_account_type);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_read_sync
 * DESCRIPTION
 *  This function reads data from a TCP connection synchronously and it will blocked until it has
 * finished reading all data.
 * PARAMETERS
 *  handle : [IN] The TCP handle.
 *  buffer : [OUT]  The data buffer.
 *  length : [OUT] The data length.
 * RETURNS
 *  > 0: The size of the data to read.
 *  = 0: The server sent FIN.
 *  < 0: Error code in VM_TCP_RESULT
 * EXAMPLE
 * <code>
 *  static VMINT g_tcp_sync_hd;
 *  void hl_tcp_sync(void)
 *  {
 *      VMINT read_data;
 *      VMBYTE buf[1024];
 *      VMINT ret;
 *      g_tcp_sync_hd = vm_tcp_connect_sync("123.123.123.123", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NON_PROXY);
 *      read_data = vm_tcp_read_sync(g_tcp_sync_hd, buf, 1024);
 *      ret = vm_tcp_close_sync(g_tcp_sync_hd);
 *  }
 * </code>
 *****************************************************************************/
VMINT vm_tcp_read_sync(VM_TCP_HANDLE handle, void* buffer, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_write_sync
 * DESCRIPTION
 *  This function writes data to an open TCP connection synchronously and it will be blocked until
 * it has finished writing all data.
 * PARAMETERS
 *  handle : [IN] The TCP handle.
 *  buffer : [IN] The data to write.
 *  length : [IN] The data length.
 * RETURNS
 *  >= 0: The data size of the write data.
 *  < 0: Error code, please check VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 * static VMINT g_tcp_sync_hd;
 * #define send ("GET / HTTP/1.1\r\nHost: www.mediatek.com:80\r\n\r\n")
 * void hl_tcp_sync(void)
 * {
 *    VMINT read_data;
 *    VMINT ret;
 *    g_tcp_sync_hd = vm_tcp_connect_sync("123.123.123.123", 80, VM_BEARER_DATA_ACCOUNT_TYPE_NON_PROXY);
 *    ret = vm_tcp_write_sync(g_tcp_sync_hd, send, strlen(send));
 *    ret = vm_tcp_close_sync(g_tcp_sync_hd);
 * }
 * </code>
*****************************************************************************/
VMINT vm_tcp_write_sync(VM_TCP_HANDLE handle, void* buffer, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_close_sync
 * DESCRIPTION
 *  This function closes an open TCP connection synchronously and it will be blocked until it has finished closing the connections.
 * PARAMETERS
 *  handle : [IN] The TCP handle.
 * EXAMPLE
 * <code>
 *  static VMINT g_tcp_sync_hd;
 *  void hl_tcp_sync(void)
 *  {
 *      g_tcp_sync_hd = vm_tcp_connect_sync("123.123.123.123", 80, VM_BEARER_DATA_ACCOUNT_TYPE_NON_PROXY);
 *      vm_tcp_close_sync(g_tcp_sync_hd);
 *  }
 * </code>
*****************************************************************************/
VM_RESULT vm_tcp_close_sync(VM_TCP_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_callback
 * DESCRIPTION
 *  This is the Callback function type of TCP server.
 * PARAMETERS
 *  handle :    [IN] The TCP handle.
 *  event  :    [IN] The TCP server event.
 *  parameter : [IN] This is the client handle for the VM_TCP_SVR_EVT_ACCEPT event.
 *  user_data : [IN] The user data.
*****************************************************************************/
typedef void (*vm_tcp_server_callback)(VM_TCP_HANDLE handle, VM_TCP_EVENT event, VMINT parameter, void* user_data);
// [Liuxun]coding style needs to be void*.

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_init
 * DESCRIPTION
 *  This function initiates a server socket.
 * PARAMETERS
 *  apn :       [IN] The APN ID.
 *  port:       [IN] The server port.
 *  user_data:  [IN] The user data which will be passed to the callback.
 *  callback:   [IN] The callback function.
 * RETURNS
 *  On success - the TCP server handle.
 *  On failure - an error code in the VM_TCP_RESULT.
*****************************************************************************/
VM_TCP_SERVER_HANDLE vm_tcp_server_init(VM_BEARER_DATA_ACCOUNT_TYPE data_account_type, VMINT port, void* user_data, vm_tcp_server_callback callback);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_write
 * DESCRIPTION
 *  This function sends data from a server to a client.
 * PARAMETERS
 *  server_handle : [IN]The server handle.
 *  client_handle: [IN] The client handle.
 *  data: [IN] The data to be sent.
 *  length:   [IN] The data length.
 * RETURNS
 *  On success -  the number of written bytes.
 *  On failure - an error code in the VM_TCP_RESULT.
*****************************************************************************/
VMINT vm_tcp_server_write(VM_TCP_SERVER_HANDLE server_handle, VM_TCP_HANDLE client_handle, const void* data, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_read
 * DESCRIPTION
 *  This function represents the server that reads data received by a client.
 * PARAMETERS
 *  server_handle : [IN] The server handle.
 *  client_handle:  [IN] The client handle.
 *  data:           [IN] The data to be read.
 *  length:         [IN] The data length.
 * RETURNS
 *  On success - the number of bytes received.
 *  On failure - an error code in the VM_TCP_RESULT.
*****************************************************************************/
VMINT vm_tcp_server_read(VM_TCP_SERVER_HANDLE server_handle, VM_TCP_HANDLE client_handle, void* data, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_close_client
 * DESCRIPTION
 *  This function represents the server that closes the client.
 * PARAMETERS
 *  client_handle: [IN] The client handle.
 * RETURNS
 * On success - returns 0, anything else represents failure.
*****************************************************************************/
VM_RESULT vm_tcp_server_close_client(VM_TCP_HANDLE client_handle);


/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_deinit
 * DESCRIPTION
 *  This function de-initializes the socket server.
 * PARAMETERS
 *  handle: [IN] The server handle.
 * RETURNS
 * 0
*****************************************************************************/
VM_RESULT vm_tcp_server_release(VM_TCP_SERVER_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_init_sync
 * DESCRIPTION
 *  This function initiates socket server synchronously.
 * PARAMETERS
 *  apn:    [IN] The APN ID.
 *  port:   [IN] The Port number associated to the server.
 * RETURNS
 * >=0: server handle.
 * < 0: error code in the VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_sockaddr_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *  }
 * </code>
*****************************************************************************/
VM_TCP_SERVER_HANDLE vm_tcp_server_init_sync(VM_BEARER_DATA_ACCOUNT_TYPE apn, VMINT port);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_accept_sync
 * DESCRIPTION
 *  This function waits for the incoming connections and returns a new connection socket handle.
 *
 * PARAMETERS
 *  server_handle:  [IN] The server handle.
 *  address:        [OUT] The address of the remote host.
 * RETURNS
 * >=0: The TCP client handle.
 * < 0: an error code in the VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_ip_address_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *  }
 * </code>
*****************************************************************************/
VM_TCP_HANDLE vm_tcp_server_accept_sync(VM_TCP_SERVER_HANDLE server_handle, vm_soc_address_t* address);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_write_sync
 * DESCRIPTION
 *  This function sends data to a host which was previously connected.
 * PARAMETERS
 *  client_handle:  [IN] The client handle.
 *  data:           [IN] The data to be sent.
 *  length:         [IN] The data length.
 * RETURNS
 * >=0: the number of bytes sent
 * <0 : an error code in VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_ip_address_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *  }
 * </code>
*****************************************************************************/
VMINT vm_tcp_server_write_sync(VM_TCP_HANDLE client_handle, const void* data, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_read_sync
 * DESCRIPTION
 *  This function reads data synchronously.
 * PARAMETERS
 *  client_handle:  [IN] The client handle.
 *  data:           [IN]  The buffer for read data.
 *  length:         [IN]  The buffer length.
 * RETURNS
 *  >= 0: the number of bytes read.
 *  < 0: an error code in VM_TCP_RESULT.
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_sockaddr_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *  }
 * </code>
*****************************************************************************/
VMINT vm_tcp_server_read_sync(VM_TCP_HANDLE client_handle, void* data, VMUINT length);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_close_client_sync
 * DESCRIPTION
 *  This function closes the client socket synchronously.
 * PARAMETERS
 *  client_handle: [IN] The client handle
 * RETURNS
 *  VM_TCP_RESULT
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_sockaddr_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *  }
 * </code>
*****************************************************************************/
VM_RESULT vm_tcp_server_close_client_sync(VM_TCP_HANDLE client_handle);

/*****************************************************************************
 * FUNCTION
 *    vm_tcp_server_release_sync
 * DESCRIPTION
 *  This function de-initiates socket server synchronously.
 * PARAMETERS
 *  server_handle: [IN] The server handle
 * RETURNS
 *  VM_TCP_RESULT
 * EXAMPLE
 * <code>
 *  void hl_tcp_sync_srv(void)
 *  {
 *      vm_ip_address_t addr;
 *      VMINT ret;
 *      char buf[1024] = {0};
 *      VMINT hdl_c;
 *      VMINT hdl_s = vm_tcp_server_init_sync(1, 3500);
 *      hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
 *      ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
 *      ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
 *      vm_tcp_server_close_client_sync(hdl_c);
 *      vm_tcp_server_release_sync(hdl_s);
 *  }
 * </code>
*****************************************************************************/
VM_RESULT vm_tcp_server_release_sync(VM_TCP_SERVER_HANDLE server_handle);


#ifdef __cplusplus
}
#endif

#endif /* VMTCP_SDK_H */

