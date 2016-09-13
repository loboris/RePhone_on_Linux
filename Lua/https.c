
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

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

#define REPLY_SEGMENT_LENGTH 512
#define CONTENT_TYPE_WWWFORM  "Content-Type: application/x-www-form-urlencoded"
#define CONTENT_TYPE_FORMDATA "Content-Type: multipart/form-data"
#define CONTENT_TYPE_MIXED "Content-Type: multipart/mixed"
#define CONTENT_TYPE_OCTET "application/octet-stream"
#define CONTENT_TYPE_TEXT "text/plain"

extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

static int g_channel_id = 0;
static int g_request_id = -1;
static int g_read_seg_num;

static int g_https_header_cb_ref = LUA_NOREF;
static int g_https_response_cb_ref = LUA_NOREF;
static int g_postdata_len = 0;
static int g_post_type = 0;
static int g_http_wait = 0;
static cb_func_param_httpsdata_t https_cb_params_data;
static cb_func_param_httpsheader_t https_cb_params_header;

// dinamicaly allocated variables
static VMUINT8 *g_https_url = NULL;
static char    *g_https_field_names = NULL;
static char    *g_https_file_names = NULL;
static short   *g_https_path_names = NULL;
static char    *g_postdata = NULL;
static char    *g_postdata_ptr = NULL;
static unsigned char *g_post_content = NULL;

static vm_https_request_context_t *g_post_context = NULL;


//-------------------------------------
static void free_buffers(int free_type)
{
	// ** Free post buffers
	vm_free(g_post_context);
	g_post_context = NULL;
	vm_free(g_post_content);
	g_post_content = NULL;
	vm_free(g_postdata);
	g_postdata = NULL;
	vm_free(g_https_field_names);
	g_https_field_names = NULL;
	vm_free(g_https_file_names);
	g_https_file_names = NULL;
	vm_free(g_https_path_names);
	g_https_path_names = NULL;

	g_postdata_ptr = NULL;

	vm_free(g_https_url);
	g_https_url = NULL;

	if ((free_type > 0) && (https_cb_params_data.busy == 0)) {
		// Clear all used buffers, close file, send CB request if necessary
	    https_cb_params_data.state = -1 * https_cb_params_data.state; // error
		if (https_cb_params_data.ffd >= 0) {
			// Receiving to file, close it
			vm_fs_close(https_cb_params_data.ffd);
			https_cb_params_data.ffd = -1;
		}
		else if (https_cb_params_data.reply != NULL) {
			// Receiving to buffer
			https_cb_params_data.reply[https_cb_params_data.len] = '\0';
		}
		if (g_https_response_cb_ref != LUA_NOREF) {
			https_cb_params_data.cb_ref = g_https_response_cb_ref;
			https_cb_params_data.busy = 1;
			remote_lua_call(CB_FUNC_HTTPS_DATA, &https_cb_params_data);
		}
	}
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

    if (ret != 0) {
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
    // Free all used data
    free_buffers(2);
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


//----------------------------------------------------------------------------------------
static void data_received(VMUINT8* reply_segment, VMUINT32 reply_segment_len, VMBOOL more)
{
	if (https_cb_params_data.ffd >= 0) {
		// ** Receive to file **
	    VMUINT written_bytes = 0;
		https_cb_params_data.len += reply_segment_len;
    	int res = vm_fs_write(https_cb_params_data.ffd, reply_segment, reply_segment_len, &written_bytes);

		if ((more == 0) || (res < 0)) {
        	vm_fs_close(https_cb_params_data.ffd);
        	https_cb_params_data.ffd = -1;
			if (res < 0) https_cb_params_data.more = -1;
			else https_cb_params_data.more = 0;
		}
	}
	else if (https_cb_params_data.reply != NULL) {
		// ** Receive to buffer **
		VMUINT8 c;
		for (int i = 0; i < reply_segment_len; i++) {
			if (https_cb_params_data.len < https_cb_params_data.maxlen) {
				c = reply_segment[i];
				if ((c >= 0x7F) || ((c < 0x20) && ((c != 0x0A) && (c != 0x0D) && (c != 0x09)))) {
					// non printable char, possible binary data
					https_cb_params_data.state = https_cb_params_data.state = 3;
				}
				https_cb_params_data.reply[https_cb_params_data.len++] = c;
			}
			else https_cb_params_data.more++;
		}
		https_cb_params_data.reply[https_cb_params_data.len] = '\0';
	}
	if (more == 0) {
		// Last data received
		if (g_https_response_cb_ref != LUA_NOREF) {
			if (https_cb_params_data.busy == 0) {
				// Send Callback function request
				https_cb_params_data.cb_ref = g_https_response_cb_ref;
				https_cb_params_data.busy = 1;
				remote_lua_call(CB_FUNC_HTTPS_DATA, &https_cb_params_data);
			}
		}
		else if (g_http_wait) {
			// get/post function waiting for response
			https_cb_params_data.busy = 1;
			vm_signal_post(g_shell_signal);
		}
	}
}

// First response from server is received,
// header and 1st part of the response data
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

    if (result != 0) {
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
    }
    else {
        g_request_id = request_id;
        // Header received, call cb function if registered
    	if (g_https_header_cb_ref != LUA_NOREF) {
    		if (https_cb_params_header.busy == 0) {
    			https_cb_params_header.busy = 0;
				https_cb_params_header.cb_ref = g_https_header_cb_ref;
				https_cb_params_header.header = reply_header;
				https_cb_params_header.len = reply_header_len;
				remote_lua_call(CB_FUNC_HTTPS_HEADER, &https_cb_params_header);
    		}
    	}

    	// Process received data, init CB function if necessary
    	data_received(reply_segment, reply_segment_len, more);

        if (more) {
            ret = vm_https_read_content(request_id, ++g_read_seg_num, REPLY_SEGMENT_LENGTH);
            if (ret != 0) {
                vm_https_cancel(request_id);
                vm_https_unset_channel(g_channel_id);
            }
        }
        else {
            // last data received, unset channel
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
            g_channel_id = 0;
            g_read_seg_num = 0;
        }
    }
}

// More response data is received from server
//-------------------------------------------------------------------------
static void https_send_read_read_content_rsp_cb(VMUINT16 request_id,
                                                VMUINT8 seq_num,
                                                VMUINT8 result,
                                                VMBOOL more,
                                                VMUINT8* reply_segment,
                                                VMUINT32 reply_segment_len)
{
    int ret = -1;

	// Process received data, init CB function if necessary
	data_received(reply_segment, reply_segment_len, more);

    if (more > 0) {
    	// Request next data chunk
        ret = vm_https_read_content(request_id,				// Request ID
                                    ++g_read_seg_num,		// Sequence number (for debug purpose)
									REPLY_SEGMENT_LENGTH);	// The suggested segment data length of replied data in the peer buffer of
                                             	 	 	 	// response. 0 means use reply_segment_len in MSG_ID_WPS_HTTP_REQ or
                                             	 	 	 	// read_segment_length in previous request.
        if (ret != 0) {
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
        }
    }
    else {
        // last data received, unset channel
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
        g_channel_id = 0;
        g_read_seg_num = 0;
    }
}

//----------------------------------------------------------------
int https_get_post(lua_State* L, int index, lua_CFunction cb_func)
{
	if (https_cb_params_data.state != 0) {
		vm_log_error("Https busy.");
    	return https_cb_params_data.state;
	}

	/*
	// CB function must be defined
	if (g_https_response_cb_ref == LUA_NOREF) {
		vm_log_error("Callback function not set.");
    	return -1;
	}
	*/

	// Check url, will be handled later
    if (lua_type(L, 1) != LUA_TSTRING) {
		vm_log_error("URL missing.");
    	return -2;
    }
    if (index == 3) {
    	// POST function, check post argument
        if ((lua_type(L, 2) != LUA_TSTRING) && (lua_type(L, 2) != LUA_TTABLE)) {
    		vm_log_error("POST data missing.");
        	return -3;
		}
    }

	g_http_wait = 0;
	int max_wait_time = 30000;

	// Free data buffer if necessary
    if (https_cb_params_data.reply != NULL) {
    	free(https_cb_params_data.reply);
    	https_cb_params_data.reply = NULL;
    }

    // Init some variables
    https_cb_params_data.ffd = -1;
	https_cb_params_data.maxlen = 16*1024;
    https_cb_params_data.state = 1; // get to buffer
	https_cb_params_data.busy = 0;

	// Check optional parameters
	int rec_type = 0;

	if (lua_gettop(L) > index) {
	    if (lua_type(L, index+1) == LUA_TNUMBER) {
	    	max_wait_time = luaL_checkinteger(L, index+1);
	    	if (max_wait_time < 5) max_wait_time = 5;
	    	if (max_wait_time > 120) max_wait_time = 120;
	    	max_wait_time *= 1000;
	    }
	}
    if (lua_type(L, index) == LUA_TSTRING) {
    	int rec_type = 1;
    	// File name is given
        const char* fname = luaL_checkstring(L, index);
        https_cb_params_data.ffd = file_open(fname, O_CREAT);
        if (https_cb_params_data.ffd < 0) {
    		vm_log_error("File open failed.");
    		https_cb_params_data.ffd = -1;
    	    https_cb_params_data.state = 0; // free
        	return -4;
        }
        // ** receive to file **
	    https_cb_params_data.state = 2;
    }
    else if (lua_type(L, index) == LUA_TNUMBER) {
    	// Buffer size given
    	https_cb_params_data.maxlen = luaL_checkinteger(L, index);
    	// Set max buffer length
    	if (https_cb_params_data.maxlen < 1024) https_cb_params_data.maxlen = 1024;
    	else if (https_cb_params_data.maxlen > (200 * 1024)) https_cb_params_data.maxlen = 128*1024;
    }
    if (rec_type == 0) {
		// ** Receive to buffer **, allocate response buffer
		https_cb_params_data.reply = malloc(https_cb_params_data.maxlen+1);
		if (https_cb_params_data.reply != NULL) {
			https_cb_params_data.len = 0;
			https_cb_params_data.busy = 0;
			https_cb_params_data.more = 0;
		}
		else {
			vm_log_error("Memory allocation failed");
			return -5;
		}
    }

    // ** Initiate get/post request ***
	remote_CCall(L, cb_func);
	if (g_shell_result < 0)	return g_shell_result;

	if (g_https_response_cb_ref == LUA_NOREF) {
		// *** Response cb function not registered, wait for response ***
		g_http_wait = 1;
	    if (vm_signal_timed_wait(g_shell_signal, max_wait_time) != 0) g_shell_result = -978;
	    else g_shell_result = 978;
		g_http_wait = 0;
	}

	return g_shell_result;
}

//=========================
int _https_get(lua_State* L)
{
    vm_https_callbacks_t callbacks = { (vm_https_set_channel_response_callback)https_send_request_set_channel_rsp_cb,
    								   (vm_https_unset_channel_response_callback)https_unset_channel_rsp_cb,
									   (vm_https_release_all_request_response_callback)https_send_release_all_req_rsp_cb,
									   (vm_https_termination_callback)https_send_termination_ind_cb,
                                       (vm_https_send_response_callback)https_send_read_request_rsp_cb,
									   (vm_https_read_content_response_callback)https_send_read_read_content_rsp_cb,
                                       (vm_https_cancel_response_callback)https_send_cancel_rsp_cb,
									   (vm_https_status_query_response_callback)https_send_status_query_rsp_cb };
    // Get url
	size_t sl;
	const char* url = luaL_checklstring(L, 1, &sl);
	// Allocate url memory and copy url
	vm_free(g_https_url);
	g_https_url = vm_calloc(sl+1);
	if (g_https_url == NULL) {
		vm_log_error("Allocating URL buffer failed");
		free_buffers(1);
		g_shell_result = -6;
	}
	else {
		strncpy(g_https_url, url, sl);

		g_shell_result = vm_https_register_context_and_callback(gprs_bearer_type, &callbacks);
		if (g_shell_result != 0) {
		    // Free all used data
		    free_buffers(1);
		    g_shell_result = -7;
			vm_log_error("Register context & cb failed");
		}
		else {
			g_shell_result = vm_https_set_channel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			if (g_shell_result != 0) {
			    // Free all used data
			    free_buffers(1);
			    g_shell_result = -8;
				vm_log_error("Set channel failed");
			}
		}
	}

	vm_signal_post(g_shell_signal);

	return 1;
}

//===========================
int _https_post(lua_State* L)
{
	int ret;
	size_t sl;

	vm_https_callbacks_t callbacks = { (vm_https_set_channel_response_callback)https_post_request_set_channel_rsp_cb,
    								   (vm_https_unset_channel_response_callback)https_unset_channel_rsp_cb,
									   (vm_https_release_all_request_response_callback)https_send_release_all_req_rsp_cb,
									   (vm_https_termination_callback)https_send_termination_ind_cb,
                                       (vm_https_send_response_callback)https_send_read_request_rsp_cb,
									   (vm_https_read_content_response_callback)https_send_read_read_content_rsp_cb,
                                       (vm_https_cancel_response_callback)https_send_cancel_rsp_cb,
									   (vm_https_status_query_response_callback)https_send_status_query_rsp_cb };
	// Get URL
    const char* url = luaL_checklstring(L, 1, &sl);

    free_buffers(0);

    g_https_url = vm_calloc(sl+1);
	if (g_https_url == NULL) {
	    vm_log_error("URL buffer allocation error");
	    free_buffers(1);
	    ret = -10;
		goto exit;
	}
    strncpy(g_https_url, url, sl);
    int totalalloc = (sl+1);

	if (lua_isstring(L, 2)) {
		g_post_type = 0;
		const char* pdata = luaL_checklstring(L, 2, &sl);
		if ((sl <= 0) || (pdata == NULL)) {
		    free_buffers(1);
		    vm_log_error("Wrong string post data");
		    ret = -11;
		    goto exit;
		}
		g_postdata = vm_calloc(sl+1);
		if (g_postdata == NULL) {
		    free_buffers(1);
		    vm_log_error("Post data buffer allocation error");
		    ret = -12;
		    goto exit;
		}
	    totalalloc += (sl+1);

		strncpy(g_postdata, pdata, sl);
		g_postdata[sl] = '\0';
		g_postdata_len = sl;
	    vm_log_debug("[POST string]: allocated memory = %d", totalalloc);
    }
    else if (lua_istable(L, 2)) {
		g_post_type = 1;

		// ==== iterate the table to find number of entries and total data size ====
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

						int ffnd = 1;
						VMWCHAR wfn[128];
					    full_fname((char *)value, wfn, 128);
					    // Check if file exists
						vm_fs_info_ex_t fileinfoex;

					    VM_FS_HANDLE fhex = vm_fs_find_first_ex((VMWSTR)wfn, &fileinfoex);
					    if (fhex >= 0) {
					    	if (fileinfoex.attributes & VM_FS_ATTRIBUTE_DIRECTORY) ffnd = 0;
					    	vm_fs_find_close_ex(fhex);
					    }
					    else ffnd = 0;
						if (ffnd) {
							int fnmlen = (vm_wstr_string_length((VMWSTR)wfn)+1)*2;
							pathnames_size += fnmlen;
						}
						else {
							vm_log_error("[POST]: File '%s' not found", value);
							err++;
						}
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
	        free_buffers(1);
	        vm_log_error("No post entries");
		    ret = -13;
	        goto exit;
	    }
	    if (err != 0) {
	        free_buffers(1);
		    ret = -14;
	        goto exit;
	    }

		// ** Allocate buffers
        g_post_context = vm_calloc(sizeof(vm_https_request_context_t));
		if (g_post_context == NULL) {
		    free_buffers(1);
		    vm_log_error("POST context allocation error");
		    ret = -15;
		    goto exit;
		}
	    totalalloc += sizeof(vm_https_request_context_t);

	    g_post_content = vm_calloc(sizeof(vm_https_content_t)*nentries);
 		if (g_post_content == NULL) {
 		    free_buffers(1);
 		    vm_log_error("POST content allocation error");
 		    ret = -16;
 	        goto exit;
    	}
	    totalalloc += (sizeof(vm_https_content_t)*nentries);

	    if (pdata_size > 0) {
			g_postdata = vm_calloc(pdata_size);
			if (g_postdata == NULL) {
				free_buffers(1);
				vm_log_error("POST data allocation error");
			    ret = -17;
			    goto exit;
			}
		    totalalloc += pdata_size;
 		}

	    g_https_field_names = vm_calloc(fnames_size);
  		if (g_https_field_names == NULL) {
  		    free_buffers(1);
  		    vm_log_error("POST field names allocation error");
  		    ret = -18;
  		    goto exit;
  		}
	    totalalloc += fnames_size;

	    if (filenames_size > 0) {
			g_https_file_names = vm_calloc(filenames_size);
			if (g_https_file_names == NULL) {
				free_buffers(1);
				vm_log_error("POST file names allocation error");
	  		    ret = -19;
			    goto exit;
			}
		    totalalloc += filenames_size;
  		}
  		if (pathnames_size > 0) {
			g_https_path_names = vm_calloc(pathnames_size);
			if (g_https_path_names == NULL) {
				free_buffers(1);
				vm_log_error("POST path names allocation error");
	  		    ret = -20;
			    goto exit;
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

		// ==== iterate the table and populate data buffers ====
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
					short wfn[128];
				    full_fname((char *)value, wfn, 128);
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

	// Initiate POST request
    ret = vm_https_register_context_and_callback(gprs_bearer_type, &callbacks);
    if (ret != 0) {
		free_buffers(1);
        vm_log_error("Register context & cb failed");
	    ret = -25;
    }
    else {
    	ret = vm_https_set_channel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        if (ret != 0) {
    		free_buffers(1);
            vm_log_debug("Set channel failed");
    	    ret = -26;
        }
    }

exit:
    g_shell_result = ret;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=====================================
static int _https_cancel( lua_State* L )
{
	if (g_request_id >= 0) {
		vm_https_cancel(g_request_id);
	}
	vm_https_unset_channel(g_channel_id);
	g_channel_id = 0;
	g_read_seg_num = 0;

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=====================================
static int https_cancel( lua_State* L )
{
	remote_CCall(L, &_https_cancel);
	return g_shell_result;
}

//------------------------------------------
static int _https_end(lua_State* L, int res)
{
	if (abs(res) == 978) {
		// ** waiting for response
		int n = 2;
		if (res == 978) {
			// response received
			lua_pushinteger(L, https_cb_params_data.len);
			if (https_cb_params_data.reply != NULL) {
				lua_pushlstring(L, https_cb_params_data.reply, https_cb_params_data.len);
				free(https_cb_params_data.reply);
			}
			else {
				lua_pushstring(L, "__Receive_To_File__");
			}
		}
		else {
			// timeout
			remote_CCall(L, &_https_cancel);
			lua_pushinteger(L, -6);
			n = 1;
			vm_log_error("Timeout waiting for response");
		}
	    vm_thread_sleep(50);
	    https_cb_params_data.reply = NULL;
	    https_cb_params_data.maxlen = 0;
	    https_cb_params_data.len = 0;
	    https_cb_params_data.more = 0;
	    https_cb_params_data.busy = 0;
	    https_cb_params_data.state = 0;
	    return n;
	}
	else {
		lua_pushinteger(L, res);
		return 1;
	}
}

// https.get(url [,fname | bufsize])
//===============================
static int https_get(lua_State* L)
{
	int res = https_get_post(L, 2, &_https_get);
	return _https_end(L, res);
}

// https.get(url, post_data [,fname | bufsize])
//==========================
int https_post(lua_State* L)
{
	int res = https_get_post(L, 3, &_https_post);
	return _https_end(L, res);
}

//=================================
static int https_on( lua_State* L )
{
	int pcb = 0;
	int res = -1;
	size_t sl;
    const char *method = NULL;

	method = luaL_checklstring( L, 1, &sl );
	if (method == NULL) {
	  vm_log_error("Method string expected");
	  return 0;
	}

	if (strcmp(method,"response") == 0) {
		if (g_https_response_cb_ref != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);
			pcb = 1;
			res = 0;
		}
		if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
			g_https_response_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
			vm_log_debug("Response CB registered");
			res = 1;
		}
		else if (pcb) vm_log_debug("Response CB unregistered");
	}
	else if (strcmp(method,"header") == 0) {
		if (g_https_header_cb_ref != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, g_https_header_cb_ref);
			pcb = 1;
			res = 0;
		}
		if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
			g_https_header_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
			vm_log_debug("Header CB registered");
			res = 1;
		}
		else if (pcb) vm_log_debug("Header CB unregistered");
	}
	else {
	  lua_pop(L, 1);
	  vm_log_error("Unknown method");
	}

	lua_pushinteger(L, res);
	return 1;
}

//=======================================
static int https_getstate( lua_State* L )
{
	lua_pushinteger(L, https_cb_params_data.state);
	return 1;
}




#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE https_map[] = {
		{ LSTRKEY("get"), LFUNCVAL(https_get)},
		{ LSTRKEY("post"), LFUNCVAL(https_post)},
		{ LSTRKEY("on"), LFUNCVAL(https_on)},
		{ LSTRKEY("cancel"), LFUNCVAL(https_cancel)},
		{ LSTRKEY("getstate"), LFUNCVAL(https_getstate)},
        { LNILKEY, LNILVAL }
};


LUALIB_API int luaopen_https(lua_State* L)
{
	https_cb_params_data.reply = NULL;
	https_cb_params_data.maxlen = 0;
	https_cb_params_data.ffd = -1;
	https_cb_params_data.busy = 0;
	https_cb_params_data.state = 0;

	luaL_register(L, "https", https_map);
    return 1;
}
