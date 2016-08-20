
#include <string.h>
#include <stdio.h>
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmboard.h"
#include "vmthread.h"
#include "vmdatetime.h"
#include "vmlog.h"
#include "vmwdt.h"
#include "vmbt_spp.h"
#include "vmusb.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

//#define TEST_USB2	1

#define LUA_UART   "uart"

#define SERIAL_BUFFER_SIZE  256

char retarget_rx_buffer[SERIAL_BUFFER_SIZE];
unsigned retarget_rx_buffer_head = 0;
unsigned retarget_rx_buffer_tail = 0;
VM_SIGNAL_ID retarget_rx_signal_id;

extern int sys_wdt_rst_time;
extern int no_activity_time;
extern cb_func_param_bt_t bt_cb_params;
extern int g_usb_status;			// status of the USB cable connection

int uart_tmo[2] = {-1, -1};

VMUINT8 uart_has_userdata[2] = {0,0};
uart_info_t *uart_data[2] = {NULL,NULL};


//-------------------------
void retarget_putc(char ch)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
   	if (retarget_target >= 0) vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)&ch, 1, &writen_len, g_owner_id);
   	else if ((bt_cb_params.connected) && (retarget_target == -1000)) vm_bt_spp_write(bt_cb_params.id, &ch, 1);
}

//---------------------------------
void retarget_puts(const char *str)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    VM_DCL_BUFFER_LENGTH len = strlen(str);

    if (retarget_target >= 0) vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
    else if ((bt_cb_params.connected) && (retarget_target == -1000)) vm_bt_spp_write(bt_cb_params.id, (char *)str, len);
}

//----------------------------------------------------
void retarget_write(const char *str, unsigned int len)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    if (retarget_target >= 0) vm_dcl_write(retarget_target, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
    else if ((bt_cb_params.connected) && (retarget_target == -1000)) vm_bt_spp_write(bt_cb_params.id, (char *)str, len);
}

// !!this may run from lua tty thread!!
// wait for character while timeout expires
//------------------------
int retarget_getc(int tmo)
{
	int n = 0;
	vm_mutex_lock(&retarget_rx_mutex);
	while (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
		vm_mutex_unlock(&retarget_rx_mutex);
		if (tmo == 0) return -1;

		// wait 1 seconds for character
		vm_signal_timed_wait(retarget_rx_signal_id, 1000);
		vm_mutex_lock(&retarget_rx_mutex);

		if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
			// no char
			n++;
			sys_wdt_rst_time = 0;	// wdg reset
			no_activity_time++;		// increase no activity counter
			if (n >= tmo) {
				vm_mutex_unlock(&retarget_rx_mutex);
				return -1;
			}
		}
		else break; // got char
	}

	// get one char from buffer
    unsigned char ch = retarget_rx_buffer[retarget_rx_buffer_tail % SERIAL_BUFFER_SIZE];
	retarget_rx_buffer_tail++;

	vm_mutex_unlock(&retarget_rx_mutex);
	sys_wdt_rst_time = 0;  // wdg reset
	no_activity_time = 0;
	return ch;
}

/*
// wait for multiple character on serial port
//-----------------------------------------------------------------
int retarget_waitchars(unsigned char *buf, int *count, int timeout)
{
	if (retarget_target < 0) return -1;

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
*/
//-------------------
void _uart_cb(int id)
{
	uart_tmo[id] = -1;
	uart_info_t *p = uart_data[id];
	if ((p != NULL) && (p->buffer != NULL) && (p->cb_ref != LUA_NOREF) && (p->bufptr > 0)) {
		if (p->busy == 0) {
			p->busy = 1;
			remote_lua_call(CB_FUNC_UART_RECV, p);
		}
	}
}

//--------------------------------------------------------------------------------------------------
static void __retarget_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if (event == VM_DCL_SIO_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE+1];
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
        	data[returned_len] = '\0';
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
        	else {
        		// data from non shell retarget device
        	    VM_DCL_BUFFER_LENGTH writen_len = 0;
        	    size_t len;

        		int id = -1;
        		if (device_handle == retarget_usb_handle) id = 0;
        		else if (device_handle == retarget_uart1_handle) id = 1;

        		if (id >= 0) {
					uart_tmo[id] = 0;
        			uart_info_t *p = uart_data[id];
        			if ((p != NULL) && (p->buffer != NULL) && (p->cb_ref != LUA_NOREF)) {
						if (uart_tmo[id] < 0) uart_tmo[id] = 0;
						if (p->bufptr == 0) uart_tmo[id] = 0;
        				if ((p->bufptr + returned_len) > (UART_BUFFER_LEN-1)) returned_len = UART_BUFFER_LEN - p->bufptr - 1;
						memcpy(p->buffer+p->bufptr, (VMUINT8 *)data, returned_len);
						p->bufptr += returned_len;
						p->buffer[p->bufptr] = 0;
						if ((strchr((const char *)p->buffer, '\n') != NULL) || (p->bufptr == UART_BUFFER_LEN)) {
							uart_tmo[id] = -1;
							if (p->busy == 0) {
								p->busy = 1;
								remote_lua_call(CB_FUNC_UART_RECV, p);
							}
						}
        			}
        		}
        	}
        }
    }
    if (event == VM_DCL_SIO_UART_READY_TO_WRITE) {

    }
}

#if defined (TEST_USB2)
//---------------------------------------------------------------------------------------------
static void __usb2_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if (event == VM_DCL_SIO_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE+1];
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
        	data[returned_len] = '\0';
			printf("USB2 read [%s]\n", data);
        }
    }
    if (event == VM_DCL_SIO_UART_READY_TO_WRITE) {

    }
}
#endif

//-------------------------------
void retarget_setup(lua_State *L)
{
    VM_DCL_HANDLE uart_handle;
    vm_dcl_sio_control_dcb_t settings;
    
    g_owner_id = vm_dcl_get_owner_id();

    g_usb_status = vm_usb_get_cable_status();

    if (retarget_usb_handle != -1) return;  // already setup

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
    vm_dcl_add_event(uart_handle, VM_DCL_SIO_UART_READY_TO_WRITE, (void*)NULL);

    retarget_usb_handle = uart_handle;

#if defined (USE_UART1_TARGET)
    // configure UART1
    vm_dcl_config_pin_mode(VM_PIN_P7, VM_DCL_PIN_MODE_UART); // Rx1
    vm_dcl_config_pin_mode(VM_PIN_P8, VM_DCL_PIN_MODE_UART); // Tx1

    uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, g_owner_id);
    if (uart_handle != VM_DCL_STATUS_OK) {
        vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);
        vm_dcl_register_callback(uart_handle, VM_DCL_SIO_UART_READY_TO_READ,
                                 (vm_dcl_callback)__retarget_irq_handler, (void*)NULL);
        vm_dcl_add_event(uart_handle, VM_DCL_SIO_UART_READY_TO_WRITE, (void*)NULL);
        retarget_uart1_handle = uart_handle;
    }
#endif

    if (g_usb_status) retarget_target = retarget_usb_handle;
    else {
		#if defined (USE_UART1_TARGET)
    	retarget_target = retarget_uart1_handle;
		#else
    	retarget_target = -1;
		#endif
    }

#if defined (TEST_USB2)
    // configure USB2 serial port
    uart_handle = vm_dcl_open(VM_DCL_SIO_USB_PORT2, g_owner_id);
    if (uart_handle >= 0) {
        vm_dcl_register_callback(uart_handle, VM_DCL_SIO_UART_READY_TO_READ,
                                 (vm_dcl_callback)__usb2_irq_handler, (void*)NULL);
        vm_dcl_add_event(uart_handle, VM_DCL_SIO_UART_READY_TO_WRITE, (void*)NULL);
    	printf("USB2 serial OK\n");
    }
    else {
    	printf("USB2 serial error %d\n", uart_handle);
    }
#endif

}


//==================================
static int uart_create(lua_State *L)
{
    VMUINT32 id = luaL_checkinteger(L, 1) & 0x01;

	if (((id == 0) && (retarget_target == retarget_usb_handle)) ||
		((id == 1) && (retarget_target == retarget_uart1_handle))) {
		vm_log_error("Cannot open UART on active shell port");
		lua_pushnil(L);
		return 1;
	}

	if (((id == 0) && (retarget_usb_handle < 0)) || ((id == 1)  && (retarget_uart1_handle == 0))) {
		vm_log_error("Selected UART not available");
		lua_pushnil(L);
		return 1;
	}

	if (uart_has_userdata[id]) {
    	return luaL_error(L, "uart already created!");
    }

	if ((lua_type(L, 3) == LUA_TTABLE) && (id == 1)) {
		vm_dcl_sio_control_dcb_t settings;
		int ipar;

		settings.owner_id = g_owner_id;
		settings.config.dsr_check = 0;
		settings.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
		settings.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
		settings.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
		settings.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
		settings.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
		settings.config.sw_xoff_char = 0x13;
		settings.config.sw_xon_char = 0x11;

		lua_getfield(L, 3, "bit");
		if (!lua_isnil(L, -1)) {
			if ( lua_isnumber(L, -1) ) {
				ipar = luaL_checkinteger( L, -1 );
				if ((ipar >= 5) && (ipar <= 5)) settings.config.data_bits_per_char_length = ipar;
			}
			else lua_pop(L, 1);
		}
		lua_getfield(L, 2, "par");
		if (!lua_isnil(L, -1)) {
			if ( lua_isnumber(L, -1) ) {
				ipar = luaL_checkinteger( L, -1 );
				if ((ipar >= 0) && (ipar <= 4)) settings.config.parity = ipar;
			}
			else lua_pop(L, 1);
		}
		lua_getfield(L, 2, "stop");
		if (!lua_isnil(L, -1)) {
			if ( lua_isnumber(L, -1) ) {
				ipar = luaL_checkinteger( L, -1 );
				if ((ipar >= 1) && (ipar <= 3)) settings.config.stop_bits = ipar;
			}
			else lua_pop(L, 1);
		}
		lua_getfield(L, 2, "bdr");
		if (!lua_isnil(L, -1)) {
			if ( lua_isnumber(L, -1) ) {
				ipar = luaL_checkinteger( L, -1 );
				if ((ipar >= 75) && (ipar <= 921600)) settings.config.baud_rate = ipar;
			}
			else lua_pop(L, 1);
		}

		if (id == 0) vm_dcl_control(retarget_usb_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);
		else vm_dcl_control(retarget_uart1_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);
	}

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
		uart_info_t *p;
	    int ref;

		// register uart Lua callback function
		lua_pushvalue(L, 2);
		ref = luaL_ref(L, LUA_REGISTRYINDEX);

		// Create userdata for this uart
		p = (uart_info_t *)lua_newuserdata(L, sizeof(timer_info_t));
		luaL_getmetatable(L, LUA_UART);
		lua_setmetatable(L, -2);
		p->uart_id = id;
		p->cb_ref = ref;
		p->busy = 0;
		p->bufptr = 0;

		uart_data[id] = p;

	    return 1;
    }
    else return luaL_error(L, "Callback function not given!");
}

//==================================
static int uart_delete(lua_State *L)
{
    uart_info_t *p = ((uart_info_t *)luaL_checkudata(L, -1, LUA_UART));

	p->uart_id = -1;
    if (p->cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
		p->cb_ref = LUA_NOREF;
	}

    uart_has_userdata[p->uart_id] = 0;
    uart_data[p->uart_id] = NULL;

	return 0;
}

//=================================
static int uart_write(lua_State *L)
{
	int id = -1;
	uart_info_t *p = ((uart_info_t *)luaL_checkudata(L, 1, LUA_UART));
	id = p->uart_id;

    VM_DCL_BUFFER_LENGTH writen_len = 0;
    size_t len;

    const char *str = luaL_checklstring(L, 2, &len);

    if ((id == 0) && (retarget_usb_handle >= 0)) vm_dcl_write(retarget_usb_handle, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
    else if ((id == 1)  && (retarget_uart1_handle >= 0)) vm_dcl_write(retarget_uart1_handle, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);

	return 0;
}


//------------------------------------
static int uart_tostring(lua_State *L)
{
    uart_info_t *p = ((uart_info_t *)luaL_checkudata(L, -1, LUA_UART));
    char state[8];
    if (p->cb_ref == LUA_NOREF) sprintf(state,"Deleted");
   	else sprintf(state,"Active");
    lua_pushfstring(L, "uart%d (%s)", p->uart_id, state);
    return 1;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE uart_map[] =
{
    {LSTRKEY("create"), LFUNCVAL(uart_create)},
    {LSTRKEY("delete"), LFUNCVAL(uart_delete)},
    {LSTRKEY("write"), LFUNCVAL(uart_write)},
    {LNILKEY, LNILVAL}
};

const LUA_REG_TYPE uart_table[] = {
  {LSTRKEY("__gc"), LFUNCVAL(uart_delete)},
  {LSTRKEY("__tostring"), LFUNCVAL(uart_tostring)},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_uart(lua_State *L)
{
    luaL_newmetatable(L, LUA_UART);			// create metatable for uart handles
    lua_pushvalue(L, -1);					// push metatable
    lua_setfield(L, -2, "__index");			// metatable.__index = metatable
    luaL_register(L, NULL, uart_table);		// uart methods

    luaL_register(L, "uart", uart_map);
    return 1;
}



