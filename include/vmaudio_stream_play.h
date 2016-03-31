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

#ifndef VMAUDIO_STREAM_PLAY_SDK_H
#define VMAUDIO_STREAM_PLAY_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmaudio.h"


/* bistream result */


/* format type */

/* PCM bit-stream audio configuration structure */
typedef struct
{
    VM_AUDIO_CODEC codec_type;                      /* Codec type */
    VMBOOL is_stereo;                               /* FALSE: mono, TRUE: stereo*/
    VMUINT8 bit_per_sample;                         /* 8 or 16, commonly use 16 bits*/
    VM_AUDIO_SAMPLE_FREQUENCY sample_frequency;     /* 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000*/
} vm_audio_stream_play_config_t;


/* buffer status */
typedef struct
{
    VMUINT total_buffer_size;  /* Total buffer size */
    VMUINT free_buffer_size;   /* Free buffer size */
} vm_audio_stream_play_buffer_status_t;


/* play audio parameters */
typedef struct
{
    VMUINT 	start_time;         /* Start time, 0 means from the beginning */
    VM_AUDIO_VOLUME volume;     /* Volume level, range is from 0 to 6 */
    VM_AUDIO_DEVICE audio_path; /* Path to play,  please refer to VM_AUDIO_DEVICE in vmaudio.h */
} vm_audio_stream_play_start_parameters_t;


/*****************************************************************************
 * FUNCTION
 *    vm_audio_stream_play_open
 * DESCRIPTION
 *  Opens a PCM type bit-stream audio.
 * PARAMETERS
 *  handle:        [OUT] Bit-stream handle.
 *  audio_type:    [IN]  Current type, supports PCM.
 *  callback:      [IN]  Callback function.
 *  user_data:     [IN]  User data
 * RETURNS
 *	VM_RESULT
 *****************************************************************************/
VM_RESULT vm_audio_stream_play_open(VM_AUDIO_HANDLE* handle, const vm_audio_stream_play_config_t* audio_type, vm_audio_result_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_stream_play_finished
 * DESCRIPTION
 *  Tells the driver to put no more data through.
 * PARAMETERS
 *  handle : [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_finish(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_stream_play_close
 * DESCRIPTION
 *  Closes the bit-stream audio.
 * PARAMETERS
 *  handle : [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_close(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_stream_play_get_buffer_status
 * DESCRIPTION
 *  Gets the data buffer status including free buffer and total buffer size.
 * PARAMETERS
 *  handle : [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 *  status : [OUT] the status of buffer.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_get_buffer_status(VM_AUDIO_HANDLE handle, vm_audio_stream_play_buffer_status_t* status);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_stream_play_put_data
 * DESCRIPTION
 *  Puts the audio data to the buffer.
 * PARAMETERS
 *  handle :        [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 *  buffer :        [IN] Buffer data to put.
 *  buffer_size :   [IN] Data buffer size.
 *  written :       [OUT] The real data size that was put.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_put_data(VM_AUDIO_HANDLE handle, const void* buffer, VMUINT 	buffer_size, VMUINT* written);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_stream_play_start
 * DESCRIPTION
 *  Starts to play audio.
 * PARAMETERS
 *  handle :        [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 *  parameters :    [IN] Audio parameters.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_start(VM_AUDIO_HANDLE handle, vm_audio_stream_play_start_parameters_t* parameters);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_stream_play_stop
 * DESCRIPTION
 *  Stop playing the audio.
 * PARAMETERS
 *  handle : [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_stop(VM_AUDIO_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_stream_play_get_play_time
 * DESCRIPTION
 *  Gets the current play time.
 * PARAMETERS
 *  handle :        [IN] Bit-stream handle, received by vm_audio_stream_play_open.
 *  current_time :  [OUT] Current play time in ms.
 * RETURNS
 *	VM_RESULT
*****************************************************************************/
VM_RESULT vm_audio_stream_play_get_play_time(VM_AUDIO_HANDLE handle, VMUINT* current_time);

#ifdef __cplusplus
}
#endif

#endif /* VMAUDIO_STREAM_PLAY_SDK_H */

