/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include "vmstdlib.h"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define loslib_c
#define LUA_LIB

#include "lua.h"
#include "shell.h"
#include "lstate.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "lundump.h"
#include "term.h"
#include "CheckSumUtils.h"

#include "vmdatetime.h"
#include "vmpwr.h"
#include "vmtype.h"
#include "vmfirmware.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmwdt.h"
#include "vmsystem.h"
#include "vmdcl.h"
#include "vmlog.h"
#include "vmtimer.h"
#include "vmusb.h"


#define SHOW_LOG_FATAL		0x01
#define SHOW_LOG_ERROR		0x02
#define SHOW_LOG_WARNING	0x03
#define SHOW_LOG_INFO		0x04
#define SHOW_LOG_DEBUG		0x10

#define MAX_SYSTEM_PARAMS_SIZE	1020
#define SYSVAR_SIZE 20

extern void retarget_putc(char ch);
extern int retarget_getc(int tmo);
extern void _scheduled_startup(void);

extern int retarget_target;
extern VM_DCL_HANDLE retarget_usb_handle;
extern VM_DCL_HANDLE retarget_uart1_handle;
extern int no_activity_time;		// no activity counter
extern int sys_wdt_tmo;				// HW WDT timeout in ticks: 13001 -> 59.999615 seconds
extern int sys_wdt_rst;				// time at which hw wdt is reset in ticks: 10834 -> 50 seconds, must be < 'sys_wdt_tmo'
extern int max_no_activity_time;	// time with no activity before shut down in seconds
extern int wakeup_interval;			// regular wake up interval in seconds
extern int led_blink;				// led blink during session
extern int wdg_reboot_cb;			// Lua callback function called before reboot
extern int shutdown_cb;				// Lua callback function called before shutdown
extern int alarm_cb;				// Lua callback function called on alarm
extern int key_cb;					// Lua callback function called on key up/down
extern VMUINT8 alarm_flag;
extern cb_func_param_bt_t bt_cb_params;
extern int g_memory_size_b;
extern int g_reserved_heap;
extern int g_max_heap_inc;

int show_log = 0;

//-----------------------------------------------------------------------------
void _log_printf(int type, const char *file, int line, const char *msg, ...)
{
  if ((retarget_target != retarget_usb_handle) && (retarget_target != retarget_uart1_handle)) return;

  va_list ap;
  char *pos, message[256];
  int sz;
  int nMessageLen = 0;

  memset(message, 0, 256);
  pos = message;

  sz = 0;
  va_start(ap, msg);
  nMessageLen = vsnprintf(pos, 256 - sz, msg, ap);
  va_end(ap);

  if( nMessageLen<=0 ) return;

  if ((type == 1) && (show_log & SHOW_LOG_FATAL)) printf("\n[FATAL] %s:%d %s\n", file, line, message);
  else if ((type == 2) && (show_log & SHOW_LOG_ERROR)) printf("\n[ERROR] %s:%d %s\n", file, line, message);
  else if ((type == 3) && (show_log & SHOW_LOG_WARNING)) printf("\n[WARNING] %s:%d %s\n", file, line, message);
  else if ((type == 4) && (show_log & SHOW_LOG_INFO)) printf("\n[INFO] %s:%d %s\n", file, line, message);
  else if ((type == 5) && (show_log & SHOW_LOG_DEBUG)) printf("\n[DEBUG] %s:%d %s\n", file, line, message);
  else if (type == 0) printf("%s\n", message);
}

//--------------------------
void _writertc(uint32_t val)
{
    volatile uint32_t *regprot = (uint32_t *)(REG_BASE_ADDRESS + 0x710068);
    volatile uint32_t *regwrtgr = (uint32_t *)(REG_BASE_ADDRESS + 0x710074);
    volatile uint32_t *regbbpu = (uint32_t *)(REG_BASE_ADDRESS + 0x710000);
    *regprot = val;
    *regwrtgr = (uint32_t)0x0001;
    while (*regbbpu & 0x40) {}
}


//====================================
static int _os_remove (lua_State *L) {
  VMWCHAR ucs_name[VM_FS_MAX_PATH_LENGTH+1];
  const char *filename = luaL_checkstring(L, 1);

  vm_chset_ascii_to_ucs2(ucs_name, VM_FS_MAX_PATH_LENGTH, filename);

  lua_pushinteger(L, vm_fs_delete(ucs_name));

  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}

//===================================
static int os_remove (lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);
	remote_CCall(L, &_os_remove);
	return g_shell_result;
}

//===================================
static int os_rename (lua_State *L) {
	const char *fromname = luaL_checkstring(L, 1);
	const char *toname = luaL_checkstring(L, 2);

    g_fcall_message.message_id = CCALL_MESSAGE_FRENAME;
    g_CCparams.cpar1 = (char *)fromname;
    g_CCparams.cpar2 = (char *)toname;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    lua_pushinteger(L, g_shell_result);
	return 1;
}

//====================================
static int os_tmpname (lua_State *L) {
  char buff[64]; //LUA_TMPNAMBUFSIZE];
  int err;
  int r = rand();

  sprintf(buff,"_tmp_%d.tmp", r);
  lua_pushstring(L, buff);

  return 1;
}

//===================================
static int os_getenv (lua_State *L) {
  lua_pushnil(L);
  return 1;
}

//==================================
static int os_clock (lua_State *L) {
	g_fcall_message.message_id = CCALL_MESSAGE_TICK;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    lua_pushnumber(L, ((float)g_CCparams.upar1 / (float)CLOCKS_PER_SEC));
    return 1;
}

//==================================
static int sys_tick (lua_State *L) {
	//remote_CCall(L, &_sys_tick);

	g_fcall_message.message_id = CCALL_MESSAGE_TICK;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);
    lua_pushinteger(L, g_CCparams.upar1);
    return 1;
}

//=====================================
static int sys_elapsed (lua_State *L) {
	int tmstart = luaL_checkinteger(L, 1);
	//remote_CCall(L, &_sys_elapsed);
	g_fcall_message.message_id = CCALL_MESSAGE_TICK;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    VMUINT32 dur;
    if (g_CCparams.upar1 > tmstart) dur = g_CCparams.upar1 - tmstart;
    else dur = g_CCparams.upar1 + (0xFFFFFFFF - tmstart);

    lua_pushinteger(L, dur);

    return 1;
}

/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

//===============================================================
static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

//-------------------------------------------------------------------
static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}

//-------------------------------------------------------
static int getboolfield (lua_State *L, const char *key) {
  int res;
  lua_getfield(L, -1, key);
  res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}

//----------------------------------------------------------
static int getfield (lua_State *L, const char *key, int d) {
  int res;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    res = (int)lua_tointeger(L, -1);
  else {
    if (d < 0)
      return luaL_error(L, "field " LUA_QS " missing in date table", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}

//---------------------------------
static int os_date (lua_State *L) {
  const char *s = luaL_optstring(L, 1, "%c");
  time_t t = luaL_opt(L, (time_t)luaL_checknumber, 2, 0x7FFFFFFF);

  if (t == 0x7FFFFFFF) {
	  // get time from rtc
	  VMUINT rtct;
	  vm_time_get_unix_time(&rtct);
	  t = rtct;
  }

  struct tm *stm;
  if (*s == '!') {
	// UTC
    stm = gmtime(&t);
    s++;  // skip '!'
  }
  else stm = localtime(&t);

  if (stm == NULL)  /* invalid date? */
    lua_pushnil(L);
  else if (strcmp(s, "*t") == 0) { // time to table
    lua_createtable(L, 0, 9);      // 9 = number of fields
    setfield(L, "sec", stm->tm_sec);
    setfield(L, "min", stm->tm_min);
    setfield(L, "hour", stm->tm_hour);
    setfield(L, "day", stm->tm_mday);
    setfield(L, "month", stm->tm_mon+1);
    setfield(L, "year", stm->tm_year+1900);
    setfield(L, "wday", stm->tm_wday+1);
    setfield(L, "yday", stm->tm_yday+1);
    setboolfield(L, "isdst", stm->tm_isdst);
  }
  else {
    char cc[3];
    luaL_Buffer b;
    cc[0] = '%'; cc[2] = '\0';
    luaL_buffinit(L, &b);
    for (; *s; s++) {
      if (*s != '%' || *(s + 1) == '\0')  /* no conversion specifier? */
        luaL_addchar(&b, *s);
      else {
        size_t reslen;
        char buff[200];  /* should be big enough for any conversion result */
        cc[1] = *(++s);
        reslen = strftime(buff, sizeof(buff), cc, stm);
        luaL_addlstring(&b, buff, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  return 1;
}

//---------------------------------
static int os_time (lua_State *L) {
  time_t t;
  VMUINT rtct;
  if (lua_isnoneornil(L, 1)) {
	// called without args?
    vm_time_get_unix_time(&rtct);  // get current time
    t = rtct;
  }
  else {
    struct tm ts;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);  /* make sure table is at the top */
    ts.tm_sec = getfield(L, "sec", 0);
    ts.tm_min = getfield(L, "min", 0);
    ts.tm_hour = getfield(L, "hour", 12);
    ts.tm_mday = getfield(L, "day", -1);
    ts.tm_mon = getfield(L, "month", -1) - 1;
    ts.tm_year = getfield(L, "year", -1) - 1900;
    ts.tm_isdst = getboolfield(L, "isdst");
    t = mktime(&ts);
  }
  if (t == (time_t)(-1))
    lua_pushnil(L);
  else
    lua_pushnumber(L, (lua_Number)t);
  return 1;
}

static int os_difftime (lua_State *L) {
  lua_pushnumber(L, difftime((time_t)(luaL_checknumber(L, 1)),
                             (time_t)(luaL_optnumber(L, 2, 0))));
  return 1;
}

/* }====================================================== */

//======================================
static int os_setlocale (lua_State *L) {
  static const int cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME};
  static const char *const catnames[] = {"all", "collate", "ctype", "monetary", "numeric", "time", NULL};
  const char *l = luaL_optstring(L, 1, NULL);
  int op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  return 1;
}

//=====================================
static int _os_battery (lua_State *L) {
	lua_pushinteger(L, vm_pwr_get_battery_level());
	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 1;
}

//====================================
static int os_battery (lua_State *L) {
	remote_CCall(L, &_os_battery);
	return g_shell_result;
}

//====================================
static int _os_reboot (lua_State *L) {
    vm_log_info("REBOOT");

    _scheduled_startup();
	vm_pwr_reboot();
	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//===================================
static int os_reboot (lua_State *L) {
	remote_CCall(L, &_os_reboot);
	return g_shell_result;
}

//======================================
static int _os_shutdown (lua_State *L) {
	vm_log_info("SHUTDOWN");
	_scheduled_startup();
	vm_pwr_shutdown(777);
	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=====================================
static int os_shutdown (lua_State *L) {
	remote_CCall(L, &_os_shutdown);
	return g_shell_result;
}

//===============================================
static int _os_scheduled_startup (lua_State *L) {
  VMUINT rtct;
  vm_date_time_t start_time;
  struct tm *time_now;
  time_t nsec = 0;

  vm_time_get_unix_time(&rtct);  // get current time

  if (lua_istable(L, 1)) {
	  // ** get scheduled time from table
	  nsec = rtct;
	  time_now = gmtime(&nsec);
	  lua_settop(L, 1);  // make sure table is at the top
	  time_now->tm_sec = getfield(L, "sec", 0);
	  time_now->tm_min = getfield(L, "min", 0);
	  time_now->tm_hour = getfield(L, "hour", 12);
	  time_now->tm_mday = getfield(L, "day", -1);
	  time_now->tm_mon = getfield(L, "month", -1) - 1;
	  time_now->tm_year = getfield(L, "year", -1) - 1900;
	  time_now->tm_isdst = getboolfield(L, "isdst");
	  nsec = mktime(time_now);
  }
  else if (lua_isnumber(L, 1)) {
	  // ** wake up at current time + seconds increment
	  nsec = luaL_checkinteger( L, 1 );
	  if (nsec > 0) {
		  nsec += rtct;
		  time_now = gmtime(&nsec);
	  }
  }
  else {
	  vm_log_error("Wrong argument!");
	  goto exit;
  }

  if (nsec <= 0) {
	  // ** wake up at next interval
	  no_activity_time = max_no_activity_time - 2;
	  vm_log_info("WAKE UP SCHEDULED at next interval (%d sec)", wakeup_interval);
	  goto exit;
  }

  if (nsec < rtct) {
	  vm_log_error("WAKE UP SCHEDULED in past!");
	  goto exit;
  }

  start_time.day = time_now->tm_mday;
  start_time.hour = time_now->tm_hour;
  start_time.minute = time_now->tm_min;
  start_time.second = time_now->tm_sec;
  start_time.month = time_now->tm_mon + 1;
  start_time.year = time_now->tm_year + 1900;

  vm_pwr_scheduled_startup(&start_time, VM_PWR_STARTUP_ENABLE_CHECK_DHMS);
  alarm_flag = 1;
  vm_log_info("WAKE UP SCHEDULED at %s", asctime(time_now));
  //vm_pwr_shutdown(778);

exit:
  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//==============================================
static int os_scheduled_startup (lua_State *L) {
	remote_CCall(L, &_os_scheduled_startup);
	return g_shell_result;
}

//===========================
int _os_getver(lua_State* L) {
	VMCHAR value[128] = {0};
	lua_pushstring(L, LUA_RELEASE);
	VMUINT written = vm_firmware_get_info(value, sizeof(value)-1, VM_FIRMWARE_HOST_VERSION);
	lua_pushstring(L, value);
	written = vm_firmware_get_info(value, sizeof(value)-1, VM_FIRMWARE_BUILD_DATE_TIME);
	lua_pushstring(L, value);
	g_shell_result = 3;
	vm_signal_post(g_shell_signal);
    return 3;
}

//===================================
static int os_getver (lua_State *L) {
	remote_CCall(L, &_os_getver);
	return g_shell_result;
}

//============================
int os_getmem(lua_State* L) {
	//VMCHAR value[32] = {0};
	//VMUINT written = vm_firmware_get_info(value, sizeof(value)-1, VM_FIRMWARE_HOST_MAX_MEM);
    global_State *g = G(L);
	lua_pushinteger(L, g->totalbytes );
	lua_pushinteger(L, g->memlimit );
	//lua_pushinteger(L, vm_str_strtoi(value)*1024);
	lua_pushinteger(L, g_memory_size_b );
	lua_pushinteger(L, g_max_heap_inc );
    return 4;
}

//===================================
static int _os_mkdir (lua_State *L) {
  const char *dirname = luaL_checkstring(L, 1);
  VMWCHAR ucs_dirname[VM_FS_MAX_PATH_LENGTH+1];

  vm_chset_ascii_to_ucs2(ucs_dirname, VM_FS_MAX_PATH_LENGTH, dirname);

  lua_pushinteger(L, vm_fs_create_directory(ucs_dirname));
  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}

//==================================
static int os_mkdir (lua_State *L) {
    const char *dirname = luaL_checkstring(L, 1);
	remote_CCall(L, &_os_mkdir);
	return g_shell_result;
}

//===================================
static int _os_rmdir (lua_State *L) {
  const char *dirname = luaL_checkstring(L, 1);
  VMWCHAR ucs_dirname[VM_FS_MAX_PATH_LENGTH+1];

  vm_chset_ascii_to_ucs2(ucs_dirname, VM_FS_MAX_PATH_LENGTH, dirname);

  lua_pushinteger(L, vm_fs_remove_directory(ucs_dirname));
  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}

//==================================
static int os_rmdir (lua_State *L) {
	const char *dirname = luaL_checkstring(L, 1);
	remote_CCall(L, &_os_rmdir);
	return g_shell_result;
}

//==================================
static int _os_copy (lua_State *L) {
  VMWCHAR ucs_fromname[VM_FS_MAX_PATH_LENGTH+1];
  VMWCHAR ucs_toname[VM_FS_MAX_PATH_LENGTH+1];

  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);

  vm_chset_ascii_to_ucs2(ucs_fromname, VM_FS_MAX_PATH_LENGTH, fromname);
  vm_chset_ascii_to_ucs2(ucs_toname, VM_FS_MAX_PATH_LENGTH, toname);

  lua_pushinteger(L, vm_fs_copy(ucs_toname, ucs_fromname, NULL));

  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}

//==============================
static int os_copy(lua_State* L)
{
	const char *fromname = luaL_checkstring(L, 1);
	const char *toname = luaL_checkstring(L, 2);
	remote_CCall(L, &_os_copy);
	return g_shell_result;
}


//====================================
static int _os_listfiles(lua_State* L)
{
    VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
    VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
    VMWCHAR path[VM_FS_MAX_PATH_LENGTH] = {0};
    VMWCHAR fullname[VM_FS_MAX_PATH_LENGTH] = {0};
    VM_FS_HANDLE filehandle = -1;
    VM_FS_HANDLE filehandleex = -1;
    vm_fs_info_t fileinfo;
    vm_fs_info_ex_t fileinfoex;
    VMINT ret = 0;
    struct tm filetime;

    const char *fspec = luaL_optstring(L, 1, "*");

    sprintf(filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fspec);
    vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);
    vm_wstr_get_path(wfilename, path);

    // get the max file name length
    int len = 0;
    filehandle = vm_fs_find_first(wfilename, &fileinfo);
    if (filehandle >= 0) {
        do {
        	vm_chset_ucs2_to_ascii(filename, VM_FS_MAX_PATH_LENGTH, fileinfo.filename);
        	int flen = strlen(filename);
        	if (flen > len) len = flen;
            ret = vm_fs_find_next(filehandle, &fileinfo);
        } while (0 == ret);
        vm_fs_find_close(filehandle);
    }
    int total = 0;
    int nfiles = 0;
    filehandle = vm_fs_find_first(wfilename, &fileinfo);
    if (filehandle >= 0) {
    	vm_chset_ucs2_to_ascii(filename, VM_FS_MAX_PATH_LENGTH, path);
    	printf("%s\n", filename);
        do {
        	vm_wstr_copy(fullname, path);
        	vm_wstr_concatenate(fullname,(VMCWSTR)(&fileinfo.filename));
            filehandleex = vm_fs_find_first_ex(fullname, &fileinfoex);
        	vm_chset_ucs2_to_ascii(filename, VM_FS_MAX_PATH_LENGTH, fileinfoex.full_filename);
        	if (fileinfoex.attributes & VM_FS_ATTRIBUTE_DIRECTORY) {
            	printf(" %*s %8s\n", len, filename, "<DIR>");
        	}
        	else {
				filetime.tm_hour = fileinfoex.modify_datetime.hour;
				filetime.tm_min = fileinfoex.modify_datetime.minute;
				filetime.tm_sec = fileinfoex.modify_datetime.second;
				filetime.tm_mday = fileinfoex.modify_datetime.day;
				filetime.tm_mon = fileinfoex.modify_datetime.month-1;
				filetime.tm_year = fileinfoex.modify_datetime.year-1900;
				char ftime[20] = {0};
				strftime(ftime, 18, "%D %T", &filetime);
				printf(" %*s %8u %s\n", len, filename, fileinfoex.file_size, ftime);
				total += fileinfoex.file_size;
				nfiles++;
        	}
            vm_fs_find_close_ex(filehandleex);

            /* find the next file */
            ret = vm_fs_find_next(filehandle, &fileinfo);
        } while (0 == ret);
        vm_fs_find_close(filehandle);
        printf("\nTotal %d byte(s) in %d file(s)\n", total, nfiles);
        printf("Drive free: %d,  App data free: %d\n", vm_fs_get_disk_free_space(vm_fs_get_internal_drive_letter()), vm_fs_app_data_get_free_space());
    }
    else {
        printf("No files found.\n");
    }

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}


//===================================
static int os_listfiles(lua_State* L)
{
	remote_CCall(L, &_os_listfiles);
	return g_shell_result;
}

//=================================
static int _os_exit (lua_State *L) {
  //exit(luaL_optint(L, 1, EXIT_SUCCESS));
  printf("\n[SYSEVT] APP RESTART\n");
  vm_pmng_restart_application();
  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//=================================
static int os_exit (lua_State *L) {
	remote_CCall(L, &_os_exit);
	return g_shell_result;
}

//=====================================
static int os_retarget (lua_State *L) {
	int targ = luaL_checkinteger(L,1);

	if ((targ == 0) && (retarget_usb_handle >= 0)) {
		retarget_target = retarget_usb_handle;
		lua_pushboolean(L, TRUE);
		return 1;
	}
	else if ((targ == 1) && (retarget_uart1_handle >= 0)) {
		retarget_target = retarget_uart1_handle;
		lua_pushboolean(L, TRUE);
		return 1;
	}
	else if ((targ == 2) && (bt_cb_params.connected)) {
		retarget_target = -1000;
		lua_pushboolean(L, TRUE);
		return 1;
	}
	lua_pushboolean(L, FALSE);
	return 1;
}

//====================================
static int os_getchar (lua_State *L) {
	int ch;
	int tmo = luaL_checkinteger(L,1);
	if (tmo < 0) tmo =1;

	ch = retarget_getc(tmo);
	if (ch < 0) lua_pushnil(L);
	else lua_pushinteger(L, ch);

	return 1;
}

//======================================
static int os_getstring (lua_State *L) {
    luaL_Buffer b;
    int i, ch;
	int count = luaL_checkinteger(L,1);
	int tmo = luaL_checkinteger(L,2);
	if (count < 1) count = 1;

	luaL_buffinit(L, &b);
	for (i=0; i<count; i++) {
		ch = retarget_getc(tmo);
		if (ch < 0) break;
		luaL_addchar(&b, ch);
	}
    luaL_pushresult(&b);

    return 1;
}

//====================================
static int os_putchar (lua_State *L) {
	char *c = luaL_checkstring(L,1);
	retarget_putc(*c);
	return 0;
}

//=====================================
static int os_ledblink (lua_State *L) {
	if (lua_gettop(L) >= 1) {
		int lb = luaL_checkinteger(L,1);
		if ((lb == 0) || (lb == BLUELED) || (lb == REDLED) || (lb == GREENLED)) {
			led_blink = lb;
		}
	}
	lua_pushinteger(L, led_blink);
	return 1;
}

//=======================================
static int os_wakeup_int (lua_State *L) {
	if (lua_gettop(L) >= 1) {
		int wui = luaL_checkinteger(L,1);  // wake up interval in minutes
		if (wui > 0) wakeup_interval = wui * 60;
	}
	lua_pushinteger(L, wakeup_interval / 60);
	return 1;
}

//=======================================
static int os_noact_time (lua_State *L) {
	if (lua_gettop(L) >= 1) {
		int noact = luaL_checkinteger(L,1);	// maximum time with no activity in seconds

		no_activity_time = 0;				// reset no activity counter
		if (noact > 0) {
			// set max no activity limit
			if (noact < 10) noact = 10;
			max_no_activity_time = noact;
		}
	}
	lua_pushinteger(L, max_no_activity_time);
	return 1;
}

//=================================
static int _os_usb (lua_State *L) {
	lua_pushinteger(L, vm_usb_get_cable_status());
	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 1;
}

//================================
static int os_usb (lua_State *L) {
	remote_CCall(L, &_os_usb);
	return g_shell_result;
}

//=====================================
static int os_onwdg_cb (lua_State *L) {
    if (wdg_reboot_cb != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, wdg_reboot_cb);
	    wdg_reboot_cb = LUA_NOREF;
    }
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    wdg_reboot_cb = luaL_ref(L, LUA_REGISTRYINDEX);
	}
    return 0;
}

//=======================================
static int os_onshdwn_cb (lua_State *L) {
    if (shutdown_cb != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, shutdown_cb);
	    shutdown_cb = LUA_NOREF;
    }
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    shutdown_cb = luaL_ref(L, LUA_REGISTRYINDEX);
	}
    return 0;
}

//=======================================
static int os_onalarm_cb (lua_State *L) {
    if (alarm_cb != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, alarm_cb);
    	alarm_cb = LUA_NOREF;
    }
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    alarm_cb = luaL_ref(L, LUA_REGISTRYINDEX);
	}
    return 0;
}

//=====================================
static int os_onkey_cb (lua_State *L) {
    if (key_cb != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, key_cb);
    	key_cb = LUA_NOREF;
    }
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    key_cb = luaL_ref(L, LUA_REGISTRYINDEX);
	}
    return 0;
}


//===================================
static int os_readreg(lua_State* L)
{
    uint32_t adr = luaL_checkinteger(L, 1);

    volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + adr);
    uint32_t val = *reg;

    lua_pushinteger (L, val);

	return 1;
}

//===================================
static int os_writereg(lua_State* L)
{
    uint32_t adr = luaL_checkinteger(L, 1);
    uint32_t val = luaL_checkinteger(L, 2);

    volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + adr);
    *reg = val;

	return 0;
}

//=======================================
static int os_writertcreg(lua_State* L)
{
    unsigned int adr = luaL_checkinteger(L, 1);
    uint32_t val = luaL_checkinteger(L, 2);

    if ((adr >= 0x710000) && (adr <= 0x710078)) {
		volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + adr);
		*reg = val;

		_writertc(0x586A);
		_writertc(0x9136);
    }

	return 0;
}

//=================================
static int os_showlog(lua_State* L)
{
    show_log = luaL_checkinteger(L, 1) & 0x1F;

	return 0;
}

//================================
static int os_exists(lua_State* L)
{
	  const char *filename = luaL_checkstring(L, 1);

	  lua_pushinteger(L, file_exists(filename));
	  return 1;
}

//------------------------------------------------------------------
static int writer(lua_State* L, const void* p, size_t size, void* u)
{
  UNUSED(L);
  return (fwrite(p, size, 1, (FILE*)u) != 1) && (size != 0);
}

#define toproto(L,i) (clvalue(L->top+(i))->l.p)

//===================================
static int os_compile( lua_State* L )
{
  //printf("%s  %s\n",LUA_RELEASE,LUA_COPYRIGHT);

  size_t len;
  const char *filename = luaL_checklstring( L, 1, &len );

  // check here that filename end with ".lua".
  if ((len < 5) || (strcmp( filename + len - 4, ".lua") != 0)) return luaL_error(L, "not a .lua file");

  if (luaL_loadfile(L,filename) != 0) return luaL_error(L, "error opening source file");
  Proto* f = toproto(L, -1);

  int stripping = 1;      // strip debug information?
  char output[64];
  strcpy(output, filename);
  output[strlen(output) - 2] = 'c';
  output[strlen(output) - 1] = '\0';

  FILE* D = fopen(output,"wb");
  if (D == NULL) return luaL_error(L, "error opening destination file");

  lua_lock(L);
  luaU_dump(L, f, writer, D, stripping);
  lua_unlock(L);

  if (ferror(D)) return luaL_error(L, "error writing to destination file");
  if (fclose(D)) return luaL_error(L, "error closing destination file");

  return 0;
}

//----------------------------
void check(int depth, int prn)
{
	// every run groves the stack by 16 bytes
	char c = 100;
	if ((prn) || (depth == 0)) printf("stack adr: %p [%d]\n", &c, c);
	if (depth <= 0) return;
	check(depth-1, 0);
}

//=====================================
static int os_teststack( lua_State* L )
{
	int n = luaL_checkinteger(L, 1);
	check(n, 1);
	return 0;
}

//====================================
static int os_testheap( lua_State* L )
{
	uint32_t n = luaL_checkinteger(L, 1) & 0xFFFFFF00;
	char *buf = NULL;
	buf = malloc(n);
	while (buf == NULL) {
		n -= 256;
		buf = malloc(n);
	}
	printf("block adr: %p, size: %d\n", buf, n);
	free(buf);
	return 0;
}

//======================================
static int _os_testcheap( lua_State* L )
{
	uint32_t n = luaL_checkinteger(L, 1) & 0xFFFFFF00;
	char *buf = NULL;
	buf = vm_malloc(n);
	while (buf == NULL) {
		n -= 256;
		buf = vm_malloc(n);
	}
	printf("cblock adr: %p, size: %d\n", buf, n);
	vm_free(buf);
	vm_signal_post(g_shell_signal);
	return 0;
}

//=====================================
static int os_testcheap( lua_State* L )
{
	remote_CCall(L, &_os_testcheap);
	return 0;
}

//-----------------------------------------------------------------
int _get_paramstr(char **param_buf, char **par_end, char **var_ptr)
{
	int psize, err = 0;

	psize = vm_fs_app_data_get_free_space();
	if (psize > MAX_SYSTEM_PARAMS_SIZE) {
		// no parameters
    	vm_log_debug("parameters not found");
		return -1;
	}

	char *parambuf = vm_calloc(MAX_SYSTEM_PARAMS_SIZE+1);
	if (parambuf == NULL) {
    	vm_log_error("error allocating buffer");
		return -2;
	}

	VM_FS_HANDLE phandle = vm_fs_app_data_open(VM_FS_MODE_READ, VM_FALSE);
	if (phandle < VM_FS_SUCCESS) {
    	vm_log_error("error opening params");
		return -3;
	}

	if (vm_fs_app_data_read(phandle, parambuf, MAX_SYSTEM_PARAMS_SIZE, &psize) < 0) {
    	vm_log_error("error reading params");
    	vm_fs_app_data_close(phandle);
		return -4;
	}

	vm_fs_app_data_close(phandle);

	parambuf[psize] = '\0';

	// === Check crc ===
	char *parend = strstr(parambuf, "\n__SYSVAR=\"");	// end of __SYSPAR table
	if (parend == NULL) {
		err = -5;
		goto exit;
	}

	char *varptr = parend + 11;					// start of __SSVAR
	char *varend = strstr(varptr, "\"");	// end of __SYSVAR
	if (varend == NULL){
		err = -6;
		goto exit;
	}
	if ((varend - varptr) != SYSVAR_SIZE) {
		err = -7;
		goto exit;
	}

	// calculate crc
	CRC16_Context contex;
	uint16_t parcrc;
	CRC16_Init( &contex );
	CRC16_Update( &contex, parambuf, (varptr - parambuf) + SYSVAR_SIZE - 4);
	CRC16_Final( &contex, &parcrc );
	// compare
	char crcstr[5] = {'\0'};
	memcpy(crcstr, varptr + SYSVAR_SIZE - 4, 4);
	uint16_t crc = (uint16_t)strtol(crcstr, NULL, 16);
	if (crc != parcrc) {
		err = -8;
	}
	else {
		*param_buf = parambuf;
		*par_end = parend;
		*var_ptr = varptr;
		return 0;
	}

exit:
	vm_log_error("SYSPAR wrong format");
	return err;
}

//----------------------------
int _read_params(lua_State *L)
{
	char *param_buf = NULL;
	char *parend = NULL;
	char *sysvar = NULL;

	g_shell_result = _get_paramstr(&param_buf, &parend, &sysvar);
	if (g_shell_result >= 0) {
		*parend = '\0';
		g_shell_result = luaL_loadbuffer(L, param_buf, strlen(param_buf), "param");
		if (g_shell_result) {
			lua_pop(L, 1);  // pop error message from the stack
			vm_log_error("error executing parameters");
			g_shell_result = -10;
		}
		else {
			vm_log_debug("%s", param_buf);
			g_shell_result = 1;
		}
	}

	if (param_buf != NULL) vm_free(param_buf);

	vm_signal_post(g_shell_signal);
	return 0;
}

//----------------------------------
static int _get_params(lua_State *L)
{
	char *param_buf = NULL;
	char *parend = NULL;
	char *sysvar = NULL;

	g_shell_result = _get_paramstr(&param_buf, &parend, &sysvar);

	if (g_shell_result < 0) lua_pushinteger(L, g_shell_result);
	else lua_pushstring(L, param_buf);

	if (param_buf != NULL) vm_free(param_buf);

	vm_signal_post(g_shell_signal);
	return 0;
}

//-------------------------------------------------------
void _get_sys_vars(int doset, void *heapsz, void *wdgtmo)
{
    // get system variables
	char *param_buf = NULL;
	char *parend = NULL;
	char *sysvar = NULL;

	int res = _get_paramstr(&param_buf, &parend, &sysvar);
	if (res >= 0) {
		// get c heap size & wdg timeout
		char hexstr[9] = {'\0'};
		memcpy(hexstr, sysvar, 8);
		int val = (int)strtol(hexstr, NULL, 16);
		if (doset) {
			if ((val >= (32*1024)) && (val <= (256*1024))) g_reserved_heap = val;
		}
		else {
			*(int *)heapsz = val;
		}

		// get watchdog timeout
		memcpy(hexstr, sysvar+8, 8);
		val = (int)strtol(hexstr, NULL, 16);
		if (doset) {
			if ((val >= 10) && (val <= 3600)) {
				sys_wdt_tmo = (val * 216685) / 1000;	// set watchdog timeout
				sys_wdt_rst = sys_wdt_tmo - 1083;		// reset wdg 5 sec before expires
			}
		}
		else {
			*(int *)wdgtmo = val;
		}
	}

	if (param_buf != NULL) vm_free(param_buf);
}

//-----------------------------------
static int _get_sysvars(lua_State *L)
{
	int cheapsz = -1;
	int wdgtmo = -1;

	_get_sys_vars(0, &cheapsz, &wdgtmo);

	lua_pushinteger(L, cheapsz);
	lua_pushinteger(L, wdgtmo);

	vm_signal_post(g_shell_signal);
	return 0;
}

//-----------------------------------------------------------------------------------------
static int _table_tostring(lua_State *L, char *buf, int *idx, int top, int list, int level)
{
    int klen = 0;
    int vlen = 0;
    int val_type;
    int num_key;
    int items = 0;
    int doadd = 0;
    char sep[2] = {0};
    char quot[2] = {0};
    const char* key;
    const char* value;
    char strkey[64];

    lua_pushnil(L);  // first key
    while (lua_next(L, top) != 0) {
    	// Pops a key from the stack, and pushes a key-value pair from the table
    	// 'key' (at index -2) and 'value' (at index -1)
		val_type = lua_type(L, -2);  // get key type
		if (val_type == LUA_TNUMBER) {
			num_key = lua_tonumber(L, -2);
			klen = sprintf(strkey,"[%d]", num_key);
		}
		else {
			key = lua_tolstring(L, -2, &klen);
			klen = sprintf(strkey,"%s", key);
		}
		doadd = 0;
		val_type = lua_type(L, -1);  // get value type
		switch (val_type) {
			case LUA_TBOOLEAN:
			case LUA_TNUMBER:
				quot[0] = '\0';
				value = lua_tolstring(L, -1, &vlen);
				if ((*idx + klen + vlen + 2) <= MAX_SYSTEM_PARAMS_SIZE) doadd = 1;
				break;
			case LUA_TSTRING:
				quot[0] = '"';
				doadd = 1;
				value = lua_tolstring(L, -1, &vlen);
				if ((*idx + klen + vlen + 4) <= MAX_SYSTEM_PARAMS_SIZE) doadd = 1;
				break;
			case LUA_TTABLE:
				if ((*idx + klen + 4) <= MAX_SYSTEM_PARAMS_SIZE) {
					*idx += sprintf(buf + *idx, "%s%s={", sep, strkey);
					if (list) printf("%*s%s%s={\n", level, " ", sep, strkey);
					items += _table_tostring(L, buf, idx, lua_gettop(L), list, level*2);
					*idx += sprintf(buf + *idx, "}");
					if (list) printf("%*s}\n", level, " ");
				}
				break;
		    case LUA_TFUNCTION:
				if (list) printf("%*s%s%s=<FUNCTION>\n", level, " ", sep, strkey);
		    	break;
		    case LUA_TUSERDATA:
				if (list) printf("%*s%s%s=<USERDATA>\n", level, " ", sep, strkey);
		    	break;
		    case LUA_TNIL:
				if (list) printf("%*s%s%s=<NIL>\n", level, " ", sep, strkey);
		    	break;
		    default:
				if (list) printf("%*s%s%s=<UNKNOWN TYPE: %d>\n", level, " ", sep, strkey, val_type);
		}
		if (doadd) {
			*idx += sprintf(buf + *idx, "%s%s=%s%s%s", sep, strkey, quot, value, quot);
			if (list) printf("%*s%s%s=%s%s%s\n", level, " ", sep, strkey, quot, value, quot);
			items++;
		}
		if (items > 0) sep[0] = ',';
	    lua_pop(L, 1);  // removes 'value'; keeps 'key' for next iteration
    }
	return items;
}

//======================================
static int table_to_string(lua_State *L)
{
	if (!lua_istable(L, -1)) return luaL_error(L, "table argument expected");

	int list = 0;
	if (lua_isnumber(L, 1)) list = luaL_checkinteger(L, 1) & 1;

	char *buf = calloc(MAX_SYSTEM_PARAMS_SIZE+1, 1);
	if (buf == NULL) return luaL_error(L, "error allocating buffer");

	int idx;
    int items = 0;

    idx = sprintf(buf, "{");
	if (list) printf("{\n");

    items = _table_tostring(L, buf, &idx, lua_gettop(L), list, 2);

	idx += sprintf(buf+idx, "}");
	if (list) printf("}\n");

	if (!list) {
		lua_pushinteger(L, items);
		lua_pushstring(L, buf);
		free(buf);
		return 2;
	}
	else {
		free(buf);
		return 0;
	}
}

//-----------------------------------
static int _save_params(lua_State *L)
{
	int err = 0;

	char* param_buf = vm_calloc(MAX_SYSTEM_PARAMS_SIZE+1);
	if (param_buf == NULL) {
    	vm_log_error("error allocating buffer");
    	err = -1;
    	goto exit1;
	}

    lua_getglobal(L, "__SYSPAR");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);  // removes nil
    	vm_log_error("param table not found");
    	err = -2;
    	goto exit;
	}

    int idx = 0;
    int items = 0;

    idx = sprintf(param_buf, "__SYSPAR={");

    items = _table_tostring(L, param_buf, &idx, lua_gettop(L), 0, 0);

	idx += sprintf(param_buf+idx, "}\n__SYSVAR=\"%08X%08X", sys_wdt_tmo, g_reserved_heap);

	lua_pop(L, 1);  // removes '__SYSPAR' table

	// calculate crc
	CRC16_Context contex;
	uint16_t parcrc = 0;
	CRC16_Init( &contex );
	CRC16_Update( &contex, param_buf, idx );
	CRC16_Final( &contex, &parcrc );
	idx += sprintf(param_buf+idx, "%04X\"", parcrc);

	err = items;
	VM_FS_HANDLE phandle = vm_fs_app_data_open(VM_FS_MODE_CREATE_ALWAYS_WRITE, VM_FALSE);
	if (phandle < VM_FS_SUCCESS) {
    	vm_log_error("error opening params");
		err = phandle;
		goto exit;
	}

	int psize;
	err = vm_fs_app_data_write(phandle, param_buf, idx, &psize);
	if (err < 0) {
    	vm_log_error("error writing params");
	}
	else {
		err = psize;
		vm_log_debug("%s", param_buf);
	}

	vm_fs_app_data_close(phandle);

exit:
	vm_free(param_buf);
	param_buf = NULL;
exit1:
	g_shell_result = err;
	vm_signal_post(g_shell_signal);
	return 0;
}


//==================================
static int read_params(lua_State *L)
{
	remote_CCall(L, &_read_params);

	if (g_shell_result < 0) lua_pushinteger(L, g_shell_result);
	else {
		lua_pcall(L, 0, 0, 0);
    	lua_pushinteger(L, 0);
	}

	return 1;
}

//=================================
static int get_params(lua_State *L)
{
	remote_CCall(L, &_get_params);

	return 1;
}

//=================================
static int get_sysvars(lua_State *L)
{
	remote_CCall(L, &_get_sysvars);

	return 2;
}

//=================================
static int save_params(lua_State *L)
{
	remote_CCall(L, &_save_params);

	lua_pushinteger(L, g_shell_result);

	return 1;
}

// set watchdog timeout
//======================================
static int os_wdg_timeout (lua_State *L)
{
	g_shell_result = 0;
	if (lua_gettop(L) >= 1) {
		int wdgtmo = luaL_checkinteger(L,1);	// watchdog timeout in seconds

		if (wdgtmo < 10) wdgtmo = 10;
		if (wdgtmo > 3600) wdgtmo = 3600;
		sys_wdt_tmo = (wdgtmo * 216685) / 1000;	// set watchdog timeout
		sys_wdt_rst = sys_wdt_tmo - 1083;		// reset wdg 5 sec before expires

		remote_CCall(L, &_save_params);

		lua_pushinteger(L, (sys_wdt_tmo * 1000) / 216685);
		lua_pushinteger(L, g_shell_result);
		return 2;
	}
	else {
		lua_pushinteger(L, (sys_wdt_tmo * 1000) / 216685);
		return 1;
	}
}

// set C heap size
//=====================================
static int os_cheap_size (lua_State *L)
{
	g_shell_result = 0;
	if (lua_gettop(L) >= 1) {
		int hsz = luaL_checkinteger(L,1);	// cheap size in 32K increments
		if (hsz < (32*1024)) hsz = (32*1024);
		if (hsz > (8*32*1024)) hsz = (8*32*1024);
		hsz &= 0x0FF8000;
		g_reserved_heap = hsz;

		remote_CCall(L, &_save_params);
		lua_pushinteger(L, g_reserved_heap);
		lua_pushinteger(L, g_shell_result);
		return 2;
	}
	else {
		lua_pushinteger(L, g_reserved_heap);
		return 1;
	}
}

//=====================================
static int set_term_input(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		use_term_input = luaL_checkinteger(L, 1) & 0x01;
	}
	lua_pushinteger(L, use_term_input);

	return 1;
}

//===================================
static int sys_random( lua_State* L )
{
    int randn, minn, maxn;
    uint16_t i;
    uint8_t sd = 0;

    maxn = RAND_MAX; //0x7FFFFFFE;
    minn = 0;
    if (lua_gettop(L) >= 1) maxn = luaL_checkinteger( L, 1 );
    if (lua_gettop(L) >= 2) minn = luaL_checkinteger( L, 2 );
    if (lua_gettop(L) >= 3) sd = luaL_checkinteger( L, 3 );
    if (maxn < minn) maxn = minn+1;
    if (maxn <= 0) maxn = 1;

    i = 0;
    do {
      randn = rand();
      if (randn > maxn) randn = randn % (maxn+1);
      i++;
    } while ((i < 10000) && (randn < minn));

    if (randn < minn) randn = minn;

    lua_pushinteger(L,randn);
    return 1;
}



extern int luaB_dofile (lua_State *L);


#define MIN_OPT_LEVEL 1
#include "lrodefs.h"

const LUA_REG_TYPE syslib[] = {
  {LSTRKEY("clock"),    	LFUNCVAL(os_clock)},
  {LSTRKEY("date"),     	LFUNCVAL(os_date)},
  {LSTRKEY("difftime"), 	LFUNCVAL(os_difftime)},
  {LSTRKEY("execute"),  	LFUNCVAL(luaB_dofile)},
  {LSTRKEY("exit"),     	LFUNCVAL(os_exit)},
  {LSTRKEY("getenv"), 		LFUNCVAL(os_getenv)},
  {LSTRKEY("remove"),   	LFUNCVAL(os_remove)},
  {LSTRKEY("rename"),   	LFUNCVAL(os_rename)},
  {LSTRKEY("setlocale"),	LFUNCVAL(os_setlocale)},
  {LSTRKEY("time"),     	LFUNCVAL(os_time)},
  {LSTRKEY("tmpname"),	  	LFUNCVAL(os_tmpname)},
  // additional
  {LSTRKEY("copy"),     	LFUNCVAL(os_copy)},
  {LSTRKEY("exists"),     	LFUNCVAL(os_exists)},
  {LSTRKEY("mkdir"),    	LFUNCVAL(os_mkdir)},
  {LSTRKEY("rmdir"),    	LFUNCVAL(os_rmdir)},
  {LSTRKEY("list"),     	LFUNCVAL(os_listfiles)},
  {LSTRKEY("compile"),		LFUNCVAL(os_compile)},
  {LSTRKEY("shell_linetype"),LFUNCVAL(set_term_input)},
  {LSTRKEY("table2str"),	LFUNCVAL(table_to_string)},
  {LNILKEY, LNILVAL}
};

/* }====================================================== */

const LUA_REG_TYPE sys_map[] = {
		  {LSTRKEY("ver"),  		LFUNCVAL(os_getver)},
		  {LSTRKEY("mem"),  		LFUNCVAL(os_getmem)},
		  {LSTRKEY("shutdown"), 	LFUNCVAL(os_shutdown)},
		  {LSTRKEY("reboot"),   	LFUNCVAL(os_reboot)},
		  {LSTRKEY("schedule"), 	LFUNCVAL(os_scheduled_startup)},
		  {LSTRKEY("battery"),  	LFUNCVAL(os_battery)},
		  {LSTRKEY("retarget"), 	LFUNCVAL(os_retarget)},
		  {LSTRKEY("getchar"),  	LFUNCVAL(os_getchar)},
		  {LSTRKEY("getstring"),	LFUNCVAL(os_getstring)},
		  {LSTRKEY("putchar"),  	LFUNCVAL(os_putchar)},
		  {LSTRKEY("ledblink"), 	LFUNCVAL(os_ledblink)},
		  {LSTRKEY("wkupint"),  	LFUNCVAL(os_wakeup_int)},
		  {LSTRKEY("noacttime"),	LFUNCVAL(os_noact_time)},
		  {LSTRKEY("wdg"),			LFUNCVAL(os_wdg_timeout)},
		  {LSTRKEY("usb"),      	LFUNCVAL(os_usb)},
		  {LSTRKEY("onreboot"), 	LFUNCVAL(os_onwdg_cb)},
		  {LSTRKEY("onshutdown"),	LFUNCVAL(os_onshdwn_cb)},
		  {LSTRKEY("onalarm"),		LFUNCVAL(os_onalarm_cb)},
		  {LSTRKEY("onkey"),		LFUNCVAL(os_onkey_cb)},
		  {LSTRKEY("showlog"),		LFUNCVAL(os_showlog)},
		  {LSTRKEY("readreg"), 		LFUNCVAL(os_readreg) },
		  {LSTRKEY("writereg"), 	LFUNCVAL(os_writereg) },
		  {LSTRKEY("writertcreg"),	LFUNCVAL(os_writertcreg) },
		  {LSTRKEY("teststack"),	LFUNCVAL(os_teststack) },
		  {LSTRKEY("testheap"),		LFUNCVAL(os_testheap) },
		  {LSTRKEY("testcheap"),	LFUNCVAL(os_testcheap) },
		  {LSTRKEY("read_params"),	LFUNCVAL(read_params) },
		  {LSTRKEY("save_params"),	LFUNCVAL(save_params) },
		  {LSTRKEY("get_params"),	LFUNCVAL(get_params) },
		  {LSTRKEY("get_sysvars"),	LFUNCVAL(get_sysvars) },
		  {LSTRKEY("random"),		LFUNCVAL(sys_random) },
		  {LSTRKEY("tick"),			LFUNCVAL(sys_tick) },
		  {LSTRKEY("elapsed"),		LFUNCVAL(sys_elapsed) },
		  {LSTRKEY("c_heapsize"),	LFUNCVAL(os_cheap_size) },
		  {LNILKEY, LNILVAL }
};



#define GLOBAL_NUMBER(l, name, value) \
    lua_pushnumber(L, value);         \
    lua_setglobal(L, name)

LUALIB_API int luaopen_os (lua_State *L) {
    GLOBAL_NUMBER(L, "LOG_FATAL", SHOW_LOG_FATAL);
    GLOBAL_NUMBER(L, "LOG_ERROR", SHOW_LOG_ERROR);
	GLOBAL_NUMBER(L, "LOG_WARNING", SHOW_LOG_WARNING);
	GLOBAL_NUMBER(L, "LOG_INFO", SHOW_LOG_INFO);
	GLOBAL_NUMBER(L, "LOG_DEBUG", SHOW_LOG_DEBUG);
	GLOBAL_NUMBER(L, "LOG_ALL", 0x1F);
	GLOBAL_NUMBER(L, "LOG_NONE", 0x00);

    lua_newtable(L);				// create table
    lua_setglobal(L, "__SYSPAR");	// create global "__SYSPAR" table

    LREGISTER(L, LUA_OSLIBNAME, syslib);
}

LUALIB_API int luaopen_sys(lua_State* L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0
    // random number init
    VMUINT rtct;
	vm_time_get_unix_time(&rtct);
	srand(rtct);

    luaL_register(L, "sys", sys_map);

    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

