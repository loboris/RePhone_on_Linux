
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

static int btspp_recv_ref = LUA_NOREF;

extern lua_State *L;
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
        		/* write SPP_DATA example string to remote side */
        	}
        	break;

        case VM_BT_SPP_EVENT_READY_TO_READ:
			// read data from remote side and print it out to log
			ret = vm_bt_spp_read(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size);
			if (ret > 0) {
				if (retarget_target == -1000) {
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
		                lua_rawgeti(L, LUA_REGISTRYINDEX, btspp_recv_ref);
						lua_pushinteger(L, ret);
						lua_pushstring(L, g_btspp_buf);
		                lua_call(L, 2, 0);
		            }
		            else {
						vm_log_debug("[BTSPP] Data received from [0x%02x:%02x:%02x:%02x:%02x:%02x]\n[%s]",
							((g_btspp_addr.nap & 0xff00) >> 8),
							(g_btspp_addr.nap & 0x00ff),
							(g_btspp_addr.uap),
							((g_btspp_addr.lap & 0xff0000) >> 16),
							((g_btspp_addr.lap & 0x00ff00) >> 8),
							(g_btspp_addr.lap & 0x0000ff),
							g_btspp_buf);
						//ret = vm_bt_spp_write(cntx->connection_id, SPP_DATA, strlen(SPP_DATA));
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

// Init SPP service and related resources
//-------------------------------
static void app_btspp_start(void)
{
    if ((g_btcm_hdl < 0) || (g_btspp_hdl >= 0)) return;

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
        return;
    }
    vm_log_debug("[BTSPP] SPP Open, hndl=%d", g_btspp_hdl);

    result = vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);

    g_btspp_min_buf_size = vm_bt_spp_get_min_buffer_size();
    vm_log_debug("[BTSPP] Min buffer size = %d", g_btspp_min_buf_size);

    g_btspp_buf = vm_calloc(g_btspp_min_buf_size);
    if (g_btspp_buf == NULL) {
        vm_log_debug("[BTSPP] Buffer allocation error");
    	return;
    }
    g_btspp_min_buf_size = g_btspp_min_buf_size / 2;

    result = vm_bt_spp_bind(g_btspp_hdl, UUID);
    if (result < 0){
        vm_bt_spp_close(g_btspp_hdl);
        vm_log_debug("[BTSPP] Bind error %d", result);
        return;
    }
}

// BT service callback handler
//-----------------------------------------------------------------
static void app_btcm_cb(VMUINT evt, void * param, void * user_data)
{
    VMINT ret;
    switch (evt) {
        case VM_BT_CM_EVENT_ACTIVATE:
            {
            	vm_bt_cm_device_info_t dev_info = {0};
            	vm_bt_cm_activate_t *active = (vm_bt_cm_activate_t *)param;

				// display host info
				ret = vm_bt_cm_get_host_device_info(&dev_info);
				if (ret == 0) {
					vm_log_debug("[BTCM] Host info: [%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
						((dev_info.device_address.nap & 0xff00) >> 8),
						(dev_info.device_address.nap & 0x00ff),
						(dev_info.device_address.uap),
						((dev_info.device_address.lap & 0xff0000) >> 16),
						((dev_info.device_address.lap & 0x00ff00) >> 8),
						(dev_info.device_address.lap & 0x0000ff));
				}
				else {
					vm_log_debug("[BTCM] Get host dev info error %d", ret);
				}
            }

            // set BT device host name
            ret = vm_bt_cm_set_host_name(g_b_name);

            // set bt device as visible
            if (vm_bt_cm_get_visibility() != VM_BT_CM_VISIBILITY_ON) {
            	ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
            }
            break;

        case VM_BT_CM_EVENT_DEACTIVATE:
            ret = vm_bt_cm_exit(g_btcm_hdl);
            g_btcm_hdl = -1;
            vm_log_debug("[BTCM] BT CM Closed");
            break;

        case VM_BT_CM_EVENT_SET_VISIBILITY:
        	{
        		vm_bt_cm_device_info_t dev_info = {0};
        		vm_bt_cm_set_visibility_t *visi = (vm_bt_cm_set_visibility_t *)param;
                vm_log_debug("[BTCM] Set visibility: hdl[%d] res[%d]", visi->handle, visi->result);
        	}
            break;

        case VM_BT_CM_EVENT_SET_NAME:
            vm_log_debug("[BTCM] Set name to [%s]", g_b_name);
        	break;

        case VM_BT_CM_EVENT_CONNECT_REQ:
            vm_log_debug("[BTCM] Connection request");
        	break;

        default: {
            break;
        }
    }
}

// Init BT service and turn on BT if necessary
// ---------------------------------------
static void btcm_start(void)
{
    VMINT ret;

    if (g_btcm_hdl < 0) {
		g_btcm_hdl = vm_bt_cm_init(
			app_btcm_cb,
			VM_BT_CM_EVENT_ACTIVATE			|
			VM_BT_CM_EVENT_DEACTIVATE		|
			VM_BT_CM_EVENT_SET_VISIBILITY	|
			VM_BT_CM_EVENT_SET_NAME			|
			VM_BT_CM_EVENT_CONNECT_REQ,
			NULL);
        vm_log_debug("[BTSTART] BT init, hndl=%d", g_btcm_hdl);
    }

    ret = vm_bt_cm_get_power_status();

    if (VM_BT_CM_POWER_OFF == ret) {
    	// Turn on BT if not yet on
        ret = vm_bt_cm_switch_on();
        vm_log_debug("[BTSTART] BT switch on");
    }
    else if (VM_BT_CM_POWER_ON == ret) {
    	// if BT is already on
        vm_bt_cm_device_info_t dev_info = {0};
        // set bt device host name
        ret = vm_bt_cm_set_host_name(g_b_name);
        ret = vm_bt_cm_get_host_device_info(&dev_info);
        vm_log_debug("[BTSTART] BT on, host device: [%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
            ((dev_info.device_address.nap & 0xff00) >> 8),
            (dev_info.device_address.nap & 0x00ff),
            (dev_info.device_address.uap),
            ((dev_info.device_address.lap & 0xff0000) >> 16),
            ((dev_info.device_address.lap & 0x00ff00) >> 8),
            (dev_info.device_address.lap & 0x0000ff));

        // set bt device as visible
        if (vm_bt_cm_get_visibility() != VM_BT_CM_VISIBILITY_ON) {
        	ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
        }
    }
}

//===============================
static int bt_start(lua_State *L)
{
	const char *bthost = luaL_checkstring(L, 1);
	strcpy(g_b_name, bthost);
	btcm_start();

    return 0;
}

//===================================
static int bt_spp_start(lua_State *L)
{
    if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    if (btspp_recv_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, btspp_recv_ref);
		lua_pushvalue(L, 1);
		btspp_recv_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    // init SPP services
    app_btspp_start();
    lua_pushinteger(L, g_btspp_hdl);
    return 1;
}

//===============================
static int bt_stop(lua_State *L)
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

	if (g_btcm_hdl >= 0) {
		vm_bt_cm_exit(g_btcm_hdl);
		g_btcm_hdl = -1;
	}
	// turn off BT
    if (vm_bt_cm_get_power_status() == VM_BT_CM_POWER_ON) {
    	vm_bt_cm_switch_off();
    }

	return 0;
}

//==================================
static int bt_spp_stop(lua_State *L)
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

	return 0;
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

