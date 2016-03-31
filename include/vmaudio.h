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


#ifndef VMAUDIO_SDK_H
#define VMAUDIO_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* audio handle */
typedef VMINT32 VM_AUDIO_HANDLE;

typedef VMINT32 VM_AUDIO_INTERRUPT_HANDLE;

/* path to play */
typedef enum
{
    VM_AUDIO_DEVICE_SPEAKER = 0,        /* reserved  */
    VM_AUDIO_DEVICE_MICROPHONE = 1,     /* reserved  */
    VM_AUDIO_DEVICE_SPEAKER2 = 4,       /* Earphone, Car-kit  */
    VM_AUDIO_DEVICE_LOUDSPEAKER = 5,    /* Loudspeaker for free sound  */
    VM_AUDIO_DEVICE_SPEAKER_BOTH = 6,   /* Output from earphone and loud speaker  */
    VM_AUDIO_DEVICE_BT_HEADSET = 8,     /* reserved  */
	VM_AUDIO_DEVICE_MAX = 0x7F
} VM_AUDIO_DEVICE;

/* media type */
typedef enum
{
    VM_AUDIO_FORMAT_NONE = -1,      /* NONE */
    VM_AUDIO_FORMAT_AMR = 3,        /* AMR */
    VM_AUDIO_FORMAT_MP3 = 5,        /* MP3 */
    VM_AUDIO_FORMAT_AAC = 6,        /* AAC */
    VM_AUDIO_FORMAT_WAV = 13,       /* WAV */
    VM_AUDIO_FORMAT_MIDI = 17,      /* MIDI */
    VM_AUDIO_FORMAT_IMELODY = 18,   /* IMELODY */
    VM_AUDIO_FORMAT_OTHER = 100,    /* OTHER */
	VM_AUDIO_FORMAT_MAX = 0x7F
} VM_AUDIO_FORMAT;

typedef enum
{
    VM_AUDIO_CODEC_NONE,        /* No type */
    VM_AUDIO_CODEC_SILENT,      /* reserved */
    VM_AUDIO_CODEC_AMR = 2,     /* AMR */
    VM_AUDIO_CODEC_AMRWB = 3,   /* Wide-band AMR */
    VM_AUDIO_CODEC_AAC = 4,     /* AAC */
    VM_AUDIO_CODEC_MP3 = 5,     /* MP3 */
    VM_AUDIO_CODEC_MP4A = 6,    /* reserved */
    VM_AUDIO_CODEC_MP4AG = 7,   /* reserved */
    VM_AUDIO_CODEC_WAV = 8,	    /* RAW data WAV */
    VM_AUDIO_CODEC_ADPCM = 9,   /* compressed WAV*/
    VM_AUDIO_CODEC_PCM = 10,	/* RAW data, for input data strength calculation*/
    VM_AUDIO_CODEC_MAX = 0x7F	/* Total codec type */
} VM_AUDIO_CODEC;

typedef enum
{
    VM_AUDIO_SAMPLE_FREQUENCY_8000,        /* 8000  */
    VM_AUDIO_SAMPLE_FREQUENCY_11025,       /* 11025 */
    VM_AUDIO_SAMPLE_FREQUENCY_16000,       /* 16000 */
    VM_AUDIO_SAMPLE_FREQUENCY_22050,       /* 22050 */
    VM_AUDIO_SAMPLE_FREQUENCY_24000,       /* 24000 */
    VM_AUDIO_SAMPLE_FREQUENCY_32000,       /* 32000 */
    VM_AUDIO_SAMPLE_FREQUENCY_44100,       /* 44100 */
    VM_AUDIO_SAMPLE_FREQUENCY_48000,       /* 48000 */
    VM_AUDIO_SAMPLE_FREQUENCY_MAX = 0x7F   /* Total sample frequency number */
} VM_AUDIO_SAMPLE_FREQUENCY;

/* audio operation result */
typedef enum
{
    VM_AUDIO_SUCCEED = 0,                   /* Audio operation successful */
    VM_AUDIO_FAILED = -1,                   /* Audio operation failed */
    VM_AUDIO_RESULT_STOP = 1,               /* Stop playing audio */
    VM_AUDIO_RESULT_END_OF_FILE = 5,        /* Playing ends (end of file) */
    VM_AUDIO_DATA_REQUEST = 6,              /* Audio stream plays use current data */
    VM_AUDIO_RESULT_INTERRUPT = 7,          /* Interrupted by other task, for example a voice call */
    VM_AUDIO_RESULT_INTERRUPT_RESUME = 8,   /* Resume after being interrupted */
    VM_AUDIO_RESULT_MAX = 0x7FFFFFFF        /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_AUDIO_RESULT;

/* Audio volume level enum for 7 level volume */
typedef enum
{
    VM_AUDIO_VOLUME_0,                  /* Volume level 0 (mute) */
    VM_AUDIO_VOLUME_1,                  /* Volume level 1 */
    VM_AUDIO_VOLUME_2,                  /* Volume level 2 */
    VM_AUDIO_VOLUME_3,                  /* Volume level 3 */
    VM_AUDIO_VOLUME_4,                  /* Volume level 4 */
    VM_AUDIO_VOLUME_5,                  /* Volume level 5 */
    VM_AUDIO_VOLUME_6,                  /* Volume level 6 */
    VM_AUDIO_VOLUME_NUM = 0x7FFFFFFF    /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_AUDIO_VOLUME;

/*****************************************************************************
 * FUNCTION
 *  vm_audio_result_callback
 * DESCRIPTION
 *  Audio callback function pointer
 * PARAMETERS
 *  handle:     [IN] The audio handle.
 *  result:     [IN] The result of the operation.
 *  user_data:  [IN] User data.
 *****************************************************************************/
typedef void (*vm_audio_result_callback)(VM_AUDIO_HANDLE handle, VM_AUDIO_RESULT result, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_audio_set_volume
 * DESCRIPTION
 *  Sets the play volume level.
 * PARAMETERS
 *  volume : [IN] Desired volume level, the range spreads from 0-6.
 * RETURNS
 * void
*****************************************************************************/
void vm_audio_set_volume(VM_AUDIO_VOLUME volume);


/*****************************************************************************
 * FUNCTION
 *  vm_audio_get_volume
 * DESCRIPTION
 *  Gets the currently set volume level.
 * RETURNS
 *  VM_AUDIO_VOLUME
*****************************************************************************/
VM_AUDIO_VOLUME vm_audio_get_volume(void);

/*****************************************************************************
 * FUNCTION
 *  vm_audio_register_interrupt_callback
 * DESCRIPTION
 *  Registers an interrupt callback. When the audio playback is interrupted,
 *  this callback will be invoked.
 * PARAMETERS
 * callback:    [IN] Callback function pointer.
 * user_data:   [IN] User data.
 * RETURNS
 *  VM_AUDIO_INTERRUPT_HANDLE or if it fails it returns 0
*****************************************************************************/
VM_AUDIO_INTERRUPT_HANDLE vm_audio_register_interrupt_callback(vm_audio_result_callback callback, void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_audio_clear_interrupt_callback
 * DESCRIPTION
 *  Clears interrupt callback
 * PARAMETERS
 *  handle: [IN] The handle received from vm_audio_register_interrupt_callback
 * RETURNS
 * void
*****************************************************************************/
void vm_audio_clear_interrupt_callback(VM_AUDIO_INTERRUPT_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *  vm_audio_is_playing
 * DESCRIPTION
 *  This is to check if there is another applications that is currently playing audio
 * RETURNS
 *  TRUE:   Another application is playing
 *  FLASE:  There is no other application playing audio
*****************************************************************************/
VMBOOL vm_audio_is_playing(void);


/*****************************************************************************
 * FUNCTION
 *    vm_audio_play_is_calling
 * DESCRIPTION
 *  Check if it is in calling now
 * RETURNS
 *  TRUE or FALSE
*****************************************************************************/
VMBOOL vm_audio_is_calling(void);

#ifdef __cplusplus
}
#endif

#endif /* VMAUDIO_SDK_H */
