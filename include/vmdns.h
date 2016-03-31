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

#ifndef VMDNS_SDK_H
#define VMDNS_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmgsm_gprs.h"
#include "vmbearer.h"

/* The DNS handle */
typedef VMINT VM_DNS_HANDLE;

/* The DNS maximum address length */
#define VM_SOC_DNS_MAX_ADDRESS 5

/* The DNS query result struct */
typedef struct
{
    VMUINT address[VM_SOC_DNS_MAX_ADDRESS];     /* The IP address. */
    VMINT number;                               /* The number of result. */
    VMINT error_cause;                          /* The vm_ps_cause_enum */
} vm_dns_result_t;

/* DNS result */
typedef enum
{
    VM_DNS_SUCCESS = 0,             /* The success result. */
    VM_DNS_ERROR = -1,              /* The general error result. */
    VM_DNS_INVALID_PARAMETER = -2,  /* The parameter is invalid result. */
    VM_DNS_LIMIT_RESOURCE = -3,     /* The resource is used up result. */
} VM_DNS_RESULT;

/*****************************************************************************
 * FUNCTION
 *  vm_dns_get_host_by_name_callback
 * DESCRIPTION
 *  This is the callback function of vm_dns_get_host_by_name, it will be called if it doesn't get the DNS result immediately.
 * PARAMETERS
 * handle :     [IN] The DNS handle.
 * result :     [IN] The DNS result.
 * user_data :  [IN] The user data.
 * RETURNS
 *  VM_RESULT
*****************************************************************************/
typedef VM_RESULT (*vm_dns_get_host_by_name_callback)(VM_DNS_HANDLE handle, vm_dns_result_t* result, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_dns_get_host_by_name
 * DESCRIPTION
 *  This function retrieves the IP addresses associated with the specified host name.
 * PARAMETERS
 * data_account_type :  [IN] The data account type bearer.
 * host :               [IN] The domain to be resolved.
 * result :             [OUT] The DNS parsed result.
 * callback :           [IN] The callback function.
 * user_data :          [IN] The ser data.
 * RETURNS
 *  VM_DNS_HANDLE
*****************************************************************************/
VM_DNS_HANDLE vm_dns_get_host_by_name(VM_BEARER_DATA_ACCOUNT_TYPE data_account_type,
                                      const VMCHAR* host,
                                      vm_dns_result_t* result,
                                      vm_dns_get_host_by_name_callback callback, void* user_data);

#ifdef __cplusplus
}
#endif
#endif
