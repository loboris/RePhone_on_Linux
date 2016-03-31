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




#ifndef VMAUDIO_RECORD_SDK_H
#define VMAUDIO_RECORD_SDK_H



#ifdef __cplusplus
extern "C" {
#endif

#include "vmaudio.h"

/* record operation result */
typedef enum
{
    VM_AUDIO_RECORD_SUCCESS = 0,
    VM_AUDIO_RECORD_TERMINATED = 1,         /* Record was terminated, like stop record */
    VM_AUDIO_RECORD_INTERRUPT = 7,          /* Interrupted by other application like voice call */
    VM_AUDIO_RECORD_INTERRUPT_RESUME = 8,   /* Resume from interrupt */
    VM_AUDIO_RECORD_ERROR = -1,             /* General recording error */
    VM_AUDIO_RECORD_ERROR_NO_SPACE = -2,    /* No space to record. If it is recording to a T card and the card is disconnected while recording, the application will receive this message*/
    VM_AUDIO_RECORD_RESULT_MAX = 0x7FFFFFFF /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_AUDIO_RECORD_RESULT;

/******************************************************************************  VM_AUDIO_RECORD_RESULT
 * FUNCTION
 *  vm_audio_record_result_callback
 * DESCRIPTION
 *  Record callback function.
 * PARAMETERS
 *  result:     [IN] Operation result.
 *  user_data:  [IN] User data.
 *****************************************************************************/
typedef void(*vm_audio_record_result_callback)(VM_AUDIO_RECORD_RESULT result, void* user_data);

/*****************************************************************************
 * FUNCTION
 *   vm_audio_record_start
 * DESCRIPTION
 *   Starts recording.
 * PARAMETERS
 *  filename :  [IN] Recording file name.
 *  format :    [IN] Recording file format, VM_FORMAT_AMR and VM_FORMAT_WAV.
 *  callback :  [IN] Callback function. The application should handle VM_AUDIO_RECORD_RESULT/VM_AUDIO_RECORD_ERROR_NO_SPACE within this callback.
 *  user_data : [IN] User data.
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RECORD_RESULT

*****************************************************************************/
VM_RESULT vm_audio_record_start(VMCWSTR filename, VM_AUDIO_FORMAT format, vm_audio_record_result_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_record_pause
 * DESCRIPTION
 *  Pause recording.
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RECORD_RESULT
*****************************************************************************/
VM_RESULT vm_audio_record_pause(void);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_record_resume
 * DESCRIPTION
 *  Resume recording.
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RECORD_RESULT
*****************************************************************************/
VM_RESULT vm_audio_record_resume(void);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_record_stop
 * DESCRIPTION
 *  Stops recording.
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RECORD_RESULT
*****************************************************************************/
VM_RESULT vm_audio_record_stop(void);


#ifdef __cplusplus
}
#endif

#endif /* VMAUDIO_RECORD_SDK_H */
