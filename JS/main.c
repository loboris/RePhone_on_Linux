
#include <string.h>

#include "vmtype.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "vmdcl.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"
#include "vmthread.h"

#include "v7.h"
#include "sj_prompt.h"

struct v7* v7;

VM_TIMER_ID_PRECISE sys_timer_id = 0;

long timezone = 0;

extern void retarget_setup();
extern void js_init_gpio(struct v7 *v7);
extern void js_init_audio(struct v7 *v7);
extern void js_init_gsm(struct v7 *v7);


int sys_wdt_tmo = 13001;			// ** HW WDT timeout in ticks: 13001 -> 59.999615 seconds **
int sys_wdt_rst = 10834;			// time at which hw wdt is reset in ticks: 10834 -> 50 seconds, must be < 'sys_wdt_tmo'

void key_init(void)
{
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD, 0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map = 0x05;
    vm_dcl_control(kbd_handle, VM_DCL_KBD_COMMAND_CONFIG_PIN, (void*)(&kbdmap));

    vm_dcl_close(kbd_handle);
}

void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{

    vm_log_debug("tick");
}

VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code)
{
    /* output log to monitor or catcher */
    vm_log_info("key event=%d,key code=%d", event, code); /* event value refer to VM_KEYPAD_EVENT */

    if(code == 30) {
        if(event == 3) { // long pressed

        } else if(event == 2) { // down
            printf("key is pressed\n");
        } else if(event == 1) { // up
        }
    }
    return 0;
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch(message) {
    case VM_EVENT_CREATE:
//        sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
        break;
    case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void)
{
    v7_val_t exec_result;

    v7 = v7_create();
    retarget_setup();
    
    js_init_gpio(v7);
    js_init_audio(v7);
    js_init_gsm(v7);
    
    v7_exec_file(v7, "init.js", &exec_result);
    
    sj_prompt_init(v7);



    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
