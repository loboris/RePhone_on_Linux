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

#ifndef VMGSM_CELL_SDK_H
#define VMGSM_CELL_SDK_H

#include "vmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cell info struct. */
typedef struct
{
    VMUINT16    arfcn;  /* ARFCN. */
    VMUINT8     bsic;   /* BSIC. */
    VMUINT8     rxlev;  /* Received signal level. */
    VMUINT16    mcc;    /* MCC. */
    VMUINT16    mnc;    /* MNC. */
    VMUINT16    lac;    /* LAC. */
    VMUINT16    ci;     /* CI. */
} vm_gsm_cell_info_t;

/* Cell open result */
typedef enum
{
    VM_GSM_CELL_OPEN_SUCCESS            = 0,    /* Opens successfully. */
    VM_GSM_CELL_OPEN_ALREADY_OPEN       = 1,    /* Already opened. */
    VM_GSM_CELL_OPEN_ERROR_NO_RESOURCE  = -1,   /* Lack of resource. */
    VM_GSM_CELL_OPEN_ERROR_REGISTER     = -2,   /* Registration error. */
    VM_GSM_CELL_RESULT_MAX = 0x7FFFFFFF  /* Forces the compiler to use 4-byte integer for enum. */
} VM_GSM_CELL_RESULT;

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_cell_open
 * DESCRIPTION
 *    Opens CELL ID resource.
 * PARAMETERS
 *    void
 * RETURNS
 *    VM_RESULT. It returns 0 if the open operation is successful.
 *    Refer to VM_GSM_CELL_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_gsm_cell_open(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_cell_close
 * DESCRIPTION
 *    Closes CELL ID resource.
 * PARAMETERS
 *    void
 * RETURNS
 *    void
 *****************************************************************************/
void vm_gsm_cell_close(void);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_cell_get_current_cell_info
 * DESCRIPTION
 *    Gets current cell information. The vm_gsm_cell_open() should be called
 *    before calling this function. Otherwise, NULL will be returned to cell_info.
 * PARAMETERS
 *    cell_info: [OUT] Contains the current cell information after this function returns.
 * RETURNS
 *    VM_RESULT. Refer to VM_GSM_CELL_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_gsm_cell_get_current_cell_info(vm_gsm_cell_info_t* cell_info);


/*****************************************************************************
 * FUNCTION
 *    vm_gsm_cell_get_neighbor_cell_info
 * DESCRIPTION
 *    Gets neighbor cell information.  The vm_gsm_cell_open() should be called
 *    before calling this function. Otherwise, NULL will be returned to cell_info.
 * PARAMETERS
 *    cell_info:  [OUT] Contains the neighbor cell information after this function returns.
 *    cell_index: [IN]  The index number of the neighbor cells. The total number
 *                      of neighbor cells is obtained by calling vm_gsm_cell_get_neighbor_number().
 *                      If there are 3 neighbor cells, the range of index will be
 *                      0, 1 and 2.
 * RETURNS
 *    VM_RESULT. Refer to VM_GSM_CELL_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_gsm_cell_get_neighbor_cell_info(vm_gsm_cell_info_t* cell_info, VMINT cell_index);

/*****************************************************************************
 * FUNCTION
 *    vm_gsm_cell_get_neighbor_number
 * DESCRIPTION
 *    Gets the total number of neighbor cells. The vm_gsm_cell_open() should be called
 *    before calling this function.
 * PARAMETERS
 *    void
 * RETURNS
 *    The total number of neighbor cells.
 *****************************************************************************/
VMINT vm_gsm_cell_get_neighbor_number(void);

#ifdef __cplusplus
}
#endif
#endif /* VMGSM_CELL_SDK_H */

