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

#ifndef VMMEMORY_SDK_H
#define VMMEMORY_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/*****************************************************************************
 * FUNCTION
 *    vm_malloc
 * DESCRIPTION
 *    Allocates the specified size of memory. Uses vm_pmng_get_memory_status()
 *    to query the available max size of memory that can be allocated. User should
 *    configure the memory pool size of the VXP. The max size of available memory
 *    may not be allocated due to memory fragmentation.
 * PARAMETERS
 *    size : [IN] The number of bytes to be allocated.
 * RETURNS
 *    A pointer to the memory allocated. NULL if allocation failed.
 *****************************************************************************/
void* vm_malloc(VMUINT size);

/*****************************************************************************
 * FUNCTION
 *    vm_calloc
 * DESCRIPTION
 *    Allocates the specified size of memory and clear the memory with zeros.
 * PARAMETERS
 *    size : [IN] The number of bytes to be allocated.
 * RETURNS
 *    A pointer to the memory allocated. NULL if allocation failed.
 *****************************************************************************/
void* vm_calloc(VMUINT size);

/*****************************************************************************
 * FUNCTION
 *    vm_realloc
 * DESCRIPTION
 *    Reallocates the specified size of memory for the memory already allocated before.
 *    The content of the memory will be "copied".
 * PARAMETERS
 *    ptr :  [IN] The pointer to the memory to be copied. If allocation succeeds,
 *           the memory pointed to by the ptr will be freed.
 *    size : [IN] The number of bytes to be allocated.
 * RETURNS
 *    A pointer to the memory allocated. Returns NULL if allocation failed, and
 *    the memory pointed to by the ptr will not be freed.
 *****************************************************************************/
void* vm_realloc(void* ptr, VMUINT size);

/*****************************************************************************
 * FUNCTION
 *    vm_free
 * DESCRIPTION
 *    Frees the memory allocated by vm_malloc(), vm_calloc(), vm_realloc(),
 *    or vm_malloc_dma(). Frees null is allowed.
 * PARAMETERS
 *    ptr : [IN] The pointer to the memory to be freed. It can be NULL.
 * RETURNS
 *    void
 *****************************************************************************/
void vm_free(void* ptr);

/*****************************************************************************
 * FUNCTION
 *    vm_malloc_dma
 * DESCRIPTION
 *    Allocates non-cacheable (e.g. DMA - Direct Memory Access), which provide
 *    faster memory access. This function is mostly used by graphic related features.
 * PARAMETERS
 *    size : [IN] The number of bytes to be allocated.
 * RETURNS
 *    A pointer to the memory allocated. Returns NULL if allocation failed.
 *****************************************************************************/
void* vm_malloc_dma(VMUINT size);


#ifdef __cplusplus
}
#endif

#endif /* VMSYS_SDK_H_ */
