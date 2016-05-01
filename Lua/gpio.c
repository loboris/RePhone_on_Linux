
#include <string.h>

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "vmfirmware.h"
#include "vmdatetime.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"

#define INPUT             0
#define OUTPUT            1
#define INPUT_PULLUP_HI   2
#define INPUT_PULLUP_LOW  3
#define EINT              4
#define EINT_PULLUP_HI    5
#define EINT_PULLUP_LOW   6

#define REDLED           17
#define GREENLED         15
#define BLUELED          12

#define HIGH              1
#define LOW               0

#define MAX_EINT_PINS     5

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);


typedef struct {
	int                               			pin;
	VM_DCL_HANDLE                     			eint_handle;
	int                               			cb_ref;
    vm_dcl_eint_control_config_t				eint_config;
    vm_dcl_eint_control_sensitivity_t			sens_data;
    vm_dcl_eint_control_hw_debounce_t 			deboun_time;
	vm_dcl_eint_control_auto_change_polarity_t	auto_change;
} gpio_eint_t;

gpio_eint_t gpio_eint_pins[MAX_EINT_PINS];

typedef struct {
	VMUINT8 autounmask;
	VMUINT8 sensit;
	VMUINT8 polar;
	VMUINT8 deboun;
	VMUINT8 deboun_time;
	int autopol;
} gpio_eint_config_t;


//--------------
void _delay_us()
{
	int volatile i = 0;
	while (i < 19) {
		i++;
	}
}

static int g_eintcount = 0;
gpio_eint_config_t eint_config;


// EINT callback, to be invoked when EINT triggers.
//-----------------------------------------------------------------------------------------
static void eint_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
	if (event == VM_DCL_EINT_EVENTS_START) {
		if (g_eintcount > 10000) g_eintcount = 0;
		g_eintcount++;
		int pin = -1;

		for (int i=0; i<MAX_EINT_PINS; i++) {
			if (gpio_eint_pins[i].eint_handle == device_handle) {
				pin = gpio_eint_pins[i].pin;
				break;
			}
		}
		//vm_log_info("pin=%d; eint count = %d", pin, g_eintcount);
	    VM_DCL_HANDLE handle;
	    vm_dcl_gpio_control_level_status_t value;

	    gpio_get_handle(REDLED, &handle);
	    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_OUT, &value);

	    if (value.level_status) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	    else vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	}
}

//------------------------------------
static void _clear_eint_entry(int idx)
{
    memset(&gpio_eint_pins[idx], 0, sizeof(gpio_eint_t));
	gpio_eint_pins[idx].cb_ref = LUA_NOREF;
	gpio_eint_pins[idx].pin = -1;
	gpio_eint_pins[idx].eint_handle = VM_DCL_HANDLE_INVALID;

}

//----------------------------------------------------------
static int getfield (lua_State *L, const char *key, int d) {
  int res;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    res = (int)lua_tointeger(L, -1);
  else {
    if (d < 0) return luaL_error(L, "field " LUA_QS " missing", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}

//----------------------------------------------
int _gpio_eint(int pin, vm_dcl_callback eint_cb)
{
    VM_DCL_HANDLE handle;
    VM_DCL_STATUS status;
    int i, err=0;
    int ei_idx = -1;

    for (i=0; i<MAX_EINT_PINS; i++) {
    	if (gpio_eint_pins[i].pin == pin) {
    	    vm_log_info("pin already assigned");
    		return -1;
    	}
    }
    for (i=0; i<MAX_EINT_PINS; i++) {
    	if (gpio_eint_pins[i].pin < 0) {
    		ei_idx = i;
    		break;
    	}
    }
    if (ei_idx < 0) {
	    vm_log_info("no free EINT pins");
    	return -2;
    }

    status = vm_dcl_config_pin_mode(pin, VM_DCL_PIN_MODE_EINT); /* Sets the pin to EINT mode */

    if (status != VM_DCL_STATUS_OK) {
	    vm_log_info("pin EINT mode error");
    	return -3;
    }

    // Opens and attaches EINT pin
    gpio_eint_pins[ei_idx].eint_handle = vm_dcl_open(VM_DCL_EINT, PIN2EINT(pin));
    if (VM_DCL_HANDLE_INVALID == gpio_eint_pins[ei_idx].eint_handle) {
    	_clear_eint_entry(i);
	    vm_log_info("pin handle error");
        return -4;
    }
    gpio_eint_pins[ei_idx].pin = pin;

    // Before configuring the EINT, we mask it firstly.
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    if (status != VM_DCL_STATUS_OK) {
      err++;
      vm_log_info("VM_DCL_EINT_COMMAND_MASK  = %d", status);
      goto exit;
    }

    // Registers the EINT callback
    status = vm_dcl_register_callback(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_EVENT_TRIGGER, eint_cb, (void*)NULL );
    if (status != VM_DCL_STATUS_OK) {
        err++;
        vm_log_info("VM_DCL_EINT_EVENT_TRIGGER = %d", status);
        goto exit;
    }

    // ==================
    // = Coufigure EINT =
    // ==================

    // Configures edge to trigger */
    gpio_eint_pins[ei_idx].sens_data.sensitivity = eint_config.sensit;
    gpio_eint_pins[ei_idx].eint_config.act_polarity = eint_config.polar;
    // Configure debounce: 1 means enabling the HW debounce; 0 means disabling.
    gpio_eint_pins[ei_idx].eint_config.debounce_enable = eint_config.deboun;
    // Sets the auto unmask
    gpio_eint_pins[ei_idx].eint_config.auto_unmask = eint_config.autounmask;

    // Sets the EINT sensitivity
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_SET_SENSITIVITY, (void*)&gpio_eint_pins[ei_idx].sens_data);
    if (status != VM_DCL_STATUS_OK) {
      err++;
      vm_log_info("VM_DCL_EINT_COMMAND_SET_SENSITIVITY = %d", status);
      goto exit;
    }

    // Sets debounce time
    gpio_eint_pins[ei_idx].deboun_time.debounce_time = eint_config.deboun_time;
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE, (void*)&gpio_eint_pins[ei_idx].deboun_time);
    if (status != VM_DCL_STATUS_OK) {
      err++;
      vm_log_info("VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE = %d", status);
      goto exit;
    }

    // Configure auto change polarity
    gpio_eint_pins[ei_idx].auto_change.auto_change_polarity = eint_config.autopol;
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
	status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle ,VM_DCL_EINT_COMMAND_SET_AUTO_CHANGE_POLARITY,(void *)&gpio_eint_pins[ei_idx].auto_change);
	if(status != VM_DCL_STATUS_OK) {
        err++;
		vm_log_info("VM_DCL_EINT_COMMAND_CONFIG change = %d", status);
        goto exit;
	}

	// *** Make sure to call this API at the end as the EINT will be unmasked in this statement.
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_CONFIG, (void*)&gpio_eint_pins[ei_idx].eint_config);
    if (status != VM_DCL_STATUS_OK) {
      err++;
      vm_log_info("VM_DCL_EINT_COMMAND_CONFIG = %d", status);
      goto exit;
    }

    // ** Unmask EINT
    status = vm_dcl_control(gpio_eint_pins[ei_idx].eint_handle, VM_DCL_EINT_COMMAND_UNMASK, NULL);
    if (status != VM_DCL_STATUS_OK) {
      err++;
      vm_log_info("VM_DCL_EINT_COMMAND_UNMASK  = %d", status);
      goto exit;
    }

exit:
    if (err) {
    	err = (err+4) * -1;
        vm_dcl_close(gpio_eint_pins[ei_idx].eint_handle);
    	_clear_eint_entry(ei_idx);
    }
    return ei_idx;
}

//================================
static int gpio_eint(lua_State* L)
{
    VM_DCL_HANDLE handle;
    VM_DCL_STATUS status;
    int i, err=0;
    int ei_idx = -1;

    int pin = luaL_checkinteger(L, 1);

    if (lua_istable(L, 2)) {
    	lua_settop(L, 2);  // make sure table is at the top
    	eint_config.autounmask = getfield(L, "autounmask", 0);
    	eint_config.autopol = getfield(L, "autopol", 0);
    	eint_config.sensit = getfield(L, "sensitivity", 0);
    	eint_config.polar = getfield(L, "polarity", 0);
    	eint_config.deboun = getfield(L, "deboun", 0);
    	eint_config.deboun_time = getfield(L, "debountime", 10);
    }
    else {
        eint_config.autounmask = 0;
        eint_config.sensit = 0;
        eint_config.polar = 0;
        eint_config.deboun = 0;
        eint_config.deboun_time = 10;
        eint_config.autopol = 0;
    }

    lua_pushinteger(L, _gpio_eint(pin, (vm_dcl_callback)eint_callback));
    return 1;
}

//=====================================
static int gpio_eint_mask(lua_State* L)
{
    int pin = luaL_checkinteger(L, 1);
    for (int i=0; i<MAX_EINT_PINS; i++) {
    	if (gpio_eint_pins[i].pin == pin) {
    	    int mask = luaL_checkinteger(L, 2);
    	    if (mask != 0) mask = 1;

    	    if (mask) vm_dcl_control(gpio_eint_pins[i].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    	    else vm_dcl_control(gpio_eint_pins[i].eint_handle, VM_DCL_EINT_COMMAND_UNMASK, NULL);
    	    lua_pushinteger(L, mask);
    	}
    }
    return 1;
}

//--------------------------
int _eint_pin_close(int pin)
{
    for (int i=0; i<MAX_EINT_PINS; i++) {
    	if (gpio_eint_pins[i].pin == pin) {
    		if (gpio_eint_pins[i].eint_handle != VM_DCL_HANDLE_INVALID) vm_dcl_close(gpio_eint_pins[i].eint_handle);
        	_clear_eint_entry(i);
            return 0;
    	}
    }
    return -1;
}

//=====================================
static int gpio_eintclose(lua_State* L)
{
    int pin = luaL_checkinteger(L, 1);

	lua_pushinteger(L, _eint_pin_close(pin));
    return 1;
}

//================================
static int gpio_mode(lua_State* L)
{
    VM_DCL_HANDLE handle;
    int pin = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);

    if (gpio_get_handle(pin, &handle) == VM_DCL_HANDLE_INVALID) {
        return luaL_error(L, "invalid pin handle");
    }
    
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    
    if (mode == OUTPUT) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    }
    else if ((mode == INPUT) || (mode == INPUT_PULLUP_HI) || (mode == INPUT_PULLUP_LOW)) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
        if (mode == INPUT) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_DISABLE_PULL, NULL);
        else {
        	vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
        	if (mode == INPUT_PULLUP_HI) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
        	else if (mode == INPUT_PULLUP_LOW) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_PULL_LOW, NULL);
        }
    }

    return 0;
}

//================================
static int gpio_read(lua_State* L)
{
    VM_DCL_HANDLE handle;
    vm_dcl_gpio_control_level_status_t data;
    vm_dcl_gpio_control_direction_t dir;

    int pin = luaL_checkinteger(L, 1);
    
    gpio_get_handle(pin, &handle);

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_DIRECTION, &dir);
    if (dir.direction_status == 1) {
        return luaL_error(L, "pin is output");
    }

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);

    lua_pushnumber(L, data.level_status);

    return 1;
}

//=================================
static int gpio_write(lua_State* L)
{
    VM_DCL_HANDLE handle;
    vm_dcl_gpio_control_direction_t dir;

    int pin = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);

    gpio_get_handle(pin, &handle);

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_DIRECTION, &dir);
    if (dir.direction_status == 0) {
        return luaL_error(L, "pin is input");
    }

    if (value) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    else vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);

    return 0;
}


//==================================
static int gpio_toggle(lua_State* L)
{
    VM_DCL_HANDLE handle;
    vm_dcl_gpio_control_direction_t dir;
    vm_dcl_gpio_control_level_status_t value;

    int pin = luaL_checkinteger(L, 1);

    gpio_get_handle(pin, &handle);

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_DIRECTION, &dir);
    if (dir.direction_status == 0) {
        return luaL_error(L, "pin is input");
    }
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_OUT, &value);

    if (value.level_status) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    else vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

    return 0;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)

#define GLOBAL_NUMBER(l, name, value) \
    lua_pushnumber(L, value);         \
    lua_setglobal(L, name)

const LUA_REG_TYPE gpio_map[] = { { LSTRKEY("mode"), LFUNCVAL(gpio_mode) },
								  { LSTRKEY("eintopen"), LFUNCVAL(gpio_eint) },
								  { LSTRKEY("eintclose"), LFUNCVAL(gpio_eintclose) },
								  { LSTRKEY("eintmask"), LFUNCVAL(gpio_eint_mask) },
                                  { LSTRKEY("read"), LFUNCVAL(gpio_read) },
                                  { LSTRKEY("write"), LFUNCVAL(gpio_write) },
                                  { LSTRKEY("toggle"), LFUNCVAL(gpio_toggle) },
#if LUA_OPTIMIZE_MEMORY > 0
                                  { LSTRKEY("OUTPUT"), LNUMVAL(OUTPUT) },
                                  { LSTRKEY("INPUT"), LNUMVAL(INPUT) },
                                  { LSTRKEY("HIGH"), LNUMVAL(HIGH) },
                                  { LSTRKEY("LOW"), LNUMVAL(LOW) },
                                  { LSTRKEY("INPUT_PULLUP_HI"), LNUMVAL(INPUT_PULLUP_HI) },
                                  { LSTRKEY("INPUT_PULLUP_LOW"), LNUMVAL(INPUT_PULLUP_LOW) },
#endif
                                  { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_gpio(lua_State* L)
{
    for (int i=0; i<MAX_EINT_PINS; i++) {
    	_clear_eint_entry(i);
    }

    lua_register(L, "pinMode", gpio_mode);
    lua_register(L, "pinEintOpen", gpio_eint);
    lua_register(L, "pinEintClose", gpio_eintclose);
    lua_register(L, "pinEintMask", gpio_eint_mask);
    lua_register(L, "digitalRead", gpio_read);
    lua_register(L, "digitalWrite", gpio_write);

    GLOBAL_NUMBER(L, "OUTPUT", OUTPUT);
    GLOBAL_NUMBER(L, "INPUT", INPUT);
    GLOBAL_NUMBER(L, "HIGH", HIGH);
    GLOBAL_NUMBER(L, "LOW", LOW);
    GLOBAL_NUMBER(L, "INPUT_PULLUP_HI", INPUT_PULLUP_HI);
    GLOBAL_NUMBER(L, "INPUT_PULLUP_LOW", INPUT_PULLUP_LOW);
    GLOBAL_NUMBER(L, "EINT", EINT);
    GLOBAL_NUMBER(L, "EINT_PULLUP_HI", EINT_PULLUP_HI);
    GLOBAL_NUMBER(L, "EINT_PULLUP_LOW", EINT_PULLUP_LOW);
    GLOBAL_NUMBER(L, "REDLED", REDLED);
    GLOBAL_NUMBER(L, "GREENLED", GREENLED);
    GLOBAL_NUMBER(L, "BLUELED", BLUELED);

#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0

    luaL_register(L, "gpio", gpio_map);
    // Add constants
    MOD_REG_NUMBER(L, "OUTPUT", OUTPUT);
    MOD_REG_NUMBER(L, "INPUT", INPUT);
    MOD_REG_NUMBER(L, "HIGH", HIGH);
    MOD_REG_NUMBER(L, "LOW", LOW);
    MOD_REG_NUMBER(L, "INPUT_PULLUP_HI", INPUT_PULLUP_HI);
    MOD_REG_NUMBER(L, "INPUT_PULLUP_LOW", INPUT_PULLUP_LOW);
    MOD_REG_NUMBER(L, "EINT", EINT);
    MOD_REG_NUMBER(L, "EINT_PULLUP_HI", EINT_PULLUP_HI);
    MOD_REG_NUMBER(L, "EINT_PULLUP_LOW", EINT_PULLUP_LOW);
    MOD_REG_NUMBER(L, "REDLED", REDLED);
    MOD_REG_NUMBER(L, "GREENLED", GREENLED);
    MOD_REG_NUMBER(L, "BLUELED", BLUELED);
    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
