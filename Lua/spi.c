// Module for interfacing with the SPI interface

#include <stdint.h>
#include <string.h>

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_spi.h"
#include "vmlog.h"
#include "vmmemory.h"

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "shell.h"

#define SPI_SCLK_PIN_NAME			27
#define SPI_MOSI_PIN_NAME			28
#define SPI_MISO_PIN_NAME			29
#define VM_SPI_READ_BUFFER_SIZE		128
#define VM_SPI_WRITE_BUFFER_SIZE	128

typedef struct _VM_READ_BUFFER
{
    VMUINT8 read_buffer[VM_SPI_READ_BUFFER_SIZE];
	VMUINT8 read_len;
}VM_READ_BUFFER;


typedef struct _VM_WRITE_BUFFER
{
    VMUINT8 write_buffer[VM_SPI_WRITE_BUFFER_SIZE];
	VMUINT8 write_len;
}VM_WRITE_BUFFER;

// SPI data transfer buffers
static VM_READ_BUFFER* g_spi_read_data;
static VM_WRITE_BUFFER* g_spi_write_data;

static VM_DCL_HANDLE g_spi_handle = VM_DCL_HANDLE_INVALID;
static VM_DCL_HANDLE g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
static VM_DCL_HANDLE g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
static int spi_dc_state = 1;

// SPI data transfer buffer
static vm_dcl_spi_control_read_write_t g_spi_write_and_read = {NULL,0,NULL,0};

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);



// Lua: spi.setup(])
//================================
static int _spi_setup(lua_State* L)
{
	VM_DCL_HANDLE gpio_handle;
	vm_dcl_spi_config_parameter_t conf_data;
    int result = VM_DCL_HANDLE_INVALID;

    if (g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi_cs_handle);
    	g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
    }
    if (g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi_dc_handle);
    	g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
    }

	// SPI_MOSI gpio config
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_MOSI_PIN_NAME);
    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_4, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	// SPI_MISO gpio config
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_MISO_PIN_NAME);
    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;

    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_4, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	// SPI_SCL gpio config
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_SCLK_PIN_NAME);
    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_4, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	if (VM_DCL_HANDLE_INVALID == g_spi_handle) {
		g_spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1, 0);
		if (VM_DCL_HANDLE_INVALID != g_spi_handle) {
			g_spi_read_data = (VM_READ_BUFFER*)vm_malloc_dma(sizeof(VM_READ_BUFFER));
			if (g_spi_read_data == NULL) {
				vm_dcl_close(g_spi_handle);
				g_spi_handle = VM_DCL_HANDLE_INVALID;
				goto exit;
			}
			g_spi_write_data = (VM_WRITE_BUFFER*)vm_malloc_dma(sizeof(VM_WRITE_BUFFER));
			if (g_spi_write_data == NULL) {
				vm_dcl_close(g_spi_handle);
				g_spi_handle = VM_DCL_HANDLE_INVALID;
				vm_free(g_spi_read_data);
				g_spi_write_and_read.read_data_ptr = NULL;
				goto exit;
			}
		}
		else {
			vm_log_error("Error opening spi");
			goto exit;
		}
	}

	// set default config
	conf_data.clock_high_time = 5;
	conf_data.clock_low_time = 5;
	conf_data.cs_hold_time = 15;
	conf_data.cs_idle_time = 15;
	conf_data.cs_setup_time= 15;
	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
	conf_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	conf_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	conf_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
	conf_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;

	if (lua_istable(L, 1)) {
		int param;
		// get configuration
		lua_getfield(L, 1, "mode");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    int param = luaL_checkinteger( L, -1 );
		    if ((param >= 0) && (param < 4)) {
		        switch (param) {
		            case 0:
		            	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
		            	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
		                break;
		            case 1:
		            	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
		            	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_1;
		                break;
		            case 2:
		            	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_1;
		            	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
		                break;
		            case 3:
		            	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_1;
		            	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_1;
		                break;
		        }
		    }
		  }
		}
		lua_getfield(L, 1, "endian");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if (param == 0) {
		    	conf_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
		    	conf_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
		    }
		    else if (param == 1) {
		    	conf_data.rx_endian = VM_DCL_SPI_ENDIAN_BIG;
		    	conf_data.tx_endian = VM_DCL_SPI_ENDIAN_BIG;
		    }
		  }
		}
		lua_getfield(L, 1, "msb");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if (param == 0) {
		    	conf_data.rx_msbf = VM_DCL_SPI_LSB_FIRST;
		    	conf_data.tx_msbf = VM_DCL_SPI_LSB_FIRST;
		    }
		    else if (param == 1) {
		    	conf_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
		    	conf_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
		    }
		  }
		}
		lua_getfield(L, 1, "clk");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if ((param >= 0) && (param <= 255)) {
		    	conf_data.clock_high_time = param;
		    	conf_data.clock_low_time = param;
		    }
		  }
		}
		lua_getfield(L, 1, "speed");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if (param < 86) param = 86;
		    if (param > 22000) param = 22000;
		    param = 22000 / param;
		    if (param > 255) param = 255;
	    	conf_data.clock_high_time = param;
	    	conf_data.clock_low_time = param;
			vm_log_debug("SPI clock set to %d kHz", param);
		  }
		}
		lua_getfield(L, 1, "cs");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    gpio_get_handle(param, &g_spi_cs_handle);
		    if (g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
		        vm_dcl_control(g_spi_cs_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
		        vm_dcl_control(g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		    }
		    else {
		    	g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
				vm_log_error("error initializing CS on pin #%d", param);
		    }
		  }
		}
		lua_getfield(L, 1, "dc");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    gpio_get_handle(param, &g_spi_dc_handle);
		    if (g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
		        vm_dcl_control(g_spi_dc_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
		        vm_dcl_control(g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		        spi_dc_state = 1;
		    }
		    else {
		    	g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
				vm_log_error("error initializing DC on pin #%d", param);
		    }
		  }
		}
	}


	result = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&conf_data);

	if (VM_DCL_STATUS_OK != result) {
		vm_log_error("spi ioctl set config para fail. status: %d", result);
	}
	else {
		vm_dcl_spi_clock_t spi_data;
		vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_QUERY_CLOCK,(void *)&spi_data);
		vm_log_debug("spi base clock: %d MHz", spi_data.clock);
		vm_dcl_spi_mode_t spi_data_mode;
		spi_data_mode.mode = VM_DCL_SPI_MODE_PAUSE;
		spi_data_mode.parameter = 0;
		spi_data_mode.is_enabled = VM_TRUE;
		vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_MODE,(void *)&spi_data_mode);
	}

exit:
    lua_pushinteger(L, result);
    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

// Lua: spi.setup(])
//================================
static int spi_setup(lua_State* L)
{
	remote_CCall(&_spi_setup);
	return g_shell_result;
}

//==================================
static int _spi_deinit(lua_State* L)
{
	if (g_spi_handle != VM_DCL_HANDLE_INVALID) {	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}

		vm_dcl_close(g_spi_handle);
		g_spi_handle = VM_DCL_HANDLE_INVALID;
		vm_free(g_spi_read_data);
		vm_free(g_spi_write_data);
		g_spi_write_and_read.write_data_ptr = NULL;
		g_spi_write_and_read.read_data_ptr = NULL;
	}
    if (g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi_cs_handle);
    	g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
    }
    if (g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi_dc_handle);
    	g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
    }
    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=================================
static int spi_deinit(lua_State* L)
{
	remote_CCall(&_spi_deinit);
	return g_shell_result;
}

//--------------------------------------
static int _spi_read_write(lua_State* L)
{

	VM_DCL_STATUS status;

	// set DC
    if (g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
   		if (spi_dc_state) vm_dcl_control(g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
   		else vm_dcl_control(g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

	// activate CS
    if (g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

    g_spi_write_and_read.read_data_ptr = g_spi_read_data->read_buffer;
	g_spi_write_and_read.read_data_length = g_spi_read_data->read_len;
	g_spi_write_and_read.write_data_ptr = g_spi_write_data->write_buffer;
	g_spi_write_and_read.write_data_length = g_spi_write_data->write_len;

    status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&g_spi_write_and_read);

	// deactivate CS
    if (g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }

    if (VM_DCL_STATUS_OK != status)
	{
		vm_log_debug("spi_read_write: status = %d", status);
	    g_shell_result = 0;
		vm_signal_post(g_shell_signal);
		return 0;
	}

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 0;
}

// Lua: wrote = spi.send(data1, [data2], ..., [datan] )
// data can be either a string, a table or an 8-bit number
//===============================
static int spi_send(lua_State* L)
{
    const char* pdata;
    size_t datalen, i;
    int numdata;
    uint32_t wrote = 0;
    unsigned argn;

    if (lua_gettop(L) < 1) return luaL_error(L, "invalid number of arguments");

	memset(g_spi_write_data->write_buffer, 0xFF, VM_SPI_WRITE_BUFFER_SIZE);
	memset(g_spi_read_data->read_buffer, 0xFF, VM_SPI_READ_BUFFER_SIZE);
    g_spi_write_data->write_len = 0;
    g_spi_read_data->read_len = 0;

    for (argn = 1; argn <= lua_gettop(L); argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if (numdata < 0 || numdata > 255) return luaL_error(L, "numeric data must be from 0 to 255");

            g_spi_write_data->write_buffer[g_spi_write_data->write_len] = numdata;
            g_spi_write_data->write_len++;
            if (g_spi_write_data->write_len >= VM_SPI_WRITE_BUFFER_SIZE) {
            	remote_CCall(&_spi_read_write);
            	g_spi_write_data->write_len = 0;
                if (g_shell_result) wrote += VM_SPI_WRITE_BUFFER_SIZE;
            }
        }
        else if (lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for (i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                numdata = (int)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                if (numdata < 0 || numdata > 255) return luaL_error(L, "numeric data must be from 0 to 255");

                g_spi_write_data->write_buffer[g_spi_write_data->write_len] = numdata;
                g_spi_write_data->write_len++;
                if (g_spi_write_data->write_len >= VM_SPI_WRITE_BUFFER_SIZE) {
                	remote_CCall(&_spi_read_write);
                	g_spi_write_data->write_len = 0;
                	if (g_shell_result) wrote += VM_SPI_WRITE_BUFFER_SIZE;
                }
            }
        }
        else {
            pdata = luaL_checklstring(L, argn, &datalen);
            for (i = 0; i < datalen; i++) {
            	g_spi_write_data->write_buffer[g_spi_write_data->write_len] = pdata[i];
            	g_spi_write_data->write_len++;
                if (g_spi_write_data->write_len >= VM_SPI_WRITE_BUFFER_SIZE) {
                	remote_CCall(&_spi_read_write);
                	g_spi_write_data->write_len = 0;
                	if (g_shell_result) wrote += VM_SPI_WRITE_BUFFER_SIZE;
                }
            }
        }
    }

    if (g_spi_write_data->write_len) {
    	remote_CCall(&_spi_read_write);
    	if (g_shell_result) wrote += g_spi_write_data->write_len;
    }

    lua_pushinteger(L, wrote);
    return 1;
}

// Lua: read = spi.recv(size)
//===============================
static int spi_recv(lua_State* L)
{
    uint32_t size = luaL_checkinteger(L, 1);
    int i;
    luaL_Buffer b;

    luaL_buffinit(L, &b);

	memset(g_spi_write_data->write_buffer, 0xFF, VM_SPI_WRITE_BUFFER_SIZE);
	memset(g_spi_read_data->read_buffer, 0xFF, VM_SPI_READ_BUFFER_SIZE);
    g_spi_write_data->write_len = 0;
    int rd = 0;

    if (size > 0) {
		do {
			if (size >= VM_SPI_READ_BUFFER_SIZE) {
				g_spi_read_data->read_len = VM_SPI_READ_BUFFER_SIZE;
				remote_CCall(&_spi_read_write);
				if (!g_shell_result) break;

				for (i = 0; i < VM_SPI_READ_BUFFER_SIZE; i++) {
					luaL_addchar(&b, g_spi_read_data->read_buffer[i]);
				}

				size -= VM_SPI_READ_BUFFER_SIZE;
				rd += VM_SPI_READ_BUFFER_SIZE;
			}
			else {
				g_spi_read_data->read_len = size;
				remote_CCall(&_spi_read_write);
				if (!g_shell_result) break;

				for (i = 0; i < size; i++) {
					luaL_addchar(&b, g_spi_read_data->read_buffer[i]);
				}

				rd += size;
				size = 0;
			}
		} while(size > 0);
    }

    luaL_pushresult(&b);
    lua_pushinteger(L, rd);
    return 2;
}

// Lua: read = spi.txrx(data, [data, ...], size)
//===============================
static int spi_txrx(lua_State* L)
{
    const char* pdata;
    size_t datalen;
    int numdata;
    int argn;
    int i;
    luaL_Buffer b;

    size_t size = luaL_checkinteger(L, -1);
    int top = lua_gettop(L);

    if (size <= 0) size = 0;
    else if (size > VM_SPI_READ_BUFFER_SIZE) return luaL_error(L, "read data length exceeded");

	if (lua_gettop(L) < 2) return luaL_error(L, "invalid number of arguments");
    
	memset(g_spi_write_data->write_buffer, 0xFF, VM_SPI_WRITE_BUFFER_SIZE);
	memset(g_spi_read_data->read_buffer, 0xFF, VM_SPI_READ_BUFFER_SIZE);
	g_spi_write_data->write_len = 0;
	g_spi_read_data->read_len = size;

    for (argn = 1; argn < top; argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if (numdata < 0 || numdata > 255) return luaL_error(L, "numeric data must be from 0 to 255");

            if (g_spi_write_data->write_len > VM_SPI_WRITE_BUFFER_SIZE) return luaL_error(L, "write data length exceeded");
            g_spi_write_data->write_buffer[g_spi_write_data->write_len] = numdata;
            g_spi_write_data->write_len++;
            
        }
        else if(lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for (i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                numdata = (int)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                if (numdata < 0 || numdata > 255) return luaL_error(L, "numeric data must be from 0 to 255");

                if (g_spi_write_data->write_len > VM_SPI_WRITE_BUFFER_SIZE) return luaL_error(L, "write data length exceeded");
                g_spi_write_data->write_buffer[g_spi_write_data->write_len] = numdata;
                g_spi_write_data->write_len++;
            }
        }
        else {
            pdata = luaL_checklstring(L, argn, &datalen);
            for (i = 0; i < datalen; i++) {
                if (g_spi_write_data->write_len > VM_SPI_WRITE_BUFFER_SIZE) return luaL_error(L, "write data length exceeded");
                g_spi_write_data->write_buffer[g_spi_write_data->write_len] = pdata[i];
                g_spi_write_data->write_len++;
            }
        }
    }


	luaL_buffinit(L, &b);

	remote_CCall(&_spi_read_write);
    printf("spiWriteRead[");
    if (!g_shell_result) {
		luaL_pushresult(&b);
    	lua_pushinteger(L, -1);
    	lua_pushinteger(L, -1);
    }
    else {
        for (i = 0; i < size; i++) {
            luaL_addchar(&b, g_spi_read_data->read_buffer[i]);
			printf("%02x ", *(g_spi_read_data->read_buffer+i));
        }

		luaL_pushresult(&b);
    	lua_pushinteger(L, g_spi_write_data->write_len);
    	lua_pushinteger(L, g_spi_read_data->read_len);
    }
    printf("]\n");

    return 3;
}

//================================
static int spi_setdc(lua_State* L)
{
    spi_dc_state = luaL_checkinteger(L, -1) & 0x01;

    return 0;
}

// Module function map
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE spi_map[] = {
		{ LSTRKEY("setup"), LFUNCVAL(spi_setup) },
        { LSTRKEY("close"), LFUNCVAL(spi_deinit) },
		{ LSTRKEY("write"), LFUNCVAL(spi_send) },
        { LSTRKEY("read"), LFUNCVAL(spi_recv) },
        { LSTRKEY("txrx"), LFUNCVAL(spi_txrx) },
        { LSTRKEY("setdc"), LFUNCVAL(spi_setdc) },
        { LNILKEY, LNILVAL }
};

#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)


LUALIB_API int luaopen_spi(lua_State* L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0
    luaL_register(L, "spi", spi_map);
    // Add constants
    MOD_REG_NUMBER(L, "SPI_LSB_FIRST", 0);
    MOD_REG_NUMBER(L, "SPI_MSB_FIRST", 1);
    MOD_REG_NUMBER(L, "SPI_ENDIAN_LITTLE", 0);
    MOD_REG_NUMBER(L, "SPI_ENDIAN_BIG", 0);

    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
