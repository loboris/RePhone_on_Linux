#include <string.h>
#include "vmtype.h"
#include "tp_goodix_gt9xx.h"
#include "tp_i2c.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "vmdrv_tp.h"
#include "vmlog.h"
#include "vmdatetime.h"
#include "vmthread.h"

#define TPD_RESET_ISSUE_WORDAROUND
#define CTP_MAX_RESET_COUNT 3

const VMUINT8 gpio_ctp_reset_pin = 19;
const VMUINT8 gpio_ctp_i2c_scl_pin = 1;
//====================================
const VMUINT8 gpio_ctp_i2c_sda_pin = 2;		// Software I2C
//const VMUINT8 gpio_ctp_i2c_sda_pin = 0xFF;	// Hardware I2C (NOT WORKING)
//====================================
const VMUINT8 gpio_ctp_eint_pin = 52;

VM_DCL_HANDLE gpio_ctp_eint_handle;

VMUINT32 CTP_DELAY_TIME = 100;  // hardware i2c speed

VMUINT8 gt9xx_config[] = {
    0x43, 0xF0, 0x00, 0xF0, 0x00, 0x0A, 0x05, 0x00, 0x01, 0x08, 0x28, 0x0F, 0x50, 0x32, 0x03, 0x05, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x03, 0x06, 0x28, 0x00, 0x31, 0x0D, 0x03, 0x06, 0x00, 0x9B,
    0x44, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x1B, 0x19, 0x41, 0x94, 0xC5, 0x02, 0x00,
    0x00, 0x00, 0x04, 0xBB, 0x1B, 0x00, 0x9A, 0x21, 0x00, 0x80, 0x28, 0x00, 0x69, 0x31, 0x00, 0x58, 0x3B, 0x00, 0x58,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0E, 0x20, 0x05, 0x05, 0x20, 0x22, 0x77, 0x0F, 0x00, 0x00, 0x00, 0x50, 0x28, 0x12, 0x10,
    0x0E, 0x0C, 0x0A, 0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0A, 0x08, 0x06, 0x05, 0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x02, 0x03,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x47, 0x01
};

static void delay_ms(VMUINT32 millisecs)
{
    VMUINT32 timeStop;
    VMUINT32 timeStart;
    VMUINT32 Freq = 0;
    volatile VMUINT32 i;
    millisecs = millisecs * 1000;

    timeStart = vm_time_ust_get_count();
    while(Freq < millisecs) {
        for(i = 0; i < 5000; i++) {
        }
        timeStop = vm_time_ust_get_count();
        Freq = timeStop - timeStart + 1;
    }
}

VM_DRV_TP_BOOL CTP_I2C_write_byte(VMUINT16 ucBufferIndex, VMUINT8 pucData)
{
    return ctp_i2c_send(CTP_SLAVE_ADDR, ucBufferIndex, &pucData, 1);
}

static VM_DRV_TP_BOOL ctp_goodix_gt9xx_set_device_mode(VM_DRV_TP_DEVICE_MODE mode);

static void ctp_goodix_gt9xx_power(VM_DRV_TP_BOOL ON)
{
    static VM_DRV_TP_BOOL power_status = VM_DRV_TP_TRUE;

    if(ON == power_status) {
        return;
    }

    power_status = ON;

    if(ON) {
        ctp_goodix_gt9xx_set_device_mode(VM_DRV_TP_ACTIVE_MODE);
    } else {
        ctp_goodix_gt9xx_set_device_mode(VM_DRV_TP_SLEEP_MODE);
    }
}

static VM_DRV_TP_BOOL ctp_goodix_gt9xx_get_information(ctp_info_t* ctp_info)
{
    VM_DRV_TP_BOOL ret;
    VMUINT8 cfg;

    ret = CTP_I2C_read(CTP_VERSION_INFO_REG, (VMUINT8*)ctp_info, sizeof(ctp_info_t));
    //if (ret != VM_DRV_TP_TRUE) return VM_DRV_TP_FALSE;

    ret = CTP_I2C_read(CTP_CONFIG_REG_BASE, &cfg, 1);
    return ret;
}

VM_DRV_TP_BOOL ctp_goodix_gt9xx_set_configuration(void)
{
    VM_DRV_TP_BOOL ret;
    VMUINT8* cfg_p;
    ctp_info_t ctp_info;

    if (ctp_goodix_gt9xx_get_information(&ctp_info) != VM_DRV_TP_TRUE) return VM_DRV_TP_FALSE;

    ret = CTP_I2C_send(CTP_CONFIG_REG_BASE, gt9xx_config, sizeof(gt9xx_config));

    return ret;
}

static VM_DRV_TP_PEN_STATE ctp_goodix_gt9xx_hisr(void)
{
    VMUINT8 lvalue;

    CTP_I2C_read(CTP_TOUCH_INFO_REG_BASE, &lvalue, 1);

    if (lvalue & CTP_STAT_NUMBER_TOUCH) return VM_DRV_TP_DOWN;
    else return VM_DRV_TP_UP;
}

void ctp_init_EINT(void)
{
    vm_dcl_eint_control_config_t eint_config;
    vm_dcl_eint_control_sensitivity_t sens_data;
    vm_dcl_eint_control_hw_debounce_t deboun_time;
    vm_dcl_eint_control_auto_change_polarity_t auto_change;
    VM_DCL_STATUS status;

    memset(&eint_config, 0, sizeof(vm_dcl_eint_control_config_t));
    memset(&sens_data, 0, sizeof(vm_dcl_eint_control_sensitivity_t));
    memset(&deboun_time, 0, sizeof(vm_dcl_eint_control_hw_debounce_t));

    status = vm_dcl_control(
        gpio_ctp_eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL); /* Usually, before we config eint, we mask it firstly. */
    if(status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_MASK  = %d", status);
    }

    sens_data.sensitivity = 0;
    eint_config.act_polarity = 0;
    eint_config.auto_unmask = 1;

    status = vm_dcl_control(
        gpio_ctp_eint_handle, VM_DCL_EINT_COMMAND_SET_SENSITIVITY, (void*)&sens_data); /* set eint sensitivity */
    if(status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_SET_SENSITIVITY = %d", status);
    }

    status = vm_dcl_control(
        gpio_ctp_eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL); /* Usually, before we config eint, we mask it firstly. */
    if(status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_MASK  = %d", status);
    }

    deboun_time.debounce_time = 1; /* debounce time 1ms */
    status = vm_dcl_control(
        gpio_ctp_eint_handle, VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE, (void*)&deboun_time); /* set debounce time */
    if(status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_SET_HW_DEBOUNCE = %d", status);
    }

    status = vm_dcl_control(
        gpio_ctp_eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL); /* Usually, before we config eint, we mask it firstly. */
    if(status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_MASK  = %d", status);
    }

    eint_config.debounce_enable = 0; /* 1 means enable hw debounce, 0 means disable. */

    status = vm_dcl_control(
        gpio_ctp_eint_handle,
        VM_DCL_EINT_COMMAND_CONFIG,
        (void*)&eint_config); /* Please call this api finally, because we will unmask eint in this command. */

    if (status != VM_DCL_STATUS_OK) {
        vm_log_info("VM_EINT_CMD_CONFIG = %d", status);
    }

    // status = vm_dcl_control(gpio_ctp_eint_handle,VM_DCL_EINT_COMMAND_UNMASK,NULL);  /* call this function to unmask
    // this eint. */
    // if(status != VM_DCL_STATUS_OK)
    //{
    //	 vm_log_info("VM_EINT_CMD_CONFIG = %d", status);
    //}
}

static VM_DRV_TP_BOOL ctp_goodix_gt9xx_init(void)
{
    VM_DCL_HANDLE sda_handle, scl_handle;
    VM_DCL_HANDLE eint_handle, reset_handle;
    VM_DRV_TP_BOOL ack;
    ctp_info_t ctp_info;
    VMUINT16 reg_value;

    VMUINT8 i = 0;

    volatile VMUINT32* VSIM1_CON2 = (VMUINT32*)0xA0700188;
    volatile VMUINT32* VSIM1_CON0 = (VMUINT32*)0xA0700180;


    // turn on VSIM1
    *VSIM1_CON2 = *VSIM1_CON2 | 0x0002;
    *VSIM1_CON0 = *VSIM1_CON0 | 0x0001;

    // init i2c
    if (gpio_ctp_i2c_sda_pin != 0xFF) CTP_DELAY_TIME = 2;

    ctp_i2c_configure(CTP_SLAVE_ADDR, CTP_DELAY_TIME);

    eint_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_eint_pin);
    reset_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_reset_pin);

    ////////////////////////////reset//////////////////////////////////
    vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);

    vm_dcl_control(reset_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(reset_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(reset_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    delay_ms(5);
    vm_dcl_control(reset_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    delay_ms(10);
    //////////////////////////////////////////////////////////////////
    vm_dcl_close(eint_handle);
    vm_dcl_close(reset_handle);

    // vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
    // vm_dcl_control(eint_handle, gpio_ctp_eint_pin_M_EINT, NULL);
    // EINT_Set_Sensitivity(custom_eint_get_channel(touch_panel_eint_chann), EDGE_SENSITIVE);
    ctp_init_EINT();

    ack = ctp_goodix_gt9xx_set_configuration();
    if (ack == VM_DRV_TP_FALSE) {
        //vm_log_error("ctp_goodix_gt9xx_set_configuration fail!!!");
        return VM_DRV_TP_FALSE;
    }
    //vm_log_info("ctp_goodix_gt9xx_set_configuration OK!!!");

    ack = ctp_goodix_gt9xx_get_information(&ctp_info);

    if (ack == VM_DRV_TP_FALSE) {
        vm_log_error("TP read information fail");
        return VM_DRV_TP_FALSE;
    }

    return VM_DRV_TP_TRUE;
}

static VM_DRV_TP_BOOL ctp_goodix_gt9xx_parameters(vm_drv_tp_parameters_t* para, VMUINT32 get_para, VMUINT32 set_para)
{
    return VM_DRV_TP_TRUE;
}

//----------------------------------------------------------------------------------------
static VM_DRV_TP_BOOL ctp_read_one_point(VMUINT32 x_base, vm_drv_tp_single_event_t* event)
{
    VMUINT8 point_info[CTP_POINT_INFO_LEN];
    CTP_I2C_read(x_base, point_info, CTP_POINT_INFO_LEN);

    event->x = point_info[1] + (point_info[2] << 8);
    event->y = point_info[3] + (point_info[4] << 8);
    event->z = 32;
    //printf("id: %d x=%d y=%d\n", point_info[0], event->x, event->y);

    return VM_DRV_TP_TRUE;
}

//-----------------------------------------------------------------------------------------
static VM_DRV_TP_BOOL ctp_read_all_point(vm_drv_tp_multiple_event_t* tpes, VMUINT32 points)
{
    VMUINT32 i = 0;
    vm_drv_tp_single_event_t get_one_point;

    if ((points < 1) || (points > 5)) return VM_DRV_TP_FALSE;

    for (i = 0; i < points; i++) {
        ctp_read_one_point(CTP_POINT_INFO_REG_BASE + CTP_POINT_INFO_LEN * i, &get_one_point);
        tpes->points[i].x = 239 - get_one_point.x;
        tpes->points[i].y = get_one_point.y;
        tpes->points[i].z = get_one_point.z;
    }

    return VM_DRV_TP_TRUE;
}

//------------------------------------------------------------------------
VM_DRV_TP_BOOL ctp_goodix_gt9xx_get_data(vm_drv_tp_multiple_event_t* tpes)
{
    VMUINT8 lvalue = 0;
    VMUINT32 model = 0;
    VMUINT8 buf_status;
    VM_DRV_TP_BOOL ret = VM_DRV_TP_TRUE;
    VMUINT8 rst_char = 0;

    tpes->time_stamp = (VMUINT16)vm_drv_tp_get_time_stamp();
    tpes->padding = VM_DRV_TP_PATTERN;

    CTP_I2C_read(CTP_TOUCH_INFO_REG_BASE, &lvalue, 1);

    model = (VMUINT32)(lvalue & CTP_STAT_NUMBER_TOUCH);
    buf_status = lvalue & 0xF0;

    //printf("model: %d buf_status: %d\n", model, buf_status);

    if (model > 5) // goodix only support 5 fingers
    {
        ret = VM_DRV_TP_FALSE;
        goto exit_get_data;
    }

    tpes->model = (VMUINT16)model; // read out all touch points coordinates.

    if (model == 0) {
        ret = VM_DRV_TP_FALSE;
        CTP_I2C_read(CTP_POINT_INFO_REG_BASE + 1, &lvalue, 1);
        goto exit_get_data;
    }

    if (model <= 10) {
        if ((buf_status & 0x80) != 0) ctp_read_all_point(tpes, model);
        else goto exit_get_data;
    }
    else {
        if (model == 0x0d) {
            // TODO: check this event
        } else if(model == 0x0e) {
            // TODO: check this event
        } else {
            // TODO: check this event
        }

        goto exit_get_data;
    }

exit_get_data:

    CTP_I2C_send(CTP_TOUCH_INFO_REG_BASE, &rst_char, 1);

    return ret;
}

static VM_DRV_TP_BOOL ctp_goodix_gt9xx_set_device_mode(VM_DRV_TP_DEVICE_MODE mode)
{
    VMUINT8 suspend_command = 0x05;
    VM_DCL_HANDLE eint_handle;
    ctp_info_t ctp_info;
    VMUINT8 ctp_buffer[2] = { 0 };

    if (mode == VM_DRV_TP_ACTIVE_MODE) {
        eint_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_eint_pin);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        delay_ms(15);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_MODE_2, NULL);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
        vm_dcl_close(eint_handle);
    } else if((mode == VM_DRV_TP_SLEEP_MODE)) {
        eint_handle = vm_dcl_open(VM_DCL_GPIO, gpio_ctp_eint_pin);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
        vm_dcl_control(eint_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
        delay_ms(5);
        CTP_I2C_send(CTP_POWER_MODE_REG, &suspend_command, 1);
        CTP_I2C_read(CTP_POWER_MODE_REG, ctp_buffer, 1);
        vm_dcl_close(eint_handle);
    } else if(mode == VM_DRV_TP_FIRMWARE_UPDATE) {
        // need wdt
    }

    return VM_DRV_TP_FALSE;
}

static VMUINT32 ctp_goodix_gt9xx_command(VMUINT32 cmd, void* p1, void* p2) // p1: input p2: output
{
#define TPD_SYNC_REG 0x711
#define TPD_DIFF_DATA_REG 0xD80
#define TPD_CHANNEL_CONFIG_REG 0x6D5
#define TPD_RAW_DATA_1_REG 0x880
#define TPD_RAW_DATA_2_REG 0x9C0

#define MAX_DRIVING_CHANNEL 16
#define MAX_SENSING_CHANNLE 10
#define MAX_DIFF_DATA_SIZE (MAX_DRIVING_CHANNEL * MAX_SENSING_CHANNLE * 2)
#define MAX_RAW_DATA_SIZE MAX_DIFF_DATA_SIZE

    char** dst = (char**)p2;
    ctp_info_t ctp_info;
    VM_DRV_TP_BOOL ret = VM_DRV_TP_TRUE;
    VMUINT8* cfg = (VMUINT8*)p1;
    VMUINT8 mode;
    VMUINT8* tpd_raw_data;
    VMUINT8 tmp[MAX_DIFF_DATA_SIZE];
    VMUINT8* ptr;
    VMUINT8 row;
    VMUINT8 illegal = 0;

    if(dst != NULL)
        *dst = NULL;

    switch(cmd) {
    case 8:
        CTP_I2C_read(CTP_TOUCH_INFO_REG_BASE, &illegal, 1);
        if((illegal & 0x80) == 0)
            ret = 1;
        else
            ret = 0;
        break;
    case VM_DRV_TP_COMMAND_GET_VERSION: // get firmware version
        if(ctp_goodix_gt9xx_get_information(&ctp_info)) {
            memcpy(tpd_raw_data, (VMUINT8*)&ctp_info, sizeof(ctp_info_t));
            *dst = (char*)tpd_raw_data;
        } else {
            ret = VM_DRV_TP_FALSE;
        }
        break;

    case VM_DRV_TP_COMMAND_GET_CONFIG:
        if(CTP_I2C_read(CTP_CONFIG_REG_BASE, tpd_raw_data, VM_DRV_TP_CONFIG_LENGTH)) {
            *dst = (char*)tpd_raw_data;
        } else {
            ret = VM_DRV_TP_FALSE;
        }
        break;

    case VM_DRV_TP_COMMAND_LOAD_INT_CONFIG:
        ctp_goodix_gt9xx_set_configuration();
        break;

    case VM_DRV_TP_COMMAND_LOAD_EXT_CONFIG:
        cfg[105] = 1;
        CTP_I2C_send(CTP_CONFIG_REG_BASE, cfg, VM_DRV_TP_CONFIG_LENGTH);
        break;

    case VM_DRV_TP_COMMAND_GET_DIFF_DATA:
        break;

    case VM_DRV_TP_COMMAND_GET_FW_BUFFER:
        break;

    case VM_DRV_TP_COMMAND_DO_FW_UPDATE:
        break;
    default:
        ret = VM_DRV_TP_FALSE;
    }

    return ret;
}

static vm_drv_tp_function_list_t ctp_custom_func = { ctp_goodix_gt9xx_init,       ctp_goodix_gt9xx_set_device_mode,
                                                     ctp_goodix_gt9xx_hisr,       ctp_goodix_gt9xx_get_data,
                                                     ctp_goodix_gt9xx_parameters, ctp_goodix_gt9xx_power,
                                                     ctp_goodix_gt9xx_command };
//----------------------
int tp_gt9xx_init(void)
{
    gpio_ctp_eint_handle = vm_dcl_open(VM_DCL_EINT, 3);

    if (VM_DCL_HANDLE_INVALID == gpio_ctp_eint_handle) {
        vm_log_error("TP open EINT error");
        return -1;
    }

    return vm_drv_tp_setup_driver(&ctp_custom_func);
}
