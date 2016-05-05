
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

#include "shell.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

//#define USE_SCREEN_MODULE

// used external functions
extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);
extern void retarget_setup();
extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);
extern int luaopen_bt(lua_State *L);
extern int luaopen_timer(lua_State *L);
extern int luaopen_gpio(lua_State *L);
extern int luaopen_i2c(lua_State *L);
extern int luaopen_tcp(lua_State* L);
extern int luaopen_https(lua_State* L);
extern int luaopen_gprs(lua_State* L);
extern int luaopen_struct(lua_State* L);
extern int luaopen_sensor(lua_State* L);
extern int luaopen_cjson(lua_State *l);
extern int luaopen_hash_md5(lua_State *L);
extern int luaopen_hash_sha1(lua_State *L);
extern int luaopen_hash_sha2(lua_State *L);
#if defined USE_SCREEN_MODULE
extern int luaopen_screen(lua_State *L);
#endif


#define REDLED               17
#define GREENLED             15
#define BLUELED              12
#define SYS_TIMER_INTERVAL   22		// ticks, 0.10153 seconds
#define MAX_WDT_RESET_COUNT 145		// max run time with 50 sec reset = 7250 seconds, ~2 hours

// Global variables
lua_State *L = NULL;
int sys_wdt_rst_time = 0;
int no_activity_time = 0;			// no activity counter
int max_no_activity_time = 300;	    // time with no activity before shut down in seconds
int wakeup_interval = 20*60;		// regular wake up interval in seconds
int led_blink = BLUELED;			// led blinking during session, set to 0 for no led blink
int sys_wdt_time = 0;
int wdg_reboot_cb = LUA_NOREF;		// Lua callback function called before reboot
int shutdown_cb = LUA_NOREF;		// Lua callback function called before shutdown


// Local variables
static int sys_wdt_tmo = 13001;		// in ticks, 13001 = 59.999615 seconds
static int sys_wdt_rst = 10834;		// time at which wdt is reset in ticks, 10834 = 49.99891 seconds
static int sys_wdt_rst_count = 0;	// watchdog resets counter
static int sys_timer_tick = 0;		// used for timing inside HISR timer callback function
static int do_wdt_reset = 0;		// used for communication between HISR timer and wdg system timer

static VM_WDT_HANDLE wdg_handle = -1;
static VM_TIMER_ID_NON_PRECISE wdg_timer_id = NULL;


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

// **hisr timer**, runs every 101,530 msek
// handles wdg reset, blink led
//---------------------------------------------
static void sys_timer_callback(void* user_data)
{
	sys_wdt_rst_time += SYS_TIMER_INTERVAL;
	sys_wdt_time += SYS_TIMER_INTERVAL;

    if (do_wdt_reset > 0) return;

	if ((sys_wdt_rst_time < sys_wdt_rst) && (sys_wdt_time > sys_wdt_rst)) {
		// reset wdt
		sys_wdt_rst_count++;
		do_wdt_reset = 1;
		return;
	}

	if ((led_blink == BLUELED) || (led_blink == REDLED) || (led_blink == GREENLED)) {
	    VM_DCL_HANDLE ghandle;
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
		if (sys_wdt_rst_count > MAX_WDT_RESET_COUNT) {
			vm_log_debug("[SYSTMR] WDT MAX RESETS, REBOOT");
			if (wdg_reboot_cb != LUA_NOREF) {
				// execute callback function
			    lua_rawgeti(L, LUA_REGISTRYINDEX, wdg_reboot_cb);
				if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
					lua_call(L, 0, 0);
				}
				else {
					lua_remove(L, -1);
				}
			}
			vm_pwr_reboot();
		}
		vm_wdt_reset(wdg_handle);
		sys_wdt_time = 0;
		do_wdt_reset = 0;
	}
}

//------------------------------
void _scheduled_startup (void) {
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
  //vm_log_debug("[SYSTMR] WAKE UP scheduled at %s", asctime(time_now));
}

// system timer callback
// handles wdg reset and scheduled shutdown/wake up
//-------------------------------------------------------------------------------
static void wdg_timer_callback(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
	_reset_wdg();
	if (no_activity_time > max_no_activity_time) {
		if ((wakeup_interval > 0) && (shutdown_cb != LUA_NOREF)) {
			// execute callback function
		    lua_rawgeti(L, LUA_REGISTRYINDEX, shutdown_cb);
			if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
				lua_call(L, 0, 0);
			}
			else {
				lua_remove(L, -1);
			}
			// Check again
			if (no_activity_time <= max_no_activity_time) return;
		}
		no_activity_time = 0;
		if (wakeup_interval > 0) {
			VM_USB_CABLE_STATUS usb_stat = vm_usb_get_cable_status();
			_scheduled_startup();
			if (usb_stat == 0) vm_pwr_shutdown(778);
		}
	}
}

//-------------------------------------------------------------------
static VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code) {
    if (code == 30) {
        if (event == VM_KEYPAD_EVENT_LONG_PRESS) {

        } else if (event == VM_KEYPAD_EVENT_DOWN) {
            printf("\n[KEY] pressed\n");
        } else if (event == VM_KEYPAD_EVENT_UP) {
        	printf("\n[KEY] APP REBOOT\n");
            vm_pwr_reboot();
        }
    }
    return 0;
}

//===============================
static int msleep_c(lua_State *L)
{
    long ms = lua_tointeger(L, -1);
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

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    // ** If not needed, comment any of the fallowing "luaopen..."
    luaopen_audio(L);
    luaopen_gsm(L);
    luaopen_bt(L);
    luaopen_timer(L);
    luaopen_gpio(L);
	#if defined USE_SCREEN_MODULE
    luaopen_screen(L);
	#endif
    luaopen_i2c(L);
    luaopen_tcp(L);
    luaopen_https(L);
    luaopen_gprs(L);
    luaopen_sensor(L);
    luaopen_struct(L);
    luaopen_cjson(L);
    luaopen_hash_md5(L);
    luaopen_hash_sha1(L);
    luaopen_hash_sha2(L);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    // execute "init.lua"
    luaL_dofile(L, "init.lua");

    /*
    const char *script = "audio.play('nokia.mp3')";
	int error;
	error = luaL_loadbuffer(L, script, strlen(script), "line") ||
			lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s", lua_tostring(L, -1));
		lua_pop(L, 1);  // pop error message from the stack
	}
	*/

    handle = vm_thread_create(shell_thread, L, 245);
}

//---------------------------------------------------
static void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:
            lua_setup();
            break;

		case SHELL_MESSAGE_ID:
			// MANY vm_xxx FUNCTIONS CAN BE EXECUTED ONLY FROM THE MAIN THREAD!!
			// execute lua "docall(L, 0, 0)", WAITS for execution!!
			shell_docall(L);
			break;

        case SHELL_MESSAGE_QUIT:
        	vm_log_debug("[SYSEVT] APP REBOOT");
            //vm_pmng_restart_application();
            vm_pwr_reboot();
            break;

        case VM_EVENT_PAINT:
        	// The graphics system is ready for application to use
        	//vm_log_debug("[SYSEVT] GRAPHIC READY");
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

	VM_TIMER_ID_HISR sys_timer_id = vm_timer_create_hisr("WDGTMR");
    vm_timer_set_hisr(sys_timer_id, sys_timer_callback, NULL, SYS_TIMER_INTERVAL, SYS_TIMER_INTERVAL);
    wdg_timer_id = vm_timer_create_non_precise(100, wdg_timer_callback, NULL);
	wdg_handle = vm_wdt_start(sys_wdt_tmo);

    retarget_setup();

    vm_log_info("LUA for RePhone started");

    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
