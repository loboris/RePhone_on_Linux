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

#ifndef VMTAG_SDK_H
#define VMTAG_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* Error codes for vm_tag_get_tag. */
typedef enum
{
    VM_TAG_SUCCESS = VM_SUCCESS,        /* Get tag successfully. */
    VM_TAG_FAIL = VM_FAIL,              /* Failed to get tag. */
    VM_TAG_INSUFFICIENT_BUFFER = -2,    /* Input buffer too short. */
    VM_TAG_NOT_FOUND = -3,              /* Tag does not exist, e.g. wrong TAG
                                           ID is used, or the tag is not
                                           defined by Project Setting tool.
                                         */
    VM_TAG_RESULT_MAX = 0x7FFFFFFF
} VM_TAG_RESULT;

/* TAG ID definition. Modify content of tag in Project Setting tool. */
typedef enum
{
    VM_TAG_ID_DEVELOPER          = 0x01, /* Developer name in VMWSTR. */
    VM_TAG_ID_APP_ID             = 0x02, /* Application ID in VMINT. */
    VM_TAG_ID_APP_NAME           = 0x04, /* Application name in VMWSTR. */
    VM_TAG_ID_VERSION            = 0x05, /* Application version in VMINT.
                                            The digits in version info
                                            are stored in each byte,
                                            starting from the second
                                            least significant byte.
                                            For example, if "2.7.8" is set in
                                            the Project Setting tool, the
                                            integer value is 0x08070200.
                                          */
    VM_TAG_ID_MEMORY_REQUIREMENT = 0x0f, /* Required bytes in VMINT. */
    VM_TAG_ID_MAX = 0x7FFFFFFF
} VM_TAG_ID;

/*****************************************************************************
* FUNCTION
*    vm_tag_get_tag
* DESCRIPTION
*  Get one tag from a given LinkIt application. This API gets information from
*  VXP files on disk, or the current running VXP application.
* PARAMETERS
*  filename : [IN] Path to a VXP file to retrieve tag from;
*                  Pass NULL for current application.
*  tag_id : [IN] Refer to VM_TAG_ID for tag information to retrieve.
*  buffer : [OUT] Value of the tag assigned by tag_id. Pass NULL to
*                 retrieve required buffer size from buffer_size.
*                 Refer to VM_TAG_ID for data type of the tag.
*  buffer_size : [OUT] Required buffer size for the tag information.
* RETURNS
*  VM_RESULT. Refer to VM_TAG_RESULT for detailed error codes.
*
* EXAMPLE
*	  <code>
*     VMINT appVersion = 0;     // Version is VMINT
*     VMWSTR appName = NULL;    // Application Name is VMWSTR
*     VMINT reqSize = 0;
*
*     // Application version is VMINT, always 4 bytes.
*     if(VM_IS_SUCCEEDED(vm_tag_get_tag(NULL,
*                                       VM_TAG_ID_VERSION,
*                                       &appVersion,
*                                       sizeof(appVersion))))
*     {
*       vm_log_info("version=%d.%d.%d", (appVersion >> 1) & 0xFF,
*                                       (appVersion >> 2) & 0xFF,
*                                       (appVersion >> 3) & 0xFF);
*     }
*
*	  // Get required buffer size for Application Name information.
* 	  if (VM_IS_SUCCEEDED(vm_tag_get_tag(NULL,
*                                        VM_TAG_ID_APP_NAME,
*                                        NULL,
*                                        &reqSize)))
*	  {
*		appName = (VMWSTR*)vm_calloc(reqSize);
*		if(appName)
*		{
*			if(VM_IS_SUCCEEDED(vm_tag_get_tag(NULL,
*                                             VM_TAG_ID_APP_NAME,
*                                             appName,
*                                             &reqSize)))
*			{
*				// You can then proceeed to
*			}
*		}
*	  }
*     // free it when reserved
*     vm_free(tagName);
*	  </code>
*****************************************************************************/
VM_RESULT vm_tag_get_tag(VMCWSTR filename,
                         VM_TAG_ID tag_id,
                         void* buffer,
                         VMUINT* buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* VMTAG_SDK_H */
