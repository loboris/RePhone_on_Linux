
#include "vmtimer.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


#define LUA_TIMER   "timer"
#define MIN_TIMER_INTERVAL 5


//-----------------------------------------------------------------------------
static void __timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    timer_info_t *p = (timer_info_t *)user_data;
    if ((p->cb_ref != LUA_NOREF) && (p->state <= 0)) {
        p->runs++;
		if (p->busy == 0) {
			p->busy = 1;
			remote_lua_call(CB_FUNC_TIMER, p);
		}
		else {
			// callback function not yet finished
			p->failed++;
		}
    }
    else if (p->cb_ref != LUA_NOREF) p->pruns++;
}

//------------------------------------------------
static void _failed(lua_State *L, timer_info_t *p)
{
	luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
	p->timer_id = -1;
	p->cb_ref = LUA_NOREF;
	p->state = 2;
}

//====================================
static int _timer_create(lua_State *L)
{
	timer_info_t *p;
    int ref;
    int state = 0;
    int interval = luaL_checkinteger(L, 1);
    if (interval < MIN_TIMER_INTERVAL) interval = MIN_TIMER_INTERVAL;

	// register timer Lua callback function
	lua_pushvalue(L, 2);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Check if starting in paused mode
	if (lua_isnumber(L,3)) {
		state = luaL_checkinteger(L, 3);
		if ((state < -1) || (state > 2)) state = 0;
	}

	// Create userdata for this timer
	p = (timer_info_t *)lua_newuserdata(L, sizeof(timer_info_t));
	luaL_getmetatable(L, LUA_TIMER);
	lua_setmetatable(L, -2);

	p->busy = 0;
	p->cb_ref = ref;
	p->runs = 0;
	p->pruns = 0;
	p->failed = 0;
	p->state = state;
	p->last_state = state;
	p->interval = interval;
	if (p->state != 2) {
		p->timer_id = vm_timer_create_precise(interval, __timer_callback, p);
		if (p->timer_id >= 0) {
			// Created in running/paused state
			if (p->state == -1) {
				p->runs++;
				remote_lua_call(CB_FUNC_TIMER, p);
			}
		}
		else _failed(L, p); // Create failed, set deleted state
	}
	else p->timer_id = -1; // created in stop state

	lua_pushinteger(L, p->timer_id);
    g_shell_result = 2;

	vm_signal_post(g_shell_signal);
    return 1;
}

// Create timer, returns timer & timer_id
//===================================
static int timer_create(lua_State *L)
{
    VMUINT32 interval = luaL_checkinteger(L, 1);

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
		// the timer must be created from the main thread!
		remote_CCall(L, &_timer_create);
		return g_shell_result;
    }
    else return luaL_error(L, "Callback function not given!");
}

//====================================
static int _timer_delete(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    vm_timer_delete_precise(p->timer_id);
    p->timer_id = -1;

	vm_signal_post(g_shell_signal);
    return 0;
}

// Delete the timer, remove cb function
//===================================
static int timer_delete(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    if (p->timer_id >= 0) remote_CCall(L, &_timer_delete);

    if (p->cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
		p->cb_ref = LUA_NOREF;
	}
    p->state = 2;
	return 0;
}

// Stop the timer, leave cb function
//=================================
static int timer_stop(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    // delete the timer, but leave the cb refference
    if (p->timer_id >= 0) remote_CCall(L, &_timer_delete);

	p->last_state = p->state;  // save last state
    p->state = 2;
	return 0;
}

//===================================
static int _timer_start(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    p->timer_id = vm_timer_create_precise(p->interval, __timer_callback, p);
    if (p->timer_id >= 0) {
		// Created in running/paused state
    	if (p->last_state != 2) p->state = p->last_state; // restore saved state
    	else p->state = 0;
    	p->busy = 0;
    	p->runs = 0;
    	p->pruns = 0;
    	p->failed = 0;
    	if (p->state == -1) {
            p->runs++;
    		remote_lua_call(CB_FUNC_TIMER, p);
    	}
    }
	else _failed(L, p); // Create failed, set deleted state

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

// Start (recreate) stopped timer
//=================================
static int timer_start(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    // if stopped (timer deleted, but cb ref exists), create the timer
    if ((p->timer_id < 0) && (p->cb_ref != LUA_NOREF)) remote_CCall(L, &_timer_start);

	return 0;
}

// Disable execution of cb function
//==================================
static int timer_pause(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    // if running, pause
    if (p->state <= 0) {
    	p->last_state = p->state;
    	p->state = 1;
    }
    return 0;
}

// Enable execution of cb function
//===================================
static int timer_resume(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));
    int wait = 0;
    if (lua_gettop(L) > 1) {
    	wait = luaL_checkinteger(L, 2) & 1;
    }
    // if paused, resume
    if (p->state == 1) {
		if (wait) {
			// wait for next timer run
			uint32_t prun = p->pruns;
			while (p->pruns == prun) {
			    vm_thread_sleep(1);
			}
		}
		p->state = p->last_state;
    }
    return 0;
}

// Change timer's callback function
//===============================
static int timer_cb(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    if (p->cb_ref != LUA_NOREF) {
    	// Active timer
		if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
			uint8_t state = p->state;
			p->state = 1;

			luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
			p->cb_ref = LUA_NOREF;

			// register new timer Lua callback function
			lua_pushvalue(L, 2);
			p->cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

			p->state = state;  //restore state
		}
    }
    return 0;
}

//============================================
static int _timer_changeinterval(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));
    int interval = luaL_checkinteger(L, 2);
    if (interval < MIN_TIMER_INTERVAL) interval = MIN_TIMER_INTERVAL;

    vm_timer_delete_precise(p->timer_id); // Delete the timer

	p->interval = interval; // set new interval

	// Re-create the timer
	p->timer_id = vm_timer_create_precise(interval, __timer_callback, p);
	if (p->timer_id >= 0) {
		if (p->state == -1) {
			p->runs++;
			remote_lua_call(CB_FUNC_TIMER, p);
		}
	}
	else _failed(L, p); // Create failed, set deleted state

    lua_pushinteger(L, p->timer_id);

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 0;
}

// Change timer's interval, return new timer ID
//===========================================
static int timer_changeinterval(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));
    int interval = luaL_checkinteger(L, 2);
    if (interval < MIN_TIMER_INTERVAL) interval = MIN_TIMER_INTERVAL;

    if ((p->timer_id >= 0) && (p->cb_ref != LUA_NOREF)) {
    	// Active timer
    	if (p->state != 2) {
    		// Timer in running/paused state, recreate with new interval
    		remote_CCall(L, &_timer_changeinterval);
    	}
    	else p->interval = interval;  // stopped, just change interval
    }
    else lua_pushinteger(L, p->timer_id);

	return 1;
}

//==================================
static int timer_getid(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    lua_pushinteger(L, p->timer_id);

    return 1;
}

//=====================================
static int timer_getstate(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, 1, LUA_TIMER));

    if (p->cb_ref == LUA_NOREF) lua_pushinteger(L, 3);
    else lua_pushinteger(L, p->state);
    lua_pushinteger(L, p->last_state);

    return 2;
}

//--------------------------------
static int _timer_gc(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    int res = 0;
    if (p->timer_id >= 0) res = vm_timer_delete_precise(p->timer_id);
	vm_log_debug("gc: timer deleted id=%d, stat=%d", p->timer_id, res);
    p->timer_id = -1;
    p->state = 2;

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
    return 0;
}

// when time is nil, garbage collector releases resources
// but first we have to delete the timer
//-------------------------------
static int timer_gc(lua_State *L)
{
	// check argument
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    remote_CCall(L, &_timer_gc);

    if (p->cb_ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref);
		p->cb_ref = LUA_NOREF;
	}
	return g_shell_result;
}

//-------------------------------------
static int timer_tostring(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));
    char state[8];
    if (p->cb_ref == LUA_NOREF) sprintf(state,"Deleted");
    else {
    	if (p->state == 1) sprintf(state,"Paused");
    	else if (p->state == 2) sprintf(state,"Stopped");
    	else sprintf(state,"Running");
    }
    lua_pushfstring(L, "timer (%s): interval=%d, id=%d, runs=%d, paused_runs=%d, fail=%d",
    		state, p->interval, p->timer_id, p->runs, p->pruns, p->failed);
    return 1;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"


const LUA_REG_TYPE timer_map[] =
{
    {LSTRKEY("create"), LFUNCVAL(timer_create)},
    {LSTRKEY("delete"), LFUNCVAL(timer_delete)},
    {LSTRKEY("start"), LFUNCVAL(timer_start)},
    {LSTRKEY("stop"), LFUNCVAL(timer_stop)},
    {LSTRKEY("pause"), LFUNCVAL(timer_pause)},
    {LSTRKEY("resume"), LFUNCVAL(timer_resume)},
    {LSTRKEY("changecb"), LFUNCVAL(timer_cb)},
    {LSTRKEY("changeint"), LFUNCVAL(timer_changeinterval)},
    {LSTRKEY("getid"), LFUNCVAL(timer_getid)},
    {LSTRKEY("getstate"), LFUNCVAL(timer_getstate)},
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
