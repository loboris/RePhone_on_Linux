
#include <string.h>

#include "vmsystem.h"
#include "vmlog.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmaudio_play.h"
#include "vmaudio_record.h"

//#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

#define MAX_NAME_LEN 64  // Max length of file name

static VMINT g_audio_handle = -1;			// The handle of play
static VMINT g_audio_interrupt_handle = 0;	// The handle of interrupt

// The callback function when playing.
//---------------------------------------------------------------------------------------------
static void audio_play_callback(VM_AUDIO_HANDLE handle, VM_AUDIO_RESULT result, void* userdata)
{
  //printf("AUDIO_CB: %d\n", result);
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

// The callback function of recording
//------------------------------------------------------------------------------
static void audio_record_callback(VM_AUDIO_RECORD_RESULT result, void* userdata)
{
  switch (result)
  {
      case VM_AUDIO_RECORD_ERROR_NO_SPACE:
      case VM_AUDIO_RECORD_ERROR:
    	// not enough space
        vm_audio_record_stop();
        break;
      default:
        //vm_log_info("callback result = %d", result);
        break;
  }
}


// Play the audio file.
//==================================
static int _audio_play(lua_State *L)
{
  VMWCHAR w_file_name[MAX_NAME_LEN] = {0};
  vm_audio_play_parameters_t play_parameters;
  int format = VM_AUDIO_FORMAT_MP3;

  if (g_audio_handle >= 0) {
      vm_audio_play_stop(g_audio_handle);
      vm_audio_play_close(g_audio_handle);
  }

  char *name = lua_tostring(L, 1);
  if (lua_type(L, 2) == LUA_TNUMBER) {
	format = luaL_checkinteger(L, 2);
    if ((format != VM_AUDIO_FORMAT_NONE) &&
    	(format != VM_AUDIO_FORMAT_AMR) &&
    	(format != VM_AUDIO_FORMAT_MP3) &&
    	(format != VM_AUDIO_FORMAT_AAC) &&
    	(format != VM_AUDIO_FORMAT_WAV) &&
    	(format != VM_AUDIO_FORMAT_MIDI) &&
    	(format != VM_AUDIO_FORMAT_IMELODY) &&
    	(format != VM_AUDIO_FORMAT_OTHER) &&
    	(format != VM_AUDIO_FORMAT_MAX)) format = VM_AUDIO_FORMAT_MP3;
  }

  full_fname((char *)name, w_file_name, MAX_NAME_LEN);

  g_shell_result = 0;

  // set play parameters
  memset(&play_parameters, 0, sizeof(vm_audio_play_parameters_t));
  play_parameters.filename = w_file_name;
  play_parameters.reserved = 0;  							// not used, set to 0 */
  play_parameters.format = format;							// file format
  play_parameters.output_path = VM_AUDIO_DEVICE_SPEAKER2;	// set device to output
  play_parameters.async_mode = 0;
  play_parameters.callback = audio_play_callback;
  play_parameters.user_data = NULL;
  g_audio_handle = vm_audio_play_open(&play_parameters);
  if (g_audio_handle < VM_OK) {
    vm_log_error("audio open failed");
    g_shell_result = -1;
  }
  // start to play
  if (vm_audio_play_start(g_audio_handle) == VM_AUDIO_SUCCEED) {
	  // register interrupt callback
	  g_audio_interrupt_handle = vm_audio_register_interrupt_callback(audio_play_callback,NULL);
  }
  else g_shell_result = -2;

  vm_signal_post(g_shell_signal);
  return 0;
}

//=================================
static int audio_play(lua_State *L)
{
    char *name = luaL_checkstring(L, 1);

    g_shell_result = -3;
    if (file_exists(name) == 1) {
    	remote_CCall(L, &_audio_play);
    }
    lua_pushinteger(L, g_shell_result);
    return 1;
}

//====================================
static int _audio_record(lua_State *L)
{
  VMWCHAR w_file_name[MAX_NAME_LEN] = {0};
  vm_audio_play_parameters_t play_parameters;
  int format = VM_AUDIO_FORMAT_AMR;

  char *name = lua_tostring(L, 1);
  if (lua_type(L, 2) == LUA_TNUMBER) {
	format = luaL_checkinteger(L, 2);
	if ((format != VM_AUDIO_FORMAT_AMR) && (format != VM_AUDIO_FORMAT_WAV)) format = VM_AUDIO_FORMAT_AMR;
  }

  full_fname((char *)name, w_file_name, MAX_NAME_LEN);

  g_shell_result = vm_audio_record_start(w_file_name, format, audio_record_callback, NULL);

  vm_signal_post(g_shell_signal);
  return 0;
}

//=================================
static int audio_record(lua_State *L)
{
    char *name = luaL_checkstring(L, 1);

    g_shell_result = -3;
    if ((file_exists(name) != 1) && (file_exists(name) != 2)) {
    	remote_CCall(L, &_audio_record);
    }
    lua_pushinteger(L, g_shell_result);
    return 1;
}

//==================================
static int _audio_stop(lua_State *L)
{
  int ret = -1;
  if (g_audio_handle >= 0) {
	  ret = vm_audio_play_stop(g_audio_handle);
      ret = vm_audio_play_close(g_audio_handle);

      if (g_audio_interrupt_handle != 0) {
        vm_audio_clear_interrupt_callback(g_audio_interrupt_handle);
      }
  }
  lua_pushinteger(L, ret);
  vm_signal_post(g_shell_signal);
  return 0;
}

//=================================
static int audio_stop(lua_State *L)
{
   	remote_CCall(L, &_audio_stop);
    return 1;
}

//===================================
static int _audio_pause(lua_State *L)
{
  int ret = -1;
  if (g_audio_handle >= 0) {
	  ret = vm_audio_play_pause(g_audio_handle);
  }
  lua_pushinteger(L, ret);
  vm_signal_post(g_shell_signal);
  return 0;
}

//==================================
static int audio_pause(lua_State *L)
{
   	remote_CCall(L, &_audio_pause);
    return 1;
}

//====================================
static int _audio_resume(lua_State *L)
{
  int ret = -1;
  if (g_audio_handle >= 0) {
	  ret = vm_audio_play_resume(g_audio_handle);
  }
  lua_pushinteger(L, ret);
  vm_signal_post(g_shell_signal);
  return 0;
}

//===================================
static int audio_resume(lua_State *L)
{
   	remote_CCall(L, &_audio_resume);
    return 1;
}

//===================================
static int _audio_rstop(lua_State *L)
{
  lua_pushinteger(L, vm_audio_record_stop());

  vm_signal_post(g_shell_signal);
  return 0;
}

//==================================
static int audio_rstop(lua_State *L)
{
   	remote_CCall(L, &_audio_rstop);
    return 1;
}

//====================================
static int _audio_rpause(lua_State *L)
{
	lua_pushinteger(L, vm_audio_record_pause());

  vm_signal_post(g_shell_signal);
  return 0;
}

//===================================
static int audio_rpause(lua_State *L)
{
   	remote_CCall(L, &_audio_rpause);
    return 1;
}

//=====================================
static int _audio_rresume(lua_State *L)
{
  lua_pushinteger(L, vm_audio_record_resume());

  vm_signal_post(g_shell_signal);
  return 0;
}

//====================================
static int audio_rresume(lua_State *L)
{
   	remote_CCall(L, &_audio_rresume);
    return 1;
}

//========================================
static int _audio_set_volume(lua_State *L)
{
    int volume = lua_tointeger(L, -1);
    if (volume < 0) volume = 0;
    if (volume > 6) volume = 6;

    vm_audio_set_volume(volume);

	vm_signal_post(g_shell_signal);
	return 0;
}

//=======================================
static int audio_set_volume(lua_State *L)
{
    int volume = lua_tointeger(L, -1);
   	remote_CCall(L, &_audio_set_volume);
    return 0;
}

//========================================
static int _audio_get_volume(lua_State *L)
{
    int volume = vm_audio_get_volume();

    lua_pushinteger(L, volume);

	vm_signal_post(g_shell_signal);
	return 0;
}

//=======================================
static int audio_get_volume(lua_State *L)
{
   	remote_CCall(L, &_audio_get_volume);
    return 1;
}

//======================================
static int _audio_get_time(lua_State *L)
{
	VMUINT time = 0;
    if (g_audio_handle >= 0) {
    	vm_audio_play_get_current_time(g_audio_handle, &time);
    }

    lua_pushinteger(L, time);

	vm_signal_post(g_shell_signal);
	return 0;
}

//=====================================
static int audio_get_time(lua_State *L)
{
   	remote_CCall(L, &_audio_get_time);
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
    {LSTRKEY("get_time"), LFUNCVAL(audio_get_time)},
    {LSTRKEY("record"), LFUNCVAL(audio_record)},
    {LSTRKEY("rec_stop"), LFUNCVAL(audio_rstop)},
    {LSTRKEY("rec_pause"), LFUNCVAL(audio_rpause)},
    {LSTRKEY("rec_resume"), LFUNCVAL(audio_rresume)},
    {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_audio(lua_State *L)
{
    luaL_register(L, "audio", audio_map);
    return 1;
}
