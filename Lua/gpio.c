
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmfirmware.h"
#include "lua.h"
#include "lauxlib.h"
#include "vmdatetime.h"

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW 0

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);

void _delay_us()
{
	int volatile i = 0;
	while (i < 19) {
		i++;
	}
}



int gpio_mode(lua_State* L)
{
    VM_DCL_HANDLE handle;
    int pin = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);

    if (gpio_get_handle(pin, &handle) == VM_DCL_HANDLE_INVALID) {
        return luaL_error(L, "invalid pin handle");
        //return 0;
    }
    
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    
    if (mode == INPUT) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
    } else if (mode == OUTPUT) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    } else {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    }

    return 0;
}

int gpio_read(lua_State* L)
{
    VM_DCL_HANDLE handle;
    vm_dcl_gpio_control_level_status_t data;
    int pin = luaL_checkinteger(L, 1);
    
    gpio_get_handle(pin, &handle);
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);

    lua_pushnumber(L, data.level_status);

    return 1;
}

int gpio_write(lua_State* L)
{
    VM_DCL_HANDLE handle;
    int pin = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);

    gpio_get_handle(pin, &handle);

	VM_TIME_UST_COUNT start_count = vm_time_ust_get_count();
    for (int i=0; i < 10000; i++) {
    		vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
            vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }
	VM_TIME_UST_COUNT end_count = vm_time_ust_get_count();
	lua_pushinteger(L, vm_time_ust_get_duration(start_count, end_count));

	if (value) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    } else {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

    return 1;
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
                                  { LSTRKEY("read"), LFUNCVAL(gpio_read) },
                                  { LSTRKEY("write"), LFUNCVAL(gpio_write) },
#if LUA_OPTIMIZE_MEMORY > 0
                                  { LSTRKEY("OUTPUT"), LNUMVAL(OUTPUT) },
                                  { LSTRKEY("INPUT"), LNUMVAL(INPUT) },
                                  { LSTRKEY("HIGH"), LNUMVAL(HIGH) },
                                  { LSTRKEY("LOW"), LNUMVAL(LOW) },
                                  { LSTRKEY("INPUT_PULLUP"), LNUMVAL(INPUT_PULLUP) },
#endif
                                  { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_gpio(lua_State* L)
{
    lua_register(L, "pinMode", gpio_mode);
    lua_register(L, "digitalRead", gpio_read);
    lua_register(L, "digitalWrite", gpio_write);

    GLOBAL_NUMBER(L, "OUTPUT", OUTPUT);
    GLOBAL_NUMBER(L, "INPUT", INPUT);
    GLOBAL_NUMBER(L, "HIGH", HIGH);
    GLOBAL_NUMBER(L, "LOW", LOW);
    GLOBAL_NUMBER(L, "INPUT_PULLUP", INPUT_PULLUP);

#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  // #if LUA_OPTIMIZE_MEMORY > 0

    luaL_register(L, "gpio", gpio_map);
    // Add constants
    MOD_REG_NUMBER(L, "OUTPUT", OUTPUT);
    MOD_REG_NUMBER(L, "INPUT", INPUT);
    MOD_REG_NUMBER(L, "HIGH", HIGH);
    MOD_REG_NUMBER(L, "LOW", LOW);
    MOD_REG_NUMBER(L, "INPUT_PULLUP", INPUT_PULLUP);
    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
