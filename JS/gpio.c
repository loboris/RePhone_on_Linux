
#include "vmdcl.h"
#include "vmdcl_gpio.h"

#include "v7.h"

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW 0

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);

static v7_val_t gpio_mode(struct v7* v7)
{
    VM_DCL_HANDLE handle;
    v7_val_t pinv = v7_arg(v7, 0);
    v7_val_t modev = v7_arg(v7, 1);

    int pin, mode;

    if(!v7_is_number(pinv) || !v7_is_number(modev)) {
        printf("Invalid arguments\n");
        return v7_create_undefined();
    }

    pin = v7_to_number(pinv);
    mode = v7_to_number(modev);

    if(gpio_get_handle(pin, &handle)) {
        printf("Can't get io handle\n");
        return v7_create_undefined();
    }

    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);

    if(mode == INPUT) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
    } else if(mode == OUTPUT) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    } else {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    }

    return v7_create_boolean(1);
}

static v7_val_t gpio_read(struct v7* v7)
{
    VM_DCL_HANDLE handle;
    v7_val_t pinv = v7_arg(v7, 0);
    v7_val_t modev = v7_arg(v7, 1);

    int pin;
    vm_dcl_gpio_control_level_status_t data;

    if(!v7_is_number(pinv)) {
        printf("Invalid arguments\n");
        return v7_create_undefined();
    }

    pin = v7_to_number(pinv);

    gpio_get_handle(pin, &handle);
    vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);

    return v7_create_number(data.level_status);
}

static v7_val_t gpio_write(struct v7* v7)
{
    VM_DCL_HANDLE handle;
    v7_val_t pinv = v7_arg(v7, 0);
    v7_val_t valuev = v7_arg(v7, 1);

    int pin, value;

    if(!v7_is_number(pinv) || !v7_is_number(valuev)) {
        printf("Invalid arguments\n");
        return v7_create_undefined();
        ;
    }

    pin = v7_to_number(pinv);
    value = v7_to_number(valuev);

    gpio_get_handle(pin, &handle);
    if(value) {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    } else {
        vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }

    return v7_create_boolean(1);
}

void js_init_gpio(struct v7* v7)
{
    v7_val_t gpio = v7_create_object(v7);
    v7_set(v7, v7_get_global(v7), "gpio", 4, 0, gpio);
    v7_set_method(v7, gpio, "mode", gpio_mode);
    v7_set_method(v7, gpio, "read", gpio_read);
    v7_set_method(v7, gpio, "write", gpio_write);
}
