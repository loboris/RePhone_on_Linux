
#include <string.h>

#include "vmsystem.h"
#include "vmlog.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmaudio_play.h"

#include "v7.h"

#define MAX_NAME_LEN 32 /* Max length of file name */

static VMINT g_audio_handle = -1;          /* The handle of play */
static VMINT g_audio_interrupt_handle = 0; /* The handle of interrupt */

/* The callback function when playing. */
void audio_play_callback(VM_AUDIO_HANDLE handle, VM_AUDIO_RESULT result, void* userdata)
{
    switch(result) {
    case VM_AUDIO_RESULT_END_OF_FILE:
        /* When the end of file is reached, it needs to stop and close the handle */
        vm_audio_play_stop(g_audio_handle);
        vm_audio_play_close(g_audio_handle);
        g_audio_handle = -1;
        break;
    case VM_AUDIO_RESULT_INTERRUPT:
        /* The playback is terminated by another application, for example an incoming call */
        vm_audio_play_stop(g_audio_handle);
        vm_audio_play_close(g_audio_handle);
        g_audio_handle = -1;
        break;
    default:
        break;
    }
}

/* Play the audio file. */
static void _audio_play(char* name)
{
    VMINT drv;
    VMWCHAR w_file_name[MAX_NAME_LEN] = { 0 };
    VMCHAR file_name[MAX_NAME_LEN];
    vm_audio_play_parameters_t play_parameters;

    /* get file path */
    drv = vm_fs_get_removable_drive_letter();
    if(drv < 0) {
        drv = vm_fs_get_internal_drive_letter();
        if(drv < 0) {
            vm_log_fatal("not find driver");
            return;
        }
    }
    sprintf(file_name, (VMSTR) "%c:\\%s", drv, name);
    vm_chset_ascii_to_ucs2(w_file_name, MAX_NAME_LEN, file_name);

    /* set play parameters */
    memset(&play_parameters, 0, sizeof(vm_audio_play_parameters_t));
    play_parameters.filename = w_file_name;
    play_parameters.reserved = 0;                           /* no use, set to 0 */
    play_parameters.format = VM_AUDIO_FORMAT_MP3;           /* file format */
    play_parameters.output_path = VM_AUDIO_DEVICE_SPEAKER2; /* set device to output */
    play_parameters.async_mode = 0;
    play_parameters.callback = audio_play_callback;
    play_parameters.user_data = NULL;
    g_audio_handle = vm_audio_play_open(&play_parameters);
    if(g_audio_handle >= VM_OK) {
        vm_log_info("open success");
    } else {
        vm_log_error("open failed");
    }
    /* start to play */
    vm_audio_play_start(g_audio_handle);
    /* register interrupt callback */
    g_audio_interrupt_handle = vm_audio_register_interrupt_callback(audio_play_callback, NULL);
}

static v7_val_t audio_play(struct v7* v7)
{
    v7_val_t namev = v7_arg(v7, 0);
    const char* name;
    size_t len;

    if(!v7_is_string(namev)) {
        return v7_create_undefined();
    }

    name = v7_to_string(v7, &namev, &len);

    _audio_play((char*)name);

    return v7_create_boolean(1);
}

static v7_val_t audio_stop(struct v7* v7)
{
    if(g_audio_handle >= 0) {
        vm_audio_play_stop(g_audio_handle);
        vm_audio_play_close(g_audio_handle);

        if(g_audio_interrupt_handle != 0) {
            vm_audio_clear_interrupt_callback(g_audio_interrupt_handle);
        }
    }

    return v7_create_boolean(1);
}

static v7_val_t audio_pause(struct v7* v7)
{
    if(g_audio_handle >= 0) {
        vm_audio_play_pause(g_audio_handle);
        return v7_create_boolean(1);
    }

    return v7_create_undefined();
}

static v7_val_t audio_resume(struct v7* v7)
{
    if(g_audio_handle >= 0) {
        vm_audio_play_resume(g_audio_handle);
        return v7_create_boolean(1);
    }
    return v7_create_undefined();
}

static v7_val_t audio_set_volume(struct v7* v7)
{
    v7_val_t volumev = v7_arg(v7, 0);
    int volume;

    if(!v7_is_number(volumev)) {
        return v7_create_undefined();
    }

    volume = v7_to_number(volumev);

    vm_audio_set_volume(volume);

    return v7_create_boolean(1);
}

static v7_val_t audio_get_volume(struct v7* v7)
{
    int volume = vm_audio_get_volume();

    return v7_create_number(volume);
}

void js_init_audio(struct v7* v7)
{
    v7_val_t audio = v7_create_object(v7);
    v7_set(v7, v7_get_global(v7), "audio", 5, 0, audio);
    v7_set_method(v7, audio, "play", audio_play);
    v7_set_method(v7, audio, "stop", audio_stop);
    v7_set_method(v7, audio, "pause", audio_pause);
    v7_set_method(v7, audio, "resume", audio_resume);
    v7_set_method(v7, audio, "set_volume", audio_set_volume);
    v7_set_method(v7, audio, "get_volume", audio_get_volume);
}
