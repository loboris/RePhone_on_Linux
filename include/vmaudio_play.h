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


#ifndef VMAUDIO_PLAY_SDK_H
#define VMAUDIO_PLAY_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

#include "vmaudio.h"

/* open parameters */
typedef struct
{
    VMWSTR                  filename;       /* File name of audio file, if playing from a data string, this should be set to NULL */
    void*                   data;           /* String data, if playing a file. Can be ignored if a file name is used*/
    VMUINT32                data_len;       /* The length of string data. Can be ignored if a file name is used */
    VM_AUDIO_FORMAT         format;         /* Audio format, please refer VM_AUDIO_FORMAT */
    VMINT8                  reserved;       /* Reserved. Please set it to zero */
    VM_AUDIO_DEVICE         output_path;    /* Output path. Uses VM_AUDIO_DEVICE_SPEAKER2 as a default setting */
    VMBOOL                  async_mode;     /* Async mode, if this is TRUE, play and stop will be asynchronous */
    vm_audio_result_callback callback;      /* Callback function */
    void*                   user_data;      /* User data */
} vm_audio_play_parameters_t;

/*****************************************************************************
 * FUNCTION
 *  vm_audio_play_open
 * DESCRIPTION
 *  Opens an audio file.
 * PARAMETERS
 *  open_parameters : [IN] Audio open.
 * RETURNS
 *  If it can be opened successfully it will return the handle otherwise returns -1.
*****************************************************************************/
VM_AUDIO_HANDLE vm_audio_play_open(vm_audio_play_parameters_t* open_parameters);

/*****************************************************************************
 * FUNCTION
 *    vm_audio_play_start
 * DESCRIPTION
 *  Plays the audio
 * PARAMETERS
 *  handle : [IN] The audio handle, received by vm_audio_play_open
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RESULT
 *      VM_AUDIO_SUCCEED : playback is successful.
*****************************************************************************/
VM_RESULT vm_audio_play_start(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_play_pause
 * DESCRIPTION
 *  Pause the audio playback.
 * PARAMETERS
 *  handle : [IN] The audio handle, received by vm_audio_play_open
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RESULT
 *      VM_AUDIO_SUCCEED : paused successfully.
 *      others : pausing failed.
*****************************************************************************/
VM_RESULT vm_audio_play_pause(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_play_resume
 * DESCRIPTION
 *  Resume audio playback
 * PARAMETERS
 *  handle : [IN] The audio handle, received by vm_audio_play_open
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RESULT
 *      VM_AUDIO_SUCCEED : resume success.
 *      others : resume failed.
*****************************************************************************/
VM_RESULT vm_audio_play_resume(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_play_get_current_time
 * DESCRIPTION
 *  Get the current play time.
 * PARAMETERS
 *  handle :            [IN]  The audio handle, received by vm_audio_play_open
 *  current_time :      [OUT] The play time (ms).
 * RETURNS
 *  VM_RESULT refer to VM_AUDIO_RESULT
 *      VM_AUDIO_SUCCEED :  get time success.
 *      others :            get time failed.
 *****************************************************************************/
VM_RESULT vm_audio_play_get_current_time(VM_AUDIO_HANDLE handle, VMUINT* current_time);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_play_stop
 * DESCRIPTION
 *  Stops the opened audio.
 * PARAMETERS
 *  handle : [IN] The audio handle, received by vm_audio_play_open
* RETURNS
*  VM_RESULT refer to VM_AUDIO_RESULT
*	   VM_AUDIO_SUCCEED :  get time success.
*	   others : 		   stop failed.
*****************************************************************************/
VM_RESULT vm_audio_play_stop(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *   vm_audio_play_close
 * DESCRIPTION
 *  Closes the opened audio.
 * PARAMETERS
 *  handle : [IN] The audio handle, received by vm_audio_play_open
* RETURNS
*  VM_RESULT refer to VM_AUDIO_RESULT
*	   VM_AUDIO_SUCCEED :  get time success.
*	   others : 		   close failed.
*****************************************************************************/
VM_RESULT vm_audio_play_close(VM_AUDIO_HANDLE handle);


#ifdef __cplusplus
}
#endif

#endif /* VMAUDIO_PLAY_SDK_H */
