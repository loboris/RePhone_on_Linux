
#include <string.h>

#include "vmsystem.h"
#include "vmlog.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmaudio_play.h"

#include "lua.h"
#include "lauxlib.h"

#define MAX_NAME_LEN 32  /* Max length of file name */

static VMINT g_audio_handle = -1;  /* The handle of play */
static VMINT g_audio_interrupt_handle = 0; /* The handle of interrupt */

/* The callback function when playing. */
void audio_play_callback(VM_AUDIO_HANDLE handle, VM_AUDIO_RESULT result, void* userdata)
{
  switch (result)
  {
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
static void _audio_play(char *name)
{
  VMINT drv ;
  VMWCHAR w_file_name[MAX_NAME_LEN] = {0};
  VMCHAR file_name[MAX_NAME_LEN];
  vm_audio_play_parameters_t play_parameters;

  /* get file path */
  drv = vm_fs_get_removable_drive_letter();
  if(drv <0)
  {
    drv = vm_fs_get_internal_drive_letter();
    if(drv <0)
    {
      vm_log_fatal("not find driver");
      return ;
    }
  }
  sprintf(file_name, (VMSTR)"%c:\\%s", drv, name);
  vm_chset_ascii_to_ucs2(w_file_name, MAX_NAME_LEN, file_name);

  /* set play parameters */
  memset(&play_parameters, 0, sizeof(vm_audio_play_parameters_t));
  play_parameters.filename = w_file_name;
  play_parameters.reserved = 0;  /* no use, set to 0 */
  play_parameters.format = VM_AUDIO_FORMAT_MP3; /* file format */
  play_parameters.output_path = VM_AUDIO_DEVICE_SPEAKER2; /* set device to output */
  play_parameters.async_mode = 0;
  play_parameters.callback = audio_play_callback;
  play_parameters.user_data = NULL;
  g_audio_handle = vm_audio_play_open(&play_parameters);
  if(g_audio_handle >= VM_OK)
  {
    vm_log_info("open success");
  }
  else
  {
    vm_log_error("open failed");
  }
  /* start to play */
  vm_audio_play_start(g_audio_handle);
  /* register interrupt callback */
  g_audio_interrupt_handle = vm_audio_register_interrupt_callback(audio_play_callback,NULL);
}


int audio_play(lua_State *L)
{
    char *name = lua_tostring(L, -1);

    _audio_play(name);

    return 0;
}

int audio_stop(lua_State *L)
{
  if(g_audio_handle >= 0)
  {
	  vm_audio_play_stop(g_audio_handle);
      vm_audio_play_close(g_audio_handle);

      if(g_audio_interrupt_handle!=0)
      {
        vm_audio_clear_interrupt_callback(g_audio_interrupt_handle);
      }
  }

  return 0;
}

int audio_pause(lua_State *L)
{
  if(g_audio_handle >= 0)
  {
	  vm_audio_play_pause(g_audio_handle);
	  return 0;
  }

  return -1;
}

int audio_resume(lua_State *L)
{
  if(g_audio_handle >= 0)
  {
	  vm_audio_play_resume(g_audio_handle);
	  return 0;
  }
  return -1;
}


int audio_set_volume(lua_State *L)
{
    int volume = lua_tointeger(L, -1);

	vm_audio_set_volume(volume);

	return 0;
}

int audio_get_volume(lua_State *L)
{
    int volume = vm_audio_get_volume();

    lua_pushnumber(L, volume);

	return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE audio_map[] =
{
    {LSTRKEY("play"), LFUNCVAL(audio_play)},
    {LSTRKEY("stop"), LFUNCVAL(audio_stop)},
    {LSTRKEY("pause"), LFUNCVAL(audio_pause)},
    {LSTRKEY("resume"), LFUNCVAL(audio_resume)},
    {LSTRKEY("set_volume"), LFUNCVAL(audio_set_volume)},
    {LSTRKEY("get_volume"), LFUNCVAL(audio_get_volume)},
    {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_audio(lua_State *L)
{
    luaL_register(L, "audio", audio_map);
    return 1;
}
