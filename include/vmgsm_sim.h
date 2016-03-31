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

#ifndef VMGSM_SIM_SDK_H
#define VMGSM_SIM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* The return type of vm_get_sim_card_status(). */
typedef enum
{
    VM_GSM_SIM_STATUS_ERROR   = -1,        /* Failed to get status. */
    VM_GSM_SIM_STATUS_VACANT  = 0,         /* No SIM card is detected or the SIM card is not working. */
    VM_GSM_SIM_STATUS_WORKING = 1,         /* The SIM card is working. */
    VM_GSM_SIM_STATUS_MAX     = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_SIM_STATUS;

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_iccid_callback
 * DESCRIPTION
 *    A Callback declaration to get the ICC ID.
 * PARAMETERS
 *    iccid:     [OUT] The ICC ID retrieved by this callback.
 *    user_data: [IN]  The data content to be used by this callback.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_gsm_sim_get_iccid_callback)(VMCHAR* iccid, void* user_data);

typedef enum
{
    VM_GSM_SIM_NO_SIM = 0,         /* No SIM card */
    VM_GSM_SIM_SIM1   = 1,         /* SIM card 1 */
    VM_GSM_SIM_ID_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_SIM_ID;

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_card_count
 * DESCRIPTION
 *    Gets the total number of SIM cards in the device.
 * PARAMETERS
 *    void
 * RETURNS
 *    The total number of SIM cards in the device.
 *    If there is no SIM card in the device, it will return 0.
 *****************************************************************************/
VMINT vm_gsm_sim_get_card_count(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_active_sim_card
 * DESCRIPTION
 *    Gets the active SIM card.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_GSM_SIM_ID. If there is no SIM card, it returns 0.
 *****************************************************************************/
VM_GSM_SIM_ID vm_gsm_sim_get_active_sim_card(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_max_supported_card_count
 * DESCRIPTION
 *    Gets the maximum number of SIM cards the device can support.
 * RETURNS
 *    An integer, represents the maximum number of the SIM cards the device can support.
 *****************************************************************************/
VMINT vm_gsm_sim_get_max_supported_card_count(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_imei
 * DESCRIPTION
 *    Retrieves the IMEI number of the device.
 * PARAMETERS
 *    sim_card_id: [IN] The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 * RETURNS
 *    An ASCII string of the IMEI number of the device. If NULL is returned, the
 *    caller should try again to get the value.
 *****************************************************************************/
VMCSTR vm_gsm_sim_get_imei(VM_GSM_SIM_ID sim_card_id);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_card_status
 * DESCRIPTION
 *    Gets the status of a given SIM card.
 * PARAMETERS
 *    sim_card_id: [IN] The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 * RETURNS
 *    VM_GSM_SIM_STATUS, the status of the given SIM card.
 *****************************************************************************/
VM_GSM_SIM_STATUS vm_gsm_sim_get_card_status(VM_GSM_SIM_ID sim_card_id);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_has_card
 * DESCRIPTION
 *    Queries the device if a SIM card is available in the device or not.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_TRUE, if there is a SIM card in the device is available. VM_FALSE, if
 *    there is no SIM card available.
 *****************************************************************************/
VMBOOL vm_gsm_sim_has_card(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_set_active_sim_card
 * DESCRIPTION
 *    Sets the SIM card, as specified by sim_card_id, to active.
 * PARAMETERS
 *    sim_card_id: [IN] The Index of the SIM card, e.g. VM_GSM_SIM_SIM1, to be
 *                      set to active.
 * RETURNS
 *    VM_TRUE, if the specified SIM card is set to active successfully. Otherwise,
 *    it returns VM_FALSE.
 *****************************************************************************/
VMBOOL vm_gsm_sim_set_active_sim_card(VM_GSM_SIM_ID sim_card_id);

#define VM_GSM_SIM_MAX_PLMN_LENGTH 6
/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_home_plmn
 * DESCRIPTION
 *    Gets the PLMN of the home network of the specified SIM card.
 * PARAMETERS
 *    sim_card_id: [IN]  The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 *    buffer:      [OUT] The buffer to be filled with the PLMN, retrieved from the device.
 *    buffer_size: [IN]  The size of buffer. It shall be greater than or equal to
 *                       (VM_GSM_SIM_MAX_PLMN_LENGTH + 1).
 * RETURNS
 *    It returns 0, if this function can find a valid PLMN. A negative number is
 *    returned if there is an error. The error code -2 means a parameter error.
 *    The error code -3 means no SIM card. The error code -4 means internal error.
 *    The error code -5 means not supported SIM card.
 *****************************************************************************/
VM_RESULT vm_gsm_sim_get_home_plmn(VM_GSM_SIM_ID sim_card_id, VMCHAR* buffer, VMUINT buffer_size);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_iccid_with_sim
 * DESCRIPTION
 *    Gets the ICC ID of the specified SIM card. The callback function will set the
 *    iccid when it returns.
 * PARAMETERS
 *    sim_card_id: [IN]  The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 *    iccid:       [OUT] The buffer to store the value of iccid that is retrieved by the callback.
 *    iccid_size:  [IN]  The size of iccid buffer in bytes.
 *    user_data:   [IN]  The data content to be used by the callback.
 * RETURNS
 *    It returns 0, if this function gets the iccid successfully. A negative number is
 *    returned if there is an error. The error code -2 means a parameter error.
 *    The error code -3 means no SIM card. The error code -4 means internal error.
 *    The error code -5 means not supported SIM card.
 *****************************************************************************/
VM_RESULT vm_gsm_sim_get_iccid_with_sim(VM_GSM_SIM_ID sim_card_id, vm_gsm_sim_get_iccid_callback callback,
                                        VMSTR iccid, VMINT iccid_size, void* user_data);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_imsi
 * DESCRIPTION
 *    Retrieves the IMSI number of the specified SIM card.
 * PARAMETERS
 *    sim_card_id: [IN] The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 * RETURNS
 *    The IMSI string of the specified SIM card. If NULL is returned, the caller
 *    should try again to get the value.
 *****************************************************************************/
VMCSTR vm_gsm_sim_get_imsi(VM_GSM_SIM_ID sim_card_id);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_sim_get_network_plmn
 * DESCRIPTION
 *    Gets network PLMN for the specified SIM card.
 * PARAMETERS
 *    sim_card_id: [IN]  The Index of the SIM card, e.g. VM_GSM_SIM_SIM1.
 *    buffer:      [OUT] The buffer to be filled with the PLMN, retrieved from the device.
 *    buffer_size: [IN]  The size of buffer. It shall be greater than or equal to
 *                       (VM_GSM_SIM_MAX_PLMN_LENGTH + 1).
 * RETURNS
 *    It returns 0, if this function can find a valid PLMN. Otherwise, a non-zero
 *    is returned.
 *****************************************************************************/
VMINT vm_gsm_sim_get_network_plmn(VM_GSM_SIM_ID sim_card_id, VMCHAR* buffer, VMUINT buffer_size);



#ifdef __cplusplus
}
#endif

#endif /* VMSIM_H_ */

