
#include <stdio.h>
#include <string.h>

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

#include "shell.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern void retarget_setup();
extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);
extern int luaopen_timer(lua_State *L);
extern int luaopen_gpio(lua_State *L);
extern int luaopen_screen(lua_State *L);
extern int luaopen_i2c(lua_State *L);
extern int luaopen_tcp(lua_State* L);
extern int luaopen_https(lua_State* L);
extern int luaopen_gprs(lua_State* L);
extern int luaopen_os(lua_State* L);

lua_State *L = NULL;

int _started = 0;

#define SYS_TIMER_INTERVAL 250

VM_THREAD_HANDLE g_usr_thread = -1;
//VM_TIMER_ID_PRECISE sys_timer_id = 0;
#if defined (LUA_USE_WDG)
VM_WDT_HANDLE sys_wdt_id = -1;
int sys_wdt_tmo = 2168;
#endif

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

/*
//-------------------------------------------------------------------------------
static void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
}
*/

//------------------------------------------------------------
VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code) {
    /* output log to monitor or catcher */
    //vm_log_info("key event=%d,key code=%d",event,code); /* event value refer to VM_KEYPAD_EVENT */

    if (code == 30) {
        if (event == VM_KEYPAD_EVENT_LONG_PRESS) {

        } else if (event == VM_KEYPAD_EVENT_DOWN) {
            printf("\n[KEY] pressed\n");
        } else if (event == VM_KEYPAD_EVENT_UP) {
        	printf("\n[KEY] APP RESTART\n");
            vm_pmng_restart_application();
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

//--------------
void lua_setup()
{
    VM_THREAD_HANDLE handle;

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    luaopen_audio(L);
    luaopen_gsm(L);
    luaopen_timer(L);
    luaopen_gpio(L);
    luaopen_screen(L);
    luaopen_i2c(L);
    luaopen_tcp(L);
    luaopen_https(L);
    luaopen_gprs(L);
    luaopen_os(L);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    luaL_dofile(L, "init.lua");

    if (0)
    {
        const char *script = "audio.play('nokia.mp3')";
        int error;
        error = luaL_loadbuffer(L, script, strlen(script), "line") ||
                lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1);  /* pop error message from the stack */
        }
    }

    handle = vm_thread_create(shell_thread, L, 245);
}

//--------------------------------------------
void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:
            //sys_timer_id = vm_timer_create_precise(SYS_TIMER_INTERVAL, sys_timer_callback, NULL);
            lua_setup();
            break;
		case SHELL_MESSAGE_ID:
			// MANY vm_xxx FUNCTIONS CAN BE EXECUTED ONLY FROM THE MAIN THREAD!!
			// execute lua "docall(L, 0, 0)", WAITS for execution!!
			shell_docall(L);
			break;

        case SHELL_MESSAGE_QUIT:
        	printf("\n[SYSEVT] APP RESTART\n");
            vm_pmng_restart_application();
            break;
        case VM_EVENT_PAINT:
        	// The graphics system is ready for application to use
        	//printf("\n[SYSEVT] GRAPHIC READY\n");
        	break;
        case VM_EVENT_QUIT:
        	printf("\n[SYSEVT] QUIT\n");
            break;
        default:
        	printf("\n[SYSEVT] UNHANDLED EVENT: %d\n", message);
    }
}

/****************/
/*  Entry point */
/****************/
void vm_main(void)
{
    retarget_setup();
    vm_log_info("LUA for RePhone started");

    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
