// Module for interfacing with the I2C interface

#include <stdint.h>

#include "vmdcl.h"
#include "vmdcl_i2c.h"

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

VM_DCL_HANDLE g_i2c_handle = VM_DCL_HANDLE_INVALID;

// Lua: i2c.setup(address [, speed])
static int i2c_setup(lua_State* L)
{
    vm_dcl_i2c_control_config_t conf_data;
    int result;
    uint8_t address = luaL_checkinteger(L, 1);
    uint32_t speed = 100; // default speed - 100kbps

    if(lua_gettop(L) > 1) {
        speed = luaL_checkinteger(L, 2);
    }

    if(speed >= 400) {
        conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE;
    } else {
        conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
    }

    if(g_i2c_handle == VM_DCL_HANDLE_INVALID) {
        g_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
    }

    conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
    conf_data.get_handle_wait = TRUE;
    conf_data.reserved_1 = 0;
    conf_data.delay_length = 0;
    conf_data.slave_address = (address << 1);
    conf_data.fast_mode_speed = speed;
    conf_data.high_mode_speed = 0;
    result = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONFIG, &conf_data);

    lua_pushinteger(L, result);
    return 1;
}

// Lua: wrote = i2c.send(data1, [data2], ..., [datan] )
// data can be either a string, a table or an 8-bit number
static int i2c_send(lua_State* L)
{
    const char* pdata;
    size_t datalen, i;
    int numdata;
    uint32_t wrote = 0;
    unsigned argn;
    vm_dcl_i2c_control_continue_write_t param;
    uint8_t wbuf[8]; // max size - 8
    uint8_t wbuf_index = 0;

    param.data_ptr = wbuf;

    if(lua_gettop(L) < 1)
        return luaL_error(L, "invalid number of arguments");
    for(argn = 1; argn <= lua_gettop(L); argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if(lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if(numdata < 0 || numdata > 255)
                return luaL_error(L, "numeric data must be from 0 to 255");

            wbuf[wbuf_index] = numdata;
            wbuf_index++;
            if(wbuf_index >= 8) {
                param.data_length = wbuf_index;
                param.transfer_number = wbuf_index;
                vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
                wbuf_index = 0;
                wrote += 8;
            }
        } else if(lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for(i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                numdata = (int)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                if(numdata < 0 || numdata > 255)
                    return luaL_error(L, "numeric data must be from 0 to 255");

                wbuf[wbuf_index] = numdata;
                wbuf_index++;
                if(wbuf_index >= 8) {
                    param.data_length = wbuf_index;
                    param.transfer_number = wbuf_index;
                    vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
                    wbuf_index = 0;
                    wrote += 8;
                }
            }
        } else {
            pdata = luaL_checklstring(L, argn, &datalen);
            for(i = 0; i < datalen; i++) {
                wbuf[wbuf_index] = numdata;
                wbuf_index++;
                if(wbuf_index >= 8) {
                    param.data_length = wbuf_index;
                    param.transfer_number = wbuf_index;
                    vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
                    wbuf_index = 0;
                    wrote += 8;
                }
            }
        }
    }

    if(wbuf_index) {
        param.data_length = wbuf_index;
        param.transfer_number = wbuf_index;
        vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
        wrote += wbuf_index;
    }

    lua_pushinteger(L, wrote);
    return 1;
}

// Lua: read = i2c.recv(size)
static int i2c_recv(lua_State* L)
{
    uint32_t size = luaL_checkinteger(L, 1);
    int i;
    luaL_Buffer b;
    vm_dcl_i2c_control_continue_read_t param;
    VM_DCL_STATUS status;
    char rbuf[8];

    if(size == 0)
        return 0;

    param.data_ptr = rbuf;

    luaL_buffinit(L, &b);

    do {
        if(size >= 8) {
            param.data_length = 8;
            param.transfer_number = 8;
            status = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, &param);
            if(status != VM_DCL_STATUS_OK) {
                break;
            }

            for(i = 0; i < 8; i++) {
                luaL_addchar(&b, rbuf[i]);
            }

            size -= 8;
        } else {
            param.data_length = size;
            param.transfer_number = size;
            status = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, &param);
            if(status != VM_DCL_STATUS_OK) {
                break;
            }

            for(i = 0; i < size; i++) {
                luaL_addchar(&b, rbuf[i]);
            }

            size = 0;
        }
    } while(size > 0);
    luaL_pushresult(&b);
    return 1;
}

// Lua: read = i2c.txrx(addr, size)
static int i2c_txrx(lua_State* L)
{
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
    size_t size = luaL_checkinteger(L, -1);
    int top = lua_gettop(L);

    if(size <= 0) {
        return 0;
    } else if(size > 8) {
        return luaL_error(L, "read data length must not exceed 8");
    }
    
    if(lua_gettop(L) < 2)
        return luaL_error(L, "invalid number of arguments");
    for(argn = 1; argn < top; argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if(lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if(numdata < 0 || numdata > 255)
                return luaL_error(L, "numeric data must be from 0 to 255");

            if(wbuf_index > 8) {
                return luaL_error(L, "write data length must not exceed 8");
            }
            wbuf[wbuf_index] = numdata;
            wbuf_index++;
            
        } else if(lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for(i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                numdata = (int)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                if(numdata < 0 || numdata > 255)
                    return luaL_error(L, "numeric data must be from 0 to 255");

                if(wbuf_index > 8) {
                    return luaL_error(L, "write data length must not exceed 8");
                }
                wbuf[wbuf_index] = numdata;
                wbuf_index++;
            }
        } else {
            pdata = luaL_checklstring(L, argn, &datalen);
            for(i = 0; i < datalen; i++) {
                if(wbuf_index > 8) {
                    return luaL_error(L, "write data length must not exceed 8");
                }
                wbuf[wbuf_index] = numdata;
                wbuf_index++;
            }
        }
    }

    param.out_data_length = wbuf_index;
    param.out_data_ptr = wbuf;
    param.in_data_length = size;
    param.in_data_ptr = rbuf;

    if(vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, &param) != VM_DCL_STATUS_OK) {
        return 0;
    }

    luaL_buffinit(L, &b);
    for(i = 0; i < size; i++)
        luaL_addchar(&b, rbuf[i]);

    luaL_pushresult(&b);
    return 1;
}

// Module function map
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE i2c_map[] = { { LSTRKEY("setup"), LFUNCVAL(i2c_setup) },
                                 { LSTRKEY("send"), LFUNCVAL(i2c_send) },
                                 { LSTRKEY("recv"), LFUNCVAL(i2c_recv) },
                                 { LSTRKEY("txrx"), LFUNCVAL(i2c_txrx) },
                                 { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_i2c(lua_State* L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0
    luaL_register(L, "i2c", i2c_map);
    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
