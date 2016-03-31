

#include <string.h>

#include "vmlog.h"
#include "vmgsm_gprs.h"
#include "vmchset.h"
#include "vmbearer.h"

#include "lua.h"
#include "lauxlib.h"


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

//==================================
static int gprs_setapn(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}

	int len=0;
	int ipar=0;
	const char *param;

	lua_getfield(L, 1, "apn");
	if (!lua_isnil(L, -1)) {
	  if( lua_isstring(L, -1) ) {
	    param = luaL_checklstring( L, -1, &len );
	    if(len > VM_GSM_GPRS_APN_MAX_LENGTH) return luaL_error( L, "apn wrong" );
	    strncpy(apn_info.apn, param, len);
	  }
	  else return luaL_error( L, "wrong arg type: apn" );
	}
	else return luaL_error( L, "apn missing" );

	lua_getfield(L, 1, "useproxy");
	if (!lua_isnil(L, -1)) {
	  if( lua_isnumber(L, -1) ) {
	    ipar = luaL_checkinteger( L, -1 );
	    if (ipar == 0) apn_info.using_proxy = VM_FALSE;
	    else apn_info.using_proxy = VM_TRUE;
	  }
	  else return luaL_error( L, "wrong arg type: useproxy" );
	}
	else apn_info.using_proxy = VM_FALSE;

	if (apn_info.using_proxy != VM_FALSE) {
		lua_getfield(L, 1, "proxy");
		if (!lua_isnil(L, -1)) {
		  if( lua_isstring(L, -1) ) {
			param = luaL_checklstring( L, -1, &len );
			if(len > VM_GSM_GPRS_APN_MAX_PROXY_ADDRESS_LENGTH) return luaL_error( L, "proxy wrong" );
			strncpy(apn_info.proxy_address, param, len);
		  }
		  else return luaL_error( L, "wrong arg type: proxy" );
		}
		else return luaL_error( L, "proxy missing" );
	}

	gprs_bearer_type = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN;

	lua_pushinteger(L, vm_gsm_gprs_set_customized_apn_info(&apn_info));
    return 1;
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

