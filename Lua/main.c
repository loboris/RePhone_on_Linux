
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vmtype.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmgsm_tel.h"
#include "vmgsm_sim.h"
#include "vmtimer.h"
#include "vmdcl.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"
#include "vmthread.h"
#include "vmwdt.h"
#include "vmpwr.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdatetime.h"
#include "vmusb.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmmemory.h"

#include "shell.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

//#include "ts_wdt_sw.h"


//#define USE_SCREEN_MODULE

// used external functions
extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);

extern int luaopen_sys(lua_State *L);
extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);
extern int luaopen_bt(lua_State *L);
extern int luaopen_uart(lua_State *L);
extern int luaopen_timer(lua_State *L);
extern int luaopen_gpio(lua_State *L);
extern int luaopen_i2c(lua_State *L);
extern int luaopen_spi(lua_State *L);
extern int luaopen_net(lua_State* L);
extern int luaopen_https(lua_State* L);
extern int luaopen_struct(lua_State* L);
extern int luaopen_sensor(lua_State* L);
extern int luaopen_cjson(lua_State *l);
extern int luaopen_cjson_safe(lua_State *l);
extern int luaopen_hash_md5(lua_State *L);
extern int luaopen_hash_sha1(lua_State *L);
extern int luaopen_hash_sha2(lua_State *L);
extern int luaopen_bit(lua_State *L);
extern int luaopen_mqtt(lua_State *L);
extern int luaopen_email(lua_State *L);
extern int luaopen_ftp(lua_State *L);
extern int luaopen_lcd(lua_State *L);
extern int luaopen_term(lua_State *L);
//extern int luaopen_marshal(lua_State *L);
#if defined USE_SCREEN_MODULE
extern int luaopen_screen(lua_State *L);
#endif

extern int uart_tmo[2];
extern void _uart_cb(int id);
extern int btspp_tmo;
extern void _btspp_recv_cb(void);
extern int _LcdSpiTransfer(uint8_t *buf, int len);
extern void _sitronix_LcdTransfer(uint8_t *buf, int len);


#define SYS_TIMER_INTERVAL   22		// HISR timer interval in ticks, 22 -> 0.10153 seconds
#define MAX_WDT_RESET_COUNT 145		// max run time (with 50 sec reset = 7250 seconds, ~2 hours)

// Global variables
int sys_wdt_tmo = 13001;			// ** HW WDT timeout in ticks: 13001 -> 59.999615 seconds **
int sys_wdt_rst = 10834;			// time at which hw wdt is reset in ticks: 10834 -> 50 seconds, must be < 'sys_wdt_tmo'
int sys_wdt_rst_time = 0;			// must be set to 0 periodicaly to prevent watchdog timeout
int no_activity_time = 0;			// no activity counter, set to 0 when some activity is taken
int max_no_activity_time = 300;	    // ** time with no activity before shut down in seconds **
int wakeup_interval = 20*60;		// regular wake up interval in seconds
int led_blink = BLUELED;			// led blinking during session, set to 0 for no led blink
int sys_wdt_time = 0;				// used to prevent wdg reset in critical situations (Lua PANIC)
int wdg_reboot_cb = LUA_NOREF;		// Lua callback function called before reboot
int shutdown_cb = LUA_NOREF;		// Lua callback function called before shutdown
int alarm_cb = LUA_NOREF;			// Lua callback function called on alarm
int key_cb = LUA_NOREF;				// Lua callback function called on key up/down
VMUINT8 alarm_flag = 0;
int g_usb_status = 0;				// status of the USB cable connection
int g_graphics_ready = 0;

// Local variables
static int sys_wdt_rst_count = 0;	// watchdog resets counter
static int sys_timer_tick = 0;		// used for timing inside HISR timer callback function
static int do_wdt_reset = 0;		// used for communication between HISR timer and wdg system timer
static uint32_t alarm_state = 0;
static cb_func_param_int_t alarm_cb_param;

static VM_WDT_HANDLE wdg_handle = -1;
static VM_TIMER_ID_NON_PRECISE wdg_timer_id = -1;

static cb_func_param_int_t reboot_cb_params;

//--------------------------
static void key_init(void) {
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD,0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map = 0x05;
    vm_dcl_control(kbd_handle,VM_DCL_KBD_COMMAND_CONFIG_PIN, (void *)(&kbdmap));

    vm_dcl_close(kbd_handle);
}

#define DRV_WriteReg(addr,data)     ((*(volatile uint16_t *)(addr)) = (uint16_t)(data))
#define WDT_RESTART 	            (REG_BASE_ADDRESS+0x0008)
#define WDT_RESTART_KEY		        0x1971

// **hisr timer**, runs every 101,530 msek
// handles wdg reset, blink led
//---------------------------------------------
static void sys_timer_callback(void* user_data)
{
    VM_DCL_HANDLE ghandle;

    sys_wdt_rst_time += SYS_TIMER_INTERVAL;
	sys_wdt_time += SYS_TIMER_INTERVAL;

    if (do_wdt_reset > 0) return;  // wdg reset pending

    if ((sys_wdt_rst_time < sys_wdt_rst) && (sys_wdt_time > sys_wdt_rst)) {
		// ** reset hw wdt later (in '_reset_wdg')
		sys_wdt_rst_count++;
		do_wdt_reset = 1;
		gpio_get_handle(led_blink, &ghandle);
		vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		return;
	}

	if ((led_blink == BLUELED) || (led_blink == REDLED) || (led_blink == GREENLED)) {
		sys_timer_tick += SYS_TIMER_INTERVAL;
		if (sys_timer_tick > (SYS_TIMER_INTERVAL*10)) sys_timer_tick = 0;
		gpio_get_handle(led_blink, &ghandle);
		if (sys_timer_tick == 0) vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		else if (sys_timer_tick == SYS_TIMER_INTERVAL) vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	}
}

// resets the watchdog, must be called from the main thread
//-------------------
void _reset_wdg(void)
{
	if (do_wdt_reset > 0) {
		/* ***********************************************************************
		   ** There is a bug in "vm_wdt_reset", we can only reset wdt 149 times **
		   ** If wdt is reset more then max reset count, we MUST REBOOT!        **
		   ***********************************************************************/
		if (sys_wdt_rst_count >= MAX_WDT_RESET_COUNT) {
			if ((sys_wdt_rst_count = MAX_WDT_RESET_COUNT) && (wdg_reboot_cb != LUA_NOREF)) {
				// execute callback function
				reboot_cb_params.cb_ref = wdg_reboot_cb;
				reboot_cb_params.par = 1;
				remote_lua_call(CB_FUNC_REBOOT, &reboot_cb_params);
			}
			else if (sys_wdt_rst_count > MAX_WDT_RESET_COUNT) {
				vm_log_debug("[SYSTMR] WDT MAX RESETS, REBOOT");
				vm_pwr_reboot();
			}
		}
		vm_wdt_reset(wdg_handle);
		sys_wdt_time = 0;
		do_wdt_reset = 0;
	}
}

//------------------------------
void _scheduled_startup(void) {
  if (wakeup_interval <= 0) return;

  vm_date_time_t start_time;
  VMUINT rtct;
  unsigned long nextt;
  struct tm *time_now;

  // get current time
  vm_time_get_unix_time(&rtct);  /* get current time */
  // find next wake up minute
  nextt = (rtct / 60) * 60; // minute
  nextt = (nextt / wakeup_interval) * wakeup_interval; // start of interval
  while (nextt <= rtct) {
	  nextt += wakeup_interval;
  }
  time_now = gmtime(&nextt);
  start_time.day = time_now->tm_mday;
  start_time.hour = time_now->tm_hour;
  start_time.minute = time_now->tm_min;
  start_time.second = time_now->tm_sec;
  start_time.month = time_now->tm_mon + 1;
  start_time.year = time_now->tm_year + 1900;

  vm_pwr_scheduled_startup(&start_time, VM_PWR_STARTUP_ENABLE_CHECK_HMS);
  alarm_flag = 1;
  //vm_log_debug("[SYSTMR] WAKE UP scheduled at %s", asctime(time_now));
}

// system timer callback
// handles wdg reset, scheduled shutdown/wake up and uart timeout
//-------------------------------------------------------------------------------
static void wdg_timer_callback(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
	_reset_wdg();
	// Check USB cable status change
	VM_USB_CABLE_STATUS usbstat = vm_usb_get_cable_status();
	if (g_usb_status != usbstat) {
		g_usb_status = usbstat;
		/*if ((usbstat) && (retarget_target != retarget_usb_handle)) retarget_target = retarget_usb_handle;
		else {
			if (retarget_target == retarget_usb_handle) {
				#if defined (USE_UART1_TARGET)
				retarget_target = retarget_uart1_handle;
				#else
				retarget_target = -1;
				#endif
			}
		}*/
	}

	if (no_activity_time > max_no_activity_time) {
		// ** Maximum no activity time reached
		no_activity_time = 0;
		if ((wakeup_interval > 0) && (shutdown_cb != LUA_NOREF)) {
			// execute callback function
			reboot_cb_params.cb_ref = shutdown_cb;
			reboot_cb_params.par = 2;
			remote_lua_call(CB_FUNC_REBOOT, &reboot_cb_params);
			return;
		}
		if (wakeup_interval > 0) {
			_scheduled_startup();
			if (usbstat == 0) {
				alarm_flag = 0;
				vm_pwr_shutdown(778);
			}
		}
	}
	else {
		// === Test uart's & BT timeout, alarm flag ===
		if (uart_tmo[0] >= 0) {
			uart_tmo[0]++;
			if (uart_tmo[0] > 2) _uart_cb(0);
		}
		if (uart_tmo[1] >= 0) {
			uart_tmo[1]++;
			if (uart_tmo[1] > 2) _uart_cb(1);
		}
		if (btspp_tmo >= 0) {
			btspp_tmo++;
			if (btspp_tmo > 4) _btspp_recv_cb();
		}
		// == Test alarm
		volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + 0x0710008);
	    uint32_t al_state = *reg;
	    if (al_state != alarm_state) {
	    	alarm_state = al_state;
	    	if ((alarm_flag) && (((al_state & 1) == 0)) && (alarm_cb != LUA_NOREF)) {
	    		if (wakeup_interval > 0) {
	    			_scheduled_startup();
	    		}
				alarm_cb_param.par = 0;
				alarm_cb_param.cb_ref = alarm_cb;
				remote_lua_call(CB_FUNC_INT, &alarm_cb_param);
	    	}
	    }
	}
}

//-------------------------------------------------------------------
static VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code) {
    if (code == 30) {
        if (((event != VM_KEYPAD_EVENT_DOWN) || (event != VM_KEYPAD_EVENT_UP)) && (key_cb != LUA_NOREF)) {
        	if (alarm_cb_param.busy == 0) {
        		alarm_cb_param.busy = 1;
        		alarm_cb_param.cb_ref = key_cb;
        		alarm_cb_param.par = event;
        		remote_lua_call(CB_FUNC_INT, &alarm_cb_param);
        	}
		}
    }
    return 0;
}

//===============================
static int msleep_c(lua_State *L)
{
    VMUINT32 ms = lua_tointeger(L, -1);
    vm_thread_sleep(ms);
    return 0;
}

//-------------------------
static void _led_init(void)
{
    VM_DCL_HANDLE ghandle;

    gpio_get_handle(GREENLED, &ghandle); // Green LED
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

    gpio_get_handle(BLUELED, &ghandle); // Blue LED
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

    gpio_get_handle(REDLED, &ghandle); // Red LED
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(ghandle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
}

//---------------------
static void lua_setup()
{
	VM_THREAD_HANDLE handle;

	shellL = lua_open();
    ttyL = luaL_newstate();

    lua_gc(shellL, LUA_GCSTOP, 0);  // stop garbage collector during initialization
    luaL_openlibs(shellL);          // open libraries
    luaopen_sys(shellL);

    // ** If not needed, comment any of the following "luaopen_module"
    luaopen_bit(shellL);
    luaopen_audio(shellL);
    luaopen_gsm(shellL);
    luaopen_bt(shellL);
    luaopen_uart(shellL);
    luaopen_timer(shellL);
    luaopen_gpio(shellL);
	#if defined USE_SCREEN_MODULE
    luaopen_screen(shellL);
	#endif
    luaopen_term(shellL);
    luaopen_i2c(shellL);
    luaopen_spi(shellL);
    luaopen_net(shellL);
    luaopen_https(shellL);
    luaopen_sensor(shellL);
    luaopen_struct(shellL);
    luaopen_cjson(shellL);
    luaopen_cjson_safe(shellL);
    luaopen_hash_md5(shellL);
    luaopen_hash_sha1(shellL);
    luaopen_hash_sha2(shellL);
    luaopen_mqtt(shellL);
    luaopen_email(shellL);
    luaopen_ftp(shellL);
    luaopen_lcd(shellL);
    //luaopen_marshal(shellL);

    lua_register(shellL, "msleep", msleep_c);

    lua_gc(shellL, LUA_GCRESTART, 0);  // restart garbage collector

    /*
    const char *script = "audio.play('nokia.mp3')";
	int error;
	error = luaL_loadbuffer(shellL, script, strlen(script), "line") ||
			lua_pcall(shellL, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s", lua_tostring(shellL, -1));
		lua_pop(shellL, 1);  // pop error message from the stack
	}
	*/

    vm_mutex_init(&lua_func_mutex);
    g_tty_signal = vm_signal_create();
    g_shell_signal = vm_signal_create();
    //g_reboot_signal = vm_signal_create();

    handle = vm_thread_create(shell_thread, shellL, 150);
    handle = vm_thread_create(tty_thread, ttyL, 160);
}

//extern void do_CCall(lua_State *shellL);

//---------------------------------------------------
static void handle_sysevt(VMINT message, VMINT param)
{
	cfunc_params_t *params = (cfunc_params_t *)param;
	int res;
    switch (message) {
        case VM_EVENT_CREATE:
            lua_setup();
            break;

		case SHELL_MESSAGE_ID:
			// MANY vm_xxx FUNCTIONS CAN BE EXECUTED ONLY FROM THE MAIN THREAD!!
			// All Lua C functions containing such calls are executed from here
			// execute lua "docall(shellL, 0, 0)", WAITS for execution!!
			//shell_docall(shellL);
			res = g_CCfunc((lua_State *)param);
			break;

		case CCALL_MESSAGE_TICK:
			params->upar1 = vm_time_ust_get_count();
			g_shell_result = 0;
			vm_signal_post(g_shell_signal);
			break;

		case CCALL_MESSAGE_LCDWR:
			res = _LcdSpiTransfer(params->cpar1, params->ipar1);
			g_shell_result = 0;
			vm_signal_post(g_shell_signal);
            break;

		case CCALL_MESSAGE_SIT_LCDWR:
			_sitronix_LcdTransfer(params->cpar1, params->ipar1);
			g_shell_result = 0;
			vm_signal_post(g_shell_signal);
            break;

		case CCALL_MESSAGE_FOPEN: {
			    VMWCHAR ucs_name[64];
			    full_fname(params->cpar1, ucs_name, 64);
				g_shell_result = vm_fs_open(ucs_name, params->ipar1, VM_TRUE);
				vm_signal_post(g_shell_signal);
		    }
            break;

		case CCALL_MESSAGE_FCLOSE:
			g_shell_result = vm_fs_close(params->ipar1);
			vm_signal_post(g_shell_signal);
            break;

		case CCALL_MESSAGE_FFLUSH:
			g_shell_result = vm_fs_flush(params->ipar1);
			vm_signal_post(g_shell_signal);
            break;

		case CCALL_MESSAGE_FCHECK: {
				VMWCHAR ucs_name[128];
				vm_fs_info_ex_t fileinfoex;
			    full_fname(params->cpar1, ucs_name, 128);

			    VM_FS_HANDLE fhex = vm_fs_find_first_ex((VMWSTR)ucs_name, &fileinfoex);
			    if (fhex >= 0) {
			    	if (fileinfoex.attributes & VM_FS_ATTRIBUTE_DIRECTORY) g_shell_result = 2;
			    	else g_shell_result = 1;
			    	vm_fs_find_close_ex(fhex);
			    }
			    else g_shell_result = fhex;
				vm_signal_post(g_shell_signal);
		    }
            break;

		case CCALL_MESSAGE_FSEEK: {
				VMUINT position = 0;
				g_shell_result = vm_fs_seek(params->ipar1, params->ipar2, params->ipar3);
				if (g_shell_result == 0) g_shell_result = vm_fs_get_position(params->ipar1, &position);
				g_shell_result = position;
				vm_signal_post(g_shell_signal);
			}
            break;

		case CCALL_MESSAGE_FSIZE: {
			    VMUINT size = 0;
				g_shell_result = vm_fs_get_size(params->ipar1, &size);
				if (g_shell_result == 0) g_shell_result = size;
				vm_signal_post(g_shell_signal);
		    }
            break;

		case CCALL_MESSAGE_FREAD: {
			    VMUINT read_bytes = 0;
				g_shell_result = vm_fs_read(params->ipar1, params->cpar1, params->ipar2, &read_bytes);
				//g_shell_result = read_bytes;
				vm_signal_post(g_shell_signal);
		    }
            break;

		case CCALL_MESSAGE_FWRITE: {
			    VMUINT written_bytes = 0;
		        res = vm_fs_write(params->ipar1, params->cpar1, params->ipar2, &written_bytes);
		        g_shell_result = written_bytes;
				vm_signal_post(g_shell_signal);
		    }
            break;

		case CCALL_MESSAGE_FRENAME: {
				VMWCHAR ucs_oldname[64], ucs_newname[64];
			    full_fname(params->cpar1, ucs_oldname, 64);
			    full_fname(params->cpar2, ucs_newname, 64);
				g_shell_result =  vm_fs_rename(ucs_oldname, ucs_newname);
				vm_signal_post(g_shell_signal);
		    }
            break;

        case CCALL_MESSAGE_MALLOC:
        	params->cpar1 = vm_malloc(params->ipar1);
			vm_signal_post(g_shell_signal);
            break;

        case CCALL_MESSAGE_REALLOC:
        	params->cpar2 = vm_realloc(params->cpar1, params->ipar1);
			vm_signal_post(g_shell_signal);
            break;

        case CCALL_MESSAGE_FREE:
        	vm_free(params->cpar1);
			vm_signal_post(g_shell_signal);
            break;

        case SHELL_MESSAGE_QUIT:
        	vm_log_debug("[SYSEVT] APP REBOOT");
            //vm_pmng_restart_application();
            vm_pwr_reboot();
            break;

        case VM_EVENT_PAINT:
        	// The graphics system is ready for application to use
        	g_graphics_ready = 1;
        	break;

        case VM_EVENT_QUIT:
        	vm_log_debug("[SYSEVT] QUIT");
            break;

        case 18:
        	vm_log_debug("[SYSEVT] USB PLUG IN");
            break;

        case 19:
        	vm_log_debug("[SYSEVT] USB PLUG OUT");
            break;

        default:
        	vm_log_debug("[SYSEVT] UNHANDLED EVENT: %d", message);
    }
}


/****************/
/*  Entry point */
/****************/
void vm_main(void)
{
	_led_init();

	// check power on status (RTC_SPAR0 register)
	volatile uint32_t *reg = (uint32_t *)(REG_BASE_ADDRESS + 0x0710060);
	if (*reg == 0xD3A5) {
		g_rtc_poweroff = 1;
	}
	else {
		g_rtc_poweroff = 0;
		*reg = 0xD3A5;
	}

	VM_TIMER_ID_HISR sys_timer_id = vm_timer_create_hisr("WDGTMR");
    vm_timer_set_hisr(sys_timer_id, sys_timer_callback, NULL, SYS_TIMER_INTERVAL, SYS_TIMER_INTERVAL);
    wdg_timer_id = vm_timer_create_non_precise(100, wdg_timer_callback, NULL);
	// init watchdog;
    wdg_handle = vm_wdt_start(sys_wdt_tmo);

    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

	/* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
