
#include <stdlib.h>
#include <string.h>

#include "vmsock.h"
#include "vmtcp.h"
#include "vmudp.h"
#include "vmbearer.h"
#include "vmmemory.h"
#include "vmtype.h"
#include "vmthread.h"
#include "vmgsm_gprs.h"
#include "vmlog.h"
#include "vmdns.h"
#include "sntp.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


#define NET_TYPE_TCP	1
#define NET_TYPE_UDP	2
#define NET_MAX_SOCKETS	10

typedef struct {
    char 			host[64];
    vm_dns_result_t	result;
} dns_data_t;

VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN;

vm_gsm_gprs_apn_info_t apn_info = {
		.apn = "0.0.0.0",                     // The APN name.
		.using_proxy = VM_FALSE,              // The boolean to determine if a proxy server is used.
		.proxy_type = VM_GSM_GPRS_PROXY_NONE, // The proxy server type.
		.proxy_address = "0.0.0.0",           // The proxy server address type.
		.proxy_port = 0,                      // The proxy server port type.
		.proxy_username = {0},                // The proxy server User name.
		.proxy_password = {0},                // The proxy server Password.
};

static cb_func_param_net_t cb_func_param;
static net_info_t* net_udata[NET_MAX_SOCKETS];
static dns_data_t dns_data;


//-----------------------------------------------------------------------------------
static void __tcp_callback(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
    net_info_t* p = (net_info_t*)user_data;

    if (p->cb_ref != LUA_NOREF) {
		cb_func_param.event = event;
		cb_func_param.net_info = p;
		cb_func_param.busy = 1;
		remote_lua_call(CB_FUNC_NET, &cb_func_param);
    }
    else {
		// * NO CB function reference
		if (event == VM_TCP_EVENT_CAN_READ) {
			printf("[TCP] Read\n[");
			int nread;
			char buf[256];
			do {
				nread = vm_tcp_read(handle, buf, 254);
				if (nread > 0) {
					buf[nread] = '\0';
					printf("%s", buf);
				}
			} while (nread > 0);
			printf("]\n");
		}
		else if (event == VM_TCP_EVENT_CONNECTED) {
		  p->connected = 1;
		  p->handle = handle;
		  vm_log_debug("[TCP] Connected.");
		  if (p->send_buf != NULL) {
			  int nwrt = vm_tcp_write(handle, p->send_buf, strlen(p->send_buf));
			  vm_log_debug("[TCP] Sent: %d\n", nwrt);
			  vm_free(p->send_buf);
			  p->send_buf = NULL;
		  }
		}
		else if (event == VM_TCP_EVENT_PIPE_CLOSED) {
		  p->connected = 0;
		  vm_log_debug("[TCP] Pipe closed.");
		  vm_free(p->send_buf);
		  p->send_buf = NULL;
		}
		else if (event == VM_TCP_EVENT_PIPE_BROKEN) {
		  p->connected = 0;
		  vm_log_debug("[TCP] Pipe broken.");
		  vm_free(p->send_buf);
		  p->send_buf = NULL;
		}
		else if (event == VM_TCP_EVENT_CAN_WRITE) {
		  vm_log_debug("[TCP] Can write.");
		  vm_free(p->send_buf);
		  p->send_buf = NULL;
		}
	}
}

//-----------------------------------------------------------------------------------
static void __udp_callback(VM_UDP_HANDLE handle, VM_UDP_EVENT event)
{
    net_info_t* p;
    int i;
	for (i=0; i<NET_MAX_SOCKETS; i++) {
		if (net_udata[i]->handle == handle) {
			p = net_udata[i];
			break;
		}
	}
	if (i >= NET_MAX_SOCKETS) {
		  vm_log_error("[UDP] User data not found.");
		  return;
	}

    if (p->cb_ref != LUA_NOREF) {
		cb_func_param.event = event;
		cb_func_param.net_info = p;
		cb_func_param.busy = 1;
		remote_lua_call(CB_FUNC_NET, &cb_func_param);
    }
    else {
		// * NO CB function reference
		if (event == VM_UDP_EVENT_READ) {
			printf("[UDP] Read\n[");
			int nread;
			char buf[256];
			do {
				nread = vm_udp_receive(handle, buf, 254, &p->address);
				if (nread > 0) {
					buf[nread] = '\0';
					printf("%s", buf);
				}
			} while (nread > 0);
			printf("]\n");
		}
		else if (event == VM_UDP_EVENT_PIPE_CLOSED) {
		  p->connected = 0;
		  vm_log_debug("[UDP] Pipe closed.");
		  vm_free(p->send_buf);
		  p->send_buf = NULL;
		}
		else if (event == VM_UDP_EVENT_PIPE_BROKEN) {
		  p->connected = 0;
		  vm_log_debug("[UDP] Pipe broken.");
		  vm_free(p->send_buf);
		  p->send_buf = NULL;
		}
		else if (event == VM_UDP_EVENT_WRITE) {
		  vm_log_debug("[UDP] Write.");
		  if (p->send_buf != NULL) {
			  vm_udp_send(p->handle, p->send_buf, p->bufptr, &p->address);
			  vm_log_debug("[TCP] Sent: %d\n", p->bufptr);
			  vm_free(p->send_buf);
			  p->send_buf = NULL;
		  }
		}
	}
}

//==================================
static int _tcp_create(lua_State* L)
{
    net_info_t* p;
    int ref;
    char* addr = luaL_checkstring(L, 1);
    unsigned port = luaL_checkinteger(L, 2);

    p = (net_info_t*)lua_newuserdata(L, sizeof(net_info_t));

	p->cb_ref = LUA_NOREF;
	p->send_buf = NULL;
	p->connected = 0;
	p->bufptr = 0;
	p->bufsize = 0;
	p->handle = -1;
	p->type = NET_TYPE_TCP;

    luaL_getmetatable(L, LUA_NET);
    lua_setmetatable(L, -2);

	lua_pushvalue(L, 3);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
	p->cb_ref = ref;

	if (lua_type(L, 4) == LUA_TSTRING) {
        size_t sl;
        const char* pdata = luaL_checklstring(L, 4, &sl);

        p->send_buf = vm_calloc(sl+1);
        if (p->send_buf == NULL) {
        	vm_log_error("buffer allocation error");
        }
        else {
			if ((sl <= 0) || (pdata == NULL)) {
				vm_log_error("wrong send data");
				sprintf(p->send_buf, "Rephone");
			}
			else {
				strncpy(p->send_buf, pdata, sl);
				p->send_buf[sl] = '\0';
			}
        }
    }

    p->handle = vm_tcp_connect(addr, port, gprs_bearer_type, p, __tcp_callback);
	g_shell_result = p->handle;

	vm_signal_post(g_shell_signal);
	return 0;
}

//=================================
static int tcp_create(lua_State* L)
{
    char* addr = luaL_checkstring(L, 1);
    unsigned port = luaL_checkinteger(L, 2);
    if ((lua_type(L, 3) != LUA_TFUNCTION) && (lua_type(L, 3) == LUA_TLIGHTFUNCTION)) {
    	return luaL_error(L, "callback function missing");
    }

    remote_CCall(L, &_tcp_create);

	return 1;
}

//===================================
static int _tcp_connect(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    char* addr = luaL_checkstring(L, 2);
    unsigned port = luaL_checkinteger(L, 3);

    vm_free(p->send_buf);
	p->send_buf = NULL;

    if (lua_type(L, 4) == LUA_TSTRING) {
        size_t sl;
        const char* pdata = luaL_checklstring(L, 4, &sl);

        p->send_buf = vm_calloc(sl+1);
        if (p->send_buf == NULL) {
        	vm_log_error("buffer allocation error");
        }
        else {
			if ((sl <= 0) || (pdata == NULL)) {
				vm_log_error("wrong send data");
				sprintf(p->send_buf, "Rephone");
			}
			else {
				strncpy(p->send_buf, pdata, sl);
				p->send_buf[sl] = '\0';
			}
        }
    }

    if (p->connected) vm_tcp_close(p->handle);
    p->handle = vm_tcp_connect(addr, port, gprs_bearer_type, p, __tcp_callback);

    lua_pushinteger(L, p->handle);

	vm_signal_post(g_shell_signal);
	return 0;
}

//==================================
static int tcp_connect(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    char* addr = luaL_checkstring(L, 2);
    unsigned port = luaL_checkinteger(L, 3);

    remote_CCall(L, &_tcp_connect);

	return 1;
}

//=================================
static int _tcp_write(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

    if (p->connected) {
		int len;
		char* str = (char *)luaL_checklstring(L, 2, &len);

		g_shell_result = vm_tcp_write(p->handle, str, len);
    }
    else g_shell_result = -1;

	vm_signal_post(g_shell_signal);
	return 0;
}

//================================
static int tcp_write(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    if (p->type != NET_TYPE_TCP) return luaL_error(L, "cannot write to udp socket");
    int len;
    char* str = (char *)luaL_checklstring(L, 2, &len);

    remote_CCall(L, &_tcp_write);

    lua_pushinteger(L, g_shell_result);

	return 1;
}

//================================
static int _tcp_read(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    size_t size = luaL_checkinteger(L, 2);

	g_shell_result = 1;

	if (p->connected == 0) {
    	vm_log_error("not connected");
    	lua_pushinteger(L, -10);
    	goto exit;
    }

    luaL_Buffer b;
    int nread;
    int i;

    char* buf = vm_calloc(size);
    if (buf == NULL) {
    	vm_log_error("buffer allocation error");
    	lua_pushinteger(L, -11);
    	goto exit;
    }

	nread = vm_tcp_read(p->handle, buf, size);
	if (nread < 0) {
		vm_log_error("tcp read error");
	    vm_free(buf);
    	lua_pushinteger(L, -12);
		goto exit;
	}
	else {
		luaL_buffinit(L, &b);
		for (i = 0; i < nread; i++) {
			luaL_addchar(&b, buf[i]);
		}

    	lua_pushinteger(L, nread);
		luaL_pushresult(&b);
		g_shell_result = 2;
	}

    vm_free(buf);

exit:
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int tcp_read(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    if (p->type != NET_TYPE_TCP) return luaL_error(L, "cannot read to udp socket");
    size_t size = luaL_checkinteger(L, 2);

    remote_CCall(L, &_tcp_read);

	return g_shell_result;
}

//==================================
static int _udp_create(lua_State* L)
{
    net_info_t* p;
    int ref;
    unsigned port = luaL_checkinteger(L, 1);

    p = (net_info_t*)lua_newuserdata(L, sizeof(net_info_t));

	p->cb_ref = LUA_NOREF;
	p->send_buf = NULL;
	p->connected = 0;
	p->bufptr = 0;
	p->bufsize = 0;
	p->handle = -1;
	p->type = NET_TYPE_UDP;

    luaL_getmetatable(L, LUA_NET);
    lua_setmetatable(L, -2);

	lua_pushvalue(L, 2);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
	p->cb_ref = ref;

	for (int i=0; i<NET_MAX_SOCKETS; i++) {
		if (net_udata[i] == NULL) {
			net_udata[i] = p;
			break;
		}
	}

    p->handle = vm_udp_create(port, gprs_bearer_type, __udp_callback, 0);

	vm_signal_post(g_shell_signal);
	return 0;
}

//=================================
static int udp_create(lua_State* L)
{
    unsigned port = luaL_checkinteger(L, 1);
    if ((lua_type(L, 2) != LUA_TFUNCTION) && (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
    	return luaL_error(L, "callback function missing");
    }
    int i;
	for (i=0; i<NET_MAX_SOCKETS; i++) {
		if (net_udata[i] == NULL) {
			break;
		}
	}
	if (i >= NET_MAX_SOCKETS) {
    	return luaL_error(L, "no free udp sockets");
	}

    remote_CCall(L, &_udp_create);

    //lua_pushinteger(L, g_shell_result);

	return 1;
}


//------------------------------------------------------------------------------------------------
static VM_RESULT get_host_callback(VM_DNS_HANDLE handle, vm_dns_result_t* result, void *user_data)
{
	if (g_shell_result == -9) {
		memcpy(&dns_data.result, result, sizeof(vm_dns_result_t));
		g_shell_result = 0;
		vm_signal_post(g_shell_signal);
	}
	return VM_SUCCESS;
}

//=================================
static int _udp_getIP(lua_State *L)
{
	VM_DNS_HANDLE g_handle;
	vm_dns_result_t dns_result;
	g_handle = vm_dns_get_host_by_name(gprs_bearer_type, (const VMCHAR*)dns_data.host, &dns_result, get_host_callback, NULL);

	return 0;
}
//=================================
static int _udp_write(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

    int len;
    char* str = (char *)luaL_checklstring(L, 4, &len);

    g_shell_result = vm_udp_send(p->handle, str, len, &(p->address));
	vm_signal_post(g_shell_signal);
	return 0;
}

//================================
static int udp_write(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    if (p->type != NET_TYPE_UDP) return luaL_error(L, "cannot write to tcp socket");

    int len;
    char* host = (char *)luaL_checklstring(L, 2, &len);
    int port = luaL_checkinteger(L, 3);
    char* str = (char *)luaL_checklstring(L, 4, &len);

    sprintf(dns_data.host, "%s", host);
	g_shell_result = -9;
	CCwait = 10000;
    remote_CCall(L, &_udp_getIP);

	if (g_shell_result < 0) { // no response
	    lua_pushinteger(L, -10);
	    vm_log_error("Error obtaining IP");
		return 1;
	}

	p->address.address_len = 4;
	p->address.address[0] = (dns_data.result.address[0]) & 0xFF;
	p->address.address[1] = ((dns_data.result.address[0]) & 0xFF00)>>8;
	p->address.address[2] = ((dns_data.result.address[0]) & 0xFF0000)>>16;
	p->address.address[3] = ((dns_data.result.address[0]) & 0xFF000000)>>24;
	p->address.port = port;

	vm_log_debug("[UDP] Sending to %d.%d.%d.%d:%d",
			p->address.address[0], p->address.address[1],
			p->address.address[2], p->address.address[3], p->address.port);

	remote_CCall(L, &_udp_write);

    lua_pushinteger(L, g_shell_result);

	return 1;
}

//================================
static int _udp_read(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    size_t size = luaL_checkinteger(L, 2);

	g_shell_result = 1;

    luaL_Buffer b;
    int nread;
    int i;

    char* buf = vm_calloc(size);
    if (buf == NULL) {
    	vm_log_error("buffer allocation error");
    	lua_pushinteger(L, -11);
    	goto exit;
    }

	nread = vm_udp_receive(p->handle, buf, size, &p->address);
	if (nread < 0) {
		vm_log_error("udp read error");
	    vm_free(buf);
    	lua_pushinteger(L, -12);
		goto exit;
	}
	else {
		luaL_buffinit(L, &b);
		for (i = 0; i < nread; i++) {
			luaL_addchar(&b, buf[i]);
		}

    	lua_pushinteger(L, nread);
		luaL_pushresult(&b);
		g_shell_result = 2;
	}

    vm_free(buf);

exit:
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int udp_read(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));
    if (p->type != NET_TYPE_UDP) return luaL_error(L, "cannot write to tcp socket");
    size_t size = luaL_checkinteger(L, 2);

	remote_CCall(L, &_udp_read);

	return 1;
}

//==========================
int _net_close(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

	vm_free(p->send_buf);

    if (p->handle >= 0) {
        g_shell_result = 0;
    	if (p->type == NET_TYPE_TCP) vm_tcp_close(p->handle);
    	else vm_udp_close(p->handle);
    }
    else g_shell_result = -1;
    p->connected = 0;

	vm_signal_post(g_shell_signal);
    return 0;
}

//=========================
int net_close(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

    remote_CCall(L, &_net_close);
	lua_pushinteger(L, g_shell_result);

    return 1;
}

//======================================
static int _net_ntptime (lua_State *L) {

  int tz = luaL_checkinteger( L, 1 );
  if ((tz > 14) || (tz < -12)) { tz = 0; }

  if (ntp_cb_ref != LUA_NOREF) {
	  luaL_unref(L, LUA_REGISTRYINDEX, ntp_cb_ref);
	  ntp_cb_ref = LUA_NOREF;
  }
  if (lua_gettop(L) >= 2) {
	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	  lua_pushvalue(L, 2);
	  ntp_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  	}
  }
  sntp_gettime(tz, 1);

  g_shell_result = 0;
  vm_signal_post(g_shell_signal);
  return 0;
}

//====================================
static int net_ntptime (lua_State *L) {
	int tz = luaL_checkinteger( L, 1 );
	remote_CCall(L, &_net_ntptime);
	return g_shell_result;
}


//----------------------
int net_gc(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

	vm_free(p->send_buf);
	if (p->type == NET_TYPE_TCP) vm_tcp_close(p->handle);
	else vm_udp_close(p->handle);

	for (int i=0; i<NET_MAX_SOCKETS; i++) {
		if (net_udata[i] == p) {
			net_udata[i] = NULL;
			break;
		}
	}

    return 0;
}

//----------------------------
int net_tostring(lua_State* L)
{
    net_info_t* p = ((net_info_t*)luaL_checkudata(L, 1, LUA_NET));

    if (p->type == NET_TYPE_TCP) lua_pushfstring(L, "tcp (%d), connected: %d", p->handle, p->connected);
    else lua_pushfstring(L, "udp (%d), on port: %d", p->handle, p->address.port);

    return 1;
}


//===================================
static int _gprs_setapn(lua_State *L)
{
	int len = 0;
	int ipar = 0;
	int err = 0;
	const char *param;

	lua_getfield(L, 1, "apn");
	if (!lua_isnil(L, -1)) {
	  if( lua_isstring(L, -1) ) {
	    param = luaL_checklstring( L, -1, &len );
	    if(len > VM_GSM_GPRS_APN_MAX_LENGTH) return luaL_error( L, "apn wrong" );
	    strncpy(apn_info.apn, param, len);
	  }
	  else {
 		 err = -100;
 		 vm_log_error("wrong arg type: apn" );
 		 goto exit;
	  }
	}
	else {
		err = -101;
		vm_log_error("apn missing" );
		goto exit;
	}

	lua_getfield(L, 1, "useproxy");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    ipar = luaL_checkinteger( L, -1 );
	    if (ipar == 0) apn_info.using_proxy = VM_FALSE;
	    else apn_info.using_proxy = VM_TRUE;
	  }
	  else {
	 	  err = -102;
	 	  vm_log_error("wrong arg type: useproxy" );
	 	  goto exit;
	  }
	}
	else apn_info.using_proxy = VM_FALSE;

	if (apn_info.using_proxy != VM_FALSE) {
		lua_getfield(L, 1, "proxy");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isstring(L, -1) ) {
			param = luaL_checklstring( L, -1, &len );
			if(len > VM_GSM_GPRS_APN_MAX_PROXY_ADDRESS_LENGTH) return luaL_error( L, "proxy wrong" );
			strncpy(apn_info.proxy_address, param, len);
		  }
		  else {
		 	  err = -103;
		 	  vm_log_error("wrong arg type: proxy" );
		 	  goto exit;
		  }
		}
		else {
		 	err = -104;
		 	vm_log_error("proxy arg missing" );
		 	goto exit;
		}

		lua_getfield(L, 1, "proxyport");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    ipar = luaL_checkinteger( L, -1 );
			if ((ipar < 1) || (ipar > 65536)) return luaL_error( L, "proxyport wrong" );
			apn_info.proxy_port = ipar;
		  }
		  else {
		 	  err = -105;
		 	  vm_log_error("wrong arg type: proxyport" );
		 	  goto exit;
		  }
		}
		else {
		 	err = -106;
		 	vm_log_error("proxyport arg missing" );
		 	goto exit;
		}

		lua_getfield(L, 1, "proxytype");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isnumber(L, -1) ) {
		    ipar = luaL_checkinteger( L, -1 );
			if ((ipar < 0) || (ipar > 9)) return luaL_error( L, "proxyport wrong" );
			apn_info.proxy_type = ipar;
		  }
		  else {
		 	  err = -107;
		 	  vm_log_error("wrong arg type: proxytype" );
		 	  goto exit;
		  }
		}
		else {
			apn_info.proxy_type = VM_GSM_GPRS_PROXY_NONE;
		}

		lua_getfield(L, 1, "proxyuser");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isstring(L, -1) ) {
			param = luaL_checklstring( L, -1, &len );
			if(len > VM_GSM_GPRS_APN_MAX_USERNAME_LENGTH) return luaL_error( L, "proxyuser wrong" );
			strncpy(apn_info.proxy_username, param, len);
		  }
		  else {
		 	  err = -108;
		 	  vm_log_error("wrong arg type: proxyuser" );
		 	  goto exit;
		  }
		}
		else apn_info.proxy_username[0] = '\0';

		lua_getfield(L, 1, "proxypass");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isstring(L, -1) ) {
			param = luaL_checklstring( L, -1, &len );
			if(len > VM_GSM_GPRS_APN_MAX_USERNAME_LENGTH) return luaL_error( L, "proxypass wrong" );
			strncpy(apn_info.proxy_password, param, len);
		  }
		  else {
		 	  err = -109;
		 	  vm_log_error("wrong arg type: proxypass" );
		 	  goto exit;
		  }
		}
		else apn_info.proxy_password[0] = '\0';
	}

	gprs_bearer_type = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN;

	err = vm_gsm_gprs_set_customized_apn_info(&apn_info);

exit:
	lua_pushinteger(L, err);
	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//==================================
static int gprs_setapn(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}
	remote_CCall(L, &_gprs_setapn);
	return g_shell_result;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE net_map[] = {
		{ LSTRKEY("setapn"), LFUNCVAL(gprs_setapn)},
		{ LSTRKEY("ntptime"),  	LFUNCVAL(net_ntptime)},
		{ LSTRKEY("tcp_create"), LFUNCVAL(tcp_create) },
		{ LSTRKEY("tcp_connect"), LFUNCVAL(tcp_connect) },
        { LSTRKEY("tcp_read"), LFUNCVAL(tcp_read) },
        { LSTRKEY("tcp_write"), LFUNCVAL(tcp_write) },
		{ LSTRKEY("udp_create"), LFUNCVAL(udp_create) },
        { LSTRKEY("udp_read"), LFUNCVAL(udp_read) },
        { LSTRKEY("udp_write"), LFUNCVAL(udp_write) },
        //{ LSTRKEY("udp_close"), LFUNCVAL(udp_close) },
        { LSTRKEY("close"), LFUNCVAL(net_close) },
        { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE net_type_table[] = {
        { LSTRKEY("__gc"), LFUNCVAL(net_gc) },
        { LSTRKEY("__tostring"), LFUNCVAL(net_tostring) },
        { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_net(lua_State* L)
{
	for (int i=0; i<NET_MAX_SOCKETS; i++) {
		net_udata[i] = NULL;
	}

    luaL_newmetatable(L, LUA_NET);			// create metatable for file handles
    lua_pushvalue(L, -1);					// push metatable
    lua_setfield(L, -2, "__index");			// metatable.__index = metatable
    luaL_register(L, NULL, net_type_table);	// net methods

    luaL_register(L, "net", net_map);
    return 1;
}
