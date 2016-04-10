
#include <stdlib.h>
#include <string.h>

#include "vmsock.h"
#include "vmtcp.h"
#include "vmbearer.h"
#include "vmmemory.h"
#include "vmtype.h"
#include "vmthread.h"
#include "vmgsm_gprs.h"

#include "lua.h"
#include "lauxlib.h"

#define LUA_TCP "tcp"

extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

static char* send_buf = NULL;

typedef struct {
    VM_TCP_HANDLE handle;
    int cb_ref;
    lua_State* L;
} tcp_info_t;

/*
//---------------------------------------------------------------------------------------
static void __tcp_svr_callback(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
    tcp_info_t* p = (tcp_info_t*)user_data;
    lua_State* L = p->L;
    printf("[TCPSVR] Event %d", event);
}
*/

//-----------------------------------------------------------------------------------
static void __tcp_callback(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
    tcp_info_t* p = (tcp_info_t*)user_data;
    lua_State* L = p->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, p->cb_ref);
	if ((lua_type(L, -1) != LUA_TFUNCTION) && (lua_type(L, -1) != LUA_TLIGHTFUNCTION)) {
	  // * BAD CB function reference
	  lua_remove(L, -1);
	  if (event == VM_TCP_EVENT_CAN_READ) {
		    printf("[TCP] Read.\n");
		    int nread = -1;
		    char buf[256];
			while (nread > 0) {
				nread = vm_tcp_read(handle, buf, 254);
				if (nread < 0) {
					break;
				}
				else if (nread > 0) {
					buf[nread] = '\n';
					buf[nread+1] = '\0';
					printf("%s\n", buf);
				}
			}
	  }
	  else if (event == VM_TCP_EVENT_CONNECTED) {
		  printf("[TCP] Connected.\n");
		  if (send_buf != NULL) {
			  int nwrt = vm_tcp_write(handle, send_buf, strlen(send_buf));
			  printf("[TCP] Sent: %d\n", nwrt);
		  }
	  }
	  else if (event == VM_TCP_EVENT_PIPE_CLOSED) {
		  printf("[TCP] Closed.\n");
          if (send_buf != NULL) {
	       	vm_free(send_buf);
	      }
	  }
	  else printf("[TCP] Event: %d\n", event);
	}
	else {
		lua_pushlightuserdata(L, p);
		luaL_getmetatable(L, LUA_TCP);
		lua_setmetatable(L, -2);
		lua_pushinteger(L, (int)event);
		lua_call(L, 2, 0);
	}
}

//===========================
int tcp_connect(lua_State* L)
{
    tcp_info_t* p;
    int ref;
    char* addr = luaL_checkstring(L, 1);
    unsigned port = luaL_checkinteger(L, 2);

    p = (tcp_info_t*)lua_newuserdata(L, sizeof(tcp_info_t));

    if ((lua_type(L, 3) == LUA_TFUNCTION) || (lua_type(L, 3) == LUA_TLIGHTFUNCTION)) {
		lua_pushvalue(L, 3);
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
	    p->cb_ref = ref;
    }
    else {
    	p->cb_ref = LUA_NOREF;
        size_t sl;
        const char* pdata = luaL_checklstring(L, 3, &sl);
        if ((sl <= 0) || (pdata == NULL)) {
            return luaL_error(L, "wrong send data");
        }
        if (send_buf != NULL) {
        	vm_free(send_buf);
        }
        send_buf = vm_malloc(sl+1);
        if (send_buf == NULL) {
            return luaL_error(L, "buffer allocation error");
        }

        strncpy(send_buf, pdata, sl);
        send_buf[sl] = '\0';
    }

    luaL_getmetatable(L, LUA_TCP);
    lua_setmetatable(L, -2);

    p->L = L;
    p->handle = vm_tcp_connect(addr, port, gprs_bearer_type, p, __tcp_callback);

    return 1;
}

//=========================
int tcp_write(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    int len;
    char* str = (char *)luaL_checklstring(L, 2, &len);

    lua_pushinteger(L, vm_tcp_write(p->handle, str, len));

    return 1;
}

//========================
int tcp_read(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    size_t size = luaL_checkinteger(L, 2);

    luaL_Buffer b;
    int nread;
    int i;
    int ret;

    char* buf = vm_malloc(size);
    if(buf == NULL) {
        return luaL_error(L, "malloc() failed");
    }

    nread = vm_tcp_read(p->handle, buf, size);
    if(nread < 0) {
        ret = luaL_error(L, "tcp failed to read");
    } else if(nread == 0) {
        // receives the FIN from the server
        ret = 0;
    } else {
        luaL_buffinit(L, &b);
        for(i = 0; i < size; i++) {
            luaL_addchar(&b, buf[i]);
        }

        luaL_pushresult(&b);
        ret = 1;
    }

    free(buf);
    return ret;
}

//=========================
int tcp_close(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));

    vm_tcp_close(p->handle);

    return 0;
}


static VM_BEARER_HANDLE g_bearer_hdl;
static VM_THREAD_HANDLE g_thread_handle;
static unsigned g_bearer_id;

//--------------------------------------------------------------------
VMINT32 tcpsvr_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
	vm_soc_address_t addr;
    VMINT ret;
    char buf[1024] = {0};
    memset(buf, 0x00, 1024);

    VMINT hdl_c = vm_tcp_connect_sync("82.196.4.208", 50002, gprs_bearer_type);
	printf("[SERVER] client socket result: %d\n", hdl_c);
	ret = vm_gsm_gprs_hold_bearer(VM_GSM_GPRS_HANDLE_TYPE_TCP, hdl_c);
	printf("[SERVER] hold bearer %d\n", ret);
	ret = vm_tcp_write_sync(hdl_c, "HI", 2);
	printf("[SERVER] client sent %d\n", ret);
	ret = vm_tcp_read_sync(hdl_c, buf, 1023);
	if (ret > 0) buf[ret] = 0;
	printf("[SERVER] client received %d\n%s\n", ret, buf);

	VMINT hdl_s = vm_tcp_server_init_sync(gprs_bearer_type, 80);
	printf("[SERVER] socket result: %d\n", hdl_s);
	if (hdl_s < 0) return 0;

    while (1) {
        memset(buf, 0x00, 1024);
		hdl_c = vm_tcp_server_accept_sync(hdl_s, &addr);
		printf("[SERVER] accepted %d %s\n", hdl_c, addr.address);

		ret = vm_tcp_server_read_sync(hdl_c, buf, 1024);
		buf[ret] = 0;
		printf("[SERVER] received %d\n%s\n", ret, buf);

		ret = vm_tcp_server_write_sync(hdl_c, "aaa", 3);
		printf("[SERVER] write %d\n", ret);
		vm_tcp_server_close_client_sync(hdl_c);
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------------
static void bearer_callback(VM_BEARER_HANDLE handle, VM_BEARER_STATE event, VMUINT data_account_id, void *user_data)
{
    if (VM_BEARER_WOULDBLOCK == g_bearer_hdl)
    {
        g_bearer_hdl = handle;
    }
    if (handle == g_bearer_hdl)
    {
        switch (event)
        {
        case VM_BEARER_DEACTIVATED:
        	printf("[BEARER] deactivated\n");
            break;
        case VM_BEARER_ACTIVATING:
        	printf("[BEARER] activating\n");
            break;
        case VM_BEARER_ACTIVATED:
        	printf("[BEARER] activated\n");
        	g_bearer_id = data_account_id;
            g_thread_handle = vm_thread_create(tcpsvr_thread, NULL, 0);
            break;
        case VM_BEARER_DEACTIVATING:
        	printf("[BEARER] deactivating\n");
            break;
        default:
            break;

        }
    }
}
//===========================
int tcp_opensvr(lua_State* L)
{
	/*
    tcp_info_t* p;
    int ref;
    unsigned port = luaL_checkinteger(L, 1);

    p = (tcp_info_t*)lua_newuserdata(L, sizeof(tcp_info_t));

	p->cb_ref = LUA_NOREF;

    luaL_getmetatable(L, LUA_TCP);
    lua_setmetatable(L, -1);

    p->L = L;
    p->handle = vm_tcp_server_init(gprs_bearer_type, port, p, (vm_tcp_server_callback)__tcp_svr_callback);

    lua_pushinteger(L, p->handle);
    return 1;
    */


    g_bearer_hdl = vm_bearer_open(gprs_bearer_type, NULL, bearer_callback, VM_BEARER_IPV4);

    lua_pushinteger(L, g_bearer_hdl);
    return 1;
}

//----------------------
int tcp_gc(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));

    return 0;
}

//----------------------------
int tcp_tostring(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    lua_pushfstring(L, "tcp (%p)", p->handle);
    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE tcp_map[] = { { LSTRKEY("connect"), LFUNCVAL(tcp_connect) },
								 { LSTRKEY("opensvr"), LFUNCVAL(tcp_opensvr) },
                                 { LSTRKEY("read"), LFUNCVAL(tcp_read) },
                                 { LSTRKEY("write"), LFUNCVAL(tcp_write) },
                                 { LSTRKEY("close"), LFUNCVAL(tcp_close) },
                                 { LNILKEY, LNILVAL } };

const LUA_REG_TYPE tcp_type_table[] = { { LSTRKEY("read"), LFUNCVAL(tcp_read) },
                                        { LSTRKEY("write"), LFUNCVAL(tcp_write) },
                                        { LSTRKEY("close"), LFUNCVAL(tcp_close) },
                                        { LSTRKEY("__gc"), LFUNCVAL(tcp_gc) },
                                        { LSTRKEY("__tostring"), LFUNCVAL(tcp_tostring) },
                                        { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_tcp(lua_State* L)
{
    luaL_newmetatable(L, LUA_TCP);     /* create metatable for file handles */
    lua_pushvalue(L, -1);                /* push metatable */
    lua_setfield(L, -2, "__index");      /* metatable.__index = metatable */
    luaL_register(L, NULL, tcp_type_table); /* file methods */

    luaL_register(L, "tcp", tcp_map);
    return 1;
}
