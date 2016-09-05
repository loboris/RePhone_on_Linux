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
#define I2C_CLOCK_RATE		13000
#define I2C_MAX_BUF_SIZE	10*1024

VM_DCL_HANDLE g_i2c_handle = VM_DCL_HANDLE_INVALID;
VMUINT8 g_i2c_used_address = 0xFF;


//---------------------------------------------------------------------------
static void i2c_set_transaction_speed(vm_dcl_i2c_control_config_t *conf_data)
{
	VMUINT32 step_cnt_div;
	VMUINT32 sample_cnt_div;
	VMUINT32 temp;
	volatile uint32_t *reg;
	VMUINT16 creg;

/***********************************************************
* Note: according to datasheet
*  speed = 13MHz/(2*(step_cnt_div+1)*(sample_cnt_div+1))
************************************************************/

	// == Fast Mode Speed ==
	for (sample_cnt_div=1;sample_cnt_div<=8;sample_cnt_div++) {
		if (conf_data->fast_mode_speed > 0) temp=((conf_data->fast_mode_speed)*2*sample_cnt_div);
		else temp = 1*2*sample_cnt_div;
		step_cnt_div = (I2C_CLOCK_RATE+temp-1) / temp;	//cast the <1 part

		if (step_cnt_div <= 64) break;
	}
	if (step_cnt_div > 64) step_cnt_div = 64;
	if (sample_cnt_div > 8) sample_cnt_div = 8;

	// Set timing control register (fast mode)
	conf_data->fast_mode_speed = I2C_CLOCK_RATE /(2 * sample_cnt_div * step_cnt_div);
	//printf("fast mode speed=%d fs_sample_cnt_div=%d fs_step_cnt_div=%d\n", conf_data->fast_mode_speed, sample_cnt_div-1,step_cnt_div-1);

	reg = (uint32_t *)(REG_BASE_ADDRESS + 0x0120020);
	creg = *reg & 0x7800;
	creg = creg | ((sample_cnt_div-1) << 8) | (step_cnt_div-1);
	*reg = creg;

	// == HS Mode Speed ==
	//if (conf_data->transaction_mode == VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE)
	for (sample_cnt_div=1;sample_cnt_div<=8;sample_cnt_div++) {
		if (conf_data->high_mode_speed > 0) temp=((conf_data->high_mode_speed)*2*sample_cnt_div);
		else temp=(1*2*sample_cnt_div);
		step_cnt_div = (I2C_CLOCK_RATE+temp-1) / temp;
		if (step_cnt_div <= 8) break;
	}
	if (step_cnt_div > 8) step_cnt_div = 8;
	if (sample_cnt_div > 8) sample_cnt_div = 8;

	// Set timing control register (fast mode)
	conf_data->high_mode_speed = I2C_CLOCK_RATE /(2 * sample_cnt_div * step_cnt_div);
	//printf("high mode speed=%d hs_sample_cnt_div=%d hs_step_cnt_div=%d\n", conf_data->high_mode_speed, sample_cnt_div-1,step_cnt_div-1);

	reg = (uint32_t *)(REG_BASE_ADDRESS + 0x0120048);
	creg = *reg & 0x00F2;
	creg = creg | ((sample_cnt_div-1) << 8) | ((step_cnt_div-1) << 12);

	if (conf_data->fast_mode_speed > 400) {
		conf_data->transaction_mode = VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE;
		creg |= 1;
	}
	*reg = creg;
}

//---------------------------------------------
int _i2c_setup(VMUINT8 address, VMUINT32 speed)
{
    vm_dcl_i2c_control_config_t conf_data;
	int result = VM_DCL_HANDLE_INVALID;

    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
        g_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
        if (g_i2c_handle < 0) {
        	vm_log_error("error opening i2c %d", g_i2c_handle);
        	g_i2c_handle = VM_DCL_HANDLE_INVALID;
        }
    }
    if (g_i2c_handle != VM_DCL_HANDLE_INVALID) {
        g_i2c_used_address = (address << 1);

        conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
		conf_data.fast_mode_speed = speed;
		conf_data.high_mode_speed = speed;
		conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
		conf_data.get_handle_wait = 1;
		conf_data.reserved_1 = 0;
		conf_data.delay_length = 0;
		conf_data.slave_address = (address << 1);

		// Calculate actual speed and mode parameters
		i2c_set_transaction_speed(&conf_data);
		// Configure i2c
		result = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONFIG, (void *)&conf_data);
		if (result == VM_DCL_STATUS_OK) {
			if (conf_data.fast_mode_speed > 400) result = conf_data.high_mode_speed;
			else result = conf_data.fast_mode_speed;
		}
    }
    return result;
}

// Lua: i2c.setup(address [, speed])
//=================================
static int i2c_setup(lua_State* L)
{
    vm_dcl_i2c_control_config_t conf_data;
    uint8_t address = luaL_checkinteger(L, 1);
    VMUINT32 speed = 100; // default speed - 100kbps

    if (lua_gettop(L) > 1) {
        speed = luaL_checkinteger(L, 2);
        if (speed < 12) speed = 12;
        if (speed > 6500) speed = 6500;
    }

    int result = _i2c_setup(address, speed);

	lua_pushinteger(L, result);

    return 1;
}

//------------------------------------------
int _i2c_writedata(char *data, VMUINT32 len)
{
    if (len == 0) return 0;
    vm_dcl_i2c_control_continue_write_t param;

    int stat = 0;
    VMUINT32 remain = len;
    VMUINT32 bufptr = 0;
	param.transfer_number = 1;

	do {
		param.data_ptr = data+bufptr;
		if (remain > 8) param.data_length = 8;
		else param.data_length = remain;

		stat = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, &param);
		if (stat < 0) {
			vm_log_debug("error writing data: %d", stat);
			break;
		}
		if (remain > 8) {
			bufptr += 8;
			remain -= 8;
		}
		else {
			bufptr += remain;
			remain = 0;
		}
	} while (remain > 0);

	return stat;
}

//-----------------------------------------
int _i2c_readdata(char *data, VMUINT32 len)
{
    if (len == 0) return 0;
    vm_dcl_i2c_control_continue_read_t param;
    int status = 0;

    VMUINT32 remain = len;
    VMUINT32 bufptr = 0;
	param.transfer_number = 1;

	do {
		param.data_ptr = data+bufptr;
		if (remain > 8) param.data_length = 8;
		else param.data_length = remain;

		status = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, &param);
		if (status < 0) {
			vm_log_debug("error reading data: %d", status);
			break;
		}
		if (remain > 8) {
			bufptr += 8;
			remain -= 8;
		}
		else {
			bufptr += remain;
			remain = 0;
		}
	} while (remain > 0);

	return status;
}

// Send and receive
//----------------------------------------------------------------------------------------
int _i2c_write_and_read_data(uint8_t *wdata, uint8_t *rdata, VMUINT32 wlen, VMUINT32 rlen)
{
    vm_dcl_i2c_control_write_and_read_t param;
    int status = 0;

    param.out_data_length = wlen;
	param.out_data_ptr = wdata;
	if (rlen > 8) param.in_data_length = 8;
	else param.in_data_length = rlen;
	param.in_data_ptr = rdata;

    status = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, &param);
	if ((status == VM_DCL_STATUS_OK) && (rlen > 8)) {
		// Read remaining bytes
		status = _i2c_readdata(rdata+8, rlen-8);
	}

	return status;
}

// Lua: wrote = i2c.send(data1, [data2], ..., [datan] )
// data can be either a string, a table or an 8-bit number
//-------------------------------
static int i2c_send(lua_State* L)
{
	int err = 0;

    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
    	vm_log_error("i2c not opened");
    	err = -101;
    	goto exit;
    }
    if (lua_gettop(L) < 1) {
    	vm_log_error("invalid number of arguments");
    	err = -102;
    	goto exit;
    }

    const char* pdata;
    size_t datalen, i;
    int numdata;
    int status = 0;
    unsigned argn;
    VMUINT32 wbufsize = 256;
    uint8_t wbuf_index = 0;

    uint8_t *tbuf = NULL;
    uint8_t *wbuf = vm_malloc(wbufsize);
    if (wbuf == NULL) {
        vm_log_error("error allocating memory");
    	err = -103;
    	return 1;
    }


    for (argn = 1; argn <= lua_gettop(L); argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if (numdata < 0 || numdata > 255) {
                vm_log_debug("numeric data must be from 0 to 255");
            }
            else {
				if (wbuf_index >= wbufsize) {
					wbufsize += 256;
					if (wbufsize > I2C_MAX_BUF_SIZE) {
				    	status = -1;
				    	break;
					}
				    uint8_t *tbuf = vm_realloc(wbuf, wbufsize);
				    if (tbuf == NULL) {
				    	status = -1;
				    	break;
				    }
				    wbuf = tbuf;
				}
				wbuf[wbuf_index] = numdata;
				wbuf_index++;
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
						if (wbuf_index >= wbufsize) {
							wbufsize += 256;
							if (wbufsize > I2C_MAX_BUF_SIZE) {
						    	status = -1;
						    	break;
							}
						    uint8_t *tbuf = vm_realloc(wbuf, wbufsize);
						    if (tbuf == NULL) {
						    	status = -1;
						    	break;
						    }
						    wbuf = tbuf;
						}
						wbuf[wbuf_index] = numdata;
						wbuf_index++;
					}
                }
                else lua_pop(L, 1);
            }
			if (status < 0) break;
        }
        else if (lua_isstring(L, argn)) {
            pdata = luaL_checklstring(L, argn, &datalen);
            for (i = 0; i < datalen; i++) {
				if (wbuf_index >= wbufsize) {
					wbufsize += 256;
					if (wbufsize > I2C_MAX_BUF_SIZE) {
				    	status = -1;
				    	break;
					}
				    uint8_t *tbuf = vm_realloc(wbuf, wbufsize);
				    if (tbuf == NULL) {
				    	status = -1;
				    	break;
				    }
				    wbuf = tbuf;
				}
                wbuf[wbuf_index] = numdata;
                wbuf_index++;
            }
			if (status < 0) break;
        }
    }

    if ((status == 0) && (wbuf_index)) {
		status = _i2c_writedata(wbuf, wbuf_index);
        if (status == VM_DCL_STATUS_OK) status = wbuf_index;
    }

    vm_free(wbuf);

	lua_pushinteger(L, status);
	return 1;

exit:
	lua_pushinteger(L, err);
	return 1;
}

// Lua: read = i2c.recv(size)
//===============================
static int i2c_recv(lua_State* L)
{
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
    	vm_log_error("i2c not opened");
    	goto exit;
    }

    uint32_t size = 0;

    if (lua_type(L, 1) == LUA_TNUMBER) {
    	size = luaL_checkinteger(L, 1);
    }
    if ((size <= 0) || (size > I2C_MAX_BUF_SIZE)) {
    	vm_log_error("size must be 1 ~ %d", I2C_MAX_BUF_SIZE);
    	goto exit;
    }

    int i;
    int out_type = 0;
    luaL_Buffer b;
    VM_DCL_STATUS status = 0;
    char hbuf[4];

    uint8_t *rbuf = vm_calloc(size);
    if (rbuf == NULL) {
        vm_log_error("error allocating memory");
    	goto exit;
    }

    if (lua_isstring(L, 2)) {
        const char* sarg;
        size_t sarglen;
        sarg = luaL_checklstring(L, 2, &sarglen);
        if (sarglen == 2) {
        	if (strstr(sarg, "*h") != NULL) out_type = 1;
        	else if (strstr(sarg, "*t") != NULL) out_type = 2;
        }
    }

    if (out_type < 2) luaL_buffinit(L, &b);
    else lua_newtable(L);

    status = _i2c_readdata(rbuf, size);
    if (status == VM_DCL_STATUS_OK) {
        for (i = 0; i < size; i++) {
        	if (out_type == 0) luaL_addchar(&b, rbuf[i]);
        	else if (out_type == 1) {
        		sprintf(hbuf, "%02x;", rbuf[i]);
        		luaL_addstring(&b, hbuf);
        	}
        	else {
        	    lua_pushinteger( L, rbuf[i]);
        	    lua_rawseti(L,-2, i+1);
        	}
        }
    }

    vm_free(rbuf);

    if (out_type < 2) luaL_pushresult(&b);

	return 1;

exit:
	lua_pushnil(L);
	return 1;
}

// Lua: read = i2c.txrx(out_data, read_size)
//===============================
static int i2c_txrx(lua_State* L)
{
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
    	vm_log_error("i2c not opened");
    	goto exit;
    }
    if (lua_gettop(L) < 2) {
    	vm_log_error("invalid number of arguments");
    	goto exit;
    }

    int out_type = 0;
    VMUINT16 size;

    if (lua_type(L, -1) == LUA_TNUMBER) {
        size = luaL_checkinteger(L, -1);
        if ((size <= 0) || (size > I2C_MAX_BUF_SIZE)) {
        	vm_log_error("size must be 1 ~ %d", I2C_MAX_BUF_SIZE);
        	goto exit;
        }
    }
    else if (lua_type(L, -1) == LUA_TSTRING) {
        if (lua_gettop(L) < 3) {
        	vm_log_error("invalid number of arguments");
        	goto exit;
        }

        if (lua_type(L, -2) == LUA_TNUMBER) {
            size = luaL_checkinteger(L, -2);
            if ((size <= 0) || (size > I2C_MAX_BUF_SIZE)) {
            	vm_log_error("size must be 1 ~ %d", I2C_MAX_BUF_SIZE);
            	goto exit;
            }

            const char* sarg;
            size_t sarglen;
            sarg = luaL_checklstring(L, -1, &sarglen);
            if (sarglen == 2) {
            	if (strstr(sarg, "*h") != NULL) out_type = 1;
            	else if (strstr(sarg, "*t") != NULL) out_type = 2;
            }
        }
        else {
        	vm_log_error("size argument not found");
        }

    }
    else {
    	vm_log_error("size argument not found");
    	goto exit;
    }

    const char* pdata;
    size_t datalen;
    int numdata;
    int argn;
    int i;
    luaL_Buffer b;
    char wbuf[8];
    int wbuf_index = 0;
    int top = lua_gettop(L);

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
    if (wbuf_index == 0) {
    	vm_log_error("no data to write");
    	goto exit;
    }


    VM_DCL_STATUS status = 0;
    char hbuf[4];
    uint8_t *rbuf = vm_calloc(size);
    if (rbuf == NULL) {
        vm_log_error("error allocating memory [%d]", size);
    	goto exit;
    }

    // Send and receive
    status = _i2c_write_and_read_data(wbuf, rbuf, wbuf_index, size);

    if (status != VM_DCL_STATUS_OK) {
    	vm_log_error("write and read error %d", status);
    	lua_pushnil(L);
    }
    else {
        if (out_type < 2) luaL_buffinit(L, &b);
        else lua_newtable(L);

		// Get bytes received from write&read command (max 8)
		for (i = 0; i < size; i++) {
        	if (out_type == 0) luaL_addchar(&b, rbuf[i]);
        	else if (out_type == 1) {
        		sprintf(hbuf, "%02x;", rbuf[i]);
        		luaL_addstring(&b, hbuf);
        	}
        	else {
        	    lua_pushinteger( L, rbuf[i]);
        	    lua_rawseti(L,-2, i+1);
        	}
		}
		if (out_type < 2) luaL_pushresult(&b);
    }
    vm_free(rbuf);

	return 1;

exit:
	lua_pushnil(L);
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
        { LSTRKEY("write"), LFUNCVAL(i2c_send) },
        { LSTRKEY("read"), LFUNCVAL(i2c_recv) },
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
