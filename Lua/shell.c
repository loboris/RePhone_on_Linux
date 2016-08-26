

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "vmsystem.h"
#include "vmwdt.h"
#include "vmlog.h"
//#include "vmthread.h"
#include "vmchset.h"
#include "vmmemory.h"
#include "vmfs.h"
#include "vmgsm_gprs.h"

#define lua_c

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"
#include "shell.h"
#include "sntp.h"
#include "legc.h"
#include "term.h"


lua_State *shellL = NULL;			// Lua state
lua_State *ttyL = NULL;				// Lua state

vm_thread_message_t g_shell_message  = {SHELL_MESSAGE_ID, 0};
vm_thread_message_t g_fcall_message  = {CCALL_MESSAGE_FOPEN, &g_CCparams};
vm_thread_message_t g_cbcall_message = {CB_MESSAGE_ID, &g_CBparams};

//int g_shell_result;
lua_CFunction g_CCfunc = NULL;
int CCwait = 1000;
char* param_buf = NULL;

VM_DCL_OWNER_ID g_owner_id = 0;  // Module owner of APP
VM_DCL_HANDLE retarget_usb_handle = -1;
VM_DCL_HANDLE retarget_uart1_handle = -1;
int retarget_target = -1;
int g_rtc_poweroff = 1;

//vm_mutex_t retarget_rx_mutex;

static lua_State *globalL = NULL;
static const char *progname = LUA_PROGNAME;

extern int show_log;
extern int sys_wdt_rst_time;
extern int no_activity_time;
extern int _mqtt_check(mqtt_info_t *p);
extern void retarget_setup();
extern VMUINT8 uart_has_userdata[2];
extern uart_info_t *uart_data[2];
extern int _read_params(lua_State *L);
extern int _gsm_readbuf_free(lua_State *L);
extern unsigned int g_memory_size;
extern unsigned int g_memory_size_b;
extern vm_gsm_gprs_apn_info_t apn_info;
extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;
extern int lcd_init( lua_State* L );


//-------------------------------------------------------------
void full_fname(char *fname, VMWCHAR *ucs_name, int size)
{
	char file_name[128];
	const char *ptr;
	if (fname[1] != ':') {
		snprintf(file_name, 127, "C:\\%s", fname);
		ptr = file_name;
	}
	else ptr = fname;
	vm_chset_ascii_to_ucs2(ucs_name, size, ptr);
}

//----------------------------------------
int file_open(const char* file, int flags)
{
    VMUINT fs_mode;

    if(flags & O_CREAT) {
        fs_mode = VM_FS_MODE_CREATE_ALWAYS_WRITE;
    } else if((flags & O_RDWR) || (flags & O_WRONLY)) {
        fs_mode = VM_FS_MODE_WRITE;
    } else {
        fs_mode = VM_FS_MODE_READ;
    }

    if(flags & O_APPEND) {
        fs_mode |= VM_FS_MODE_APPEND;
    }

    g_fcall_message.message_id = CCALL_MESSAGE_FOPEN;
    g_CCparams.cpar1 = (char *)file;
    g_CCparams.ipar1 = fs_mode;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//----------------------
int file_close(int file)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FCLOSE;
    g_CCparams.ipar1 = file;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//----------------------
int file_flush(int file)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FFLUSH;
    g_CCparams.ipar1 = file;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//---------------------
int file_size(int file)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FSIZE;
    g_CCparams.ipar1 = file;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//-----------------------------------------
int file_read(int file, char* ptr, int len)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FREAD;
    g_CCparams.ipar1 = file;
    g_CCparams.ipar2 = len;
    g_CCparams.cpar1 = ptr;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//------------------------------------------
int file_write(int file, char* ptr, int len)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FWRITE;
    g_CCparams.ipar1 = file;
    g_CCparams.ipar2 = len;
    g_CCparams.cpar1 = ptr;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//---------------------------------------------
int file_seek(int file, int offset, int whence)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FSEEK;
    g_CCparams.ipar1 = file;
    g_CCparams.ipar2 = offset;
    g_CCparams.ipar3 = whence + 1;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//-----------------------------------
int file_exists(const char *filename)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FCHECK;
    g_CCparams.cpar1 = (char *)filename;
    g_CCparams.ipar1 = VM_FS_MODE_READ;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}



//-----------------------------------------------
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

//---------------------------
static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

//---------------------------------------------------
void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}

//--------------------------------------------
static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}

//-----------------------------------
static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1) && !lua_isrotable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1) && !lua_islightfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

//-----------------------------------------------------
static int docall (lua_State *L, int narg, int clear) {
  int status;
  lua_lock(L);
  int base = lua_gettop(L) - narg;  // function index
  lua_pushcfunction(L, traceback);  // push traceback function
  lua_insert(L, base);  			// put it under chunk and args
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  lua_unlock(L);
  return status;
}

//-----------------------------------------------------------
static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}

//------------------------------------------------
static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}

//-------------------------------------------------
static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  int lpos = 0;
  size_t l;
  const char *prmt = get_prompt(L, firstline);

  if (lua_readline(L, b, prmt)) return 0;	// !!no input!!

  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')				// line ends with newline?
    b[l-1] = '\0';							// remove it
  if (firstline && b[0] == '=')				// first line starts with '=' ?
    lua_pushfstring(L, "return %s", b+1);	// change it to 'return'
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}

//----------------------------------
static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1)) return -1;       // no input

  for (;;) {							// repeat until gets a complete line
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  // cannot try to add lines?
    if (!pushline(L, 0))  return -1;    // no more input?

    lua_pushliteral(L, "\n");           // add a new line...
    lua_insert(L, -2);  				// ...between the two lines
    lua_concat(L, 3);  					// join them
  }
  lua_saveline(L, 1);
  //lua_remove(L, 1);						// remove line
  return status;
}


//=============================
// Executed from Lua tty thread
//=============================
void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;

  while ((status = loadline(L)) != -1) {
    //if (status == 0) status = remote_docall();
    if (status == 0) {
    	//status = docall(L, 0, 0);
    	lua_remove(L, -1);  // remove function
    	size_t len;
    	const char *line = lua_tolstring(L, 1, &len);
    	lua_remove(L, -1);  // remove string

        g_cbcall_message.message_id = CB_FUNC_DOTTY;
        g_cbcall_message.user_data = (char *)line;
        vm_thread_send_message(g_shellthread_handle, &g_cbcall_message);
        vm_signal_wait(g_tty_signal);
        status = 0;
    }
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  // any result to print?
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  // clear stack
  fputs("\n", stdout);
  fflush(stdout);
  progname = oldprogname;
}


//--------------------------------------
// Execute Lua C function in main thread
//--------------------------------------
int remote_CCall(lua_State *L, lua_CFunction func)
{
	sys_wdt_rst_time = 0;

	g_CCfunc = func;
    g_shell_message.message_id = SHELL_MESSAGE_ID;
    g_shell_message.user_data = L;
    vm_thread_send_message(g_main_handle, &g_shell_message);

    // wait for call to finish...
    if (vm_signal_timed_wait(g_shell_signal, CCwait) != 0) {
    	g_shell_result = -1;
    	printf("NO SIGNAL FROM C-FUNCTION!\n");
    }
    CCwait = DEFAULT_CCWAIT;
	g_CCfunc = NULL;

    return g_shell_result;
}

//-----------------------------------------------
void remote_lua_call(VMUINT16 type, void *params)
{
	sys_wdt_rst_time = 0;
    g_cbcall_message.message_id = type;
    g_cbcall_message.user_data = params;
    vm_thread_send_message(g_shellthread_handle, &g_cbcall_message);
}

//---------------------------------------------------------------
static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

/*
//-------------------------------------------------------------------
static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)	// undefined?
    return;  		// does not set field
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}
*/

//--------------------------------------
static int _set_custom_apn(lua_State *L)
{
	gprs_bearer_type = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN;
    vm_gsm_gprs_set_customized_apn_info(&apn_info);
	vm_signal_post(g_shell_signal);
    return 0;
}

//======================================
static int _get_ntptime (lua_State *L) {

  int tz = luaL_checkinteger( L, -1 );
  sntp_gettime(tz, 0);

  vm_signal_post(g_shell_signal);
  return 0;
}

//-----------------------------------------
static void setup_from_params(lua_State *L)
{
	remote_CCall(L,&_read_params);

	if (g_shell_result < 0) return;

	// execute, set params
	if (lua_pcall(L, 0, 0, 0) != 0) return;

	lua_getglobal(L, "__SYSPAR");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);  // removes nil
		return;
	}

	printf("System parameters loaded\n");
    size_t klen = 0;
    size_t vlen = 0;
    const char* key;
    int ntpreq = 0;
    int lcdinit = -1;
    int tz = 0;

    lua_pushnil(L);  // first key
    while (lua_next(L, 1) != 0) {
      // Pops a key from the stack, and pushes a key-value pair from the table
      // 'key' (at index -2) and 'value' (at index -1)
	  if (lua_type(L, -2) == LUA_TSTRING) {
		  key = lua_tolstring(L, -2, &klen);

		  if (lua_type(L, -1) == LUA_TSTRING) {
		    const char* value = lua_tolstring(L, -1, &vlen);

			if ((klen == 3) && (strstr(key, "apn") == key)) {
				sprintf(apn_info.apn, value);
				printf("        'apn' = \"%s\"\n", apn_info.apn);
				remote_CCall(L,&_set_custom_apn);
				ntpreq++;
			}
		  }
		  else if (lua_type(L, -1) == LUA_TNUMBER) {
			int value = lua_tointeger(L, -1);

			if ((klen == 3) && (strstr(key, "ntp") == key)) {
				if ((value <= 14) && (value >= -12)) {
					ntpreq++;
					tz = value;
				}
			}
			else if ((klen == 7) && (strstr(key, "lcdinit") == key)) {
			    lcdinit = value;
				printf("    'lcdinit' = %d\n", value);
			}
			else if ((klen == 7) && (strstr(key, "termcol") == key)) {
				term_num_cols = value;
				printf("    'termcol' = %d\n", value);
			}
			else if ((klen == 7) && (strstr(key, "termlin") == key)) {
				term_num_lines = value;
				printf("    'termlin' = %d\n", value);
			}
			else if ((klen == 9) && (strstr(key, "inputtype") == key)) {
				use_term_input = value;
				printf("  'inputtype' = %d\n", value);
			}
		  }
	  }
      lua_pop(L, 1);  // removes 'value'; keeps 'key' for next iteration
    }
    lua_pop(L, 1);  // removes '__SYSPAR' table

    if (lcdinit >= 0) {
		lua_pushinteger(L, lcdinit);	// push lcd type param
		lcd_init(L);
		lua_pop(L, 1);					// pop lcd type param
    }
    if (ntpreq > 1) {
		lua_pushinteger(L, tz);			// push tz param
		remote_CCall(L,&_get_ntptime);
		lua_pop(L, 1);					// pop tz param
		printf("        'ntp'   time requested, tz=%d\n", tz);
    }
}

//===================================================================
VMINT32 shell_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    lua_State *L = (lua_State *)user_data;
    vm_thread_message_t message;

    g_main_handle = vm_thread_get_main_handle();
    g_shellthread_handle = vm_thread_get_current_handle();

    legc_set_mode(L, EGC_ON_MEM_LIMIT | EGC_ON_ALLOC_FAILURE, g_memory_size);

    g_shell_message.message_id = SHELL_MESSAGE_ID;

    retarget_setup();

    printf("\nLua memory: %d bytes, C heap: %d bytes\n", g_memory_size, g_memory_size_b);

    lua_settop(L, 0);		// clear stack

	show_log = 0;
    setup_from_params(L);	// Check system parameters
	show_log = 0x1F;

    lua_settop(L, 0);		// clear stack

    // Check startup files
    if (file_exists("init.lc") >= 0) {
        printf("Executing 'init.lc'\n");
        luaL_dofile(L, "init.lc");
    }
    else if (file_exists("init.lua") >= 0) {
        printf("Executing 'init.lua'\n");
        luaL_dofile(L, "init.lua");
    }
    if (file_exists("autorun.lc") >= 0) {
        printf("Executing 'autorun.lc'\n");
        luaL_dofile(L, "autorun.lc");
    }
    else if (file_exists("autorun.lua") >= 0) {
        printf("Executing 'autorun.lua'\n");
        luaL_dofile(L, "autorun.lua");
    }

    lua_settop(L, 0);  // clear stack
	vm_signal_post(g_tty_signal); // start tty

	// ==== Main loop ====
    while (1) {
    	vm_thread_get_message(&message);
        switch (message.message_id) {
            case CB_FUNC_DOTTY: {
            		no_activity_time = 0;
            		char *line = (char *)message.user_data;
            		lua_pushstring(L, line);
            		int status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
            		lua_remove(L, 1);	// remove line
            	    if (!incomplete(L, status)) {
            	    	status = docall(L, 0, 0);
            	    }
            	    report(L, status);
            	    if (status == 0 && lua_gettop(L) > 0) {  // any result to print?
            	      lua_getglobal(L, "print");
            	      lua_insert(L, 1);
            	      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
            	        l_message(progname, lua_pushfstring(L,
            	                               "error calling " LUA_QL("print") " (%s)",
            	                               lua_tostring(L, -1)));
            	    }
                	vm_signal_post(g_tty_signal);
                }
                break;

            case CB_FUNC_TIMER: {
            		timer_info_t *params = (timer_info_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->timer_id);
	                lua_pcall(L, 1, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_MQTT_TIMER: {
            		mqtt_info_t *params = (mqtt_info_t*)message.user_data;
           			_mqtt_check(params);
                }
                break;

            case CB_FUNC_MQTT_MESSAGE: {
            		mqtt_info_t *params = (mqtt_info_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref_message);
					lua_pushinteger(L, params->mRecBuf_len);
					lua_pushstring(L, params->mRecTopic);	// received message topic
					lua_pushstring(L, params->mRecBuf);		// received message
	                lua_pcall(L, 3, 0, 0);
					// Free message buffer
					vm_free(params->mRecBuf);
					params->mRecBuf = NULL;
                }
                break;

            case CB_FUNC_MQTT_DISCONNECT: {
            		mqtt_info_t *params = (mqtt_info_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref_disconnect);
					lua_pushstring(L, params->pServer);
	                lua_pcall(L, 1, 0, 0);
                }
                break;

            case CB_FUNC_INT: { // gsm_send, gsm_delete, sntp
            		cb_func_param_int_t *params = (cb_func_param_int_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->par);
	                lua_pcall(L, 1, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_EINT: {
            		cb_func_param_eint_t *params = (cb_func_param_eint_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->pin);
					lua_pushinteger(L, params->state);
					lua_pushinteger(L, params->count);
					lua_pushinteger(L, params->time);
	                lua_pcall(L, 4, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_TOUCH: {
            		cb_func_param_touch_t *params = (cb_func_param_touch_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->event);
					lua_pushinteger(L, params->x);
					lua_pushinteger(L, params->y);
	                lua_pcall(L, 3, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_REBOOT: {
            		cb_func_param_int_t *params = (cb_func_param_int_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->par);
	                lua_pcall(L, 1, 0, 0);
	                params->busy = 0;
                	//vm_signal_post(g_reboot_signal);
                }
                break;

            case CB_FUNC_ADC: {
            		cb_func_param_adc_t *params = (cb_func_param_adc_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->ival);
					lua_pushnumber(L, params->fval);
					lua_pushinteger(L, params->chan);
	                lua_pcall(L, 3, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_SMS_LIST: {
            		cb_func_param_smslist_t *params = (cb_func_param_smslist_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
	                lua_newtable( L );
	                if (params->list->message_id_list) {
	                    if (params->list->message_number > 0) {
							int i;
							int j = params->list->message_number;
							for (i=0; i < j; i++) {
								lua_pushinteger(L, (VMUINT16)params->list->message_id_list[i]);
								lua_rawseti(L,-2, i+1);
							}
	                    }
	                }
	                lua_pcall(L, 1, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_SMS_READ: {
            		cb_func_param_smsread_t *params = (cb_func_param_smsread_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);

	                if (params->stat == 3) {
						char msgdata[MAX_SMS_CONTENT_LEN] = {0};
						vm_date_time_t timestamp = params->msg->message_data->timestamp;
						//vm_date_time_t timestamp;
						//memcpy(&timestamp, &params->msg->message_data->timestamp, sizeof(vm_date_time_t));
						vm_chset_ucs2_to_ascii((VMSTR)msgdata, MAX_SMS_CONTENT_LEN, (VMCWSTR)(params->msg->message_data->number));
						lua_pushstring(L, msgdata);				// sender number
						lua_createtable(L, 0, 6);      			// message time as table, 6 = number of fields
						setfield(L, "sec", timestamp.second);
						setfield(L, "min", timestamp.minute);
						setfield(L, "hour", timestamp.hour);
						setfield(L, "day", timestamp.day);
						setfield(L, "month", timestamp.month);
						setfield(L, "year", timestamp.year);

						vm_chset_ucs2_to_ascii((VMSTR)msgdata, 320, (VMCWSTR)(params->msg->message_data->content_buffer));
						lua_pushstring(L, msgdata);					// received message
	                }
	                else lua_pushnil(L);

	        		remote_CCall(L,&_gsm_readbuf_free);
	                lua_pcall(L, params->stat, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_SMS_NEW: {
            		cb_func_param_smsnew_t *params = (cb_func_param_smsnew_t*)message.user_data;
            	    vm_gsm_sms_new_message_t * new_message_ptr = NULL;
            	    char content[MAX_SMS_CONTENT_LEN];
                    // Gets the message data.
                    new_message_ptr  =  params->msg->message_data;
                    // Converts the message content to ASCII.
                    vm_chset_ucs2_to_ascii((VMSTR)content, MAX_SMS_CONTENT_LEN, (VMWSTR)params->msg->content);

                    lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
	                lua_pushinteger(L, new_message_ptr->message_id);
	                lua_pushstring(L, new_message_ptr->number);
	                lua_pushstring(L, content);
	                lua_pcall(L, 3, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_HTTPS_HEADER: {
            		cb_func_param_httpsheader_t *params = (cb_func_param_httpsheader_t*)message.user_data;

        			lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);

        	    	lua_pushlstring(L, params->header, params->len);
					lua_pcall(L, 1, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_HTTPS_DATA: {
            		cb_func_param_httpsdata_t *params = (cb_func_param_httpsdata_t*)message.user_data;

            		lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);

					lua_pushinteger(L, params->state);
            	    if (params->reply != NULL) {
            	    	lua_pushlstring(L, params->reply, params->len);
						free(params->reply);
            	    }
            	    else {
            	    	lua_pushstring(L, "__Receive_To_File__");
            	    }
					lua_pushinteger(L, params->len);
					lua_pushinteger(L, params->more);

					lua_pcall(L, 4, 0, 0);

					params->reply = NULL;
					params->maxlen = 0;
					params->len = 0;
					params->more = 0;
					params->busy = 0;
					params->state = 0;
                }
                break;

            case CB_FUNC_NET: {
					cb_func_param_net_t *params = (cb_func_param_net_t*)message.user_data;
			        lua_rawgeti(L, LUA_REGISTRYINDEX, params->net_info->cb_ref);
			    	if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
						lua_pushlightuserdata(L, params->net_info);
						luaL_getmetatable(L, LUA_NET);
						lua_setmetatable(L, -2);
						lua_pushinteger(L, params->event);
						lua_pcall(L, 2, 0, 0);
			    	}
			    	else {
			    		lua_remove(L, -1);
			    	}
            	}
            	break;

            case CB_FUNC_UART_RECV: {
            		uart_info_t *params = (uart_info_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushinteger(L, params->uart_id);
					lua_pushinteger(L, params->bufptr);
					if ((retarget_target == -1) && (strstr((const char *)params->buffer, "<Return2ShEll>") == (const char *)params->buffer)) {
					    if (params->uart_id == 0) retarget_target = retarget_usb_handle;
					    else  retarget_target = retarget_uart1_handle;
						params->uart_id = -1;
						params->buffer[0] = 0;
						luaL_unref(L, LUA_REGISTRYINDEX, params->cb_ref);
						params->cb_ref = LUA_NOREF;
					    uart_has_userdata[params->uart_id] = 0;
					    uart_data[params->uart_id] = NULL;

						lua_pushstring(L, "Returned to shell");
					}
					else lua_pushlstring(L, params->buffer, params->bufptr);
	                params->bufptr = 0;
	                lua_pcall(L, 3, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_BT_RECV: {
            		cb_func_param_bt_t *params = (cb_func_param_bt_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushstring(L, params->addr);
					lua_pushinteger(L, params->bufptr);
					if ((retarget_target != -1000) && (strstr(params->recvbuf, "<Return2ShEll>") == params->recvbuf)) {
						retarget_target = -1000;
						lua_pushstring(L, "Returned to BT shell");
					}
					else lua_pushstring(L, params->recvbuf);
	                params->bufptr = 0;
	                lua_pcall(L, 3, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_BT_CONNECT: {
            		cb_func_param_bt_t *params = (cb_func_param_bt_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushstring(L, params->addr);
					lua_pushinteger(L, params->status);
	                lua_pcall(L, 2, 0, 0);
	                params->busy = 0;
                }
                break;

            case CB_FUNC_BT_DISCONNECT: {
            		cb_func_param_bt_t *params = (cb_func_param_bt_t*)message.user_data;
	                lua_rawgeti(L, LUA_REGISTRYINDEX, params->cb_ref);
					lua_pushstring(L, params->addr);
	                lua_pcall(L, 1, 0, 0);
	                /*if (params->disconnect_ref != LUA_NOREF) {
	                	luaL_unref(L, LUA_REGISTRYINDEX, params->disconnect_ref);
	                	params->disconnect_ref = LUA_NOREF;
	                }*/
	                params->busy = 0;
                }
                break;
        }
    }
    // ^^^^ Main loop ^^^^

    printf("\nLUA COMMAND THREAD EXITED, RESTART!\n");
    g_shell_message.message_id = SHELL_MESSAGE_QUIT;
    vm_thread_send_message(g_main_handle, &g_shell_message);
    return 0;
}
//===================================================================

//================================================================
VMINT32 tty_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    lua_State *L = (lua_State *)user_data;

    g_ttythread_handle = vm_thread_get_current_handle();

    vm_signal_wait(g_tty_signal);

    lua_settop(L, 0);  // clear stack
    printf("LUA SHELL STARTED [ver.: %s] [%s]\n", LUA_RELEASE, LUA_RELEASE_DATE);
    //===============================================================
    dotty(L);
    //===============================================================

    printf("\nLUA SHELL THREAD EXITED, RESTART!\n");
    g_shell_message.message_id = SHELL_MESSAGE_QUIT;
    vm_thread_send_message(g_main_handle, &g_shell_message);
    return 0;
}
//================================================================
