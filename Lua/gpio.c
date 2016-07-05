
#include <string.h>

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "vmdcl_pwm.h"
#include "vmdcl_adc.h"
#include "vmfirmware.h"
#include "vmdatetime.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
#include "shell.h"

#define INPUT             0
#define OUTPUT            1
#define INPUT_PULLUP_HI   2
#define INPUT_PULLUP_LOW  3
//#define EINT              4
//#define EINT_PULLUP_HI    5
//#define EINT_PULLUP_LOW   6

#define REDLED           17
#define GREENLED         15
#define BLUELED          12

#define HIGH              1
#define LOW               0

#define MAX_EINT_PINS     8
#define MAX_PWM_PINS      4
#define MAX_ADC_CHAN      4

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);


typedef struct {
	int                               			pin;
	VM_DCL_HANDLE                     			eint_handle;
    vm_dcl_eint_control_config_t				eint_config;
    vm_dcl_eint_control_sensitivity_t			sens_data;
    vm_dcl_eint_control_hw_debounce_t 			deboun_time;
	vm_dcl_eint_control_auto_change_polarity_t	auto_change;
} gpio_eint_t;

typedef struct {
	VMUINT8 autounmask;
	VMUINT8 sensit;
	VMUINT8 polar;
	VMUINT8 deboun;
	VMUINT8 deboun_time;
	int autopol;
} gpio_eint_config_t;

typedef struct {
	VM_DCL_HANDLE	adc_handle;
	int				adc_cb_ref;
	int				repeat;
	int				time;
	VMUINT32		result;
	VMINT32			vresult;
	VMINT8			chan;
	VMINT8			cb_wait;
} gpio_adc_chan_t;

static gpio_eint_t gpio_eint_pins[MAX_EINT_PINS];
static int eint_cb_ref = LUA_NOREF;

static VM_DCL_HANDLE pwm_handles[MAX_PWM_PINS];
static VMUINT32 pwm_clk[MAX_PWM_PINS];

static gpio_adc_chan_t adc_chan[MAX_ADC_CHAN];

gpio_eint_config_t eint_config;

static cb_func_param_adc_t adc_cb_params;
static cb_func_param_eint_t eint_cb_params;


// EINT callback, to be invoked when EINT triggers.
//-----------------------------------------------------------------------------------------
static void eint_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
	if (event == VM_DCL_EINT_EVENTS_START) {
		int i, pin = -1;

		for (i=0; i<MAX_EINT_PINS; i++) {
			if (gpio_eint_pins[i].eint_handle == device_handle) {
				pin = gpio_eint_pins[i].pin;
				break;
			}
		}
		// Get pin state
	    vm_dcl_gpio_control_level_status_t value;
	    vm_dcl_control(device_handle, VM_DCL_GPIO_COMMAND_RETURN_OUT, &value);

	    if (eint_cb_ref != LUA_NOREF) {
	    	// Call Lua eint callback function
			if (eint_cb_params.busy == 0) {
				eint_cb_params.cb_ref = eint_cb_ref;
				eint_cb_params.pin = pin;
				eint_cb_params.state = value.level_status;
				remote_lua_call(CB_FUNC_EINT, &eint_cb_params);
			}
	    }
	}
}

//------------------------------------
static void _clear_eint_entry(int idx)
{
    memset(&gpio_eint_pins[idx], 0, sizeof(gpio_eint_t));
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
    	    vm_log_error("pin already assigned");
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
	    vm_log_error("no free EINT pins");
    	return -2;
    }

    status = vm_dcl_config_pin_mode(pin, VM_DCL_PIN_MODE_EINT); /* Sets the pin to EINT mode */

    if (status != VM_DCL_STATUS_OK) {
	    vm_log_error("pin EINT mode error");
    	return -3;
    }

    // Opens and attaches EINT pin
    gpio_eint_pins[ei_idx].eint_handle = vm_dcl_open(VM_DCL_EINT, PIN2EINT(pin));
    if (VM_DCL_HANDLE_INVALID == gpio_eint_pins[ei_idx].eint_handle) {
    	_clear_eint_entry(i);
	    vm_log_error("pin handle error");
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

//=====================================
static int gpio_eint_open(lua_State* L)
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

//-----------------------------------
static int gpio_eint_on(lua_State* L)
{
	if (eint_cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, eint_cb_ref);
		eint_cb_ref = LUA_NOREF;
	}
    // === Register Lua callback function if given
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		// register eint callback function
		lua_pushvalue(L, -1);
		eint_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	return 0;
}

//=====================================
static int gpio_eint_mask(lua_State* L)
{
	int mask = -1, res = -1;
    int pin = luaL_checkinteger(L, 1);
    for (int i=0; i<MAX_EINT_PINS; i++) {
    	if (gpio_eint_pins[i].pin == pin) {
    	    mask = luaL_checkinteger(L, 2);
    	    if (mask != 0) mask = 1;

    	    if (mask) res =vm_dcl_control(gpio_eint_pins[i].eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    	    else res = vm_dcl_control(gpio_eint_pins[i].eint_handle, VM_DCL_EINT_COMMAND_UNMASK, NULL);
    	    if (res < 0) mask = res;
    	    break;
    	}
    }
    lua_pushinteger(L, mask);
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

//======================================
static int gpio_eint_close(lua_State* L)
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
    else return luaL_error(L, "invalid pin mode");

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
        return luaL_error(L, "cannot read, pin is output");
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
        return luaL_error(L, "cannot write, pin is input");
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
        return luaL_error(L, "cannot toggle, pin is input");
    }
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_RETURN_OUT, &value);

    if (value.level_status) vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    else vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);

    return 0;
}

// ==== PWM ============================================================================

/* Start PWM
 * gpio.pwm_start(pwm_id)
 * pwm_id: 0 -> PWM0 (GPIO13), 1 -> PWM1 (GPIO3), pwm_id: 2 -> PWM0 (GPIO2), 3 -> PWM1 (GPIO19)
 */
//=====================================
static int gpio_pwm_start(lua_State* L)
{
    VM_DCL_PWM_DEVICE dev = VM_DCL_PWM_START;
    int gpio_pin = -1;

    int pin = luaL_checkinteger(L, 1);

    if (pin == 0) gpio_pin = 13;
    else if (pin == 1) gpio_pin = 3;
    else if (pin == 2) gpio_pin = 2;
    else if (pin == 3) gpio_pin = 19;

    //set pin mode as PWM
    int status = vm_dcl_config_pin_mode(gpio_pin,VM_DCL_PIN_MODE_PWM);
    if (status != VM_DCL_STATUS_OK) {
        return luaL_error(L, "pin PWM mode error");
    }
    // get pin's pwm device for the input of vm_dcl_open
    dev = PIN2PWM(gpio_pin);
    pwm_handles[pin] = vm_dcl_open(dev,0);

    int ret = vm_dcl_control(pwm_handles[pin], VM_PWM_CMD_START,0);

    return 0;
}

/* Set PWM clock source
 * gpio.pwm_clock(pwm_id, clksrc, div)
 * pwm_id: 0 -> PWM0 (GPIO13), 1 -> PWM1 (GPIO3), pwm_id: 2 -> PWM0 (GPIO2), 3 -> PWM1 (GPIO19)
 * clksrc: clock source 0 -> 13MHz; 1 -> 32.768 kHz
 *    div:     division 0->1, 1->2, 2->4, 3->8
 * !! Main PWM clock (pwm_clk) is 13000000 / div or 32768 / div !!
 */
//=====================================
static int gpio_pwm_clock(lua_State* L)
{
    vm_dcl_pwm_set_clock_t clock;

    int pin = luaL_checkinteger(L, 1);
    int clk = luaL_checkinteger(L, 2);
    int div = luaL_checkinteger(L, 3);
    pin &=1;
    if (pwm_handles[pin] < 0) {
        return luaL_error(L, "PWM not opened");
    }
    clk &= 1;
    div &= 0x03;
    clock.source_clock = clk;
    clock.source_clock_division = div;
    int ret = vm_dcl_control(pwm_handles[pin],VM_PWM_CMD_SET_CLOCK,(void *)(&clock));

    if (clk == 0) pwm_clk[pin] = 13000000;
    else pwm_clk[pin] = 32768;
    pwm_clk[pin] /= (1 << div);

	return 0;
}

/* Set PWM in count mode
 * gpio.pwm_count(pwm_id, count, treshold)
 * pwm_id: 0 -> PWM0 (GPIO13), 1 -> PWM1 (GPIO3), pwm_id: 2 -> PWM0 (GPIO2), 3 -> PWM1 (GPIO19)
 *     count: the pwm cycle: 0 ~ 8191
 *  treshold: value at which pwm gpio goes to LOW state: 0 ~ count
 *  !! PWM FREQUENCY IS pwm_clk / count !!
 */
//=====================================
static int gpio_pwm_count(lua_State* L)
{
    vm_dcl_pwm_set_counter_threshold_t counter;

    int pin = luaL_checkinteger(L, 1);
    int count = luaL_checkinteger(L, 2);
    int duty = luaL_checkinteger(L, 3);
    pin &=1;
    if (pwm_handles[pin] < 0) {
        return luaL_error(L, "PWM not opened");
    }
    count &= 0x1FFF;
    duty &= 0x1FFF;
    if (duty > count) duty = count;
    counter.counter = count;
    counter.threshold = duty;
    int ret = vm_dcl_control(pwm_handles[pin], VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD,(void *)(&counter));

	return 0;
}

/* Set PWM in frequency mode
 * gpio.pwm_freq(pwm_id, freq, duty)
 * pwm_id: 0 -> PWM0 (GPIO13), 1 -> PWM1 (GPIO3), pwm_id: 2 -> PWM0 (GPIO2), 3 -> PWM1 (GPIO19)
 *   freq: the pwm frequency in Hz: 0 ~ pwm_clk
 *   duty: PWM duty cycle: 0 ~ 100
 */
//====================================
static int gpio_pwm_freq(lua_State* L)
{
    vm_dcl_pwm_config_t config;

    int pin = luaL_checkinteger(L, 1);
    int freq = luaL_checkinteger(L, 2);
    int duty = luaL_checkinteger(L, 3);
    pin &=1;
    if (pwm_handles[pin] < 0) {
        return luaL_error(L, "PWM not opened");
    }
    duty &= 0x007F;
    if (duty > 100) duty = 100;
    if (freq > pwm_clk[pin]) freq = pwm_clk[pin];
    if (freq < 0) freq = 0;
    config.frequency = freq;
    config.duty = duty;
    int ret = vm_dcl_control(pwm_handles[pin], VM_PWM_CMD_CONFIG,(void *)(&config));

	return 0;
}

/* Stop PWM
 * gpio.pwm_stop(pwm_id)
 * pwm_id: 0 -> PWM0 (GPIO13), 1 -> PWM1 (GPIO3)
 */
//====================================
static int gpio_pwm_stop(lua_State* L)
{
    int pin = luaL_checkinteger(L, 1);
    pin &=1;
    if (pwm_handles[pin] < 0) {
        return luaL_error(L, "PWM not opened");
    }
    int ret = vm_dcl_control(pwm_handles[pin],VM_PWM_CMD_STOP, 0);
    pwm_handles[pin] = -1;

	return 0;
}

// ==== ADC ============================================================================

//---------------------------------------------------------------------------------
void adc_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
	int _chan = -1;
	for (int i=0;i<MAX_ADC_CHAN;i++) {
		if (adc_chan[i].adc_handle == device_handle) {
			_chan = i;
			break;
		}
	}
	if (_chan < 0) {
		vm_log_debug("[ADC CB] no handle found");
		return;
	}
	if ((adc_chan[_chan].repeat > 0) && (adc_chan[_chan].repeat < 1000)) adc_chan[_chan].repeat--;

    if ((adc_chan[_chan].adc_cb_ref != LUA_NOREF) || (adc_chan[_chan].cb_wait != 0)) {
		vm_dcl_callback_data_t *data;
		vm_dcl_adc_measure_done_confirm_t * result;
		VMINT status = 0;

		if (parameter != NULL) {
			data = ( vm_dcl_callback_data_t*)parameter;
			result = (vm_dcl_adc_measure_done_confirm_t *)(data->local_parameters);

			if ( result != NULL )
			{
				double *p;
				int *v;
				p =(double*)&(result->value);
				adc_chan[_chan].result = (unsigned int)*p;
				v =(int*)&(result->volt_value);
				adc_chan[_chan].vresult = (int)*v;
			}
		}

		//if (repeat) _stop_adc();

		adc_cb_params.fval = (float)adc_chan[_chan].vresult / 1000000.0;
		if (adc_chan[_chan].adc_cb_ref != LUA_NOREF) {
			// Lua callback function
			if (adc_cb_params.busy == 0) {
				adc_cb_params.busy = 1;
				adc_cb_params.cb_ref = adc_chan[_chan].adc_cb_ref;
				adc_cb_params.ival = adc_chan[_chan].result;
				adc_cb_params.chan = adc_chan[_chan].chan;
				remote_lua_call(CB_FUNC_ADC, &adc_cb_params);
			}
		    //else vm_log_debug("[ADC CB] busy");
		}
		else if (adc_chan[_chan].cb_wait != 0) {
			vm_signal_post(g_shell_signal);
			adc_chan[_chan].cb_wait = 0;
		}
    }
	if (adc_chan[_chan].repeat == 0) {
        vm_dcl_adc_control_send_stop_t stop_data;
        stop_data.owner_id = vm_dcl_get_owner_id();
    	int status = vm_dcl_control(adc_chan[_chan].adc_handle,VM_DCL_ADC_COMMAND_SEND_STOP,(void *)&stop_data);
    	if (status < 0) {
    		vm_log_debug("[ADC CB] stop error %d", status);
    	}
	}
}

/* Configure ADC channel
 * gpio.adc_start(channel, [period, count])
 * channel: 0 -> Battery voltage, 1 -> ADC15 (GPIO1), 2 -> ADC13 (GPIO2), 3 -> GPIO3
 *  period: measurement period in miliseconds
 *   count: how many measurement to take before issuing the result, time between measurements is 'period'
 *   		time between results is 'period' * 'count'
 */
//======================================
static int gpio_adc_config(lua_State* L)
{
    VM_DCL_STATUS status;
    VMINT32 period = 1;
    VMUINT8 count = 1;
    VMUINT adcchan = 0;
    int err = 0;

    // === Get arguments
    VMUINT8 chan = luaL_checkinteger(L, 1);
	if ((chan < 0) || (chan >= MAX_ADC_CHAN)) return luaL_error(L, "not valid adc channel");

    if (chan > 0) {
    	// adc pin #1 or #2
		status = vm_dcl_config_pin_mode(chan, VM_DCL_PIN_MODE_ADC); // Sets the pin to ADC mode
		if (status != VM_DCL_STATUS_OK) {
			err = -1;
			vm_log_error("pin ADC mode error (chan %d)", chan);
			goto exit;
		}
		adcchan = PIN2CHANNEL(chan);
    }

    if (lua_gettop(L) > 1) period = (luaL_checkinteger(L, 2) * 1000) / 4615;
    if (period < 1) period = 1;
    if (lua_gettop(L) > 2) count = luaL_checkinteger(L, 3) & 0x00FF;

    // === Setup channel variables
    adc_chan[chan].cb_wait = 0;
    adc_chan[chan].repeat = 0;
    adc_chan[chan].chan = chan;
	adc_chan[chan].vresult = -999999.9;
	adc_chan[chan].result = -999999;
	adc_chan[chan].time = (period * count) * 10;

    // Check if we already have adc handle
	if (adc_chan[chan].adc_handle >= 0) {
		vm_dcl_adc_control_modify_parameter_t obj_data;
	    // Set measurement period, the unit is in ticks.
	    obj_data.period = period;
	    // Measurement count before issuing the result
	    obj_data.evaluate_count = count;
	    // Modify ADC object
	    status = vm_dcl_control(adc_chan[chan].adc_handle,VM_DCL_ADC_COMMAND_MODIFY_PARAMETER,(void *)&obj_data);
		if (status != VM_DCL_STATUS_OK) {
			err = status;
			vm_log_error("Setup ADC object error %d", status);
			goto exit;
		}
	}
	else {
		adc_chan[chan].adc_handle = vm_dcl_open(VM_DCL_ADC,0);
		if (adc_chan[chan].adc_handle < 0) {
			err = adc_chan[chan].adc_handle;
			vm_log_error("ADC open error %d", err);
			goto exit;
		}

	    vm_dcl_adc_control_create_object_t obj_data;
	    // Configure ADC module driver
	    obj_data.owner_id = vm_dcl_get_owner_id();
	    // Set physical ADC channel which should be measured.
	    obj_data.channel = adcchan;
	    // Set measurement period, the unit is in ticks.
	    obj_data.period = period;
	    // Measurement count before issuing the result
	    obj_data.evaluate_count = count;
	    // Send message to owner module
	    obj_data.send_message_primitive = 1;

	    // Create ADC object
	    status = vm_dcl_control(adc_chan[chan].adc_handle,VM_DCL_ADC_COMMAND_CREATE_OBJECT,(void *)&obj_data);
		if (status != VM_DCL_STATUS_OK) {
			err = status;
			vm_log_error("Setup ADC object error %d", status);
			goto exit;
		}

		// register ADC result callback
		status = vm_dcl_register_callback(adc_chan[chan].adc_handle, VM_DCL_ADC_GET_RESULT ,(vm_dcl_callback)adc_callback, (void *)NULL);
		if (status != VM_DCL_STATUS_OK) {
			err = status;
			vm_log_error("ADC register callback error %d", status);
			goto exit;
		}
	}

exit:
	lua_pushinteger(L, err);
    return 1;
}

/* Stop ADC
 * gpio.adc_stop(channel)
 * channel: 0 -> Battery voltage, 1 -> ADC15 (GPIO1), 2 -> ADC13 (GPIO2), 3 -> GPIO3
 */
//====================================
static int gpio_adc_stop(lua_State* L)
{
    VMUINT8 chan = luaL_checkinteger(L, 1);
	if ((chan < 0) || (chan >= MAX_ADC_CHAN)) return luaL_error(L, "not valid adc channel");

	VM_DCL_STATUS status = 1;
    if (adc_chan[chan].adc_handle >= 0) {
		if (adc_chan[chan].repeat > 0) {
			adc_chan[chan].repeat = 0;
			vm_dcl_adc_control_send_stop_t stop_data;
			stop_data.owner_id = vm_dcl_get_owner_id();
			status = vm_dcl_control(adc_chan[chan].adc_handle,VM_DCL_ADC_COMMAND_SEND_STOP,(void *)&stop_data);
			if (status != VM_DCL_STATUS_OK) {
				vm_log_debug("ADC send stop error %d", status);
			}
		}
		else status = 2;
    }
    lua_pushinteger(L, status);
    return 1;
}

/* Start ADC, return result if no callback function is given
 * gpio.adc_start(channel, [repeat], [cb_func])
 * channel: 0 -> Battery voltage, 1 -> ADC15 (GPIO1), 2 -> ADC13 (GPIO2), 3 -> GPIO3
 *  repeat: >=1000 -> continuous measurement; 1 -> measure only once; >0 -> measure 'repeat' times
 * cb_func: Lua function to call after result is ready
 */
//=====================================
static int gpio_adc_start(lua_State* L)
{
	int repeat = 1;

	VMUINT8 chan = luaL_checkinteger(L, 1);
	if ((chan < 0) || (chan >= MAX_ADC_CHAN)) return luaL_error(L, "not valid adc channel");

	if (adc_chan[chan].adc_handle < 0) return luaL_error(L, "adc channel not configured");

	if (lua_isnumber(L, 2)) {
		repeat = luaL_checkinteger(L, 2);
	}

    // === Check callback function
	if (adc_chan[chan].adc_cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, adc_chan[chan].adc_cb_ref);
		adc_chan[chan].adc_cb_ref = LUA_NOREF;
	}

	// === Stop adc if running
	VM_DCL_STATUS status = 0;
    if (adc_chan[chan].adc_handle >= 0) {
		if (adc_chan[chan].repeat > 0) {
			adc_chan[chan].repeat = 0;
			vm_dcl_adc_control_send_stop_t stop_data;
			stop_data.owner_id = vm_dcl_get_owner_id();
			status = vm_dcl_control(adc_chan[chan].adc_handle,VM_DCL_ADC_COMMAND_SEND_STOP,(void *)&stop_data);
			if (status != VM_DCL_STATUS_OK) {
				vm_log_debug("ADC send stop error %d", status);
			}
		}
    }

    // === Register Lua callback function if given as the last argument
	if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
		// register adc callback function
		lua_pushvalue(L, -1);
		adc_chan[chan].adc_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

    if (repeat < 1) repeat = 1;
    if (repeat > 1000) repeat = 1000;
    if (adc_chan[chan].adc_cb_ref == LUA_NOREF) repeat = 1;

    // === Start ADC
    if (adc_chan[chan].adc_cb_ref == LUA_NOREF) adc_chan[chan].cb_wait = 1;
    else adc_chan[chan].cb_wait = 0;
	adc_chan[chan].vresult = -999999.9;
	adc_chan[chan].result = -999999;
	adc_chan[chan].repeat = repeat;

	vm_dcl_adc_control_send_start_t start_data;
	start_data.owner_id = vm_dcl_get_owner_id();
	status = vm_dcl_control(adc_chan[chan].adc_handle,VM_DCL_ADC_COMMAND_SEND_START,(void *)&start_data);
	if (status != VM_DCL_STATUS_OK) {
		vm_log_error("ADC send start error %d", status);
	}
	else {
		if (adc_chan[chan].adc_cb_ref != LUA_NOREF) {
			lua_pushinteger(L, status);
		}
		else {
			// wait for result
			if (vm_signal_timed_wait(g_shell_signal, adc_chan[chan].time * 2) != 0) {
				vm_log_debug("No adc result");
				lua_pushinteger(L, -9);
			}
			else lua_pushnumber(L, adc_cb_params.fval);
			adc_chan[chan].cb_wait = 0;
			adc_chan[chan].repeat = 0;
		}

	}
    return 1;
}

static unsigned int ws2812_data[32];
static unsigned int ws2812_dly = 10;

//-------------------------------------------
static void ws2812_drive(int count, int bpin)
{
	int k,n,m;
	volatile uint32_t *reg;

	VMUINT32 imask = vm_irq_mask();
    for (k=0;k<count;k++) {
		for (n=0;n<24;n++) {
			if (ws2812_data[k] & 0x00800000) {
				// bit 1
				reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
				m = 0;
				while (m < ws2812_dly) {
					*reg = bpin;
					m++;
				}
				reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20308);  // set low
				m = 0;
				while (m < ws2812_dly/2) {
					*reg = bpin;
					m++;
				}
			}
			else {
				// bit 0
				reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20304);  // set high
				m = 0;
				while (m < ws2812_dly/2) {
					*reg = bpin;
					m++;
				}
				reg = (uint32_t *)(REG_BASE_ADDRESS + 0x20308);  // set low
				m = 0;
				while (m < ws2812_dly) {
					*reg = bpin;
					m++;
				}
			}
			ws2812_data[k] <<= 1;
		}
    }
	vm_irq_restore(imask);
}

//=============================
static int ws2812(lua_State* L)
{
	int count = 1;
	int nled = 1;
	unsigned int rgb = 0;
	unsigned int default_color = 0;

    unsigned int pin = luaL_checkinteger(L, 1);

    if (lua_isnumber(L, 3)) {
    	nled = luaL_checkinteger(L, 3);
    	if (nled > 32) nled = 32;
    }
    if (lua_isnumber(L, 4)) {
    	rgb = luaL_checkinteger(L, 4);
    	default_color = ((rgb & 0x0000FF00) << 8) + ((rgb & 0x00FF0000) >> 8) + (rgb & 0x000000FF);
    }

	// Set pin to output
    VM_DCL_HANDLE handle;
    if (gpio_get_handle(pin, &handle) == VM_DCL_HANDLE_INVALID) {
        return luaL_error(L, "invalid pin handle");
    }

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);


    // Turn off
	ws2812_data[0] = 0;
    ws2812_drive(1, (1 << pin));
    vm_thread_sleep(1);

    // Get data
    if (lua_istable(L, 2)) {
        int datalen = lua_objlen(L, 2);
        count = 0;
        for (int i = 0; i < datalen; i++) {
            lua_rawgeti(L, 2, i + 1);
            if (lua_isnumber(L, -1)) {
				rgb = (int)luaL_checkinteger(L, -1);
				if (count < 32) {
					ws2812_data[count] = ((rgb & 0x0000FF00) << 8) + ((rgb & 0x00FF0000) >> 8) + (rgb & 0x000000FF);
					count++;
				}
            }
            lua_pop(L, 1);
        }
        if (count == 0) {
        	count = 1;
        	ws2812_data[0] = 0;
        }
    }
    else {
    	rgb = luaL_checkinteger(L, 2);
    	// convert RGB -> GRB
    	ws2812_data[0] = ((rgb & 0x0000FF00) << 8) + ((rgb & 0x00FF0000) >> 8) + (rgb & 0x000000FF);
    }

    if (count < nled) {
    	for (int i=count;i<nled;i++) {
    		ws2812_data[i] = default_color;
    	}
    	count = nled;
    }

    ws2812_drive(count, (1 << pin));
    vm_thread_sleep(1);

	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//================================
static int ws2812dly(lua_State* L)
{
	ws2812_dly = luaL_checkinteger(L, 1);
    if ((ws2812_dly < 5) || (ws2812_dly > 50)) ws2812_dly = 10;

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

const LUA_REG_TYPE gpio_map[] = {
		{ LSTRKEY("mode"), LFUNCVAL(gpio_mode) },
		{ LSTRKEY("read"), LFUNCVAL(gpio_read) },
		{ LSTRKEY("write"), LFUNCVAL(gpio_write) },
		{ LSTRKEY("toggle"), LFUNCVAL(gpio_toggle) },
		{ LSTRKEY("eint_open"), LFUNCVAL(gpio_eint_open) },
		{ LSTRKEY("eint_close"), LFUNCVAL(gpio_eint_close) },
		{ LSTRKEY("eint_mask"), LFUNCVAL(gpio_eint_mask) },
		{ LSTRKEY("eint_on"), LFUNCVAL(gpio_eint_on) },
		{ LSTRKEY("pwm_start"), LFUNCVAL(gpio_pwm_start) },
		{ LSTRKEY("pwm_stop"), LFUNCVAL(gpio_pwm_stop) },
		{ LSTRKEY("pwm_freq"), LFUNCVAL(gpio_pwm_freq) },
		{ LSTRKEY("pwm_count"), LFUNCVAL(gpio_pwm_count) },
		{ LSTRKEY("pwm_clock"), LFUNCVAL(gpio_pwm_clock) },
		{ LSTRKEY("adc_start"), LFUNCVAL(gpio_adc_start) },
		{ LSTRKEY("adc_stop"), LFUNCVAL(gpio_adc_stop) },
		{ LSTRKEY("adc_config"), LFUNCVAL(gpio_adc_config) },
		{ LSTRKEY("ws2812"), LFUNCVAL(ws2812) },
		{ LSTRKEY("ws2812dly"), LFUNCVAL(ws2812dly)
	},
#if LUA_OPTIMIZE_MEMORY > 0
		{ LSTRKEY("OUTPUT"), LNUMVAL(OUTPUT) },
		{ LSTRKEY("INPUT"), LNUMVAL(INPUT) },
		{ LSTRKEY("HIGH"), LNUMVAL(HIGH) },
		{ LSTRKEY("LOW"), LNUMVAL(LOW) },
		{ LSTRKEY("INPUT_PULLUP_HI"), LNUMVAL(INPUT_PULLUP_HI) },
		{ LSTRKEY("INPUT_PULLUP_LOW"), LNUMVAL(INPUT_PULLUP_LOW) },
#endif
		{ LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_gpio(lua_State* L)
{
    for (int i=0; i<MAX_EINT_PINS; i++) {
    	_clear_eint_entry(i);
    }
    for (int i=0; i<MAX_PWM_PINS; i++) {
    	pwm_handles[i] = -1;
    	pwm_clk[i] = -1;
    }
    for (int i=0; i<MAX_ADC_CHAN; i++) {
    	adc_chan[i].adc_handle = -1;
    	adc_chan[i].adc_cb_ref = LUA_NOREF;
    	adc_chan[i].cb_wait = 0;
    	adc_chan[i].chan = 0;
    	adc_chan[i].repeat = 0;
    	adc_chan[i].time = 100;
    }
    adc_cb_params.cb_ref = LUA_NOREF;

    lua_register(L, "pinMode", gpio_mode);
    lua_register(L, "digitalRead", gpio_read);
    lua_register(L, "digitalWrite", gpio_write);
    lua_register(L, "digitalToggle", gpio_toggle);

    GLOBAL_NUMBER(L, "OUTPUT", OUTPUT);
    GLOBAL_NUMBER(L, "INPUT", INPUT);
    GLOBAL_NUMBER(L, "HIGH", HIGH);
    GLOBAL_NUMBER(L, "LOW", LOW);
    GLOBAL_NUMBER(L, "INPUT_PULLUP", INPUT_PULLUP_HI);
    GLOBAL_NUMBER(L, "INPUT_PULLDOWN", INPUT_PULLUP_LOW);
    //GLOBAL_NUMBER(L, "EINT", EINT);
    //GLOBAL_NUMBER(L, "EINT_PULLUP", EINT_PULLUP_HI);
    //GLOBAL_NUMBER(L, "EINT_PULLDOWN", EINT_PULLUP_LOW);
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
    MOD_REG_NUMBER(L, "INPUT_PULLUP", INPUT_PULLUP_HI);
    MOD_REG_NUMBER(L, "INPUT_PULLDOWN", INPUT_PULLUP_LOW);
    //MOD_REG_NUMBER(L, "EINT", EINT);
    //MOD_REG_NUMBER(L, "EINT_PULLUP", EINT_PULLUP_HI);
    //MOD_REG_NUMBER(L, "EINT_PULLDOWN", EINT_PULLUP_LOW);
    MOD_REG_NUMBER(L, "REDLED", REDLED);
    MOD_REG_NUMBER(L, "GREENLED", GREENLED);
    MOD_REG_NUMBER(L, "BLUELED", BLUELED);
    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
