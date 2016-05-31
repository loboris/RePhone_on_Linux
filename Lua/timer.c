
#include "vmtimer.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


#define LUA_TIMER   "timer"


//-----------------------------------------------------------------------------
static void __timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    timer_info_t *p = (timer_info_t *)user_data;
    if ((p->cb_ref != LUA_NOREF) && (p->paused == 0)) {
        p->runs++;
		if (p->busy == 0) {
			p->busy = 1;
			remote_lua_call(CB_FUNC_TIMER, p);
		}
		else {
			p->failed++;
			//vm_log_debug("[TIMER] busy %d", p->timer_id);
		}
    }
    else p->pruns++;
}

//====================================
static int _timer_create(lua_State *L)
{
	timer_info_t *p;
    int ref;
    int paused = 0;
    int interval = luaL_checkinteger(L, 1);
    if (interval < 10) interval = 10;

	// register timer Lua callback function
	lua_pushvalue(L, 2);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);

	if (lua_isnumber(L,3)) paused = luaL_checkinteger(L, 3) & 0x01;

	// Create userdata for this timer
	p = (timer_info_t *)lua_newuserdata(L, sizeof(timer_info_t));
	luaL_getmetatable(L, LUA_TIMER);
	lua_setmetatable(L, -2);

	p->busy = 0;
	p->cb_ref = ref;
	p->runs = 0;
	p->pruns = 0;
	p->failed = 0;
	p->paused = paused;
	p->timer_id = vm_timer_create_precise(interval, __timer_callback, p);

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//===================================
static int timer_create(lua_State *L)
{
    VMUINT32 interval = luaL_checkinteger(L, 1);

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
		// the timer must be created from the main thread!
		remote_CCall(&_timer_create);
		return g_shell_result;
    }
    else return luaL_error(L, "Callback function not given!");
}

//===============================
static int timer_cb(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    int paused = p->paused;
    p->paused = 1;

    if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
        if (p->cb_ref != LUA_NOREF) {
    		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
    		p->cb_ref = LUA_NOREF;
    	}
    	// register new timer Lua callback function
    	lua_pushvalue(L, 2);
    	p->cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    p->paused = paused;
    return 0;
}

//====================================
static int _timer_delete(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    lua_pushinteger(L, vm_timer_delete_precise(p->timer_id));

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//===================================
static int timer_delete(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    remote_CCall(&_timer_delete);

    if (p->cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
		p->cb_ref = LUA_NOREF;
	}
	return g_shell_result;
}

//--------------------------------
static int _timer_gc(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    int res = vm_timer_delete_precise(p->timer_id);
	vm_log_debug("gc: timer deleted id=%d, stat=%d", p->timer_id, res);

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
    return 0;
}

//-------------------------------
static int timer_gc(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    remote_CCall(&_timer_gc);

    if (p->cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
		p->cb_ref = LUA_NOREF;
	}
	return g_shell_result;
}

//==================================
static int timer_pause(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));
    p->paused = 1;
    return 0;
}

//===================================
static int timer_resume(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));
    p->paused = 0;
    return 0;
}

//-------------------------------------
static int timer_tostring(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));
    char state[8];
    if (p->cb_ref == LUA_NOREF) sprintf(state,"Deleted");
    else {
    	if (p->paused) sprintf(state,"Paused");
    	else sprintf(state,"Running");
    }
    lua_pushfstring(L, "timer (%s): id=%d, runs=%d, paused_runs=%d, fail=%d", state, p->timer_id, p->runs, p->pruns, p->failed);
    return 1;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE timer_map[] =
{
    {LSTRKEY("create"), LFUNCVAL(timer_create)},
    {LSTRKEY("delete"), LFUNCVAL(timer_delete)},
    {LSTRKEY("pause"), LFUNCVAL(timer_pause)},
    {LSTRKEY("resume"), LFUNCVAL(timer_resume)},
    {LSTRKEY("changecb"), LFUNCVAL(timer_cb)},
    {LNILKEY, LNILVAL}
};

const LUA_REG_TYPE timer_table[] = {
  //{LSTRKEY("delete"), LFUNCVAL(timer_delete)},
  {LSTRKEY("__gc"), LFUNCVAL(timer_gc)},
  {LSTRKEY("__tostring"), LFUNCVAL(timer_tostring)},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_timer(lua_State *L)
{
    luaL_newmetatable(L, LUA_TIMER);		// create metatable for timer handles
    lua_pushvalue(L, -1);					// push metatable
    lua_setfield(L, -2, "__index");			// metatable.__index = metatable
    luaL_register(L, NULL, timer_table);	// timer methods

    luaL_register(L, "timer", timer_map);
    return 1;
}
