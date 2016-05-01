
#include <string.h>
#include <stdio.h>
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmboard.h"
#include "vmthread.h"
#include "vmdatetime.h"
#include "vmlog.h"
#include "shell.h"
#include "vmwdt.h"

#define USE_UART1_TARGET

#define SERIAL_BUFFER_SIZE  256


VM_DCL_OWNER_ID g_owner_id = 0;  // Module owner of APP
VM_DCL_HANDLE retarget_device_handle = -1;
VM_DCL_HANDLE retarget_uart1_handle = -1;
int retarget_target = -1;
static vm_mutex_t retarget_rx_mutex;

static char retarget_rx_buffer[SERIAL_BUFFER_SIZE];
static unsigned retarget_rx_buffer_head = 0;
static unsigned retarget_rx_buffer_tail = 0;
static VM_SIGNAL_ID retarget_rx_signal_id;

extern int sys_wdt_rst_time;
extern int no_activity_time;
extern void _reset_wdg(void);

//-------------------------
void retarget_putc(char ch)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
   	vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)&ch, 1, &writen_len, g_owner_id);
}

//---------------------------------
void retarget_puts(const char *str)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    VM_DCL_BUFFER_LENGTH len = strlen(str);

   	vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
}

//----------------------------------------------------
void retarget_write(const char *str, unsigned int len)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
   	vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
}

// !!this runs from lua thread!!
//---------------------
int retarget_getc(void)
{
	vm_mutex_lock(&retarget_rx_mutex);
	while (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
		vm_mutex_unlock(&retarget_rx_mutex);
		// wait 1 second for character
		vm_signal_timed_wait(retarget_rx_signal_id, 1000);

		vm_mutex_lock(&retarget_rx_mutex);
		if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
			sys_wdt_rst_time = 0;	// wdg reset
			no_activity_time++;		// increase no activity counter
		}
		else break;
	}

    char ch = retarget_rx_buffer[retarget_rx_buffer_tail % SERIAL_BUFFER_SIZE];
	retarget_rx_buffer_tail++;

	vm_mutex_unlock(&retarget_rx_mutex);
	sys_wdt_rst_time = 0;  // wdg reset
	no_activity_time = 0;
	return ch;
}

// wait for multiple character on serial port
//-----------------------------------------------------------------
int retarget_waitchars(unsigned char *buf, int *count, int timeout)
{
    VM_DCL_STATUS status;
    VM_DCL_BUFFER_LENGTH returned_len = 0;
    VM_DCL_BUFFER_LENGTH total_read = 0;
	int tmo = 0;

	sys_wdt_rst_time = 0;
	_reset_wdg();
	while (tmo < timeout) {
		// try to read characters
	    status = vm_dcl_read((VM_DCL_HANDLE)retarget_target, (VM_DCL_BUFFER *)(buf+total_read),
	    		             *count - total_read, &returned_len, g_owner_id);
	    if ((status == VM_DCL_STATUS_OK) && (returned_len > 0)) {
	    	total_read += returned_len;
	    	if (total_read == *count) break;
	    }
	    vm_thread_sleep(5);
		tmo += 5;
	}
	sys_wdt_rst_time = 0;
	_reset_wdg();
	*count = total_read;
	if ((tmo < timeout) && (total_read == *count)) return 0;
	else return -1;
}

// wait for 1 character on serial port
//-----------------------------------------------
int retarget_waitc(unsigned char *c, int timeout)
{
	int count = 1;
	return retarget_waitchars(c, &count, timeout);
}

//--------------------------------------------------------------------------------------------------
static void __retarget_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
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
        if(status < VM_DCL_STATUS_OK) {
            // vm_log_info((char*)"read failed");
        }
        else if (returned_len) {
        	if (retarget_target == device_handle) {
				vm_mutex_lock(&retarget_rx_mutex);
				if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
					vm_signal_post(retarget_rx_signal_id);
				}
				for (i = 0; i < returned_len; i++) {
					retarget_rx_buffer[retarget_rx_buffer_head % SERIAL_BUFFER_SIZE] = data[i];
					retarget_rx_buffer_head++;
					if ((unsigned)(retarget_rx_buffer_head - retarget_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
						retarget_rx_buffer_tail = retarget_rx_buffer_head - SERIAL_BUFFER_SIZE;
					}
				}
				vm_mutex_unlock(&retarget_rx_mutex);
        	}
        }
    }
}

//-----------------------
void retarget_setup(void)
{
    VM_DCL_HANDLE uart_handle;
    vm_dcl_sio_control_dcb_t settings;
    
    g_owner_id = vm_dcl_get_owner_id();

    if (retarget_device_handle != -1) return;  // already setup

    settings.owner_id = g_owner_id;
    settings.config.dsr_check = 0;
    settings.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
    settings.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
    settings.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
    settings.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
    settings.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
    settings.config.sw_xoff_char = 0x13;
    settings.config.sw_xon_char = 0x11;

    // configure USB serial port
    uart_handle = vm_dcl_open(VM_DCL_SIO_USB_PORT1, g_owner_id);
    vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);

    retarget_rx_signal_id = vm_signal_create();
    vm_mutex_init(&retarget_rx_mutex);

    vm_dcl_register_callback(uart_handle, VM_DCL_SIO_UART_READY_TO_READ,
                             (vm_dcl_callback)__retarget_irq_handler, (void*)NULL);

    retarget_device_handle = uart_handle;

#if defined (USE_UART1_TARGET)
    // configure UART1
    vm_dcl_config_pin_mode(VM_PIN_P7, VM_DCL_PIN_MODE_UART); // Rx1
    vm_dcl_config_pin_mode(VM_PIN_P8, VM_DCL_PIN_MODE_UART); // Tx1

    uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, g_owner_id);
    if (uart_handle != VM_DCL_STATUS_OK) {
        vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);
        vm_dcl_register_callback(uart_handle, VM_DCL_SIO_UART_READY_TO_READ,
                                 (vm_dcl_callback)__retarget_irq_handler, (void*)NULL);
        retarget_uart1_handle = uart_handle;
    }
#endif

    retarget_target = retarget_device_handle;
}

