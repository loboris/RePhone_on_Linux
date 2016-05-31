

#include <string.h>

#include "vmlog.h"
#include "vmgsm_gprs.h"
#include "vmchset.h"
#include "vmbearer.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN;

vm_gsm_gprs_apn_info_t apn_info = {
		.apn = "0.0.0.0",                     /* The APN name.*/
		.using_proxy = VM_FALSE,              /* The boolean to determine if a proxy server is used. */
		//.proxy_type = VM_GSM_GPRS_PROXY_NONE, /* The proxy server type. */
		.proxy_address = "0.0.0.0",           /* The proxy server address type. */
		.proxy_port = 0,                      /* The proxy server port type. */
		//.proxy_username = {0},                /* The roxy server User name. */
		//.proxy_password = {0},                /* The proxy server Password. */
};

//-----------------------
void set_custom_apn(void)
{
    vm_gsm_gprs_set_customized_apn_info(&apn_info);
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
	remote_CCall(&_gprs_setapn);
	return g_shell_result;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE gprs_map[] = {
									{LSTRKEY("setapn"), LFUNCVAL(gprs_setapn)},
									{LNILKEY, LNILVAL}
								};

LUALIB_API int luaopen_gprs(lua_State *L) {

  luaL_register(L, "gprs", gprs_map);
  return 1;
}

