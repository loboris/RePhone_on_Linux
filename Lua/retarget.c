
#include <string.h>
#include <stdio.h>
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmboard.h"
#include "vmthread.h"
#include "vmlog.h"
#include "shell.h"
#include "vmwdt.h"


#define SERIAL_BUFFER_SIZE  256

/* Module owner of APP */
static VM_DCL_OWNER_ID g_owner_id = 0;
static VM_DCL_HANDLE retarget_device_handle = -1;
static vm_mutex_t retarget_rx_mutex;

static char retarget_rx_buffer[SERIAL_BUFFER_SIZE];
static unsigned retarget_rx_buffer_head = 0;
static unsigned retarget_rx_buffer_tail = 0;

#if defined (LUA_USE_WDG)
static int rxtmo = 0;
extern int sys_wdt_tmo;
extern VM_WDT_HANDLE sys_wdt_id;
#else
static VM_SIGNAL_ID retarget_rx_signal_id;
#endif

//-------------------------
void retarget_putc(char ch)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    vm_dcl_write(retarget_device_handle, (VM_DCL_BUFFER *)&ch, 1, &writen_len, g_owner_id);
}

//---------------------------------
void retarget_puts(const char *str)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    VM_DCL_BUFFER_LENGTH len = strlen(str);

    vm_dcl_write(retarget_device_handle, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
}

// this runs in lua thread!!
//---------------------
int retarget_getc(void)
{
#if defined (LUA_USE_WDG)
    while (1) {
    	vm_mutex_lock(&retarget_rx_mutex);
		if (retarget_rx_buffer_head != retarget_rx_buffer_tail) break;

		// no new characters
		vm_mutex_unlock(&retarget_rx_mutex);

	    vm_thread_sleep(10);
		rxtmo += 10;
		if (rxtmo >= (sys_wdt_tmo)) {
			rxtmo = 0;
			// waiting more than 1/2 wdg timeout, send watchdog reset request if needed
			if (sys_wdt_id >= 0) return -1;
		}
    }
#else
	vm_mutex_lock(&retarget_rx_mutex);
	if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
		vm_mutex_unlock(&retarget_rx_mutex);
		vm_signal_wait(retarget_rx_signal_id);
		vm_mutex_lock(&retarget_rx_mutex);
	}
#endif

    char ch = retarget_rx_buffer[retarget_rx_buffer_tail % SERIAL_BUFFER_SIZE];
	retarget_rx_buffer_tail++;

	vm_mutex_unlock(&retarget_rx_mutex);
	return ch;
}

//-------------------------------------------------------------------------------------------
void __retarget_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if(event == VM_DCL_SIO_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE];
        int i;
        VM_DCL_STATUS status;
        VM_DCL_BUFFER_LENGTH returned_len = 0;

        status = vm_dcl_read(device_handle,
                             (VM_DCL_BUFFER *)data,
                             SERIAL_BUFFER_SIZE,
                             &returned_len,
                             g_owner_id);
        if(status < VM_DCL_STATUS_OK)
        {
            // vm_log_info((char*)"read failed");
        }
        else if (returned_len)
        {
        	vm_mutex_lock(&retarget_rx_mutex);
			#if !defined (LUA_USE_WDG)
			if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
				vm_signal_post(retarget_rx_signal_id);
			}
			#endif
			for (i = 0; i < returned_len; i++)
			{
				retarget_rx_buffer[retarget_rx_buffer_head % SERIAL_BUFFER_SIZE] = data[i];
				retarget_rx_buffer_head++;
				if ((unsigned)(retarget_rx_buffer_head - retarget_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
					retarget_rx_buffer_tail = retarget_rx_buffer_head - SERIAL_BUFFER_SIZE;
				}
			}
        	vm_mutex_unlock(&retarget_rx_mutex);
        }
    }
    else
    {
    }
}

//-----------------------
void retarget_setup(void)
{
    VM_DCL_HANDLE uart_handle;
    vm_dcl_sio_control_dcb_t settings;
    
    g_owner_id = vm_dcl_get_owner_id();

    if (retarget_device_handle != -1)
    {
        return;
    }
    
#if 0

#if defined(__HDK_LINKIT_ONE_V1__)
        vm_dcl_config_pin_mode(VM_PIN_D0, VM_DCL_PIN_MODE_UART);
        vm_dcl_config_pin_mode(VM_PIN_D1, VM_DCL_PIN_MODE_UART);
#elif defined(__HDK_LINKIT_ASSIST_2502__)
        vm_dcl_config_pin_mode(VM_PIN_P8, VM_DCL_PIN_MODE_UART);
        vm_dcl_config_pin_mode(VM_PIN_P9, VM_DCL_PIN_MODE_UART);
#endif

    uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, g_owner_id);
#else
    uart_handle = vm_dcl_open(VM_DCL_SIO_USB_PORT1, g_owner_id);
#endif
    
    settings.owner_id = g_owner_id;
    settings.config.dsr_check = 0;
    settings.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
    settings.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
    settings.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
    settings.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
    settings.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
    settings.config.sw_xoff_char = 0x13;
    settings.config.sw_xon_char = 0x11;
    vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);

#if !defined (LUA_USE_WDG)
    retarget_rx_signal_id = vm_signal_create();
#endif
    vm_mutex_init(&retarget_rx_mutex);

    vm_dcl_register_callback(uart_handle,
                             VM_DCL_SIO_UART_READY_TO_READ,
                             (vm_dcl_callback)__retarget_irq_handler,
                             (void*)NULL);

    retarget_device_handle = uart_handle;
}

