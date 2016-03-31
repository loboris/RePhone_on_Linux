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

#ifndef VMBEARER_SDK_H
#define VMBEARER_SDK_H

#ifdef __cplusplus
extern "C" {
#endif
#include "vmtype.h"

typedef VMINT VM_BEARER_HANDLE;

typedef enum
{
    VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN = 1, /* only use gprs apn which donot have proxy */
    VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_PROXY_APN = 2, /* only use gprs apn which has a proxy */
    VM_BEARER_DATA_ACCOUNT_TYPE_WLAN = 3, /* use wlan to connect */
    VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN = 6, /* use customized apn */
    VM_BEARER_DATA_ACCOUNT_TYPE_MAX = 0x7FFFFFFF
} VM_BEARER_DATA_ACCOUNT_TYPE;

/* This enum is used to define bearer status */
typedef enum
{
    VM_BEARER_DEACTIVATED             = 0x01, /* bearer deactivated */
    VM_BEARER_ACTIVATING              = 0x02, /* bearer activating */
    VM_BEARER_ACTIVATED               = 0x04, /* bearer activated */
    VM_BEARER_DEACTIVATING            = 0x08, /* bearer deactivating */
    VM_BEARER_BEARER_STATE_MAX = 0x7FFFFFFF
} VM_BEARER_STATE;

/* network bearer result */
typedef enum
{
    VM_BEARER_OK                  = 0,  /* success */
    VM_BEARER_ERROR               = -1, /* general error */
    VM_BEARER_WOULDBLOCK          = -2, /* would block */
    VM_BEARER_LIMIT_RESOURCE      = -3, /* limited resource */
    VM_BEARER_INVALID_ACCOUNT_ID  = -4, /* invalid account id*/
    VM_BEARER_BEARER_FAIL         = -7, /* bearer fail */
    VM_BEARER_DHCP_ERROR          = -8, /* DHCP get IP error */
    VM_BEARER_IPV6_ERROR          = -11,/* IP get V6 address error */
    VM_BEARER_RESULT_MAX = 0x7FFFFFFF
} VM_BEARER_RESULT;

/* ip type */
typedef enum
{
    VM_BEARER_IP_NONE = 0,
    VM_BEARER_IPV4    = 0x01,  /* IPv4 */
    VM_BEARER_IPV6    = 0x02,  /* IPv6 */
    VM_BEARER_IPV4V6  = VM_BEARER_IPV4 | VM_BEARER_IPV6, /* IPv4v6 */
    VM_BEARER_IP_TYPE_MAX = 0x7FFFFFFF,
} VM_BEARER_IP_TYPE;

/*****************************************************************************
 * FUNCTION
 *  vm_bearer_get_data_account_id
 * DESCRIPTION
 *  this function retrieves data account id.
 * PARAMETERS
 * data_account_type : [IN] data account type
 * data_account_id : [OUT] point to data account id.
 * RETURNS
 *  VM_BEARER_RESULT.
 * RETURN VALUES
 *  VM_E_SOC_SUCCESS :               Get IP address successfully, result is filled.
 *  VM_E_SOC_INVAL :                     invalid arguments: null domain_name, etc.
 *  VM_E_SOC_INVALID_ACCOUNT :      invalid data account id
 * EXAMPLE
 * <code>
 *  void vat_vmsock_dtacct_1(void)
 *  {
 *      VMINT ret = -1;
 *      VMINT apn = VM_TCP_APN_CMNET;
 *      VMUINT dtacct_id;
 *
 *      if (vm_wifi_is_connected()) {
 *          apn = VM_TCP_APN_WIFI;
 *      }
 *
 *      ret = vm_bearer_get_data_account_id(apn, &dtacct_id);
 *  }
 * </code>
*****************************************************************************/
VM_RESULT vm_bearer_get_data_account_id(VM_BEARER_DATA_ACCOUNT_TYPE data_account_type, VMUINT* data_account_id);

typedef void (*vm_bearer_open_callback)(VM_BEARER_HANDLE handle, VM_BEARER_STATE event, VMUINT data_account_id, void* user_data);
/*****************************************************************************
 * FUNCTION
 *  vm_bearer_open
 * DESCRIPTION
 *  Open the bearer.
 *
 * PARAMETERS
 *  data_account_type      : [IN] data account type
 *  user_data      : [IN]   user data
 *  callback_ex : [IN] callback function
 *  ip : [IN] ip type vm_cbm_ip_type_enum
 * RETURN
 * >= 0: bearer handle, the bearer is opened successfully.
 * VM_BEARER_WOULDBLOCK: please check the bearer state in callback.
 * < 0: error code in VM_BEARER_RESULT.
 *****************************************************************************/
VM_BEARER_HANDLE vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE data_account_type, void* user_data, vm_bearer_open_callback callback, VM_BEARER_IP_TYPE ip_type);


/*****************************************************************************
 * FUNCTION
 *  vm_bearer_close
 * DESCRIPTION
 *  This API is used to release all bearers which are opened by this application.
 *  The bearer will be released only when all sockets are closed.
 *  If the sockets are all closed and this API is called, the bearer will be
 *  deactivated. Otherwise, the bearer will be deactivated only when
 *  all sockets which belong to this application are closed.
 *  If the bearer is opened by application directly through the API
 *  "cbm_open_bearer", the bearer can be directly deactivated when
 *  application calls the API cbm_release_bearer.
 *
 *  P.S. We assume applications shall set its module id or register the application id
 *  before calling this API.
 *
 * PARAMETERS
 *  handle      : [IN]    bearer handle
 * RETURN VALUES
 *  CBM_OK : bearer is in deactivated state.
 *  CBM_INVALID_AP_ID : invalid application id.
 *  CBM_WOULDBLOCK : the bearer is in deactivating state and the bearer info
 *                   will be reported later.
 * EXAMPLE
 *  <code>
 *  kal_uint8 ap_id;
 *  kal_int8 ret;
 *  ap_id = cbm_get_app_id ((kal_uint32)account_id);
 *  ...
 *  if ((ret = cbm_release_bearer (ap_id)) == CBM_OK)
 *  {
 *    //bearer is deactivated
 *  }
 *  else
 *  {
 *      if (ret == CBM_WOULDBLOCK)
 *      {
 *          //bearer is not completely deactivated yet
 *      }
 *  }
 *  </code>
 *  After the bearer is deactivated:
 *  <code>
 *  app_cbm_bearer_info_ind_hdlr(local_para_struct *msg)
 *  {
 *      app_cbm_bearer_info_ind_struct *ind_ptr;
 *      ind_ptr = (app_cbm_bearer_info_ind_struct *)local_para_ptr;
 *      switch (ind_ptr->state)
 *      {
 *          case CBM_DEACTIVATED:
 *               app_bearer_deactivated_hdlr();
 *               break;
 *          ...
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_bearer_close(VM_BEARER_HANDLE handle);

#ifdef __cplusplus
}
#endif
#endif

