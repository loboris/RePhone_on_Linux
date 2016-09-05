
#include "vmstdlib.h"

//#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>

#include "vmsystem.h"
#include "vmwdt.h"
#include "vmlog.h"
//#include "vmthread.h"
#include "vmchset.h"
#include "vmmemory.h"
#include "vmfs.h"
#include "vmgsm_gprs.h"

#include "shell.h"
//#include "legc.h"
#include "CheckSumUtils.h"

#define MAX_SYSTEM_PARAMS_SIZE	1020
#define SYSVAR_SIZE 20


vm_thread_message_t g_shell_message  = {SHELL_MESSAGE_ID, 0};
vm_thread_message_t g_fcall_message  = {CCALL_MESSAGE_FOPEN, &g_CCparams};
vm_thread_message_t g_cbcall_message = {CB_MESSAGE_ID, &g_CBparams};

int CCwait = 1000;
char* param_buf = NULL;

VM_DCL_OWNER_ID g_owner_id = 0;  // Module owner of APP
VM_DCL_HANDLE retarget_usb_handle = -1;
VM_DCL_HANDLE retarget_uart1_handle = -1;
int retarget_target = -1;
int g_rtc_poweroff = 1;
int show_log = 0;

//vm_mutex_t retarget_rx_mutex;

//extern int show_log;
//extern int sys_wdt_rst_time;
//extern int no_activity_time;
//extern void retarget_setup();
//extern VMUINT8 uart_has_userdata[2];
//extern uart_info_t *uart_data[2];
//extern int _read_params(lua_State *L);
//extern int _gsm_readbuf_free(lua_State *L);
//extern unsigned int g_memory_size;
//extern unsigned int g_memory_size_b;
extern int g_reserved_heap;
extern int sys_wdt_tmo;				// HW WDT timeout in ticks: 13001 -> 59.999615 seconds
extern int sys_wdt_rst;				// time at which hw wdt is reset in ticks: 10834 -> 50 seconds, must be < 'sys_wdt_tmo'

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


