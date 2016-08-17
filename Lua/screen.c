
#include <stdint.h>

#include "vmboard.h"
#include "vmmemory.h"
#include "lcd_sitronix_st7789s.h"
//#include "vmdcl.h"
//#include "vmdcl_pwm.h"
#include "vmgraphic.h"
#include "vmtouch.h"
#include "tp_goodix_gt9xx.h"
#include "vmlog.h"
#include "vmchset.h"
#include "vmgraphic_font.h"
#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


static vm_graphic_frame_t g_frame;
static const vm_graphic_frame_t* g_frame_blt_group[1];

static vm_graphic_color_argb_t g_color_argb = {0, 0xff, 0xff, 0xff};
static uint16_t g_color_565 = 0xFFFE;

static int g_touch_cb_ref = LUA_NOREF;
static cb_func_param_touch_t touch_cb_params;

#define EXTERNAL_FONT_SIZE  7776   /* external font size calculated by CheckMemSize.exe tool, 32 * 1024 */
#define FONT_CACHE_SIZE     32768   /* 32 * 1024 */
#define EXTERNAL_FONT_PATH  "C:\\MTKSansSerifLatinBasicF.ttf"
#define FONT_PATH_MAX_LENGTH    40
VMUINT8 *font_pool;

//============================================
static int screen_set_brightness(lua_State *L)
{
    int brightness = luaL_checkinteger(L, 1);
    
    _set_backlight(brightness);
    
    return 0;
}

//--------------------------------------------------------------------
static void handle_touch_event(VM_TOUCH_EVENT event, VMINT x, VMINT y)
{
    // VM_TOUCH_EVENT_TAP
    // VM_TOUCH_EVENT_RELEASE

	if (g_touch_cb_ref != LUA_NOREF) {
		touch_cb_params.event = event;
		touch_cb_params.x = x;
		touch_cb_params.y = x;
		touch_cb_params.cb_ref = g_touch_cb_ref;
        remote_lua_call(CB_FUNC_INT, &touch_cb_params);
    }
    else {
        vm_log_debug("touch: ev=%d, x=%d, y=%d\n", event, x, y);
    }
}

//===================================
static int _screen_init(lua_State *L)
{
    vm_graphic_point_t positions[1] = { 0, 0 };
    
    lcd_st7789s_init();

    g_frame.width = 240;
    g_frame.height = 240;
    g_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
    g_frame.buffer_length = (g_frame.width * g_frame.height * 2);

    if (g_frame.buffer == NULL) g_frame.buffer = (VMUINT8*)vm_malloc_dma(g_frame.width * g_frame.height * 2);
    if (g_frame.buffer == NULL) {
    	vm_log_error("Frame buffer allocation error, increase C heap size!");
    	g_shell_result = -10;
    	goto exit;
    }

    vm_log_error("Frame buffer initialized, %d bytes", g_frame.width * g_frame.height * 2);
	g_frame_blt_group[0] = &g_frame;

	if (tp_gt9xx_init() < 0) {
		vm_log_error("Touch panel init error");
		g_shell_result = -11;
		goto exit1;
	}

	vm_touch_register_event_callback(handle_touch_event);
	memset(g_frame.buffer, 0, g_frame.width * g_frame.height * 2);
	g_shell_result = vm_graphic_blt_frame(g_frame_blt_group, positions, 1);
	if (g_shell_result < 0) {
		vm_log_info("blt frame failed");
		g_shell_result = -12;
		goto exit1;
	}

	// === Init fonts ===
	VMWCHAR font_path[FONT_PATH_MAX_LENGTH + 1];
	VMWSTR font_paths_group[1];
	VMUINT32 pool_size;

	g_shell_result = vm_graphic_get_font_pool_size(EXTERNAL_FONT_SIZE, 1, FONT_CACHE_SIZE, &pool_size);
	if (g_shell_result < 0) {
		vm_log_error("get font pool size failed");
		g_shell_result = -13;
		goto exit1;
	}
	font_pool = vm_malloc(pool_size);
	if (font_pool == NULL) {
		vm_log_error("allocate font pool memory failed");
		g_shell_result = -14;
		goto exit1;
	}

	vm_log_info("allocated font pool memory: %d bytes", pool_size);
	g_shell_result = vm_graphic_init_font_pool(font_pool, pool_size);
	if (g_shell_result < 0) {
		vm_log_error("init font pool failed");
		g_shell_result = -15;
		goto exit2;
	}

	vm_chset_ascii_to_ucs2(font_path, (FONT_PATH_MAX_LENGTH + 1) * 2, EXTERNAL_FONT_PATH);
	font_paths_group[0] = font_path;

	vm_graphic_reset_font();
	g_shell_result = vm_graphic_set_font(font_paths_group, 1);
	if (g_shell_result < 0) {
		vm_log_info("set font failed");
		g_shell_result = -16;
		goto exit2;
	}

	g_shell_result = 0;
exit:
    vm_signal_post(g_shell_signal);
    return 0;
exit2:
	vm_free(font_pool);
exit1:
	vm_free(g_frame.buffer);
	g_frame.buffer = NULL;
    vm_signal_post(g_shell_signal);
    return 0;
}

//==================================
static int screen_init(lua_State *L)
{
	CCwait = 3000;
    remote_CCall(L, &_screen_init);
    lua_pushinteger(L, g_shell_result);

    if (g_shell_result >= 0) _set_backlight(50);

    return 1;
}

//======================================
static int screen_on_touch(lua_State *L)
{
	if (g_touch_cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, g_touch_cb_ref);
		g_touch_cb_ref = LUA_NOREF;
	}
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 1);
		g_touch_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
    return 0;
}

//=====================================
static int _screen_update(lua_State *L)
{
    vm_graphic_point_t positions[1] = { 0, 0 };
    vm_graphic_blt_frame(g_frame_blt_group, positions, 1);

    vm_signal_post(g_shell_signal);
    return 0;
}

//====================================
static int screen_update(lua_State *L)
{
    remote_CCall(L, &_screen_update);
    
    return 0;
}

//=======================================
static int screen_set_color(lua_State *L)
{
    uint32_t color = luaL_checkinteger(L, 1);
    vm_graphic_color_argb_t c;
    
    c.a = (uint8_t)(color >> 24);
    c.r = (uint8_t)(color >> 16);
    c.g = (uint8_t)(color >> 8);
    c.b = (uint8_t)(color);
    
    vm_graphic_set_color(c);
    
    g_color_argb = c;
    g_color_565 = ((color >> 8) & (0x1F << 11)) | ((color >> 5) & (0x3F << 5)) | ((color >> 3) & 0x1E);
    
    return 0;
}

//=======================================
static int screen_get_color(lua_State *L)
{
    uint32_t argb;
    
    argb = ((uint32_t)g_color_argb.a << 24) + ((uint32_t)g_color_argb.r << 16) + ((uint32_t)g_color_argb.g << 8) + g_color_argb.b;
    lua_pushinteger(L, argb);
    
    return 1;
}

//---------------------------------------------
static void _draw_point(uint16_t x, uint16_t y)
{
    uint16_t* pbuf = (uint16_t*)g_frame.buffer + y * 240 + x;
    
    *pbuf = g_color_565;
}

//====================================
static int _screen_point(lua_State *L)
{
    uint16_t x = luaL_checkinteger(L, 1);
    uint16_t y = luaL_checkinteger(L, 2);

    _draw_point(x, y);
    vm_signal_post(g_shell_signal);

    return 0;
}

//===================================
static int screen_point(lua_State *L)
{
    remote_CCall(L, &_screen_point);

    return 0;
}

//===================================
static int _screen_line(lua_State *L)
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

    vm_signal_post(g_shell_signal);
    return 0;
}

//==================================
static int screen_line(lua_State *L)
{
    remote_CCall(L, &_screen_line);

    return 0;
}

//========================================
static int _screen_rectangle(lua_State *L)
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

    vm_signal_post(g_shell_signal);
    return 0;
}

//=======================================
static int screen_rectangle(lua_State *L)
{
    remote_CCall(L, &_screen_rectangle);

    return 0;
}

//====================================
static int _screen_fill(lua_State *L)
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
        color = ((c >> 8) & (0x1F << 11)) | ((c >> 5) & (0x3F << 5)) | ((c >> 3) & 0x1E);;
    }
    
	int i, j;
	for (i = 0; i < h; i++) {
		uint16_t *ptr = pbuf + i * 240;
		for (j = 0; j < w; j++) {
			if ((x + j) < 240) *ptr = color;
			else break;
			ptr++;
		}
		if ((y + i) >= 240) break;
	}

    vm_signal_post(g_shell_signal);
    return 0;
}

//==================================
static int screen_fill(lua_State *L)
{
    remote_CCall(L, &_screen_fill);

    return 0;
}

//====================================
static int screen_setver(lua_State *L)
{
	_TS_VER_ = luaL_checkinteger(L, 1) & 1;

    return 0;
}


//====================================
static int _screen_write(lua_State *L)
{
    uint16_t x = luaL_checkinteger(L, 1);
    uint16_t y = luaL_checkinteger(L, 2);
    size_t len;
    const char *text = luaL_checklstring( L, 3, &len );
    int top = lua_gettop(L);

    vm_graphic_set_font_size(VM_GRAPHIC_LARGE_FONT);
    VMWCHAR wtext[128];
	vm_chset_ascii_to_ucs2(wtext, 126, text);

    if (top > 3) {
        uint32_t color = luaL_checkinteger(L, 4);
        vm_graphic_color_argb_t c;

        c.a = (color >> 24) & 0xFF;
        c.r = (color >> 16) & 0xFF;
        c.g = (color >> 8) & 0xFF;
        c.b = (color) & 0xFF;

        vm_graphic_set_color(c);
    }

    g_shell_result = vm_graphic_draw_text(&g_frame, x, y, wtext);

    if (top > 3) {
        vm_graphic_set_color(g_color_argb);
    }

 exit:
    vm_signal_post(g_shell_signal);
    return 0;
}

//===================================
static int screen_write(lua_State *L)
{
    remote_CCall(L, &_screen_write);
    lua_pushinteger(L, g_shell_result);
    return 1;
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
    {"ontouch", screen_on_touch},
    {"setver", screen_setver},
    {"write", screen_write},
    {NULL, NULL}
};

LUALIB_API int luaopen_screen(lua_State *L)
{
	g_frame.buffer = NULL;
	luaL_register(L, "screen", screen_lib);
    return 1;
}
