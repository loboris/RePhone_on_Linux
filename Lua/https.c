
#include <stdlib.h>
#include <string.h>

#include "vmgsm_gprs.h"
#include "vmhttps.h"
#include "vmlog.h"
#include "vmdatetime.h"
#include "vmmemory.h"
#include "vmbearer.h"
#include "vmchset.h"
#include "vmtype.h"
#include "vmfs.h"
#include "vmstdlib.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

#define REPLY_SEGMENT_LENGTH 128
#define CONTENT_TYPE_WWWFORM  "Content-Type: application/x-www-form-urlencoded"
#define CONTENT_TYPE_FORMDATA "Content-Type: multipart/form-data"

extern lua_State* L;
extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

static int g_channel_id = 0;
static int g_request_id = -1;
static int g_read_seg_num;
static char g_https_url[64] = {0,};
static int g_https_header_cb_ref = LUA_NOREF;
static int g_https_response_cb_ref = LUA_NOREF;
static char *g_postdata = NULL;
static int g_postdata_len = 0;
static int _post_type = 0;


//----------------------------------------------------------------------------------------------------
static void https_send_request_set_channel_rsp_cb(VMUINT32 req_id, VMUINT8 channel_id, VMUINT8 result)
{
    VMINT ret = -1;

    g_channel_id = channel_id;
    ret = vm_https_send_request(0,							// Request ID
                                VM_HTTPS_METHOD_GET,		// HTTP Method Constant
                                VM_HTTPS_OPTION_NO_CACHE,	// HTTP request options
                                VM_HTTPS_DATA_TYPE_BUFFER,	// Reply type (wps_data_type_enum)
								REPLY_SEGMENT_LENGTH,		// bytes of data to be sent in reply at a time.
                                 	 	 	 	 	 	 	// If data is more that this, multiple response would be there
								(VMUINT8 *)g_https_url,		// The request URL
                                strlen(g_https_url),		// The request URL length
                                NULL,                		// The request header
                                0,                   		// The request header length
                                NULL,						// post segment
                                0							// post segment length
								);

    if(ret != 0) {
        vm_https_unset_channel(channel_id);
    }
    vm_log_debug("[HTTPS] GET req to %s, result %d\n", g_https_url, ret);
}

//----------------------------------------------------------------------------------------------------
static void https_post_request_set_channel_rsp_cb(VMUINT32 req_id, VMUINT8 channel_id, VMUINT8 result)
{
    VMINT ret = -1;

	ret = vm_https_send_request(0,                            // Request ID
								VM_HTTPS_METHOD_POST,         // HTTP Method Constant
								VM_HTTPS_OPTION_NO_CACHE,     // HTTP request options
								VM_HTTPS_DATA_TYPE_BUFFER,    // Reply type (wps_data_type_enum)
								REPLY_SEGMENT_LENGTH,		  // bytes of data to be sent in reply at a time.
															  // If data is more that this, multiple response would be there
								g_https_url,                  // The request URL
								strlen(g_https_url),          // The request URL length
								CONTENT_TYPE_WWWFORM,         // The request header
								strlen(CONTENT_TYPE_WWWFORM), // The request header length
								g_postdata,					  // Post segment
								g_postdata_len);		 	  // Post segment length

    if(ret != 0) {
        vm_https_unset_channel(channel_id);
    }
    vm_log_debug("[HTTPS] POST req to %s, result %d\n", g_https_url, ret);
}

//------------------------------------------------------------------------
static void https_unset_channel_rsp_cb(VMUINT8 channel_id, VMUINT8 result)
{
    vm_log_debug("https_unset_channel_rsp_cb()");
    g_request_id = -1;
}

//-----------------------------------------------------------
static void https_send_release_all_req_rsp_cb(VMUINT8 result)
{
    vm_log_debug("https_send_release_all_req_rsp_cb()");
}

//---------------------------------------------
static void https_send_termination_ind_cb(void)
{
    vm_log_debug("https_send_termination_ind_cb()");
}

//--------------------------------------------------------------------
static void https_send_read_request_rsp_cb(VMUINT16 request_id,
                                           VMUINT8 result,
                                           VMUINT16 status,
                                           VMINT32 cause,
                                           VMUINT8 protocol,
                                           VMUINT32 content_length,
                                           VMBOOL more,
                                           VMUINT8* content_type,
                                           VMUINT8 content_type_len,
                                           VMUINT8* new_url,
                                           VMUINT32 new_url_len,
                                           VMUINT8* reply_header,
                                           VMUINT32 reply_header_len,
                                           VMUINT8* reply_segment,
                                           VMUINT32 reply_segment_len)
{
    int ret = -1;

    if(result != 0) {
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
    } else {
        g_request_id = request_id;
    	if (g_https_header_cb_ref != LUA_NOREF) {
			int i;
			luaL_Buffer b;

			lua_rawgeti(L, LUA_REGISTRYINDEX, g_https_header_cb_ref);
			if ((lua_type(L, -1) != LUA_TFUNCTION) && (lua_type(L, -1) != LUA_TLIGHTFUNCTION)) {
			  // * BAD CB function reference
			  lua_remove(L, -1);
			}
			else {
				luaL_buffinit(L, &b);
				for(i = 0; i < reply_header_len; i++) {
					luaL_addchar(&b, reply_header[i]);
				}
				luaL_pushresult(&b);
				lua_call(L, 1, 0);
			}
    	}
    	else {
    		fputs("\n--- Header: ---\n", stdout);
			for(int i = 0; i < reply_header_len; i++) {
				fputc(reply_header[i], stdout);
			}
    		fputs("\n---------------\n", stdout);
			fflush(stdout);
    	}

    	if (g_https_response_cb_ref != LUA_NOREF) {
			int i;
			luaL_Buffer b;

			lua_rawgeti(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);
			if ((lua_type(L, -1) != LUA_TFUNCTION) && (lua_type(L, -1) != LUA_TLIGHTFUNCTION)) {
			  // * BAD CB function reference
			  lua_remove(L, -1);
			}
			else {
				luaL_buffinit(L, &b);
				for(i = 0; i < reply_segment_len; i++) {
					luaL_addchar(&b, reply_segment[i]);
				}
				luaL_pushresult(&b);
				lua_pushinteger(L, more);
				lua_call(L, 2, 0);
			}
    	}
    	else {
			for(int i = 0; i < reply_segment_len; i++) {
				fputc(reply_segment[i], stdout);
			}
			fflush(stdout);
    	}

        if(more) {
            ret = vm_https_read_content(request_id, ++g_read_seg_num, 128);
            if(ret != 0) {
                vm_https_cancel(request_id);
                vm_https_unset_channel(g_channel_id);
            }
        }
    }
}

//-------------------------------------------------------------------------
static void https_send_read_read_content_rsp_cb(VMUINT16 request_id,
                                                VMUINT8 seq_num,
                                                VMUINT8 result,
                                                VMBOOL more,
                                                VMUINT8* reply_segment,
                                                VMUINT32 reply_segment_len)
{
    int ret = -1;

	if (g_https_response_cb_ref != LUA_NOREF) {
		int i;
		luaL_Buffer b;

		lua_rawgeti(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);
		if ((lua_type(L, -1) != LUA_TFUNCTION) && (lua_type(L, -1) != LUA_TLIGHTFUNCTION)) {
		  // * BAD CB function reference
		  lua_remove(L, -1);
		}
		else {
			luaL_buffinit(L, &b);
			for(i = 0; i < reply_segment_len; i++) {
				luaL_addchar(&b, reply_segment[i]);
			}

			luaL_pushresult(&b);
			lua_pushinteger(L, more);

			lua_call(L, 2, 0);
		}
	}
	else {
		for(int i = 0; i < reply_segment_len; i++) {
			fputc(reply_segment[i], stdout);
		}
		fflush(stdout);
	}

    if(more > 0) {
        ret = vm_https_read_content(request_id,       /* Request ID */
                                    ++g_read_seg_num, /* Sequence number (for debug purpose) */
                                    128); /* The suggested segment data length of replied data in the peer buffer of
                                             response. 0 means use reply_segment_len in MSG_ID_WPS_HTTP_REQ or
                                             read_segment_length in previous request. */
        if(ret != 0) {
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
        }
    } else {
        /* don't want to send more requests, so unset channel */
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
        g_channel_id = 0;
        g_read_seg_num = 0;
    }
}

//-----------------------------------------------------------------------
static void https_send_cancel_rsp_cb(VMUINT16 request_id, VMUINT8 result)
{
    vm_log_debug("https_send_cancel_rsp_cb()");
}

//--------------------------------------------------------
static void https_send_status_query_rsp_cb(VMUINT8 status)
{
    vm_log_debug("https_send_status_query_rsp_cb()");
}

//=========================
int https_get(lua_State* L)
{
	int ret = 0;
    vm_https_callbacks_t callbacks = { (vm_https_set_channel_response_callback)https_send_request_set_channel_rsp_cb,
    								   (vm_https_unset_channel_response_callback)https_unset_channel_rsp_cb,
									   (vm_https_release_all_request_response_callback)https_send_release_all_req_rsp_cb,
									   (vm_https_termination_callback)https_send_termination_ind_cb,
                                       (vm_https_send_response_callback)https_send_read_request_rsp_cb,
									   (vm_https_read_content_response_callback)https_send_read_read_content_rsp_cb,
                                       (vm_https_cancel_response_callback)https_send_cancel_rsp_cb,
									   (vm_https_status_query_response_callback)https_send_status_query_rsp_cb };

    char* url = luaL_checkstring(L, 1);
    strncpy(g_https_url, url, sizeof(g_https_url));

    vm_https_register_context_and_callback(gprs_bearer_type, &callbacks);
    if (ret != 0) {
        l_message(NULL, "register context & cb failed");
    }
    else {
    	ret = vm_https_set_channel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    lua_pushinteger(L, ret);

	return 1;
}

//==========================
int https_post(lua_State* L)
{
	int ret;
    vm_https_callbacks_t callbacks = { (vm_https_set_channel_response_callback)https_post_request_set_channel_rsp_cb,
    								   (vm_https_unset_channel_response_callback)https_unset_channel_rsp_cb,
									   (vm_https_release_all_request_response_callback)https_send_release_all_req_rsp_cb,
									   (vm_https_termination_callback)https_send_termination_ind_cb,
                                       (vm_https_send_response_callback)https_send_read_request_rsp_cb,
									   (vm_https_read_content_response_callback)https_send_read_read_content_rsp_cb,
                                       (vm_https_cancel_response_callback)https_send_cancel_rsp_cb,
									   (vm_https_status_query_response_callback)https_send_status_query_rsp_cb };

    char* url = luaL_checkstring(L, 1);
    strncpy(g_https_url, url, sizeof(g_https_url));

    size_t sl;
    const char* pdata = luaL_checklstring(L, 2, &sl);
    if ((sl <= 0) || (pdata == NULL)) {
        return luaL_error(L, "wrong post data");
    }
    if (g_postdata != NULL) {
    	vm_free(g_postdata);
    }
    g_postdata = vm_malloc(sl+1);
    if (g_postdata == NULL) {
        return luaL_error(L, "buffer allocation error");
    }

    strncpy(g_postdata, pdata, sl);
    g_postdata[sl] = '\0';
    g_postdata_len = sl;

    if (lua_gettop(L) >= 3) {
    	_post_type = luaL_checkinteger(L, 3);
    }
    else _post_type = 0;

    ret = vm_https_register_context_and_callback(gprs_bearer_type, &callbacks);
    if (ret != 0) {
        l_message(NULL, "register context & cb failed");
    }
    else {
    	ret = vm_https_set_channel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    lua_pushinteger(L, ret);

	return 1;
}


//=================================
static int https_on( lua_State* L )
{
	size_t sl;
    const char *method = NULL;

	method = luaL_checklstring( L, 1, &sl );
	if (method == NULL) {
	  l_message(NULL, "Method string expected" );
	  return 0;
	}

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION))
	  lua_pushvalue(L, 2);
	else {
	  l_message(NULL, "Function arg expected" );
	  return 0;
	}

	if (strcmp(method,"response") == 0) {
		if (g_https_response_cb_ref != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);
		}
		g_https_response_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else if (strcmp(method,"header") == 0) {
		if (g_https_header_cb_ref != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, g_https_header_cb_ref);
		}
		g_https_header_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else {
	  lua_pop(L, 1);
	  l_message(NULL, "Unknown method" );
	}

	return 0;
}

//=====================================
static int https_cancel( lua_State* L )
{
	if (g_request_id >= 0) {
		vm_https_cancel(g_request_id);
	}
	vm_https_unset_channel(g_channel_id);
	g_channel_id = 0;
	g_read_seg_num = 0;

	return 0;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE https_map[] = {
		{ LSTRKEY("get"), LFUNCVAL(https_get)},
		{ LSTRKEY("post"), LFUNCVAL(https_post)},
		{ LSTRKEY("on"), LFUNCVAL(https_on)},
		{ LSTRKEY("cancel"), LFUNCVAL(https_cancel)},
        { LNILKEY, LNILVAL }
};


LUALIB_API int luaopen_https(lua_State* L)
{
    luaL_register(L, "https", https_map);
    return 1;
}
