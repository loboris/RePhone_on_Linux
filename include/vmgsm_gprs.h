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

#ifndef VMGSM_GPRS_SDK_H
#define VMGSM_GPRS_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* The handle type */
typedef enum
{
    VM_GSM_GPRS_HANDLE_TYPE_TCP = 0,    /* The TCP handle. */
    VM_GSM_GPRS_HANDLE_TYPE_UDP = 1,    /* The UDP handle. */
    VM_GSM_GPRS_HANDLE_TYPE_HTTP = 2,   /* The HTTP handle. */
    VM_GSM_GPRS_HANDLE_TYPE_HTTPS = 3,  /* The HTTPS handle. */
    VM_GSM_GPRS_HANDLE_TYPE_MAX = 0x7FFFFFFF /* The 'ensure the compiler treat this enum as 32-bit integer' handle. */
} VM_GSM_GPRS_HANDLE_TYPE;

#define VM_GSM_GPRS_APN_MAX_LENGTH  31
#define VM_GSM_GPRS_APN_MAX_PROXY_ADDRESS_LENGTH    63
#define VM_GSM_GPRS_APN_MAX_USERNAME_LENGTH      31
#define VM_GSM_GPRS_APN_MAX_PASSWORD_LENGTH 31
/* The proxy type */
typedef enum
{
    /*
     * CL-WSP               9200  WAP
     * CO-WSP               9201  WAP
     * CL-SEC-WSP           9202  WAP + Security ON
     * CO-SEC-WSP           9203  WAP + Security ON
     * CO-SEC-WTA           not supported
     * CL-SEC-WTA           not supported
     * OTA-HTTP-TO/PO       80    HTTP
     * OTA-HTTP-TLS-TO/PO   443   HTTP + Security ON
     */
    VM_GSM_GPRS_PROXY_NONE  = 0,    /* The 'not specified' type. */
    VM_GSM_GPRS_PROXY_CL_WSP,       /* The WSP, Connection less type. */
    VM_GSM_GPRS_PROXY_CO_WSP,       /* The WSP, Connection oriented type. */
    VM_GSM_GPRS_PROXY_CL_SEC_WSP,   /* The WSP, Connection less, security mode type. */
    VM_GSM_GPRS_PROXY_CO_SEC_WSP,   /* The WSP, Connection oriented, security mode type. */
    VM_GSM_GPRS_PROXY_CL_SEC_WTA,   /* The WTA, Connection less, security mode type. */
    VM_GSM_GPRS_PROXY_CO_SEC_WTA,   /* The WTA type, Connection oriented, security mode type. */
    VM_GSM_GPRS_PROXY_HTTP,         /* The HTTP type. */
    VM_GSM_GPRS_PROXY_HTTP_TLS,     /* The HTTP - enable TLS type. */
    VM_GSM_GPRS_PROXY_STARTTLS,     /* The STARTTLS type */
    VM_GSM_GPRS_PROXY_MAX = 0x7FFFFFFF /* The 'ensure the compiler treat this enum as 32-bit integer' type. */
} VM_GSM_GPRS_PROXY_TYPE;



/* The APN information struct */
typedef struct
{
    VMCHAR apn[VM_GSM_GPRS_APN_MAX_LENGTH + 1];                         /* The APN name.*/
    VMBOOL using_proxy;                                                 /* The boolean to determine if a proxy server is used. */
    VM_GSM_GPRS_PROXY_TYPE proxy_type;                                  /* The proxy server type. */
    VMCHAR proxy_address[VM_GSM_GPRS_APN_MAX_PROXY_ADDRESS_LENGTH + 1]; /* The proxy server address type. */
    VMUINT16 proxy_port;                                                /* The proxy server port type. */
    VMCHAR proxy_username[VM_GSM_GPRS_APN_MAX_USERNAME_LENGTH + 1];     /* The roxy server User name. */
    VMCHAR proxy_password[VM_GSM_GPRS_APN_MAX_PASSWORD_LENGTH + 1];     /* The proxy server Password. */
} vm_gsm_gprs_apn_info_t;

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_gprs_get_apn_info
 * DESCRIPTION
 *  This function gets the APN information of the specified data_account_id.
 * PARAMETERS
 *  data_account_id: [IN]
 *  apn_info : [OUT] The APN information.
 * RETURNS
 *  VM_RESULT
*****************************************************************************/
VM_RESULT vm_gsm_gprs_get_apn_info(VMUINT data_account_id, vm_gsm_gprs_apn_info_t* apn_info);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_gprs_hold_bearer
 * DESCRIPTION
 *  This function holds the bearer.
 * PARAMETERS
 *  type : [IN] VM_GSM_GPRS_HANDLE_TYPE
 *  handle : [IN]   TCP     : handle = vm_tcp_connect.
 *                  UDP     : handle = vm_udp_create.
 *                  HTTP    : handle = vm_http_get_current_handle.
 *                            handle = vm_http_fetch_handle.
 *                  HTTPS   : This uses the channel ID which is attained from the vm_https_set_channel_response_callback.
 * RETURN VALUES
 *  VM_RESULT
 * EXAMPLE
 * <code>
 *  static VMINT g_tcp_hd;
 *
 *  void connection_manager(VMBOOL connect){
 *      if(connect){
 *          g_tcp_hd = vm_tcp_connect("http://www.mediatek.com", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN, tcp_cb);
 *      }
 *      else{
 *          vm_gsm_gprs_release_bearer();
 *      }
 *  }
 *
 *  void tcp_cb(VMINT handle, VMINT event)
 *  {
 *      VMINT ret = 0;
 *      if (VM_TCP_EVENT_CONNECTED == event || VM_TCP_EVENT_CAN_WRITE == event)
 *      {
 *          ret = vm_gprs_hold_bearer(VM_GSM_GPRS_HANDLE_TYPE_TCP, handle);
 *          ret = vm_tcp_write(handle, "GET / HTTP/1.1\r\nHOST:www.mediate.com:80\r\n\r\n", strlen("GET / HTTP/1.1\r\nHOST:www.mediate.com:80\r\n\r\n"));
 *      }
 *      if (VM_TCP_EVENT_CAN_READ == event)
 *      {
 *          VMCHAR buf[2048] = {0};
 *          ret = vm_tcp_read(handle, buf, 2048);
 *          vm_tcp_close(handle);
 *      }
 *  }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_gprs_hold_bearer(VM_GSM_GPRS_HANDLE_TYPE handle_type, VMINT handle);




/*****************************************************************************
 * FUNCTION
 *  vm_gsm_gprs_release_bearer
 * DESCRIPTION
 *  This function release the bearer.
 * PARAMETERS
 * RETURN VALUES
 * VM_RESULT
 * EXAMPLE
 * <code>
 *  static VMINT g_tcp_hd;
 *
 *  void connection_manager(VMBOOL connect){
 *      if(connect){
 *          g_tcp_hd = vm_tcp_connect("http://www.mediatek.com", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN, tcp_cb);
 *      }
 *      else{
 *          vm_gsm_gprs_release_bearer();
 *      }
 *  }
 *
 *  void tcp_cb(VMINT handle, VMINT event)
 *  {
 *      VMINT ret = 0;
 *      if (VM_TCP_EVENT_CONNECTED == event || VM_TCP_EVENT_CAN_WRITE == event)
 *      {
 *          ret = vm_gprs_hold_bearer(VM_GSM_GPRS_HANDLE_TYPE_TCP, handle);
 *          ret = vm_tcp_write(handle, "GET / HTTP/1.1\r\nHOST:www.mediate.com:80\r\n\r\n", strlen("GET / HTTP/1.1\r\nHOST:www.mediate.com:80\r\n\r\n"));
 *      }
 *      if (VM_TCP_EVENT_CAN_READ == event)
 *      {
 *          VMCHAR buf[2048] = {0};
 *          ret = vm_tcp_read(handle, buf, 2048);
 *          vm_tcp_close(handle);
 *      }
 *  }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_gprs_release_bearer(void);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_gprs_set_customized_apn_info
 * DESCRIPTION
 *  This function sets customized APN information.
 * PARAMETERS
 *  info : [IN] For customized APN information, please refer to vm_gsm_gprs_apn_info_t.
 * RETURN VALUES
 *  VM_RESULT
 * EXAMPLE
 * <code>
 * static VMINT g_hdl;
 *
 * void callback(VMINT handle, VMINT event)
 * {
 *	  vm_trace("[%d][%d]", handle, event);
 * }
 *
 *  void handle_keyevt(VMINT event, VMINT keycode) {
 *
 *	vm_gsm_gprs_apn_info_t info = {0};
 *
 *	VMUINT32 dtacct = 0;
 *	VMINT ret = 0;
 *	strcpy(info.apn, "cust apn");
 *	strcpy(info.proxy_address, "10.0.0.172");
 *	info.proxy_port = 80;
 *	info.using_proxy = 1;
 *	ret = vm_gsm_gprs_set_customized_apn_info(&info);
 *	ret = vm_gsm_gprs_get_default_apn_info(&info);
 *
 *	if (0 != strcmp(info.proxy_address, "0.0.0.0") && 0 != info.proxy_port)
 *	{
 *		ret = vm_tcp_connect((const char*)info.proxy_address, info.proxy_port, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_PROXY_APN, callback);
 *		if (ret >= 0)
 *		{
 *			g_hdl = ret;
 *		}
 *	}
 *	else
 *	{
 *		ret = vm_tcp_connect((const char*)"www.mediatek.com", 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_PROXY_APN, callback);
 *		if (ret >= 0)
 *		{
 *			g_hdl = ret;
 *		}
 *	}
 *	ret = 0;
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_gsm_gprs_set_customized_apn_info(const vm_gsm_gprs_apn_info_t* info);

/*****************************************************************************
 * FUNCTION
 *  vm_gsm_gprs_switch_mode
 * DESCRIPTION
 *  This function switches GPRS on or off.
 * PARAMETERS
 *  on:     [IN] To switch GPRS on or not.
*****************************************************************************/
void vm_gsm_gprs_switch_mode(VMBOOL on);

#ifdef __cplusplus
}
#endif

#endif /* VMGSM_GPRS_SDK_H */
