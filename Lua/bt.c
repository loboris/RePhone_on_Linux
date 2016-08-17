
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

int btspp_tmo = -1;

cb_func_param_bt_t bt_cb_params;

static VMUINT8 g_b_name[VM_BT_CM_DEVICE_NAME_LENGTH] = {0};
static VMUINT8 g_b_flags = 0;

extern int retarget_target;
extern vm_mutex_t retarget_rx_mutex;
extern VM_SIGNAL_ID retarget_rx_signal_id;

extern char retarget_rx_buffer[SERIAL_BUFFER_SIZE];
extern unsigned retarget_rx_buffer_head;
extern unsigned retarget_rx_buffer_tail;


//#define SPP_DATA "Hi from RePhone BT-SPP\n"

//---------------------------------------------------------------------
static void _btaddr2str(char *buf, vm_bt_cm_bt_address_t *g_btspp_addr)
{
	sprintf(buf, "0x%02x:%02x:%02x:%02x:%02x:%02x",
			((g_btspp_addr->nap & 0xff00) >> 8),
			(g_btspp_addr->nap & 0x00ff),
			(g_btspp_addr->uap),
			((g_btspp_addr->lap & 0xff0000) >> 16),
			((g_btspp_addr->lap & 0x00ff00) >> 8),
			(g_btspp_addr->lap & 0x0000ff));
}

//-----------------------
void _btspp_recv_cb(void)
{
	btspp_tmo = -1;
	if ((bt_cb_params.connect_ref) && (bt_cb_params.recvbuf != NULL) && (bt_cb_params.bufptr > 0)) {
		if (bt_cb_params.recv_ref != LUA_NOREF) {
			if (bt_cb_params.busy == 0) {
				bt_cb_params.busy = 1;
				bt_cb_params.cb_ref = bt_cb_params.recv_ref;
				remote_lua_call(CB_FUNC_BT_RECV, &bt_cb_params);
			}
		}
		else {
			vm_log_debug("[BTSPP] Data received from [%s], len=%d", bt_cb_params.addr, bt_cb_params.bufptr);
			bt_cb_params.bufptr = 0;
		}
	}
}

// SPP service callback handler
//-------------------------------------------------------------------------------------------
static void app_btspp_cb(VM_BT_SPP_EVENT evt, vm_bt_spp_event_cntx_t* param, void* user_data)
{
    vm_bt_spp_event_cntx_t *cntx = (vm_bt_spp_event_cntx_t*)param;
    vm_bt_cm_bt_address_t g_btspp_addr;	// Store BT mac address of BT_NAME device
    VMINT ret;
    int i;

	//vm_log_debug("[BTSPP] Event %d", evt);
	memset(&g_btspp_addr, 0, sizeof(g_btspp_addr));
	ret = vm_bt_spp_get_device_address(cntx->connection_id, &g_btspp_addr);
    switch (evt) {
    	case VM_BT_SPP_EVENT_START:
    			vm_log_debug("[BTSPP] Start");
                if (g_shell_result == -9) {
    				g_shell_result = 1;
    				vm_signal_post(g_shell_signal);
                }
    		break;

        case VM_BT_SPP_EVENT_AUTHORIZE:
        	if (bt_cb_params.connected == 0) {
				bt_cb_params.status = vm_bt_spp_accept(cntx->connection_id, bt_cb_params.recvbuf, bt_cb_params.buflen, bt_cb_params.buflen);
				bt_cb_params.connected = 1;
				bt_cb_params.id = cntx->connection_id;
				_btaddr2str(bt_cb_params.addr, &g_btspp_addr);
				if (bt_cb_params.connect_ref != LUA_NOREF) {
					bt_cb_params.cb_ref = bt_cb_params.connect_ref;
					remote_lua_call(CB_FUNC_BT_CONNECT, &bt_cb_params);
				}
				else {
					vm_log_debug("[BTSPP] Authorized: [res=%d] [%s]", bt_cb_params.status, bt_cb_params.addr);
				}
        	}
        	else {
        		// Only one connection possible
				char adr[18];
				_btaddr2str(adr, &g_btspp_addr);
        		ret = vm_bt_spp_reject(cntx->connection_id);
				vm_log_debug("[BTSPP] Rejected: [res=%d] [%s]", ret, adr);
        	}
            break;

        case VM_BT_SPP_EVENT_READY_TO_WRITE:
        	{
        	}
        	break;

        case VM_BT_SPP_EVENT_READY_TO_READ:
			// *** read data from remote side
			if (retarget_target == -1000) {
				// === Lua shell redirected to BT ====
				ret = vm_bt_spp_read(cntx->connection_id, bt_cb_params.recvbuf, bt_cb_params.buflen);
				if (ret > 0) {
					vm_mutex_lock(&retarget_rx_mutex);
					if (retarget_rx_buffer_head == retarget_rx_buffer_tail) {
						vm_signal_post(retarget_rx_signal_id);
					}
					for (i = 0; i < ret; i++) {
						retarget_rx_buffer[retarget_rx_buffer_head % SERIAL_BUFFER_SIZE] = ((VMCHAR*)bt_cb_params.recvbuf)[i];
						retarget_rx_buffer_head++;
						if ((unsigned)(retarget_rx_buffer_head - retarget_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
							retarget_rx_buffer_tail = retarget_rx_buffer_head - SERIAL_BUFFER_SIZE;
						}
					}
					vm_mutex_unlock(&retarget_rx_mutex);
				}
				else {
					vm_log_debug("[BTSPP] Data received error %d", ret);
				}
			}
			else {
				// === Callback function or info ===
				int maxlen = bt_cb_params.buflen - bt_cb_params.bufptr -  1;
				ret = vm_bt_spp_read(cntx->connection_id, bt_cb_params.recvbuf+bt_cb_params.bufptr, maxlen);
				if (ret > 0) {
					bt_cb_params.bufptr += ret;
					bt_cb_params.recvbuf[bt_cb_params.bufptr] = '\0';
					btspp_tmo = 0;

					if ((strchr(bt_cb_params.recvbuf, '\n') != NULL) || (bt_cb_params.bufptr >= (bt_cb_params.buflen-1))) {
						btspp_tmo = -1;
						_btaddr2str(bt_cb_params.addr, &g_btspp_addr);
						if (bt_cb_params.recv_ref != LUA_NOREF) {
							bt_cb_params.cb_ref = bt_cb_params.recv_ref;
							if (bt_cb_params.busy == 0) {
								bt_cb_params.busy = 1;
								remote_lua_call(CB_FUNC_BT_RECV, &bt_cb_params);
							}
							else {
								vm_log_debug("[BTSPP] CB busy, data received from [%s], len=%d", bt_cb_params.addr, bt_cb_params.bufptr);
								bt_cb_params.bufptr = 0;
							}
						}
						else {
							vm_log_debug("[BTSPP] Data received from [%s], len=%d", bt_cb_params.addr, bt_cb_params.bufptr);
							bt_cb_params.bufptr = 0;
						}
					}
				}
			}
			break;

        case VM_BT_SPP_EVENT_DISCONNECT:
        	bt_cb_params.connected = 0;
        	bt_cb_params.id = -1;
        	bt_cb_params.status = 0;

        	if (retarget_target == -1000) {
        		if (retarget_usb_handle >= 0) retarget_target = retarget_usb_handle;
        		else if (retarget_uart1_handle >= 0) retarget_target = retarget_uart1_handle;
        	}

            if (g_shell_result == -9) {
				g_shell_result = 0;
				vm_signal_post(g_shell_signal);
            }
            else {
				_btaddr2str(bt_cb_params.addr, &g_btspp_addr);

				if (bt_cb_params.disconnect_ref != LUA_NOREF) {
					bt_cb_params.cb_ref = bt_cb_params.disconnect_ref;
					remote_lua_call(CB_FUNC_BT_DISCONNECT, &bt_cb_params);
				}
				else {
					vm_log_debug("[BTSPP] Disconnect: [%s]", bt_cb_params.addr);
				}
            }
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
		vm_log_debug("%s, Host info: [%s][%02x:%02x:%02x:%02x:%02x:%02x]", hdr, dev_info.name,
			((dev_info.device_address.nap & 0xff00) >> 8),
			(dev_info.device_address.nap & 0x00ff),
			(dev_info.device_address.uap),
			((dev_info.device_address.lap & 0xff0000) >> 16),
			((dev_info.device_address.lap & 0x00ff00) >> 8),
			(dev_info.device_address.lap & 0x0000ff));
	}
	else {
		vm_log_debug("Get host dev info error: [%s] %d", hdr, ret);
	}
}

//-----------------------------
static void _set_vis_name(void)
{
    VMINT ret;
	vm_bt_cm_device_info_t dev_info = {0};
	ret = vm_bt_cm_get_host_device_info(&dev_info);
	if (ret == 0) {
	    if (strcmp(g_b_name, dev_info.name) != 0) g_b_flags |= 0x01;
	    else g_b_flags &= 0xFE;
	}

	if (g_b_flags & 0x01) {
        // set BT device host name
        ret = vm_bt_cm_set_host_name(g_b_name);
    }

    ret = 99;
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

    if ((ret == 99) && (g_shell_result == -9)) {
		lua_pushinteger(shellL, 0);
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
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
            break;

        case VM_BT_CM_EVENT_DEACTIVATE:
            ret = vm_bt_cm_exit(g_btcm_hdl);
            g_btcm_hdl = -1;
            vm_log_debug("[BTCM] BT switch OFF");
            if (g_shell_result == -9) {
				lua_pushinteger(shellL, 0);
				g_shell_result = 1;
				vm_signal_post(g_shell_signal);
            }
            break;

        case VM_BT_CM_EVENT_SET_VISIBILITY:
        	{
        		vm_bt_cm_device_info_t dev_info = {0};
        		vm_bt_cm_set_visibility_t *visi = (vm_bt_cm_set_visibility_t *)param;
                vm_log_debug("[BTCM] Set visibility [%d]", visi->result);
				if (g_shell_result == -9) {
					lua_pushinteger(shellL, 0);
					g_shell_result = 1;
					vm_signal_post(g_shell_signal);
				}
        	}
            break;

        case VM_BT_CM_EVENT_SET_NAME:
            vm_log_debug("[BTCM] Set name [%s]", g_b_name);
        	break;

        case VM_BT_CM_EVENT_PAIR_RESULT:
            vm_log_debug("[BTCM] Pair");
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
    VMUINT8 vis = 1;

    const char *bthost = luaL_checkstring(L, 1);

    if (strcmp(g_b_name, bthost) != 0) strcpy(g_b_name, bthost);

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
			VM_BT_CM_EVENT_PAIR_RESULT		|
			VM_BT_CM_EVENT_RELEASE_ALL_CONN,
			NULL);
    }
    if (g_btcm_hdl < 0) {
    	vm_log_debug("[BSTART] Init BT error: %d", g_btcm_hdl);
        lua_pushinteger(L, -1);
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
    }

    return 1;
}

//===============================
static int bt_start(lua_State *L)
{
    const char *bthost = luaL_checkstring(L, 1);
	g_shell_result = -9;
	CCwait = 4000;
	remote_CCall(L, &_bt_start);
	if (g_shell_result < 0) { // no response
		g_shell_result = 1;
        lua_pushinteger(L, -2);
	    vm_log_debug("[BTSTART] Error switching on");
	}
	return g_shell_result;
}

//====================================
static int _bt_spp_start(lua_State *L)
{
    // init SPP services

    bt_cb_params.connected = 0;

    VMUINT evt_mask = VM_BT_SPP_EVENT_START	|
        VM_BT_SPP_EVENT_BIND_FAIL			|
        VM_BT_SPP_EVENT_AUTHORIZE			|
        VM_BT_SPP_EVENT_CONNECT				|
        VM_BT_SPP_EVENT_SCO_CONNECT			|
        VM_BT_SPP_EVENT_READY_TO_WRITE		|
        VM_BT_SPP_EVENT_READY_TO_READ		|
        VM_BT_SPP_EVENT_DISCONNECT			|
        VM_BT_SPP_EVENT_SCO_DISCONNECT;

    int min_buf_size = vm_bt_spp_get_min_buffer_size();
	bt_cb_params.buflen = min_buf_size;

    bt_cb_params.recvbuf = vm_calloc(bt_cb_params.buflen);
    if (bt_cb_params.recvbuf == NULL) {
		//vm_bt_spp_close(g_btspp_hdl);
        g_shell_result = -1;
    	vm_log_error("Error allocating spp buffers");
		goto exit;
    }
    bt_cb_params.buflen /= 2;
    //vm_log_debug("[BTSPP] Tx&Rx buffer size = %d", bt_cb_params.buflen);

	g_btspp_hdl = vm_bt_spp_open(evt_mask, app_btspp_cb, NULL);
    if (g_btspp_hdl < 0) {
        vm_log_debug("[BTSPP] SPP Init error: %d", g_btspp_hdl);
        g_shell_result = -2;
    	goto exit;
    }

    int res = vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);

    g_shell_result = vm_bt_spp_bind(g_btspp_hdl, UUID);

exit:
	vm_signal_post(g_shell_signal);
    return 1;
}

//===================================
static int bt_spp_start(lua_State *L)
{
    if (g_btcm_hdl < 0) {
    	// BT not started
        vm_log_debug("[BTSPP] BT not started!");
        lua_pushinteger(L, -1);
    	return 1;
    }
    if (g_btspp_hdl >= 0) {
    	// SPP already started
        vm_log_debug("[BTSPP] SPP already started!");
        lua_pushinteger(L, 0);
    	return 1;
    }

    bt_cb_params.buflen = 11880;
    bt_cb_params.bufptr = 0;

    if (bt_cb_params.recv_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.recv_ref);
    	bt_cb_params.recv_ref = LUA_NOREF;
    }
    if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 1);
		bt_cb_params.recv_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    g_shell_result = -9;
	CCwait = 2000;
	remote_CCall(L, &_bt_spp_start);
	if (g_shell_result < 0) { // no response or error
		vm_free(bt_cb_params.recvbuf);
		bt_cb_params.recvbuf = NULL;
        lua_pushinteger(L, g_shell_result);
		g_shell_result = 1;
	}
	else {
		g_shell_result = 1;
		lua_pushinteger(L, 0);
	}
	return g_shell_result;
}

//==============================
static int _bt_off(lua_State *L)
{
	// turn off BT
    if (vm_bt_cm_get_power_status() == VM_BT_CM_POWER_ON) {
    	vm_bt_cm_switch_off();
    }
	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
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

//=========================================
static int _bt_spp_disconnect(lua_State *L)
{
	if ((g_btspp_hdl >= 0) && (bt_cb_params.connected) && (bt_cb_params.id >= 0)) {
		vm_bt_spp_disconnect(bt_cb_params.id);
	}
	bt_cb_params.connected = 0;
	bt_cb_params.id = -1;
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

	if (bt_cb_params.recvbuf) {
		vm_free(bt_cb_params.recvbuf);
		bt_cb_params.recvbuf = NULL;
	}
    if (bt_cb_params.recv_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.recv_ref);
    	bt_cb_params.recv_ref = LUA_NOREF;
    }
    if (bt_cb_params.connect_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.connect_ref);
    	bt_cb_params.connect_ref = LUA_NOREF;
    }
    if (bt_cb_params.disconnect_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.disconnect_ref);
    	bt_cb_params.disconnect_ref = LUA_NOREF;
    }

	if (retarget_target == -1000) retarget_target = 0;

	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int bt_stop(lua_State *L)
{
    g_shell_result = -9;
	CCwait = 3000;
	remote_CCall(L, &_bt_spp_disconnect);

	remote_CCall(L, &_bt_spp_stop);
	remote_CCall(L, &_bt_off);
	remote_CCall(L, &_bt_stop);
	return g_shell_result;
}

//==================================
static int bt_spp_stop(lua_State *L)
{
    g_shell_result = -9;
	CCwait = 3000;
	remote_CCall(L, &_bt_spp_disconnect);

	remote_CCall(L, &_bt_spp_stop);
	return g_shell_result;
}

//========================================
static int bt_spp_disconnect(lua_State *L)
{
    g_shell_result = -9;
	CCwait = 3000;
	remote_CCall(L, &_bt_spp_disconnect);
	return g_shell_result;
}

//=======================================
static int bt_spp_onconnect(lua_State *L)
{
    if (bt_cb_params.connect_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.connect_ref);
    	bt_cb_params.connect_ref = LUA_NOREF;
    }
    if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 1);
		bt_cb_params.connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

//=======================================
static int bt_spp_ondisconnect(lua_State *L)
{
    if (bt_cb_params.disconnect_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, bt_cb_params.disconnect_ref);
    	bt_cb_params.disconnect_ref = LUA_NOREF;
    }
    if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 1);
		bt_cb_params.disconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

//====================================
static int _bt_spp_write(lua_State *L)
{
    size_t len;
    const char *str = luaL_checklstring(L, 1, &len);

    if ((bt_cb_params.connected) && (bt_cb_params.id >= 0) && (retarget_target != -1000)) {
    	len = vm_bt_spp_write(bt_cb_params.id, (char *)str, len);
    	lua_pushinteger(L, len);
    }
    else lua_pushinteger(L, -1);
	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 1;
}

//===================================
static int bt_spp_write(lua_State *L)
{
    size_t len;
    const char *str = luaL_checklstring(L, 1, &len);

	remote_CCall(L, &_bt_spp_write);
	return g_shell_result;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE bt_map[] = {
		{LSTRKEY("start"), LFUNCVAL(bt_start)},
		{LSTRKEY("stop"), LFUNCVAL(bt_stop)},
		{LSTRKEY("spp_start"), LFUNCVAL(bt_spp_start)},
		{LSTRKEY("spp_stop"), LFUNCVAL(bt_spp_stop)},
		{LSTRKEY("spp_disconnect"), LFUNCVAL(bt_spp_disconnect)},
		{LSTRKEY("spp_onconnect"), LFUNCVAL(bt_spp_onconnect)},
		{LSTRKEY("spp_ondisconnect"), LFUNCVAL(bt_spp_ondisconnect)},
		{LSTRKEY("spp_write"), LFUNCVAL(bt_spp_write)},
        {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_bt(lua_State *L) {
  bt_cb_params.buflen = 0;
  bt_cb_params.recvbuf = NULL;
  bt_cb_params.connected = 0;
  bt_cb_params.id = -1;
  bt_cb_params.recv_ref = LUA_NOREF;
  bt_cb_params.connect_ref = LUA_NOREF;
  bt_cb_params.disconnect_ref = LUA_NOREF;

  luaL_register(L, "bt", bt_map);
  return 1;
}

