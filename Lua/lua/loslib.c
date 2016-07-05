/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


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

#include "vmdatetime.h"
#include "vmpwr.h"
#include "vmtype.h"
#include "vmfirmware.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmwdt.h"
#include "vmsystem.h"
#include "vmdcl.h"
#include "vmlog.h"
#include "vmtimer.h"
#include "vmusb.h"

//#define USE_YMODEM

#if defined USE_YMODEM
#include "ymodem.h"
#endif

#define SHOW_LOG_FATAL		0x01
#define SHOW_LOG_ERROR		0x02
#define SHOW_LOG_WARNING	0x03
#define SHOW_LOG_INFO		0x04
#define SHOW_LOG_DEBUG		0x10

extern void retarget_putc(char ch);
extern int retarget_waitc(unsigned char *c, int timeout);
extern int retarget_waitchars(unsigned char *buf, int *count, int timeout);
extern void _scheduled_startup(void);

extern int retarget_target;
extern VM_DCL_HANDLE retarget_usb_handle;
extern VM_DCL_HANDLE retarget_uart1_handle;
extern int no_activity_time;		// no activity counter
extern int sys_wdt_tmo;				// HW WDT timeout in ticks: 13001 -> 59.999615 seconds
extern int sys_wdt_rst;				// time at which hw wdt is reset in ticks: 10834 -> 50 seconds, must be < 'sys_wdt_tmo'
extern int sys_wdt_rst_usec;
extern int max_no_activity_time;	// time with no activity before shut down in seconds
extern int wakeup_interval;			// regular wake up interval in seconds
extern int led_blink;				// led blink during session
extern int wdg_reboot_cb;			// Lua callback function called before reboot
extern int shutdown_cb;				// Lua callback function called before shutdown
extern int alarm_cb;				// Lua callback function called on alarm
extern VMUINT8 alarm_flag;
extern cb_func_param_bt_t bt_cb_params;
extern int g_memory_size_b;

int show_log = 0x1F;

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

//---------------------------------
static void _writertc(uint32_t val)
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
	remote_CCall(&_os_remove);
	return g_shell_result;
}

//====================================
static int _os_rename (lua_State *L) {
  VMWCHAR ucs_oldname[VM_FS_MAX_PATH_LENGTH+1];
  VMWCHAR ucs_newname[VM_FS_MAX_PATH_LENGTH+1];

  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);

  vm_chset_ascii_to_ucs2(ucs_oldname, VM_FS_MAX_PATH_LENGTH, fromname);
  vm_chset_ascii_to_ucs2(ucs_newname, VM_FS_MAX_PATH_LENGTH, toname);

  lua_pushinteger(L, vm_fs_rename(ucs_oldname, ucs_newname));

  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}

//===================================
static int os_rename (lua_State *L) {
	const char *fromname = luaL_checkstring(L, 1);
	const char *toname = luaL_checkstring(L, 2);
	remote_CCall(&_os_rename);
	return g_shell_result;
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


//===================================
static int _os_clock (lua_State *L) {
  lua_pushnumber(L, ((float)vm_time_ust_get_count())/(float)CLOCKS_PER_SEC);
  g_shell_result = 1;
  vm_signal_post(g_shell_signal);
  return 1;
}


//==================================
static int os_clock (lua_State *L) {
	remote_CCall(&_os_clock);
	return g_shell_result;
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
	remote_CCall(&_os_battery);
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
	remote_CCall(&_os_reboot);
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
	remote_CCall(&_os_shutdown);
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
	remote_CCall(&_os_scheduled_startup);
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
	remote_CCall(&_os_getver);
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

    return 3;
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
	remote_CCall(&_os_mkdir);
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
	remote_CCall(&_os_rmdir);
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
	remote_CCall(&_os_copy);
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
	remote_CCall(&_os_listfiles);
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
	remote_CCall(&_os_exit);
	return g_shell_result;
}

//=====================================
static int os_retarget (lua_State *L) {
	int targ = luaL_checkinteger(L,1);

	if ((targ == 0) && (retarget_usb_handle >= 0)) retarget_target = retarget_usb_handle;
	else if ((targ == 1) && (retarget_uart1_handle >= 0)) retarget_target = retarget_uart1_handle;
	else if ((targ == 2) && (bt_cb_params.connected)) retarget_target = -1000;
	return 0;
}

//====================================
static int os_getchar (lua_State *L) {
	int tmo = luaL_checkinteger(L,1);

	unsigned char c;
	int res = retarget_waitc(&c, tmo);
	if (res < 0) lua_pushnil(L);
	else lua_pushinteger(L, c);

	return 1;
}

//======================================
static int os_getstring (lua_State *L) {
	int count = luaL_checkinteger(L,1);
	int tmo = luaL_checkinteger(L,2);

	if (count > 256) count = 256;
	if (count < 1) count = 1;

	unsigned char buf[256] = {0};

	int res = retarget_waitchars(buf, &count, tmo);
	if (res < 0) lua_pushnil(L);
	else lua_pushstring(L, buf);

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

// set watchdog timeout
//========================================
static int os_wdg_timeout (lua_State *L) {
	if (lua_gettop(L) >= 1) {
		int wdgtmo = luaL_checkinteger(L,1);	// watchdog timeout in seconds

		if (wdgtmo < 10) wdgtmo = 10;
		if (wdgtmo > 3600) wdgtmo = 3600;
		sys_wdt_tmo = (wdgtmo * 216685) / 1000;	// set watchdog timeout
		sys_wdt_rst = sys_wdt_tmo - 1083;		// reset wdg 5 sec before expires
        sys_wdt_rst_usec = (sys_wdt_tmo * 4615 / 1000) - 2000;
		// save to RTC_SPAR0 register
		volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + 0x0710060);
	    *reg = wdgtmo | 0xA000;
	    _writertc(0x586A);
	    _writertc(0x9136);
	}
	lua_pushnumber(L, (float)sys_wdt_tmo / 216.685);
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
	remote_CCall(&_os_usb);
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

    volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + adr);
    *reg = val;

    _writertc(0x586A);
    _writertc(0x9136);

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


void check(int depth, int prn)
{
	char c;
	if ((prn) || (depth == 0)) printf("stack at %p\n", &c);
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

#if defined USE_YMODEM

//===================================
static int _file_recv( lua_State* L )
{
  int fsize = 0;
  unsigned char c, gnm;
  char fnm[64];
  unsigned int max_len = vm_fs_get_disk_free_space(vm_fs_get_internal_drive_letter())-(1024*10);

  gnm = 0;
  if (lua_gettop(L) == 1 && lua_type( L, 1 ) == LUA_TSTRING) {
    // file name is given
    size_t len;
    const char *fname = luaL_checklstring( L, 1, &len );
    strcpy(fnm, fname);
    gnm = 1; // file name ok
  }
  if (gnm == 0) memset(fnm, 0x00, 64);

  lua_log_printf(0, NULL, 0, "Start Ymodem file transfer, max size: %d", max_len);

  while (retarget_waitc(&c, 10) == 0) {};

  fsize = Ymodem_Receive(fnm, max_len, gnm);

  while (retarget_waitc(&c, 10) == 0) {};

  if (fsize > 0) printf("\r\nReceived successfully, %d\r\n",fsize);
  else if (fsize == -1) printf("\r\nFile write error!\r\n");
  else if (fsize == -2) printf("\r\nFile open error!\r\n");
  else if (fsize == -3) printf("\r\nAborted.\r\n");
  else if (fsize == -4) printf("\r\nFile size too big, aborted.\r\n");
  else if (fsize == -5) printf("\r\nWrong file name or file exists!\r\n");
  else printf("\r\nReceive failed!\r\n");

  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//==================================
static int file_recv( lua_State* L )
	remote_CCall(&_file_recv);
	return g_shell_result;
}

//===================================
static int _file_send( lua_State* L )
{
  char res = 0;
  char newname = 0;
  unsigned char c;
  const char *fname;
  size_t len;
  int fsize = 0;

  fname = luaL_checklstring( L, 1, &len );
  char filename[64] = {0};
  const char * newfname;

  if (lua_gettop(L) >= 2 && lua_type( L, 2 ) == LUA_TSTRING) {
    size_t len;
    newfname = luaL_checklstring( L, 2, &len );
    newname = 1;
  }

  // Open the file
  int ffd = file_open(fname, 0);
  if (ffd < 0) {
    l_message(NULL,"Error opening file.");
    goto exit;
  }

  // Get file size
  if (vm_fs_get_size(ffd, &fsize) < 0) {
    vm_fs_close(ffd);
    l_message(NULL,"Error opening file.");
    goto exit;
  }

  if (newname == 1) {
    printf("Sending '%s' as '%s'\r\n", fname, newfname);
    strcpy(filename, newfname);
  }
  else strcpy(filename, fname);

  l_message(NULL,"Start Ymodem file transfer...");

  while (retarget_waitc(&c, 10) == 0) {};

  res = Ymodem_Transmit(filename, fsize, ffd);

  vm_thread_sleep(500);
  while (retarget_waitc(&c, 10) == 0) {};

  vm_fs_close(ffd);

  if (res == 0) {
    l_message(NULL,"\r\nFile sent successfuly.");
  }
  else if (res == 99) {
    l_message(NULL,"\r\nNo response.");
  }
  else if (res == 98) {
    l_message(NULL,"\r\nAborted.");
  }
  else {
    l_message(NULL,"\r\nError sending file.");
  }

exit:
  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//==================================
static int file_send( lua_State* L )
	remote_CCall(&_file_send);
	return g_shell_result;
}

#endif

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
#if defined USE_YMODEM
  {LSTRKEY("yrecv"),    	LFUNCVAL(file_recv)},
  {LSTRKEY("ysend"),    	LFUNCVAL(file_send)},
#endif
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
		  {LSTRKEY("showlog"),		LFUNCVAL(os_showlog)},
		  {LSTRKEY("readreg"), 		LFUNCVAL(os_readreg) },
		  {LSTRKEY("writereg"), 	LFUNCVAL(os_writereg) },
		  {LSTRKEY("writertcreg"),	LFUNCVAL(os_writertcreg) },
		  {LSTRKEY("teststack"),	LFUNCVAL(os_teststack) },
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

    LREGISTER(L, LUA_OSLIBNAME, syslib);
}

LUALIB_API int luaopen_sys(lua_State* L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0
    luaL_register(L, "sys", sys_map);

    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

