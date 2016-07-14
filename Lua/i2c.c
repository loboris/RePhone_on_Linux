// Module for interfacing with the I2C interface

#include <stdint.h>

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_i2c.h"
#include "vmlog.h"
#include "vmdatetime.h"
#include "shell.h"

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define I2C_SCL_PIN_NAME	43
#define I2C_SDA_PIN_NAME	44

static VM_DCL_HANDLE g_i2c_handle = VM_DCL_HANDLE_INVALID;

// Lua: i2c.setup(address [, speed])
//=================================
static int i2c_setup(lua_State* L)
{
    vm_dcl_i2c_control_config_t conf_data;
    int result;
    uint8_t address = luaL_checkinteger(L, 1);
    VMUINT32 speed = 100; // default speed - 100kbps

    if (lua_gettop(L) > 1) {
        speed = luaL_checkinteger(L, 2);
        if (speed < 10) speed = 10;
        if (speed > 10000) speed = 100000;
    }

    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
        g_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
        if (g_i2c_handle < 0) {
        	vm_log_error("error opening i2c %d", g_i2c_handle);
            lua_pushinteger(L, g_i2c_handle);
        	g_i2c_handle = VM_DCL_HANDLE_INVALID;
        }
    }
    if (g_i2c_handle != VM_DCL_HANDLE_INVALID) {
		conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
		conf_data.fast_mode_speed = speed;
		conf_data.high_mode_speed = 1;
		conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
		conf_data.get_handle_wait = 0;
		conf_data.reserved_1 = 0;
		conf_data.delay_length = 0;
		conf_data.slave_address = (address << 1);
    	vm_log_debug("configuring i2c %d", g_i2c_handle);
		result = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONFIG, (void *)&conf_data);
    	vm_log_debug("result %d", result);

		lua_pushinteger(L, result);
    }

    return 1;
}

//------------------------------------------------
static int i2c_writedata(char *data, VMUINT32 len)
{
    vm_dcl_i2c_control_continue_write_t param;

	param.data_length = len;
	param.transfer_number = 1;
	int stat = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
	if (stat < 0) {
        vm_log_debug("error writing data: %d", stat);
	}
	return stat;
}

//-----------------------------------------------
static int i2c_readdata(char *data, VMUINT32 len)
{
    vm_dcl_i2c_control_continue_read_t param;

	param.data_length = len;
	param.transfer_number = 1;
	int stat = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, &param);
	if (stat < 0) {
        vm_log_debug("error reading data: %d", stat);
	}
	return stat;
}


// Lua: wrote = i2c.send(data1, [data2], ..., [datan] )
// data can be either a string, a table or an 8-bit number
//--------------------------------
static int i2c_send(lua_State* L)
{
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "i2c not opened");
    if (lua_gettop(L) < 1) return luaL_error(L, "invalid number of arguments");

    const char* pdata;
    size_t datalen, i;
    int numdata;
    int status = 0;
    uint32_t wrote = 0;
    unsigned argn;
    uint8_t wbuf[8]; // max size - 8
    uint8_t wbuf_index = 0;

    for (argn = 1; argn <= lua_gettop(L); argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if (numdata < 0 || numdata > 255) {
                vm_log_debug("numeric data must be from 0 to 255");
            }
            else {
				wbuf[wbuf_index] = numdata;
				wbuf_index++;
				if (wbuf_index >= 8) {
					status = i2c_writedata(wbuf, 8);
					if (status != VM_DCL_STATUS_OK) break;
					wbuf_index = 0;
					wrote += 8;
				}
            }
        }
        else if (lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for (i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                if (lua_type(L, -1) == LUA_TNUMBER) {
					numdata = (int)luaL_checkinteger(L, -1);
					lua_pop(L, 1);
					if (numdata < 0 || numdata > 255) {
						vm_log_debug("numeric data must be from 0 to 255");
					}
					else {
						wbuf[wbuf_index] = numdata;
						wbuf_index++;
						if (wbuf_index >= 8) {
							status = i2c_writedata(wbuf, 8);
							if (status != VM_DCL_STATUS_OK) break;
							wbuf_index = 0;
							wrote += 8;
						}
					}
                }
                else lua_pop(L, 1);
            }
			if (status != VM_DCL_STATUS_OK) break;
        }
        else if (lua_isstring(L, argn)) {
            pdata = luaL_checklstring(L, argn, &datalen);
            for (i = 0; i < datalen; i++) {
                wbuf[wbuf_index] = numdata;
                wbuf_index++;
                if (wbuf_index >= 8) {
					status = i2c_writedata(wbuf, 8);
					if (status != VM_DCL_STATUS_OK) break;
                    wbuf_index = 0;
                    wrote += 8;
                }
            }
			if (status != VM_DCL_STATUS_OK) break;
        }
    }

    if ((status == VM_DCL_STATUS_OK) && (wbuf_index)) {
		status = i2c_writedata(wbuf, wbuf_index);
        if (status == VM_DCL_STATUS_OK) wrote += wbuf_index;
    }

	lua_pushinteger(L, wrote);
	return 1;
}

// Lua: read = i2c.recv(size)
//===============================
static int i2c_recv(lua_State* L)
{
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "i2c not opened");

    uint32_t size = luaL_checkinteger(L, 1);
    if (size <= 0) return luaL_error(L, "size must be > 0");

    int i;
    luaL_Buffer b;
    VM_DCL_STATUS status = 0;
    char rbuf[8] = {'\0'};

    luaL_buffinit(L, &b);

    do {
        if (size >= 8) {
        	status = i2c_readdata(rbuf, 8);
            if (status != VM_DCL_STATUS_OK) break;

            for (i = 0; i < 8; i++) luaL_addchar(&b, rbuf[i]);
            size -= 8;
        }
        else {
        	status = i2c_readdata(rbuf, size);
            if (status != VM_DCL_STATUS_OK) break;

            for(i = 0; i < size; i++) luaL_addchar(&b, rbuf[i]);
            size = 0;
        }
    } while(size > 0);

    luaL_pushresult(&b);
    return 1;
}

// Lua: read = i2c.txrx(out_data, read_size)
//===============================
static int i2c_txrx(lua_State* L)
{
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "i2c not opened");
    if (lua_gettop(L) < 2) return luaL_error(L, "invalid number of arguments");

    const char* pdata;
    size_t datalen;
    int numdata;
    int argn;
    int i;
    luaL_Buffer b;
    vm_dcl_i2c_control_write_and_read_t param;
    char wbuf[8];
    char rbuf[8];
    int wbuf_index = 0;
    int top = lua_gettop(L);
    VM_DCL_STATUS status = 0;

    size_t size = luaL_checkinteger(L, -1);
    if(size <= 0) {
    	lua_pushnil(L);
        return 1;
    }

    // push send data to buffer
    for (argn = 1; argn < top; argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if (numdata < 0 || numdata > 255) {
                vm_log_debug("numeric data must be from 0 to 255");
            }
            else if (wbuf_index > 8) {
            	vm_log_debug("write data length must not exceed 8");
            }
            else {
            	wbuf[wbuf_index] = numdata;
            	wbuf_index++;
            }
            
        }
        else if(lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for (i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                if (lua_type(L, -1) == LUA_TNUMBER) {
					numdata = (int)luaL_checkinteger(L, -1);
					if (numdata < 0 || numdata > 255) {
						vm_log_debug("numeric data must be from 0 to 255");
					}
					else if (wbuf_index > 8) {
						vm_log_debug("write data length must not exceed 8");
					}
					else {
						wbuf[wbuf_index] = numdata;
						wbuf_index++;
					}
                }
				lua_pop(L, 1);
            }
        }
        else if (lua_isstring(L, argn)) {
            pdata = luaL_checklstring(L, argn, &datalen);
            for (i = 0; i < datalen; i++) {
                if (wbuf_index > 8) {
                	vm_log_debug("write data length must not exceed 8");
                }
                else {
                	wbuf[wbuf_index] = numdata;
                	wbuf_index++;
                }
            }
        }
    }

    // Check if there is any data to send
    if (wbuf_index == 0) return luaL_error(L, "no data to write");

    // Send and receive
    param.out_data_length = wbuf_index;
    param.out_data_ptr = wbuf;
    if (size > 8) param.in_data_length = 8;
    else param.in_data_length = size;
    param.in_data_ptr = rbuf;


    status = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, &param);
    if (status != VM_DCL_STATUS_OK) {
    	vm_log_error("write and read error %d", status);
    	lua_pushnil(L);
    }
    else {
		luaL_buffinit(L, &b);
		if (size > 8) {
			// Get 1st 8 data bytes to buffer
			for (i = 0; i < 8; i++) luaL_addchar(&b, rbuf[i]);
			size -= 8;

			// Continue reading ...
		    do {
		        if (size >= 8) {
		        	// Get 8 bytes
		        	status = i2c_readdata(rbuf, 8);
		            if (status != VM_DCL_STATUS_OK) break;

		            for (i = 0; i < 8; i++) luaL_addchar(&b, rbuf[i]);
		            size -= 8;
		        }
		        else {
		        	// Get remaining bytes
		        	status = i2c_readdata(rbuf, size);
		            if (status != VM_DCL_STATUS_OK) break;

		            for(i = 0; i < size; i++) luaL_addchar(&b, rbuf[i]);
		            size = 0;
		        }
		    } while(size > 0);
		}
		else {
			// Get 'size' data bytes to buffer
			for (i = 0; i < size; i++) luaL_addchar(&b, rbuf[i]);
		}

		luaL_pushresult(&b);
    }

    return 1;
}

//================================
static int i2c_close(lua_State* L)
{
	VM_DCL_STATUS status = 0;
    if (g_i2c_handle != VM_DCL_HANDLE_INVALID) {
    	VM_DCL_STATUS status = vm_dcl_close(g_i2c_handle);
    	g_i2c_handle = VM_DCL_HANDLE_INVALID;
    }
	lua_pushinteger(L, status);
	return 1;
}


// Module function map
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE i2c_map[] = {
		{ LSTRKEY("setup"), LFUNCVAL(i2c_setup) },
        { LSTRKEY("send"), LFUNCVAL(i2c_send) },
        { LSTRKEY("recv"), LFUNCVAL(i2c_recv) },
        { LSTRKEY("txrx"), LFUNCVAL(i2c_txrx) },
        { LSTRKEY("close"), LFUNCVAL(i2c_close) },
        { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_i2c(lua_State* L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0
    luaL_register(L, "i2c", i2c_map);
    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
