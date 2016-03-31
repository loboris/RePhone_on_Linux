
#include <stdint.h>

#include "vmmemory.h"
#include "lcd_sitronix_st7789s.h"
#include "vmdcl.h"
#include "vmdcl_pwm.h"
#include "vmgraphic.h"
#include "vmtouch.h"
#include "tp_goodix_gt9xx.h"

#include "lua.h"
#include "lauxlib.h"

extern lua_State *L;

vm_graphic_frame_t g_frame;
const vm_graphic_frame_t* g_frame_blt_group[1];

vm_graphic_color_argb_t g_color_argb = {0, 0xff, 0xff, 0xff};
uint16_t g_color_565 = 0xFFFF;

int g_touch_cb_ref = LUA_NOREF;

static void _set_backlight(uint8_t brightness)
{
    VM_DCL_HANDLE pwm_handle;
    vm_dcl_pwm_set_clock_t pwm_clock;
    vm_dcl_pwm_set_counter_threshold_t pwm_config_adv;
    vm_dcl_config_pin_mode(3, VM_DCL_PIN_MODE_PWM);
    pwm_handle = vm_dcl_open(PIN2PWM(3), vm_dcl_get_owner_id());
    vm_dcl_control(pwm_handle, VM_PWM_CMD_START, 0);
    pwm_config_adv.counter = 100;
    pwm_config_adv.threshold = brightness;
    pwm_clock.source_clock = 0;
    pwm_clock.source_clock_division = 3;
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_CLOCK, (void*)(&pwm_clock));
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD, (void*)(&pwm_config_adv));
    vm_dcl_close(pwm_handle);
}


int screen_set_brightness(lua_State *L)
{
    int brightness = luaL_checkinteger(L, 1);
    
    _set_backlight(brightness);
    
    return 0;
}

void handle_touch_event(VM_TOUCH_EVENT event, VMINT x, VMINT y)
{
    if (event == VM_TOUCH_EVENT_TAP) {
    } else if (event == VM_TOUCH_EVENT_RELEASE) {
    }
    
    if (g_touch_cb_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_touch_cb_ref);
        lua_pushinteger(L, event);
        lua_pushinteger(L, x);
        lua_pushinteger(L, y);
        lua_call(L, 3, 0);
    }
}

int screen_init(lua_State *L)
{
    vm_graphic_point_t positions[1] = { 0, 0 };
    
    lcd_st7789s_init();

    g_frame.width = 240;
    g_frame.height = 240;
    g_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
    g_frame.buffer = (VMUINT8*)vm_malloc_dma(g_frame.width * g_frame.height * 2);
    g_frame.buffer_length = (g_frame.width * g_frame.height * 2);

    g_frame_blt_group[0] = &g_frame;
    
    tp_gt9xx_init();
    vm_touch_register_event_callback(handle_touch_event);

    _set_backlight(50);


    vm_graphic_blt_frame(g_frame_blt_group, positions, 1);
    
    return 0;
}

int screen_on_touch(lua_State *L)
{
    lua_pushvalue(L, 1);
    g_touch_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int screen_update(lua_State *L)
{
    vm_graphic_point_t positions[1] = { 0, 0 };
    vm_graphic_blt_frame(g_frame_blt_group, positions, 1);
    
    return 0;
}

int screen_set_color(lua_State *L)
{
    uint32_t color = luaL_checkinteger(L, 1);
    vm_graphic_color_argb_t c;
    
    c.a = (uint8_t)(color >> 24);
    c.r = (uint8_t)(color >> 16);
    c.g = (uint8_t)(color >> 8);
    c.b = (uint8_t)(color);
    
    vm_graphic_set_color(c);
    
    g_color_argb = c;
    g_color_565 = ((color >> 8) & (0x1F << 11)) | ((color >> 5) & (0x3F << 5)) | ((color >> 3) & 0x1F); //((uint16_t)(c.r >> 3) << 11)  + ((uint16_t)(c.g >> 2) << 5) + (c.b >> 3);
    
    return 0;
}

int screen_get_color(lua_State *L)
{
    uint32_t argb;
    
    argb = ((uint32_t)g_color_argb.a << 24) + ((uint32_t)g_color_argb.r << 16) + ((uint32_t)g_color_argb.g << 8) + g_color_argb.b;
    lua_pushinteger(L, argb);
    
    return 1;
}

static void _draw_point(uint16_t x, uint16_t y)
{
    uint16_t* pbuf = (uint16_t*)g_frame.buffer + y * 240 + x;
    
    *pbuf = g_color_565;
}

int screen_point(lua_State *L)
{
    uint16_t x = luaL_checkinteger(L, 1);
    uint16_t y = luaL_checkinteger(L, 2);

    _draw_point(x, y);

    return 0;
}

int screen_line(lua_State *L)
{
    uint16_t x1 = luaL_checkinteger(L, 1);
    uint16_t y1 = luaL_checkinteger(L, 2);
    uint16_t x2 = luaL_checkinteger(L, 3);
    uint16_t y2 = luaL_checkinteger(L, 4);
    int top = lua_gettop(L);
    
    if (top > 4) {
        uint32_t color = luaL_checkinteger(L, 5);
        vm_graphic_color_argb_t c;
        
        c.a = (color >> 24) & 0xFF;
        c.r = (color >> 16) & 0xFF;
        c.g = (color >> 8) & 0xFF;
        c.b = (color) & 0xFF;
        
        vm_graphic_set_color(c);
    }
    
    vm_graphic_draw_line(&g_frame, x1, y1, x2, y2);
    
    if (top > 4) {
        vm_graphic_set_color(g_color_argb);
    }

    return 0;
}

int screen_rectangle(lua_State *L)
{
    uint16_t x = luaL_checkinteger(L, 1);
    uint16_t y = luaL_checkinteger(L, 2);
    uint16_t w = luaL_checkinteger(L, 3);
    uint16_t h = luaL_checkinteger(L, 4);
    int top = lua_gettop(L);
    
    if (top > 4) {
        uint32_t color = luaL_checkinteger(L, 5);
        vm_graphic_color_argb_t c;
        
        c.a = (color >> 24) & 0xFF;
        c.r = (color >> 16) & 0xFF;
        c.g = (color >> 8) & 0xFF;
        c.b = (color) & 0xFF;
        
        vm_graphic_set_color(c);
    }
    
    vm_graphic_draw_rectangle(&g_frame, x, y, w, h);
    
    if (top > 4) {
        vm_graphic_set_color(g_color_argb);
    }

    return 0;
}

int screen_fill(lua_State *L)
{
    
    uint16_t color = g_color_565;
    uint16_t x = luaL_checkinteger(L, 1);
    uint16_t y = luaL_checkinteger(L, 2);
    uint16_t w = luaL_checkinteger(L, 3);
    uint16_t h = luaL_checkinteger(L, 4);
    int top = lua_gettop(L);
    
    uint16_t *pbuf = (uint16_t*)g_frame.buffer + y * 240 + x;
    
    
    if (top > 4) {
        uint32_t c = luaL_checkinteger(L, 5);
        
        color = ((c >> 8) & (0x1F << 11)) | ((c >> 5) & (0x3F << 5)) | ((c >> 3) & 0x1F);;
    }
    
    {
        int i, j;
        for (i = 0; i < h; i++) {
            uint16_t *ptr = pbuf + i * 240;
            for (j = 0; j < w; j++) {
                if ((x + j) < 240) {
                    *ptr = color;
                } else {
                    break;
                }
                ptr++;
            }
            
            if ((y + i) >= 240) {
                break;
            }
        }
    }

    return 0;
}


static const luaL_Reg screen_lib[] =
{
    {"init", screen_init},
    {"set_color", screen_set_color},
    {"get_color", screen_get_color},
    {"line", screen_line},
    {"point", screen_point},
    {"rectangle", screen_rectangle},
    {"fill", screen_fill},
    {"update", screen_update},
    {"set_brightness", screen_set_brightness},
    {"touch", screen_on_touch},
    {NULL, NULL}
};

LUALIB_API int luaopen_screen(lua_State *L)
{
    luaL_register(L, "screen", screen_lib);
    return 1;
}
