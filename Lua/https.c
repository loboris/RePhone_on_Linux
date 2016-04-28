
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
#define CONTENT_TYPE_MIXED "Content-Type: multipart/mixed"
#define CONTENT_TYPE_OCTET "application/octet-stream"
#define CONTENT_TYPE_TEXT "text/plain"

extern lua_State* L;
extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

static int g_channel_id = 0;
static int g_request_id = -1;
static int g_read_seg_num;

static int g_https_header_cb_ref = LUA_NOREF;
static int g_https_response_cb_ref = LUA_NOREF;
static int g_postdata_len = 0;
static int g_post_type = 0;

// dinamicaly allocated variables
static VMUINT8 *g_https_url = NULL;
static char    *g_https_field_names = NULL;
static char    *g_https_file_names = NULL;
static short   *g_https_path_names = NULL;
static char    *g_postdata = NULL;
static char    *g_postdata_ptr = NULL;
static unsigned char *g_post_content = NULL;

static vm_https_request_context_t *g_post_context = NULL;


//---------------------------------
static void free_post_buffers(void)
{
	// ** Free post buffers
	vm_free(g_post_context);
	g_post_context = NULL;
	vm_free(g_post_content);
	g_post_content = NULL;
	vm_free(g_postdata);
	g_postdata = NULL;
	vm_free(g_https_url);
	g_https_url = NULL;
	vm_free(g_https_field_names);
	g_https_field_names = NULL;
	vm_free(g_https_file_names);
	g_https_file_names = NULL;
	vm_free(g_https_path_names);
	g_https_path_names = NULL;

	g_postdata_ptr = NULL;
}

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
								g_https_url,				// The request URL
                                strlen(g_https_url),		// The request URL length
                                NULL,                		// The request header
                                0,                   		// The request header length
                                NULL,						// post segment
                                0							// post segment length
								);

    if(ret != 0) {
        vm_https_unset_channel(channel_id);
    }
    vm_log_debug("[HTTPS] GET req to %s, result %d", g_https_url, ret);
}


//----------------------------------------------------------------------------------------
static void _post_cb(VMUINT16 request_id, VMUINT8 sequence_number, VM_HTTPS_RESULT result)
{
	// this runs for every part of multipart data which has 'data_type' == VM_HTTPS_DATA_TYPE_BUFFER
	if (g_postdata_ptr >= (g_postdata + g_postdata_len)) return;

	// send the actual field data
	// we can send field data in 1 or more 'sequences', for the last one we set more = 0
	int more = 0;
	int dlen = 0;
	memcpy(&dlen, g_postdata_ptr, sizeof(int));
	vm_https_post(request_id, sequence_number, more, g_postdata_ptr+sizeof(int), dlen);

    g_postdata_ptr += (dlen + sizeof(int));  // next data
}

//----------------------------------------------------------------------------------------------------
static void https_post_request_set_channel_rsp_cb(VMUINT32 req_id, VMUINT8 channel_id, VMUINT8 result)
{
    VMINT ret = -1;
    if (g_post_type == 0) {
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
	    vm_log_debug("[POST] to %s, result=%d", g_https_url, ret);
    }
    else {
        g_post_context->content = (vm_https_content_t *)g_post_content;
        g_postdata_ptr = g_postdata;

		ret = vm_https_request_ext(0,                             // Request ID
									VM_HTTPS_METHOD_POST,         // HTTP Method Constant
									VM_HTTPS_OPTION_NO_CACHE,     // HTTP request options
									VM_HTTPS_DATA_TYPE_BUFFER,    // Reply type (wps_data_type_enum)
									REPLY_SEGMENT_LENGTH,		  // bytes of data to be sent in reply at a time.
																  // If data is more that this, multiple response would be there
									VM_TRUE,                      // more posts
									VM_HTTPS_DATA_TYPE_MULTIPART, // post type
									g_post_context,               // The request data
									_post_cb);				      // The callback of post status
    	vm_log_debug("[POST] to '%s', nseg=%d, result=%d", g_post_context->url, g_post_context->number_entries, ret);
    }

    if(ret != 0) {
        vm_https_unset_channel(channel_id);
    }
}

//------------------------------------------------------------------------
static void https_unset_channel_rsp_cb(VMUINT8 channel_id, VMUINT8 result)
{
    vm_log_debug("https_unset_channel_rsp_cb()");
    g_request_id = -1;
    free_post_buffers();
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

	size_t sl;
    const char* url = luaL_checklstring(L, 1, &sl);

    vm_free(g_https_url);
    g_https_url = vm_calloc(sl+1);
    strncpy(g_https_url, url, sl);

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

	size_t sl;
    const char* url = luaL_checklstring(L, 1, &sl);

    free_post_buffers();

    g_https_url = vm_calloc(sl+1);
	if (g_https_url == NULL) {
		return luaL_error(L, "url buffer allocation error");
	}
    strncpy(g_https_url, url, sl);
    int totalalloc = (sl+1);

    if ((!lua_istable(L, 2)) && (!lua_isstring(L, 2))) {
      free_post_buffers();
      return luaL_error(L, "POST data arg missing" );
    }

	if (lua_isstring(L, 2)) {
		g_post_type = 0;
		const char* pdata = luaL_checklstring(L, 2, &sl);
		if ((sl <= 0) || (pdata == NULL)) {
		    free_post_buffers();
			return luaL_error(L, "wrong post data");
		}
		g_postdata = vm_calloc(sl+1);
		if (g_postdata == NULL) {
		    free_post_buffers();
			return luaL_error(L, "buffer allocation error");
		}
	    totalalloc += (sl+1);

		strncpy(g_postdata, pdata, sl);
		g_postdata[sl] = '\0';
		g_postdata_len = sl;
	    vm_log_debug("[POST]: allocated memory = %d", totalalloc);
    }
    else if (lua_istable(L, 2)) {
		g_post_type = 1;

		// ** iterate the table to find number of entries and total data size
		int pdata_size = 0;
		int nentries = 0;
		int fnames_size = 0;
		int filenames_size = 0;
		int pathnames_size = 0;
        int err = 0;
        int nfileentry = 0;

	    lua_pushnil(L);  // first key
	    while (lua_next(L, 2) != 0) {
	      // Pops a key from the stack, and pushes a key-value pair from the table
	      // 'key' (at index -2) and 'value' (at index -1)
	      if ((lua_isstring(L, -1)) && (lua_isstring(L, -2))) {
	        size_t klen = 0;
	        size_t vlen = 0;
	        const char* key = lua_tolstring(L, -2, &klen);
	        const char* value = lua_tolstring(L, -1, &vlen);
	        if ((klen > 0) && (vlen > 0)) {
	        	nentries++;
	        	fnames_size += (klen+1);
	  	    	if (strstr(key, "file") == key) {
	  	    		if (nfileentry < 1) {
						nfileentry++;
						filenames_size += (vlen+1);

						char fn[128];
						short wfn[128];
						sprintf(fn, "C:\\%s", value);
						vm_chset_ascii_to_ucs2((VMWSTR)wfn, 256, fn);
						int fh = vm_fs_open(wfn, VM_FS_MODE_READ, VM_TRUE);
						if (fh < 0) {
							err++;
							vm_log_debug("[POST]: File '%s' not found",fn);
						}
						else vm_fs_close(fh);
						int fnmlen = (vm_wstr_string_length(wfn)+1)*2;
						pathnames_size += fnmlen;
	  	    		}
	  	    		else {
						vm_log_debug("[POST]: Only 1 file allowed!");
	  	    		}
		        }
	  	    	else pdata_size += (vlen+sizeof(int));
	        }
	      }
	      lua_pop(L, 1);  // removes 'value'; keeps 'key' for next iteration
	    }

	    if (nentries == 0) {
	        free_post_buffers();
			return luaL_error(L, "no post entries");
	    }
	    if (err != 0) {
	        free_post_buffers();
			return luaL_error(L, "file not found");
	    }

		// ** Allocate buffers
        g_post_context = vm_calloc(sizeof(vm_https_request_context_t));
		if (g_post_context == NULL) {
		    free_post_buffers();
			return luaL_error(L, "buffer allocation error 1");
		}
	    totalalloc += sizeof(vm_https_request_context_t);

	    g_post_content = vm_calloc(sizeof(vm_https_content_t)*nentries);
 		if (g_post_content == NULL) {
 		    free_post_buffers();
			return luaL_error(L, "buffer allocation error 2");
    	}
	    totalalloc += (sizeof(vm_https_content_t)*nentries);

	    if (pdata_size > 0) {
			g_postdata = vm_calloc(pdata_size);
			if (g_postdata == NULL) {
				free_post_buffers();
				return luaL_error(L, "buffer allocation error 3");
			}
		    totalalloc += pdata_size;
 		}

	    g_https_field_names = vm_calloc(fnames_size);
  		if (g_https_field_names == NULL) {
  		    free_post_buffers();
			return luaL_error(L, "buffer allocation error 4");
  		}
	    totalalloc += fnames_size;

	    if (filenames_size > 0) {
			g_https_file_names = vm_calloc(filenames_size);
			if (g_https_file_names == NULL) {
				free_post_buffers();
				return luaL_error(L, "buffer allocation error 5");
			}
		    totalalloc += filenames_size;
  		}
  		if (pathnames_size > 0) {
			g_https_path_names = vm_calloc(pathnames_size);
			if (g_https_path_names == NULL) {
				free_post_buffers();
				return luaL_error(L, "buffer allocation error 6");
			}
		    totalalloc += pathnames_size;
  		}
	    vm_log_debug("[POST]: allocated memory = %d", totalalloc);

        sprintf(g_https_url, url);

        g_post_context->number_entries = 0;
    	g_post_context->content = (vm_https_content_t *)g_post_content;
        g_post_context->header = CONTENT_TYPE_FORMDATA;
        g_post_context->header_length = strlen(CONTENT_TYPE_FORMDATA);
        g_post_context->post_segment = NULL;
        g_post_context->post_segment_length = 0;
        g_post_context->url = g_https_url;
        g_post_context->url_length = strlen(g_post_context->url);

		// ** iterate the table and populate data buffers
		vm_https_content_t cc;
		g_postdata_len = 0;
		fnames_size = 0;
		filenames_size = 0;
		pathnames_size = 0;
		nfileentry = 0;

	    lua_pushnil(L);
	    while (lua_next(L, 2) != 0) {
	      if ((lua_isstring(L, -1)) && (lua_isstring(L, -2))) {
	        size_t klen = 0;
	        size_t vlen = 0;
	        const char* key = lua_tolstring(L, -2, &klen);
	        const char* value = lua_tolstring(L, -1, &vlen);
	        if ((klen > 0) && (vlen > 0)) {
	          // field key & value are OK
			  g_post_context->number_entries++;

  	    	  if (strstr(key, "file") == key) {
  	    		if (nfileentry < 1) {
					nfileentry++;
					cc.data_type = VM_HTTPS_DATA_TYPE_FILE;
					cc.content_type = CONTENT_TYPE_OCTET;
					cc.content_type_length = strlen(CONTENT_TYPE_OCTET);

					// save file name & length
					sprintf(g_https_file_names+filenames_size, "%s", value);
					cc.filename = g_https_file_names+filenames_size;
					cc.filename_length = vlen;
					filenames_size += (vlen+1);

					// save local file path/name & length
					char fn[128];
					short wfn[128];
					sprintf(fn, "C:\\%s", value);
					vm_chset_ascii_to_ucs2((VMWSTR)wfn, 256, fn);
					int wfnlen = vm_wstr_copy(g_https_path_names+pathnames_size, wfn);
					cc.file_path_name = g_https_path_names+pathnames_size;
					cc.file_path_name_length = vm_wstr_string_length(cc.file_path_name);
					pathnames_size += ((cc.file_path_name_length+1)*2);

					// get file size
					int fh = vm_fs_open(cc.file_path_name, VM_FS_MODE_READ, VM_TRUE);
					VMUINT fsz;
					if (vm_fs_get_size(fh, &fsz) == 0)  cc.data_length = fsz;
					else cc.data_length = 0;
					vm_fs_close(fh);
  	    		}
	          }
	          else {
		       	cc.data_type = VM_HTTPS_DATA_TYPE_BUFFER;
  		        cc.content_type = CONTENT_TYPE_TEXT;
	  		    cc.content_type_length = strlen(CONTENT_TYPE_TEXT);
    	        cc.filename = "";
    	        cc.filename_length = 0;
    	  		cc.file_path_name = (VMWSTR)"\0\0";
    		    cc.file_path_name_length = 0;
				cc.data_length = vlen;       // field data length

				int dlen = vlen;
				memcpy(g_postdata+g_postdata_len, &dlen, sizeof(int));
				memcpy(g_postdata+g_postdata_len+sizeof(int), value, vlen);
	            g_postdata_len += (vlen+sizeof(int));
	          }

  	          cc.charset = VM_HTTPS_CHARSET_ASCII;
  	          // save field name & length
  		      sprintf(g_https_field_names+fnames_size, "%s", key);
  	          cc.name = g_https_field_names+fnames_size;
  	          cc.name_length = klen;  // field name length
  	          fnames_size += (klen+1);

  	          memcpy(g_post_content + (sizeof(vm_https_content_t) * (g_post_context->number_entries-1)), &cc, sizeof(vm_https_content_t));
	        }
	      }
	      // removes 'value'; keeps 'key' for next iteration
	      lua_pop(L, 1);
	    }
    }


    ret = vm_https_register_context_and_callback(gprs_bearer_type, &callbacks);
    if (ret != 0) {
		free_post_buffers();
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
