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

#include "vmdatetime.h"
#include "sntp.h"
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

#include "ymodem.h"

extern int retarget_target;
extern VM_DCL_HANDLE retarget_device_handle;
extern VM_DCL_HANDLE retarget_uart1_handle;
extern void retarget_putc(char ch);
extern int retarget_waitc(unsigned char *c, int timeout);
extern int retarget_waitchars(unsigned char *buf, int *count, int timeout);


//-----------------------------------------------------------------------------
void lua_log_printf(int type, const char *file, int line, const char *msg, ...)
{
  if ((retarget_target != retarget_device_handle) && (retarget_target != retarget_uart1_handle)) return;

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

  if (type == 1) printf("\n[FATAL]");
  else if (type == 2) printf("\n[ERROR]");
  else if (type == 3) printf("\n[WARNING]");
  else if (type == 4) printf("\n[INFO]");
  else if (type == 5) printf("\n[DEBUG]");
  if (type > 0) printf(" %s:%d ", file, line);

  printf("%s\n", message);
}


/*
//====================================================================
static int os_pushresult (lua_State *L, int i, const char *filename) {
  int en = errno;  // calls to Lua API may change this value
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    lua_pushfstring(L, "%s: %s", filename, strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}

static int os_execute (lua_State *L) {
  lua_pushinteger(L, system(luaL_optstring(L, 1, NULL)));
  return 1;
}
*/

//===================================
static int os_remove (lua_State *L) {
  VMWCHAR ucs_name[VM_FS_MAX_PATH_LENGTH+1];
  const char *filename = luaL_checkstring(L, 1);

  vm_chset_ascii_to_ucs2(ucs_name, VM_FS_MAX_PATH_LENGTH, filename);

  lua_pushinteger(L, vm_fs_delete(ucs_name));

  return 1;
}

//===================================
static int os_rename (lua_State *L) {
  VMWCHAR ucs_oldname[VM_FS_MAX_PATH_LENGTH+1];
  VMWCHAR ucs_newname[VM_FS_MAX_PATH_LENGTH+1];

  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);

  vm_chset_ascii_to_ucs2(ucs_oldname, VM_FS_MAX_PATH_LENGTH, fromname);
  vm_chset_ascii_to_ucs2(ucs_newname, VM_FS_MAX_PATH_LENGTH, toname);

  lua_pushinteger(L, vm_fs_rename(ucs_oldname, ucs_newname));

  return 1;
}

/*
static int os_tmpname (lua_State *L) {
  char buff[64]; //LUA_TMPNAMBUFSIZE];
  int err;
  lua_tmpnam(buff, err);
  if (err)
    return luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  return 1;
}

static int os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  // if NULL push nil
  return 1;
}
*/

//==================================
static int os_clock (lua_State *L) {
  lua_pushnumber(L, ((float)vm_time_ust_get_count())/(float)CLOCKS_PER_SEC);
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
    //t = time(NULL);  /* get current time */
    vm_time_get_unix_time(&rtct);  /* get current time */
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

//===================================
static int os_battery (lua_State *L) {
	lua_pushinteger(L, vm_pwr_get_battery_level());
	return 1;
}

//===================================
static int os_reboot (lua_State *L) {
    vm_log_info("REBOOT");
	vm_pwr_reboot();
	return 0;
}

//=====================================
static int os_shutdown (lua_State *L) {
	vm_log_info("SHUTDOWN");
	vm_pwr_shutdown(777);
	return 0;
}

//==============================================
static int os_scheduled_startup (lua_State *L) {
  VMUINT rtct;
  vm_date_time_t start_time;
  struct tm *time_now;
  unsigned long epoch;

  vm_time_get_unix_time(&rtct);  /* get current time */

  int nsec = luaL_checkinteger( L, 1 );
  if (nsec < 86400) epoch = rtct + nsec;
  else epoch = nsec;

  time_now = gmtime(&epoch);
  start_time.day = time_now->tm_mday;
  start_time.hour = time_now->tm_hour;
  start_time.minute = time_now->tm_min;
  start_time.second = time_now->tm_sec;
  start_time.month = time_now->tm_mon + 1;
  start_time.year = time_now->tm_year + 1900;

  vm_pwr_scheduled_startup(&start_time, VM_PWR_STARTUP_ENABLE_CHECK_DHMS);
  vm_log_info("WAKE UP SCHEDULED at %s", asctime(time_now));
  //vm_pwr_shutdown(778);
  return 0;
}

//====================================
static int os_ntptime (lua_State *L) {

  int sntp_cb = LUA_NOREF;
  int tz = luaL_checkinteger( L, 1 );
  if ((tz > 14) || (tz < -12)) { tz = 0; }

  if (lua_gettop(L) >= 2) {
	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	  lua_pushvalue(L, 2);
	  sntp_cb = luaL_ref(L, LUA_REGISTRYINDEX);
  	}
  }
  sntp_gettime(tz, sntp_cb);

  return 0;
}

//===========================
int os_getver(lua_State* L) {
	VMCHAR value[128] = {0};
	VMUINT written = vm_firmware_get_info(value, sizeof(value)-1, VM_FIRMWARE_HOST_VERSION);
	lua_pushstring(L, value);
	written = vm_firmware_get_info(value, sizeof(value)-1, VM_FIRMWARE_BUILD_DATE_TIME);
	lua_pushstring(L, value);
    return 2;
}

extern int g_memory_size_b;

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

//==================================
static int os_mkdir (lua_State *L) {
  const char *dirname = luaL_checkstring(L, 1);
  VMWCHAR ucs_dirname[VM_FS_MAX_PATH_LENGTH+1];

  vm_chset_ascii_to_ucs2(ucs_dirname, VM_FS_MAX_PATH_LENGTH, dirname);

  lua_pushinteger(L, vm_fs_create_directory(ucs_dirname));
  return 1;
}

//==================================
static int os_rmdir (lua_State *L) {
  const char *dirname = luaL_checkstring(L, 1);
  VMWCHAR ucs_dirname[VM_FS_MAX_PATH_LENGTH+1];

  vm_chset_ascii_to_ucs2(ucs_dirname, VM_FS_MAX_PATH_LENGTH, dirname);

  lua_pushinteger(L, vm_fs_remove_directory(ucs_dirname));
  return 1;
}

//===================================
static int os_copy (lua_State *L) {
  VMWCHAR ucs_fromname[VM_FS_MAX_PATH_LENGTH+1];
  VMWCHAR ucs_toname[VM_FS_MAX_PATH_LENGTH+1];

  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);

  vm_chset_ascii_to_ucs2(ucs_fromname, VM_FS_MAX_PATH_LENGTH, fromname);
  vm_chset_ascii_to_ucs2(ucs_toname, VM_FS_MAX_PATH_LENGTH, toname);

  lua_pushinteger(L, vm_fs_copy(ucs_toname, ucs_fromname, NULL));

  return 1;
}

/*
//==============================
int os_wdtreset (lua_State *L) {
  if (sys_wdt_id) {
	sys_wdt_rst_time = 0;
  }
  return 0;
}

//====================================
static int os_wdtstop (lua_State *L) {
  if (sys_wdt_id) {
	vm_log_info("**STOP WATCHDOG\n");
	sys_wdt_id = 0;
	sys_wdt_rst_time = 0;
  }
  return 0;
}

//=====================================
static int os_wdtstart (lua_State *L) {
  int tick = luaL_checkinteger(L, 1);
  if (tick < 2) tick = 2;
  if (tick > 600) tick = 600;

  //sys_wdt_tmo = tick*1000;
  if (!sys_wdt_id) {
	vm_log_info("START WATCHDOG %d sec\n", tick);
	sys_wdt_id = 1;
	sys_wdt_rst_time = 0;
  }
  return 0;
}
*/

//===================================
static int os_listfiles(lua_State* L)
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

    return 0;
}

//=================================
static int os_exit (lua_State *L) {
  //exit(luaL_optint(L, 1, EXIT_SUCCESS));
  printf("\n[SYSEVT] APP RESTART\n");
  vm_pmng_restart_application();
  return 0;
}

//=====================================
static int os_retarget (lua_State *L) {
	int targ = luaL_checkinteger(L,1);

	if ((targ == 0) && (retarget_device_handle >= 0)) retarget_target = retarget_device_handle;
	else if ((targ == 1) && (retarget_uart1_handle >= 0)) retarget_target = retarget_uart1_handle;
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


//==================================
static int file_recv( lua_State* L )
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

  return 0;
}


//==================================
static int file_send( lua_State* L )
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
    return 0;
  }

  // Get file size
  if (vm_fs_get_size(ffd, &fsize) < 0) {
    vm_fs_close(ffd);
    l_message(NULL,"Error opening file.");
    return 0;
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
  return 0;
}


extern int luaB_dofile (lua_State *L);


#define MIN_OPT_LEVEL 1
#include "lrodefs.h"
const LUA_REG_TYPE syslib[] = {
  {LSTRKEY("ntptime"),  LFUNCVAL(os_ntptime)},
  {LSTRKEY("shutdown"), LFUNCVAL(os_shutdown)},
  {LSTRKEY("reboot"),   LFUNCVAL(os_reboot)},
  {LSTRKEY("battery"),  LFUNCVAL(os_battery)},
  {LSTRKEY("ver"),  	LFUNCVAL(os_getver)},
  {LSTRKEY("mem"),  	LFUNCVAL(os_getmem)},
  {LSTRKEY("schedule"), LFUNCVAL(os_scheduled_startup)},
  {LSTRKEY("clock"),    LFUNCVAL(os_clock)},
  {LSTRKEY("date"),     LFUNCVAL(os_date)},
  {LSTRKEY("difftime"), LFUNCVAL(os_difftime)},
  {LSTRKEY("execute"), LFUNCVAL(luaB_dofile)},
  //{LSTRKEY("execute"),LFUNCVAL(os_execute)},
  {LSTRKEY("exit"),   LFUNCVAL(os_exit)},
  //{LSTRKEY("getenv"), LFUNCVAL(os_getenv)},
  {LSTRKEY("remove"),   LFUNCVAL(os_remove)},
  {LSTRKEY("rename"),   LFUNCVAL(os_rename)},
  {LSTRKEY("copy"),     LFUNCVAL(os_copy)},
  {LSTRKEY("mkdir"),    LFUNCVAL(os_mkdir)},
  {LSTRKEY("rmdir"),    LFUNCVAL(os_rmdir)},
  {LSTRKEY("setlocale"),LFUNCVAL(os_setlocale)},
  {LSTRKEY("time"),     LFUNCVAL(os_time)},
  {LSTRKEY("list"),     LFUNCVAL(os_listfiles)},
  //{LSTRKEY("tmpname"),   LFUNCVAL(os_tmpname)},
  //{LSTRKEY("wdtstart"), LFUNCVAL(os_wdtstart)},
  //{LSTRKEY("wdtstop"),  LFUNCVAL(os_wdtstop)},
  //{LSTRKEY("wdtreset"), LFUNCVAL(os_wdtreset)},
  {LSTRKEY("retarget"), LFUNCVAL(os_retarget)},
  {LSTRKEY("getchar"), LFUNCVAL(os_getchar)},
  {LSTRKEY("getstring"), LFUNCVAL(os_getstring)},
  {LSTRKEY("putchar"), LFUNCVAL(os_putchar)},
  {LSTRKEY("yrecv"), LFUNCVAL(file_recv)},
  {LSTRKEY("ysend"), LFUNCVAL(file_send)},
  {LNILKEY, LNILVAL}
};

/* }====================================================== */



LUALIB_API int luaopen_os (lua_State *L) {
  LREGISTER(L, LUA_OSLIBNAME, syslib);
}
