
#include <string.h>
#include <time.h>

#include "vmtype.h"
#include "vmsystem.h"
#include "vmlog.h"
#include "vmtimer.h"
#include "vmbt_cm.h"
#include "vmbt_spp.h"
#include "vmstdlib.h"
#include "string.h"
#include "vmmemory.h"
#include "vmthread.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


#define UUID 0x1101					// Standard SPP UUID
#define SERIAL_BUFFER_SIZE  256

VMINT g_btcm_hdl = -1;				// handle of BT service
VMINT g_btspp_hdl = -1;				// handle of SPP service
VMINT g_btspp_connected = 0;		// spp device connected flag
VMINT g_btspp_id = 0;				// connected spp device id

static VMINT g_btspp_min_buf_size;	// size of BT buffer
static void *g_btspp_buf = NULL;	// buffer that store SPP received data
//static VMINT g_b_server_find;		// if BT_NAME device is founded or not during search process
static VMUINT8 g_b_name[64] = {0};
static VMUINT8 g_b_flags = 0;
static cb_func_param_bt_t bt_cb_params;

static int btspp_recv_ref = LUA_NOREF;

extern int retarget_target;
extern vm_mutex_t retarget_rx_mutex;
extern VM_SIGNAL_ID retarget_rx_signal_id;

extern char retarget_rx_buffer[SERIAL_BUFFER_SIZE];
extern unsigned retarget_rx_buffer_head;
extern unsigned retarget_rx_buffer_tail;


#define SPP_DATA "Hi from RePhone BT-SPP\n"
// SPP service callback handler
//------------------------------------------------------------------------------------
void app_btspp_cb(VM_BT_SPP_EVENT evt, vm_bt_spp_event_cntx_t* param, void* user_data)
{
    vm_bt_spp_event_cntx_t *cntx = (vm_bt_spp_event_cntx_t*)param;
    vm_bt_cm_bt_address_t g_btspp_addr;	// Store BT mac address of BT_NAME device
    VMINT ret;
    int i;

	memset(&g_btspp_addr, 0, sizeof(g_btspp_addr));
	ret = vm_bt_spp_get_device_address(cntx->connection_id, &g_btspp_addr);
    switch (evt) {
    	case VM_BT_SPP_EVENT_START:
    			vm_log_debug("[BTSPP] Start");
    		break;

        case VM_BT_SPP_EVENT_AUTHORIZE:
			ret = vm_bt_spp_accept(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size, g_btspp_min_buf_size);
			if (ret == 0) {
				g_btspp_connected = 1;
				g_btspp_id = cntx->connection_id;
			}

			vm_log_debug("[BTSPP] Authorize: [res=%d][0x%02x:%02x:%02x:%02x:%02x:%02x]", ret,
				((g_btspp_addr.nap & 0xff00) >> 8),
				(g_btspp_addr.nap & 0x00ff),
				(g_btspp_addr.uap),
				((g_btspp_addr.lap & 0xff0000) >> 16),
				((g_btspp_addr.lap & 0x00ff00) >> 8),
				(g_btspp_addr.lap & 0x0000ff));
            break;

        case VM_BT_SPP_EVENT_READY_TO_WRITE:
        	{
        	}
        	break;

        case VM_BT_SPP_EVENT_READY_TO_READ:
			// read data from remote side and print it out to log
			ret = vm_bt_spp_read(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size);
			if (ret > 0) {
				if (retarget_target == -1000) {
					// Lua shell redirected to BT
					vm_mutex_lock(&retarget_rx_mutex);
					if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
						vm_signal_post(retarget_rx_signal_id);
					}
					for (i = 0; i < ret; i++) {
						retarget_rx_buffer[retarget_rx_buffer_head % SERIAL_BUFFER_SIZE] = ((VMCHAR*)g_btspp_buf)[i];
						retarget_rx_buffer_head++;
						if ((unsigned)(retarget_rx_buffer_head - retarget_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
							retarget_rx_buffer_tail = retarget_rx_buffer_head - SERIAL_BUFFER_SIZE;
						}
					}
					vm_mutex_unlock(&retarget_rx_mutex);
				}
				else {
					// log the received data
					((VMCHAR*)g_btspp_buf)[ret] = 0;

		            if (btspp_recv_ref != LUA_NOREF) {
		            	bt_cb_params.par = ret;
		            	bt_cb_params.cb_ref = btspp_recv_ref;
		            	bt_cb_params.cbuf = g_btspp_buf;
		                remote_lua_call(CB_FUNC_BT, &bt_cb_params);
		            }
		            else {
		            	if (((VMCHAR*)g_btspp_buf)[0] == 'B') {
		            		vm_bt_spp_write(g_btspp_id, "Welcome to RePhone Lua shell\n", 29);
		            		retarget_target = -1000;
		            	}
		            	/*
						vm_log_debug("[BTSPP] Data received from [0x%02x:%02x:%02x:%02x:%02x:%02x]\n[%s]",
							((g_btspp_addr.nap & 0xff00) >> 8),
							(g_btspp_addr.nap & 0x00ff),
							(g_btspp_addr.uap),
							((g_btspp_addr.lap & 0xff0000) >> 16),
							((g_btspp_addr.lap & 0x00ff00) >> 8),
							(g_btspp_addr.lap & 0x0000ff),
							g_btspp_buf);
						*/
		            }
				}
			}
			else {
				vm_log_debug("[BTSPP] Data received error %d", ret);
			}
			break;

        case VM_BT_SPP_EVENT_DISCONNECT:
        	g_btspp_connected = 0;
        	if (retarget_target == -1000) retarget_target = 0;
            vm_log_debug("[BTSPP] Disconnect [0x%02x:%02x:%02x:%02x:%02x:%02x]",
                ((g_btspp_addr.nap & 0xff00) >> 8),
                (g_btspp_addr.nap & 0x00ff),
                (g_btspp_addr.uap),
                ((g_btspp_addr.lap & 0xff0000) >> 16),
                ((g_btspp_addr.lap & 0x00ff00) >> 8),
                (g_btspp_addr.lap & 0x0000ff));
			break;
    }
}

//-------------------------------------------
static void _bt_hostdev_info(const char *hdr)
{
    VMINT ret;
	vm_bt_cm_device_info_t dev_info = {0};

	// display host info
	ret = vm_bt_cm_get_host_device_info(&dev_info);
	if (ret == 0) {
		vm_log_debug("%s, Host info: [%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", hdr, dev_info.name,
			((dev_info.device_address.nap & 0xff00) >> 8),
			(dev_info.device_address.nap & 0x00ff),
			(dev_info.device_address.uap),
			((dev_info.device_address.lap & 0xff0000) >> 16),
			((dev_info.device_address.lap & 0x00ff00) >> 8),
			(dev_info.device_address.lap & 0x0000ff));
	}
	else {
		vm_log_debug("%s, Get host dev info error %d", hdr, ret);
	}
}

//-----------------------------
static void _set_vis_name(void)
{
    VMINT ret;
    if (g_b_flags & 0x01) {
        // set BT device host name
        VMINT ret = vm_bt_cm_set_host_name(g_b_name);
    	g_b_flags &= 0xFE;
    }

    if (g_b_flags & 0x02) {
        // set bt device as visible
		if (vm_bt_cm_get_visibility() != VM_BT_CM_VISIBILITY_ON) {
			ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
		}
    }
    else {
        // set bt device as not visible
		if (vm_bt_cm_get_visibility() == VM_BT_CM_VISIBILITY_ON) {
			ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_OFF);
		}
    }
}

// BT service callback handler
//-----------------------------------------------------------------
static void app_btcm_cb(VMUINT evt, void * param, void * user_data)
{
    VMINT ret;
    switch (evt) {
        case VM_BT_CM_EVENT_ACTIVATE:
           	//vm_bt_cm_activate_t *active = (vm_bt_cm_activate_t *)param;
           	_bt_hostdev_info("[BTCM] BT switch ON");
            _set_vis_name();

            lua_pushinteger(shellL, 1);
            g_shell_result = 1;
        	vm_signal_post(g_shell_signal);
            break;

        case VM_BT_CM_EVENT_DEACTIVATE:
            ret = vm_bt_cm_exit(g_btcm_hdl);
            g_btcm_hdl = -1;
            vm_log_debug("[BTCM] BT switch OFF");
            g_shell_result = 0;
        	vm_signal_post(g_shell_signal);
            break;

        case VM_BT_CM_EVENT_SET_VISIBILITY:
        	{
        		vm_bt_cm_device_info_t dev_info = {0};
        		vm_bt_cm_set_visibility_t *visi = (vm_bt_cm_set_visibility_t *)param;
                vm_log_debug("[BTCM] Set visibility [%d]", visi->result);
        	}
            break;

        case VM_BT_CM_EVENT_SET_NAME:
            vm_log_debug("[BTCM] Set name [%s]", g_b_name);
        	break;

        case VM_BT_CM_EVENT_CONNECT_REQ:
            vm_log_debug("[BTCM] Connection request");
        	break;

        case VM_BT_CM_EVENT_RELEASE_ALL_CONN:
        	vm_log_debug("[BTCM] Release all connections");
        	break;

        default: {
            break;
        }
    }
}

//================================
static int _bt_start(lua_State *L)
{
    VMINT ret;
    VMUINT8 vis = 0x02;

    const char *bthost = luaL_checkstring(L, 1);
    if (strcmp(g_b_name, bthost) != 0) g_b_flags |= 0x01;
    else g_b_flags &= 0xFE;
    if (g_b_flags & 0x01) strcpy(g_b_name, bthost);

    if (lua_gettop(L) >= 2) {
    	vis = luaL_checkinteger(L, 2) & 0x01;
    }
	if (vis) g_b_flags |= 0x02;
	else g_b_flags &= 0xFD;

	// Init BT service (start connection manager) and turn on BT if necessary
    if (g_btcm_hdl < 0) {
		g_btcm_hdl = vm_bt_cm_init(
			app_btcm_cb,
			VM_BT_CM_EVENT_ACTIVATE			|
			VM_BT_CM_EVENT_DEACTIVATE		|
			VM_BT_CM_EVENT_SET_VISIBILITY	|
			VM_BT_CM_EVENT_SET_NAME			|
			VM_BT_CM_EVENT_CONNECT_REQ		|
			VM_BT_CM_EVENT_RELEASE_ALL_CONN,
			NULL);
    }
    if (g_btcm_hdl < 0) {
    	vm_log_debug("[BSTART] Init BT error: %d", g_btcm_hdl);
        lua_pushinteger(L, 0);
        g_shell_result = 1;
    	vm_signal_post(g_shell_signal);
        return 0;
    }

    ret = vm_bt_cm_get_power_status();

    if (VM_BT_CM_POWER_OFF == ret) {
    	// Turn on BT
        ret = vm_bt_cm_switch_on();
    }
    else if (VM_BT_CM_POWER_ON == ret) {
    	// BT is already on
    	_bt_hostdev_info("[BTSTART] BT is on");

        _set_vis_name();

        lua_pushinteger(L, 1);
        g_shell_result = 1;
    	vm_signal_post(g_shell_signal);
    }

    return 1;
}

//===============================
static int bt_start(lua_State *L)
{
    const char *bthost = luaL_checkstring(L, 1);
	remote_CCall(&_bt_start);
	return g_shell_result;
}

//====================================
static int _bt_spp_start(lua_State *L)
{
    if (btspp_recv_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, btspp_recv_ref);
    if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 1);
		btspp_recv_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    // init SPP services
    if (g_btcm_hdl < 0) {
    	// BT not started
        vm_log_debug("[BTSPP] BT not started!");
    	goto exit;
    }
    if (g_btspp_hdl >= 0) {
    	// SPP already started
        vm_log_debug("[BTSPP] SPP already started!");
    	goto exit;
    }

	VMINT result;
    VMUINT evt_mask = VM_BT_SPP_EVENT_START	|
        VM_BT_SPP_EVENT_BIND_FAIL			|
        VM_BT_SPP_EVENT_AUTHORIZE			|
        VM_BT_SPP_EVENT_CONNECT				|
        VM_BT_SPP_EVENT_SCO_CONNECT			|
        VM_BT_SPP_EVENT_READY_TO_WRITE		|
        VM_BT_SPP_EVENT_READY_TO_READ		|
        VM_BT_SPP_EVENT_DISCONNECT			|
        VM_BT_SPP_EVENT_SCO_DISCONNECT;

	g_btspp_connected = 0;

	g_btspp_hdl = vm_bt_spp_open(evt_mask, app_btspp_cb, NULL);
    if (g_btspp_hdl < 0) {
        vm_log_debug("[BTSPP] SPP Init error: %d", g_btspp_hdl);
    	goto exit;
    }

    result = vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);

    g_btspp_min_buf_size = vm_bt_spp_get_min_buffer_size();
    vm_log_debug("[BTSPP] Min buffer size = %d", g_btspp_min_buf_size);

	if (g_btspp_buf == NULL) {
		g_btspp_buf = vm_calloc(g_btspp_min_buf_size);
		if (g_btspp_buf == NULL) {
			vm_bt_spp_close(g_btspp_hdl);
			g_btspp_hdl = -1;
			vm_log_debug("[BTSPP] Buffer allocation error");
			goto exit;
		}
	}
    g_btspp_min_buf_size = g_btspp_min_buf_size / 2;

    result = vm_bt_spp_bind(g_btspp_hdl, UUID);
    if (result < 0) {
        vm_bt_spp_close(g_btspp_hdl);
        g_btspp_hdl = -1;
        vm_log_debug("[BTSPP] Bind error %d", result);
    	goto exit;
    }

exit:
    lua_pushinteger(L, g_btspp_hdl);

    g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//===================================
static int bt_spp_start(lua_State *L)
{
	remote_CCall(&_bt_spp_start);
	return g_shell_result;
}

//==============================
static int _bt_off(lua_State *L)
{
	// turn off BT
    if (vm_bt_cm_get_power_status() == VM_BT_CM_POWER_ON) {
    	vm_bt_cm_switch_off();
    }
	return 0;
}

//===============================
static int _bt_stop(lua_State *L)
{
    // close Connection manager
    if (g_btcm_hdl >= 0) {
		vm_bt_cm_exit(g_btcm_hdl);
		g_btcm_hdl = -1;
	}

	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//===================================
static int _bt_spp_stop(lua_State *L)
{
	if (g_btspp_hdl >= 0) {
		vm_bt_spp_close(g_btspp_hdl);
		g_btspp_hdl = -1;
	}

	if (g_btspp_buf) {
		vm_free(g_btspp_buf);
		g_btspp_buf = NULL;
	}
	g_btspp_connected = 0;
	if (retarget_target == -1000) retarget_target = 0;

	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int bt_stop(lua_State *L)
{
	remote_CCall(&_bt_spp_stop);
	remote_CCall(&_bt_off);
	remote_CCall(&_bt_stop);
	return g_shell_result;
}

//==================================
static int bt_spp_stop(lua_State *L)
{
	remote_CCall(&_bt_spp_stop);
	return g_shell_result;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE bt_map[] = {
		{LSTRKEY("start"), LFUNCVAL(bt_start)},
		{LSTRKEY("spp_start"), LFUNCVAL(bt_spp_start)},
		{LSTRKEY("stop"), LFUNCVAL(bt_stop)},
		{LSTRKEY("spp_stop"), LFUNCVAL(bt_spp_stop)},
        {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_bt(lua_State *L) {
  luaL_register(L, "bt", bt_map);
  return 1;
}

