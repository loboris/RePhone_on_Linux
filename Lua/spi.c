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
#define VM_SPI_READ_BUFFER_SIZE		8
#define VM_SPI_WRITE_BUFFER_SIZE	8
#define TEMP_BUF_SIZE				16*1024 // must be multiple of 1024

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


VM_DCL_HANDLE g_spi_handle = VM_DCL_HANDLE_INVALID;
t_spi_params g_spi;

// SPI data transfer buffers
static VM_READ_BUFFER* g_spi_read_data;
static VM_WRITE_BUFFER* g_spi_write_data;
// SPI write&read data transfer buffer
static vm_dcl_spi_control_read_write_t g_spi_write_and_read = {NULL,0,NULL,0};

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);



//------------------
int _setup_spi(void)
{
	VM_DCL_HANDLE gpio_handle;
    int result = -1;

	if (VM_DCL_HANDLE_INVALID == g_spi_handle) {
		gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_MOSI_PIN_NAME);
	    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

		gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_SCLK_PIN_NAME);
	    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

		gpio_handle = vm_dcl_open(VM_DCL_GPIO, SPI_MISO_PIN_NAME);
	    if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
		vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
		vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

		g_spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1, 0);
		if (VM_DCL_HANDLE_INVALID == g_spi_handle) {
			vm_log_error("Error opening spi");
			goto exit;
		}
		g_spi_read_data = (VM_READ_BUFFER*)vm_malloc_dma(sizeof(VM_READ_BUFFER));
		if (g_spi_read_data == NULL) {
			vm_dcl_close(g_spi_handle);
			g_spi_handle = VM_DCL_HANDLE_INVALID;
			vm_log_error("Error allocating read buffer");
			goto exit;
		}
		else {
			g_spi_write_data = (VM_WRITE_BUFFER*)vm_malloc_dma(sizeof(VM_WRITE_BUFFER));
			if (g_spi_write_data == NULL) {
				vm_free(g_spi_read_data);
				vm_dcl_close(g_spi_handle);
				g_spi_handle = VM_DCL_HANDLE_INVALID;
				vm_log_error("Error allocating write buffer");
				goto exit;
			}
		}
	}

	result = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&g_spi.conf_data);

	if (VM_DCL_STATUS_OK != result) {
		vm_log_error("set spi config parameters failed: %d", result);
	}
	else {
		vm_dcl_spi_mode_t spi_data_mode;
		//spi_data_mode.parameter = 0;
		spi_data_mode.mode = VM_DCL_SPI_MODE_PAUSE;
		spi_data_mode.is_enabled = g_spi.pmd;
		vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_MODE,(void *)&spi_data_mode);

		spi_data_mode.mode = VM_DCL_SPI_MODE_DEASSERT;
		spi_data_mode.is_enabled = g_spi.pmd ^ 0x01;
		vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_MODE,(void *)&spi_data_mode);
	}

exit:
    return result;
}

//=================================
static int _spi_setup(lua_State* L)
{
	// set default config
	g_spi.conf_data.clock_high_time = 4;
	g_spi.conf_data.clock_low_time = 4;
	g_spi.conf_data.cs_hold_time = 1;
	g_spi.conf_data.cs_idle_time = 1;
	g_spi.conf_data.cs_setup_time= 1;
	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
	g_spi.conf_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	g_spi.conf_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	g_spi.conf_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
	g_spi.conf_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;

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
		            	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
		            	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
		                break;
		            case 1:
		            	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
		            	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_1;
		                break;
		            case 2:
		            	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_1;
		            	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
		                break;
		            case 3:
		            	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_1;
		            	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_1;
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
		    	g_spi.conf_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
		    	g_spi.conf_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
		    }
		    else if (param == 1) {
		    	g_spi.conf_data.rx_endian = VM_DCL_SPI_ENDIAN_BIG;
		    	g_spi.conf_data.tx_endian = VM_DCL_SPI_ENDIAN_BIG;
		    }
		  }
		}

		lua_getfield(L, 1, "msb");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if (param == 0) {
		    	g_spi.conf_data.rx_msbf = VM_DCL_SPI_LSB_FIRST;
		    	g_spi.conf_data.tx_msbf = VM_DCL_SPI_LSB_FIRST;
		    }
		    else if (param == 1) {
		    	g_spi.conf_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
		    	g_spi.conf_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
		    }
		  }
		}

		lua_getfield(L, 1, "clk");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if ((param >= 0) && (param <= 255)) {
		    	g_spi.conf_data.clock_high_time = param;
		    	g_spi.conf_data.clock_low_time = param;
				vm_log_debug("SPI clock is %d kHz", 32000/(param+1));
		    }
		  }
		}

		lua_getfield(L, 1, "pmd");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if ((param == 0) || (param == 1)) g_spi.pmd = param;
		  }
		}

		lua_getfield(L, 1, "speed");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    param = luaL_checkinteger( L, -1 );
		    if (param < 125) param = 125;
		    if (param > 16000) param = 16000;
		    param = (32000 / param) - 1;
		    if (param > 255) param = 255;
		    g_spi.conf_data.clock_high_time = param;
		    g_spi.conf_data.clock_low_time = param;
			vm_log_debug("SPI clock is %d kHz", 32000/(param+1));
		  }
		}

		lua_getfield(L, 1, "cs");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
			if (g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
				vm_dcl_close(g_spi.g_spi_cs_handle);
				g_spi.g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
			}
		    param = luaL_checkinteger( L, -1 );
		    gpio_get_handle(param, &g_spi.g_spi_cs_handle);
		    if (g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
		        vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
		        vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
		        vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		        g_spi.cs_pin_mask = (1, (1 << param));
		    }
		    else {
		    	g_spi.g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
				vm_log_error("error initializing CS on pin #%d", param);
		    }
		  }
		}
		else {
			vm_log_debug("CS pin not set");
		}

		lua_getfield(L, 1, "dc");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
			if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
				vm_dcl_close(g_spi.g_spi_dc_handle);
				g_spi.g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
			}
		    param = luaL_checkinteger( L, -1 );
		    gpio_get_handle(param, &g_spi.g_spi_dc_handle);
		    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
		        vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
		        vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
		        vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		        g_spi.dc_pin_mask = (1, (1 << param));
		    }
		    else {
		    	g_spi.g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
				vm_log_error("error initializing DC on pin #%d", param);
		    }
		  }
		}
		else {
			vm_log_debug("DC pin not set");
		}
	}

	int result = _setup_spi();

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
	remote_CCall(L, &_spi_setup);
	return g_shell_result;
}

//==================================
static int _spi_deinit(lua_State* L)
{
	if (g_spi_handle == VM_DCL_HANDLE_INVALID) {
		vm_log_debug("SPI not setup");
	}
	else {
		vm_dcl_close(g_spi_handle);
		g_spi_handle = VM_DCL_HANDLE_INVALID;
		vm_free(g_spi_read_data);
		vm_free(g_spi_write_data);
	}

	if (g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi.g_spi_cs_handle);
    	g_spi.g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
    }
    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(g_spi.g_spi_dc_handle);
    	g_spi.g_spi_dc_handle = VM_DCL_HANDLE_INVALID;
    }
    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=================================
static int spi_deinit(lua_State* L)
{
	remote_CCall(L, &_spi_deinit);
	return g_shell_result;
}

//----------------------------------------
int _LcdSpiTransfer(uint8_t *buf, int len)
{
  if ((g_spi.g_spi_dc_handle == VM_DCL_HANDLE_INVALID) || (g_spi.g_spi_cs_handle == VM_DCL_HANDLE_INVALID)) {
	  vm_log_error("SPI not setup");
	  return -1;
  }

  VM_DCL_STATUS status = 0;
  int count = len;
  int bptr = 0;
  uint16_t ndata;
  uint32_t rep,i;
  VM_DCL_BUFFER_LENGTH bcount,datalen;
  volatile uint32_t *reg;
  VMUINT8 *temp_buf;

  temp_buf = vm_malloc_dma(TEMP_BUF_SIZE);
  if (temp_buf == NULL) {
	  vm_log_error("buffer allocation error");
	  return -1;
  }

  while (count > 0) {
	// *** New command ***
	if (buf[bptr] == 0) break;

	// --- for command DC=0 ---
	//vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20308);  // set low
	*reg = g_spi.dc_pin_mask;
	// --- Activate chip select ---
   	//vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20308);  // set low
	*reg = g_spi.cs_pin_mask;

	count--;
	// --- send command byte ---
	temp_buf[0] = buf[bptr];
	bcount = 1;
	datalen = 1;
	status = vm_dcl_write(g_spi_handle,(VM_DCL_BUFFER*)temp_buf, datalen, &bcount, 0);

	// --- Deactivate chip select -------------------
   	//vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	//reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
	//*reg = g_spi.cs_pin_mask;

    if (VM_DCL_STATUS_OK != status) {
		vm_log_error("lcd_transfer command error: %d", status);
		// --- Deactivate chip select -------------------
	   	//vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
		*reg = g_spi.cs_pin_mask;
		break;
	}
	// --- for data DC=1 ---
   	//vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
	*reg = g_spi.dc_pin_mask;

    //printf("Cmd: %02x, ", buf[bptr]);
	// get ndata & rep
    bptr++;  // point to ndata
	ndata = (uint16_t)(buf[bptr++] << 8);
	ndata += (uint16_t)(buf[bptr++]);
	rep = (uint32_t)(buf[bptr++] << 24);
	rep += (uint32_t)(buf[bptr++] << 16);
	rep += (uint32_t)(buf[bptr++] << 8);
	rep += (uint32_t)(buf[bptr++]);
	count -= 6;
	/*if ((ndata & 0x8000)) {
		// use rep as pointer to data
		ndata &= 0x7FFF;
		uint8_t *pdata = (uint8_t *)rep;
		memcpy(temp_buf, pdata, ndata);
		rep = 1;
		//printf("buffer send %08X, %d\n", pdata, ndata);
	}*/
	//printf("n: %d, rep: %d\n", ndata, rep);

	if ((count > 0) && (ndata > 0)) {
	  // --- Activate chip select --------------------
 	  //vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
 	  //reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20308);  // set low
 	  //*reg = g_spi.cs_pin_mask;

	  status = VM_DCL_STATUS_OK;
	  int bsz = 0;
	  int bufptr = 0;
	  for (i=0; i<rep; i++) {
		if ((bsz+ndata) > TEMP_BUF_SIZE) {
			// buffer (almost) full, send buffer
			while (bsz > 0) {
				if (bsz > 1024) {
					bcount = TEMP_BUF_SIZE / 1024;
					datalen = 1024;
				}
				else {
					bcount = 1;
					datalen = bsz;
				}
				//printf("write: ptr=%d, len=%d, cnt=%d\n", bufptr, datalen, bcount);
				status = vm_dcl_write(g_spi_handle,(VM_DCL_BUFFER*)(temp_buf+bufptr), datalen, &bcount, 0);
				bsz -= datalen*bcount;
				bufptr += datalen*bcount;
				if (VM_DCL_STATUS_OK != status) break;
			}
			bsz = 0;
			bufptr = 0;
			if (VM_DCL_STATUS_OK != status) break;
		}
		// add repeat data to buffer
		memcpy(temp_buf+bsz, buf+bptr, ndata);
		bsz += ndata;
	  }
	  // send rest of the buffer
	  if ((VM_DCL_STATUS_OK == status) && (bsz > 0)) {
			int bufptr = 0;
			while (bsz > 0) {
				if (bsz > 1024) {
					bcount = bsz / 1024;
					datalen = 1024;
				}
				else {
					bcount = 1;
					datalen = bsz;
				}
				//printf("final write: ptr=%d, len=%d, cnt=%d\n", bufptr, datalen, bcount);
				status = vm_dcl_write(g_spi_handle,(VM_DCL_BUFFER*)(temp_buf+bufptr), datalen, &bcount, 0);
				bsz -= datalen*bcount;
				bufptr += datalen*bcount;
				if (VM_DCL_STATUS_OK != status) break;
			}
	  }

	  if (VM_DCL_STATUS_OK != status) {
		vm_log_error("lcd_transfer data error: %d", status);
		//vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
		*reg = g_spi.cs_pin_mask;
		break;
	  }
	  bptr += ndata;  // next command
	  count -= ndata;
	  // --- Deactivate chip select -------------------
	  //vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	  reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
	  *reg = g_spi.cs_pin_mask;
	}
	else {
		// --- Deactivate chip select -------------------
		//vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
		*reg = g_spi.cs_pin_mask;
	}
  }

  vm_free(temp_buf);

  return status;
}

//-----------------------------------------------------------------------------------------------
static int _spi_getarg(lua_State* L, t_spi_arg *arg, VMUINT8 *temp_buf, int argstart, int argend)
{
	int argn;
    const char* pdata;
    size_t datalen;
    int numdata;
    int bptr = 0;
    int i = 0;

    arg->out_type = 0;
    arg->deact_cs = 1;
    arg->send_as_cmd = 0;
    arg->read_while_write = 0;
    arg->rwb = 0xFF;

    for (argn = argstart; argn < argend; argn++) {
        // lua_isnumber() would silently convert a string of digits to an integer
        // whereas here strings are handled separately.
        if (lua_type(L, argn) == LUA_TNUMBER) {
            numdata = (int)luaL_checkinteger(L, argn);
            if ((numdata >= 0) && (numdata <= 255) && (bptr < TEMP_BUF_SIZE)) temp_buf[bptr++] = numdata;
        }
        else if(lua_istable(L, argn)) {
            datalen = lua_objlen(L, argn);
            for (i = 0; i < datalen; i++) {
                lua_rawgeti(L, argn, i + 1);
                numdata = (int)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                if ((numdata >= 0) && (numdata <= 255) && (bptr < TEMP_BUF_SIZE)) temp_buf[bptr++] = numdata;
            }
        }
        else if (lua_isstring(L, argn)) {
        	int dta = 1;
            pdata = luaL_checklstring(L, argn, &datalen);
            if (datalen == 2) {
            	if (strstr(pdata, "*h") != NULL) {
            		arg->out_type = 1;
            		dta = 0;
            	}
            	else if (strstr(pdata, "*t") != NULL) {
            		arg->out_type = 2;
            		dta = 0;
            	}
            	else if (strstr(pdata, "*c") != NULL) {
            		arg->send_as_cmd = 1;
            		dta = 0;
            	}
            	else if (strstr(pdata, "*w") != NULL) {
            		arg->read_while_write = 1;
            		dta = 0;
            	}
            	else if (strstr(pdata, "*s") != NULL) {
            		arg->deact_cs = 0;
            		dta = 0;
            	}
            }
            if (dta) {
				for (i = 0; i < datalen; i++) {
		            if (bptr < TEMP_BUF_SIZE) temp_buf[bptr++] = pdata[i];
				}
            }
        }
    }
    return bptr;
}

//--------------------------------------------
int _spi_wrrd_data(VMUINT8 *temp_buf, int len)
{
	memset(g_spi_read_data->read_buffer, 0, VM_SPI_WRITE_BUFFER_SIZE);
	memcpy(g_spi_write_data->write_buffer, temp_buf, len);

	g_spi_write_and_read.read_data_ptr = g_spi_read_data->read_buffer;
	g_spi_write_and_read.read_data_length = len;

	g_spi_write_and_read.write_data_ptr = g_spi_write_data->write_buffer;
	g_spi_write_and_read.write_data_length = len;

	int status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&g_spi_write_and_read);
	memcpy(temp_buf, g_spi_read_data->read_buffer, len);

	/*int status = VM_DCL_STATUS_OK;
	// send data
	for (int i=0; i<len; i++) {
		g_spi_write_data->write_buffer[0] = temp_buf[i];
		status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&g_spi_write_and_read);
		if (VM_DCL_STATUS_OK == status) temp_buf[i] = g_spi_read_data->read_buffer[0];
		else break;
	}*/
	return status;
}

//----------------------------------------------------------------------
int _spi_writedata(VMUINT8 *temp_buf, t_spi_arg *arg, int bptr, int len)
{
	memset(g_spi_read_data->read_buffer, 0, VM_SPI_WRITE_BUFFER_SIZE);
	memset(g_spi_write_data->write_buffer, 0xFF, VM_SPI_WRITE_BUFFER_SIZE);
	g_spi_write_and_read.read_data_ptr = g_spi_read_data->read_buffer;
	g_spi_write_and_read.read_data_length = 1;
	g_spi_write_and_read.write_data_ptr = g_spi_write_data->write_buffer;
	g_spi_write_and_read.write_data_length = 1;

	int status = VM_DCL_STATUS_OK;
	// send data
	for (int i=0; i<len; i++) {
		if ((i == 0) && (arg->send_as_cmd)) {
			// DC = 0
		    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
		   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		    }
		}
		g_spi_write_data->write_buffer[0] = temp_buf[i+bptr];
		status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&g_spi_write_and_read);
		if ((i == 0) && (arg->send_as_cmd)) {
			// DC = 1
		    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
		   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
		    }
		}
		if (VM_DCL_STATUS_OK == status) temp_buf[i+bptr] = g_spi_read_data->read_buffer[0];
		else break;
	}
	return status;
}

//---------------------------------------------------------------------
int _spi_readdata(VMUINT8 *temp_buf, t_spi_arg *arg, int bptr, int len)
{
	g_spi_write_and_read.read_data_ptr = g_spi_read_data->read_buffer;
	g_spi_write_and_read.read_data_length = 1;
	g_spi_write_and_read.write_data_ptr = g_spi_write_data->write_buffer;
	g_spi_write_and_read.write_data_length = 1;

	int status = VM_DCL_STATUS_OK;
	// read data
	for (int i=0; i<len; i++) {
		g_spi_write_data->write_buffer[0] = arg->rwb;
		status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&g_spi_write_and_read);
		if (VM_DCL_STATUS_OK == status) temp_buf[bptr+i] = g_spi_read_data->read_buffer[0];
		else break;
	}
	return status;
}

// Lua: wrote = spi.send(data1, [data2], ..., [datan] )
// data can be either a string, a table or an 8-bit number
//================================
static int _spi_send(lua_State* L)
{
    VMUINT8 *temp_buf;

    temp_buf = vm_malloc_dma(TEMP_BUF_SIZE);
    if (temp_buf == NULL) {
        g_shell_result = -2;
    	vm_signal_post(g_shell_signal);
    	return 0;
    }

    VM_DCL_BUFFER_LENGTH bcount,datalen;
    VM_DCL_STATUS status = VM_DCL_STATUS_OK;;
    t_spi_arg arg;

	// get arguments
    int wsize = _spi_getarg(L, &arg, temp_buf, 1, (lua_gettop(L)+1));
    if (wsize > TEMP_BUF_SIZE) {
    	vm_free(temp_buf);
        g_shell_result = -3;
    	vm_signal_post(g_shell_signal);
    	return 0;
    }

    int bsz = wsize;
    int wrt = 0;

    if (wsize > 0) {
    	// activate CS
   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
   	    // set DC=1
   	    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
   	   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
   	    }

   	    if ((arg.send_as_cmd) && (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID)) {
   	    	// send first byte as command
   	    	// set DC=0
   	   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
   			bcount = 1;
   			datalen = 1;
   			status = vm_dcl_write(g_spi_handle,(VM_DCL_BUFFER*)(temp_buf+wrt), datalen, &bcount, 0);

   			bsz--;
   			wrt++;
   		    // set DC=1
   	   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
   	    }

   	    if (VM_DCL_STATUS_OK == status) {
   			// send buffer
   			while (bsz > 0) {
   				if (bsz > 1024) {
   					bcount = TEMP_BUF_SIZE / 1024;
   					datalen = 1024;
   				}
   				else {
   					bcount = 1;
   					datalen = bsz;
   				}
   				status = vm_dcl_write(g_spi_handle,(VM_DCL_BUFFER*)(temp_buf+wrt), datalen, &bcount, 0);
   				bsz -= datalen*bcount;
   				wrt += datalen*bcount;
   				if (VM_DCL_STATUS_OK != status) break;
   			}
   		}

   		// deactivate CS
   	    if ((g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) && (arg.deact_cs)) {
   	   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
   	    }
    }

    // free buffer
	vm_free(temp_buf);

	if (VM_DCL_STATUS_OK != status) g_shell_result = -2;
	else g_shell_result = wsize;

	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int spi_send(lua_State* L)
{
	if (g_spi_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "SPI not setup");

	g_shell_result = -1;
    if (lua_gettop(L) > 0) remote_CCall(L, &_spi_send);

    lua_pushinteger(L, g_shell_result);
    return 1;
}

// Lua: read = spi.recv(size)
//================================
static int _spi_recv(lua_State* L)
{
    VMUINT8 *temp_buf;

    temp_buf = vm_malloc(TEMP_BUF_SIZE);
    if (temp_buf == NULL) {
        lua_pushinteger(L, -1);
        g_shell_result = 1;
    	vm_signal_post(g_shell_signal);
    	return 0;
    }

    t_spi_arg arg;
    uint32_t size = luaL_checkinteger(L, 1);
    luaL_Buffer b;
    VM_DCL_STATUS status;

    if (size < 1) size = 1;
    if (size > TEMP_BUF_SIZE) size = TEMP_BUF_SIZE;

    // check for format argument
    int rsize = _spi_getarg(L, &arg, temp_buf, 2, (lua_gettop(L)+1));

	// DC = 1
    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }
	// activate CS
    if (g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

    status = _spi_readdata(temp_buf, &arg, 0, size);

	// deactivate CS
    if ((g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) && (arg.deact_cs)){
   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }

    // Copy to Lua buffer
    if (VM_DCL_STATUS_OK == status) {
	    lua_pushinteger(L, size);
        if (arg.out_type < 2) {
            char hbuf[4];
        	luaL_buffinit(L, &b);
            for (int i = 0; i < size; i++) {
            	if (arg.out_type == 0) luaL_addchar(&b, temp_buf[i]);
            	else {
            		sprintf(hbuf, "%02x;", temp_buf[i]);
            		luaL_addstring(&b, hbuf);
            	}
    		}
    	    luaL_pushresult(&b);
        }
        else {
        	lua_newtable(L);
            for (int i = 0; i < size; i++) {
            	lua_pushinteger( L, temp_buf[i]);
            	lua_rawseti(L,-2, i+1);
            }
        }
        g_shell_result = 2;
    }
    else {
	    lua_pushinteger(L, -2);
        g_shell_result = 1;
    }

    vm_free(temp_buf);

    vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int spi_recv(lua_State* L)
{
	if (g_spi_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "SPI not setup");

	uint32_t size = luaL_checkinteger(L, 1);

    remote_CCall(L, &_spi_recv);
    return g_shell_result;
}

// Lua: read = spi.txrx(data, [data, ...], size)
//================================
static int _spi_txrx(lua_State* L)
{
    VMUINT8 *temp_buf;
    t_spi_arg arg;

    g_shell_result = 1;
    temp_buf = vm_calloc(TEMP_BUF_SIZE);
    if (temp_buf == NULL) {
        lua_pushinteger(L, -1);
    	vm_signal_post(g_shell_signal);
    	return 0;
    }

    VM_DCL_STATUS status;
    luaL_Buffer b;
    int i, bptr;
    size_t size = luaL_checkinteger(L, -1);

	// get arguments
    bptr = _spi_getarg(L, &arg, temp_buf, 1, lua_gettop(L));
    if ((bptr+size) > TEMP_BUF_SIZE) {
    	vm_free(temp_buf);
        lua_pushinteger(L, -2);
    	vm_signal_post(g_shell_signal);
    	return 0;
    }

	// DC = 1
    if (g_spi.g_spi_dc_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi.g_spi_dc_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }
	// activate CS
    if (g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) {
   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

    // write and read data
    status = _spi_writedata(temp_buf, &arg, 0, bptr);
    if (status == VM_DCL_STATUS_OK) status = _spi_readdata(temp_buf, &arg, bptr, size);

	// deactivate CS
    if ((g_spi.g_spi_cs_handle != VM_DCL_HANDLE_INVALID) && (arg.deact_cs)) {
   		vm_dcl_control(g_spi.g_spi_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }

    if (status != VM_DCL_STATUS_OK) lua_pushinteger(L, -3);
    else {
    	int startptr = bptr;
    	if (arg.read_while_write) startptr = 0;

    	lua_pushinteger(L, bptr);
        if (arg.out_type < 2) {
            char hbuf[4];
        	luaL_buffinit(L, &b);
            for (i = startptr; i < (size+bptr); i++) {
            	if (arg.out_type == 0) luaL_addchar(&b, temp_buf[i]);
            	else {
            		sprintf(hbuf, "%02x;", temp_buf[i]);
            		luaL_addstring(&b, hbuf);
            	}
    		}
    	    luaL_pushresult(&b);
        }
        else {
        	lua_newtable(L);
            for (i = startptr; i < (size+bptr); i++) {
            	lua_pushinteger( L, temp_buf[i]);
            	lua_rawseti(L,-2, i+1);
            }
        }
        g_shell_result = 2;
    }

    vm_free(temp_buf);

    vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int spi_txrx(lua_State* L)
{
	if (g_spi_handle == VM_DCL_HANDLE_INVALID) return luaL_error(L, "SPI not setup");

	if (lua_gettop(L) < 2) return luaL_error(L, "invalid number of arguments");

	size_t size = luaL_checkinteger(L, -1);

    remote_CCall(L, &_spi_txrx);
    return g_shell_result;
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
        { LNILKEY, LNILVAL }
};

#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)


LUALIB_API int luaopen_spi(lua_State* L)
{
	g_spi.g_spi_cs_handle = VM_DCL_HANDLE_INVALID;
	g_spi.g_spi_dc_handle = VM_DCL_HANDLE_INVALID;

	g_spi.cs_pin_mask = 0;
	g_spi.dc_pin_mask = 0;

	g_spi.pmd = VM_TRUE;

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
