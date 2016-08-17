

#include <string.h>
#include <time.h>

#include "vmlog.h"
#include "vmgsm_tel.h"
#include "vmchset.h"
#include "vmgsm_sim.h"
#include "vmgsm_sms.h"
#include "vmmemory.h"
#include "vmtype.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

//#define INCLUDE_CALL_FUNCTIONS

#ifdef INCLUDE_CALL_FUNCTIONS
enum VoiceCall_Status { IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };
static int g_gsm_incoming_call_cb_ref = LUA_NOREF;
static vm_gsm_tel_id_info_t g_uid_info;
static VMINT8 g_call_status = IDLE_CALL;
static VMINT8 g_incoming_number[42];
#endif

static int g_gsm_new_message_cb_ref = LUA_NOREF;
static int g_sms_list_cb_ref = LUA_NOREF;
static int g_sms_read_cb_ref = LUA_NOREF;
static int g_sms_delete_cb_ref = LUA_NOREF;
static int g_sms_send_cb_ref = LUA_NOREF;
static cb_func_param_int_t gsm_cb_params_sent;
static cb_func_param_smslist_t gsm_cb_params_list;
static cb_func_param_smsread_t gsm_cb_params_read;
static cb_func_param_int_t gsm_cb_params_delete;
static cb_func_param_smsnew_t gsm_cb_params_newsms;
static vm_gsm_sms_read_message_data_t * message_data = NULL;
static VMWCHAR * content_buff = NULL;


//static vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;


#ifdef INCLUDE_CALL_FUNCTIONS

//---------------------------------------------------------------------
static void call_listener_func(vm_gsm_tel_call_listener_data_t *data) {
  vm_log_info("call_listener_func");

  if (data->call_type == VM_GSM_TEL_INDICATION_INCOMING_CALL) {
    vm_gsm_tel_call_info_t *ind = (vm_gsm_tel_call_info_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    strcpy(g_incoming_number, (char *)ind->num_uri);
    g_call_status = RECEIVINGCALL;

    vm_log_info("incoming call");

    if (g_gsm_incoming_call_cb_ref != LUA_NOREF) {
        lua_rawgeti(shellL, LUA_REGISTRYINDEX, g_gsm_incoming_call_cb_ref);
        lua_pushstring(shellL, g_incoming_number);
        remote_lua_call(shellL, 1, 0);
    }
  }
  else if (data->call_type == VM_GSM_TEL_INDICATION_OUTGOING_CALL) {
    vm_gsm_tel_call_info_t *ind = (vm_gsm_tel_call_info_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    strcpy(g_incoming_number, (char *)ind->num_uri);
    g_call_status = CALLING;

    vm_log_info("calling");
  }
  else if (data->call_type == VM_GSM_TEL_INDICATION_CONNECTED) {
    vm_gsm_tel_connect_indication_t *ind = (vm_gsm_tel_connect_indication_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    g_call_status = TALKING;

    vm_log_info("connected");
  }
  else if (data->call_type == VM_GSM_TEL_INDICATION_CALL_ENDED) {
    g_call_status = IDLE_CALL;
    vm_log_info("endded");
  }
  else {
    vm_log_info("bad operation type");
  }
  vm_log_info("g_call_status is %d", g_call_status);
}

//----------------------------------------------------------------------------------
static void call_voiceCall_callback(vm_gsm_tel_call_actions_callback_data_t *data) {
  vm_log_info("call_voiceCall_callback");

  if (data->type_action == VM_GSM_TEL_CALL_ACTION_DIAL) {
    if (data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) g_call_status = CALLING;
    else g_call_status = IDLE_CALL;
  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_ACCEPT) {
    if (data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) g_call_status = TALKING;
    else g_call_status = IDLE_CALL;
  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_HOLD) {
  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_END_SINGLE) g_call_status = IDLE_CALL;
}

//----------------------------------------------
static int _gsm_call(const char *phone_number) {
  vm_gsm_tel_dial_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  vm_chset_ascii_to_ucs2((VMWSTR)req.num_uri, VM_GSM_TEL_MAX_NUMBER_LENGTH,
                         (VMSTR)phone_number);
  req.sim = VM_GSM_TEL_CALL_SIM_1;
  req.is_ip_dial = 0;
  req.module_id = 0;
  req.phonebook_data = NULL;

  data.action = VM_GSM_TEL_CALL_ACTION_DIAL;
  data.data_action = (void *)&req;
  data.user_data = NULL;
  data.callback = call_voiceCall_callback;

  return vm_gsm_tel_call_actions(&data);
}

//------------------------------------
static int gsm_anwser(lua_State *L) {
  int result;
  vm_gsm_tel_single_call_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  req.action_id.sim = g_uid_info.sim;
  req.action_id.call_id = g_uid_info.call_id;
  req.action_id.group_id = g_uid_info.group_id;

  data.action = VM_GSM_TEL_CALL_ACTION_ACCEPT;
  data.data_action = (void *)&req;
  data.user_data = NULL;
  data.callback = call_voiceCall_callback;

  result = vm_gsm_tel_call_actions(&data);

  lua_pushnumber(L, result);

  return 1;
}

//=================================
static int gsm_call(lua_State *L) {
  const char *phone_number = luaL_checkstring(L, 1);
  int result = _gsm_call(phone_number);
  lua_pushnumber(L, result);

  return 1;
}

//=================================
static int gsm_hang(lua_State *L) {
  int result = 0;
  vm_gsm_tel_single_call_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  // vm_log_info("callhangCall");

  if (IDLE_CALL != g_call_status) {
    req.action_id.sim = g_uid_info.sim;
    req.action_id.call_id = g_uid_info.call_id;
    req.action_id.group_id = g_uid_info.group_id;
    // req.action_id.sim = 1;
    // req.action_id.call_id = 1;
    // req.action_id.group_id = 1;

    data.action = VM_GSM_TEL_CALL_ACTION_END_SINGLE;
    data.data_action = (void *)&req;
    data.user_data = NULL;
    data.callback = call_voiceCall_callback;

    result = vm_gsm_tel_call_actions(&data);
  }

  lua_pushnumber(L, result);

  return 1;
}

//===========================================
static int gsm_on_incoming_call(lua_State *L)
{
    if (g_gsm_incoming_call_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_gsm_incoming_call_cb_ref);
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    g_gsm_incoming_call_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	    lua_pushinteger(L, 0);
	}
	else {
		lua_pushinteger(L, -1);
	}
    return 1;
}

#endif


// The callback for SMS functions
//--------------------------------------------------------------------
static void _gsm_text_callback(vm_gsm_sms_callback_t* callback_data) {

    if (callback_data->action == VM_GSM_SMS_ACTION_SEND) {
    	// === send message status ===
        if (g_sms_send_cb_ref != LUA_NOREF) {
        	gsm_cb_params_sent.par = callback_data->result;
        	gsm_cb_params_sent.cb_ref = g_sms_send_cb_ref;
            remote_lua_call(CB_FUNC_INT, &gsm_cb_params_sent);
        }
        else if (g_shell_result == -9) {
        	vm_log_debug("send sms callback, result = %d", callback_data->result);
        	lua_pushinteger(shellL, callback_data->result);
			g_shell_result = 1;
			vm_signal_post(g_shell_signal);
        }
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_QUERY) {
    	// === query message id's list ===
        if ((g_sms_list_cb_ref == LUA_NOREF) && (g_shell_result == -9)) {
            lua_newtable( shellL );
        }

    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
    		if (callback_data->action_data) {
        		vm_gsm_sms_query_message_callback_t* query_result = (vm_gsm_sms_query_message_callback_t*)callback_data->action_data;
                if (query_result->message_id_list) {
                    if (query_result->message_number <= 0) {
                    	vm_log_debug("the message_id_list is empty.");
                    }
                    else {
                        if ((g_sms_list_cb_ref == LUA_NOREF) && (g_shell_result == -9)) {
							int i;
							int j = query_result->message_number;
							for (i=0; i < j; i++) {
								lua_pushinteger( shellL, (VMUINT16)query_result->message_id_list[i]);
								lua_rawseti(shellL,-2, i+1);
							}
                        }
                        //vm_free(query_result->message_id_list);
                    }
                }
                else {
                	vm_log_debug("message_id_list == NULL.");
                }
    		}
    		else {
    			vm_log_debug("SMS gets message id list action_data is NULL.");
            }
    	}
        else {
        	vm_log_debug("Query message failed.");
        }
        if ((g_sms_list_cb_ref == LUA_NOREF) && (g_shell_result == -9)) {
			g_shell_result = 1;
			vm_signal_post(g_shell_signal);
    	}
    	else if (callback_data->action_data) {
        	gsm_cb_params_list.list = (vm_gsm_sms_query_message_callback_t*)callback_data->action_data;
        	gsm_cb_params_list.cb_ref = g_sms_list_cb_ref;
            remote_lua_call(CB_FUNC_SMS_LIST, &gsm_cb_params_list);
    	}
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_READ) {
    	// === read message ===
    	int rec_ok = 1;
    	vm_gsm_sms_read_message_data_callback_t* read_message;
    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
    		if (callback_data->action_data) {
				rec_ok = 3;
    	    	if ((g_sms_read_cb_ref == LUA_NOREF) && (g_shell_result == -9)) {
    	    		// Lua call
					read_message = (vm_gsm_sms_read_message_data_callback_t*)callback_data->action_data;
					char msgdata[MAX_SMS_CONTENT_LEN] = {0};

					vm_date_time_t timestamp = read_message->message_data->timestamp;
					//vm_date_time_t timestamp;
					//memcpy(&timestamp, &read_message->message_data->timestamp, sizeof(vm_date_time_t));

					vm_chset_ucs2_to_ascii((VMSTR)msgdata, MAX_SMS_CONTENT_LEN, (VMCWSTR)(read_message->message_data->number));
					lua_pushstring(shellL, msgdata);					// sender number
					sprintf(msgdata, "%d/%d/%d %d:%d:%d",
							timestamp.month, timestamp.day, timestamp.year, timestamp.hour, timestamp.minute, timestamp.second);
					lua_pushstring(shellL, msgdata);					// message time as string

					vm_chset_ucs2_to_ascii((VMSTR)msgdata, MAX_SMS_CONTENT_LEN, (VMCWSTR)(read_message->message_data->content_buffer));
					lua_pushstring(shellL, msgdata);					// received message
    	    	}
    		}
    		else {
    			vm_log_debug("action_data is NULL.");
            	if (g_sms_read_cb_ref == LUA_NOREF) lua_pushnil(shellL);
            }
    	}
        else {
        	vm_log_debug("read message failed.");
        	if (g_sms_read_cb_ref == LUA_NOREF) lua_pushnil(shellL);
        }

    	if (g_sms_read_cb_ref == LUA_NOREF) { // Lua call
	    	if (g_shell_result == -9) {
	    		vm_free(message_data->content_buffer);
	    		vm_free(message_data);
	    		message_data = NULL;

				if (rec_ok == 1) lua_pushnil(shellL);
				g_shell_result = rec_ok;
				vm_signal_post(g_shell_signal);
	    	}
    	}
    	else { // Lua callback & got data
        	gsm_cb_params_read.msg = (vm_gsm_sms_read_message_data_callback_t*)callback_data->action_data;
        	gsm_cb_params_read.stat = rec_ok;
        	gsm_cb_params_read.cb_ref = g_sms_read_cb_ref;
            remote_lua_call(CB_FUNC_SMS_LIST, &gsm_cb_params_read);
    	}
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_DELETE) {
    	// === delete message ===
    	int res = 0;
    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
			if (!callback_data->action_data) {
				vm_log_debug("action_data is NULL.");
				res = -1;
			}
        }
        else {
        	vm_log_debug("delete message failed.");
        	res = -2;
        }
    	if (g_sms_delete_cb_ref == LUA_NOREF) {
	    	if (g_shell_result == -9) {
				lua_pushinteger(shellL, res);
				g_shell_result = 1;
				vm_signal_post(g_shell_signal);
	    	}
    	}
    	else {
    		gsm_cb_params_delete.par = res;
        	gsm_cb_params_delete.cb_ref = g_sms_delete_cb_ref;
            remote_lua_call(CB_FUNC_INT, &gsm_cb_params_delete);
    	}
    }
}

//-----------------------------------------------------------------
static int _gsm_on_new_message_cb(vm_gsm_sms_event_t* event_data) {
    vm_gsm_sms_event_new_sms_t * event_new_message_ptr;
    vm_gsm_sms_new_message_t * new_message_ptr = NULL;
    char content[MAX_SMS_CONTENT_LEN];

    // Checks if this event is for new SMS message.
    if (event_data->event_id == VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE){
        // Get the event info.
        event_new_message_ptr = (vm_gsm_sms_event_new_sms_t *)event_data->event_info;

        // Gets the message data.
        new_message_ptr  =  event_new_message_ptr->message_data;

        // Converts the message content to ASCII.
        vm_chset_ucs2_to_ascii((VMSTR)content, MAX_SMS_CONTENT_LEN, (VMWSTR)event_new_message_ptr->content);

        if (g_gsm_new_message_cb_ref != LUA_NOREF) {
        	gsm_cb_params_newsms.msg = (vm_gsm_sms_event_new_sms_t *)event_data->event_info;
        	gsm_cb_params_read.cb_ref = g_gsm_new_message_cb_ref;
            remote_lua_call(CB_FUNC_SMS_NEW, &gsm_cb_params_newsms);
        }
        else {
        	vm_log_info("New message from: %s\n", (char *)new_message_ptr->number);
        	vm_log_info("[%s]\n", content);
        }

        return 0;
    }
    else {
    	vm_log_debug("[SMS event]: %d", event_data->event_id);
        return 0;
    }
}

//================================
static int _gsm_send(lua_State *L)
{
    VMWCHAR number[42];
    VMWCHAR content[MAX_SMS_CONTENT_LEN];

    const char *phone_number = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);

    vm_chset_ascii_to_ucs2(content, MAX_SMS_CONTENT_LEN*2, message);
	vm_chset_ascii_to_ucs2(number, 42*2, phone_number);

    int res = vm_gsm_sms_send(number, content, _gsm_text_callback, NULL);
	if (g_sms_send_cb_ref != LUA_NOREF) {
		lua_pushnumber(L, res);
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
	}
    return 1;
}

//===============================
static int gsm_send(lua_State *L)
{
    const char *phone_number = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);
    if (g_sms_send_cb_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, g_sms_send_cb_ref);
    	g_sms_send_cb_ref = LUA_NOREF;
    }
	if ((lua_type(L, 3) == LUA_TFUNCTION) || (lua_type(L, 3) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 3);
	    g_sms_send_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	if (g_sms_send_cb_ref == LUA_NOREF) CCwait = 5000;
    g_shell_result = -9;
	remote_CCall(L, &_gsm_send);
	if (g_shell_result < 0) { // no response
		lua_pushinteger(L, -1);
	    g_shell_result = 1;
	}
	return g_shell_result;
}

//===============================
static int _gsm_read(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);

	// Allocate message buffer
	if (message_data != NULL) {
		vm_free(message_data->content_buffer);
		vm_free(message_data);
		message_data = NULL;
	}
    message_data = vm_calloc(sizeof(vm_gsm_sms_read_message_data_t));
    if (message_data == NULL) {
    	vm_log_debug("message_data vm_calloc failed.");
    	g_shell_result = -1;
		vm_signal_post(g_shell_signal);
		return 1;
    }
    content_buff = vm_calloc(((MAX_SMS_CONTENT_LEN*2)+4)*sizeof(VMWCHAR));
    if (content_buff == NULL) {
        vm_log_debug("content_buff vm_calloc failed.");
    	vm_free(message_data);
    	g_shell_result = -1;
		vm_signal_post(g_shell_signal);
		return 1;
    }

    message_data->content_buffer = content_buff;
    message_data->content_buffer_size = MAX_SMS_CONTENT_LEN*2;

    int res = vm_gsm_sms_read_message(message_id, VM_TRUE, message_data, _gsm_text_callback, NULL);


    if (g_sms_read_cb_ref != LUA_NOREF) {
		lua_pushnumber(L, res);
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
	}
    return 1;
}

//=================================
int _gsm_readbuf_free(lua_State *L)
{
	if (message_data->content_buffer) vm_free(message_data->content_buffer);
	if (message_data) vm_free(message_data);
	message_data = NULL;
	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int gsm_read(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);

    if (g_sms_read_cb_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, g_sms_read_cb_ref);
    	g_sms_read_cb_ref = LUA_NOREF;
    }
	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 2);
	    g_sms_read_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

    // Read message
    if (g_sms_read_cb_ref == LUA_NOREF) CCwait = 2000;
    g_shell_result = -9;
	remote_CCall(L, &_gsm_read);
	if (g_shell_result < 0) { // no response
		remote_CCall(L, &_gsm_readbuf_free);
		lua_pushnil(L);
	    g_shell_result = 1;
	}
	return g_shell_result;
}

//=====================================
static int _gsm_sms_delete(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);

    if (g_sms_delete_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_sms_delete_cb_ref);
	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 2);
	    g_sms_delete_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else g_sms_delete_cb_ref = LUA_NOREF;

	int res = vm_gsm_sms_delete_message(message_id, _gsm_text_callback, NULL);
	if (g_sms_delete_cb_ref != LUA_NOREF) {
		lua_pushnumber(L, res);
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
	}
    return 1;
}

//=====================================
static int gsm_sms_delete(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);

    g_shell_result = -9;
	remote_CCall(L, &_gsm_sms_delete);
	if (g_shell_result < 0) { // no response
		lua_pushinteger(L, -3);
	    g_shell_result = 1;
	}
	return g_shell_result;
}

//==========================================
static int _gsm_on_new_message(lua_State *L)
{
    lua_pushnumber(L, vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE, _gsm_on_new_message_cb, NULL));
	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//=========================================
static int gsm_on_new_message(lua_State *L)
{
	g_shell_result = 1;
    if (g_gsm_new_message_cb_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, g_gsm_new_message_cb_ref);
		g_gsm_new_message_cb_ref = LUA_NOREF;
    }
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, 1);
	    g_gsm_new_message_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		remote_CCall(L, &_gsm_on_new_message);
	}
	else {
		lua_pushinteger(L, -1);
	}
	return g_shell_result;
}

//============================================
static int _gsm_num_sms_received(lua_State *L)
{
    int type = 1;
	if (lua_isnumber(L, 1)) {
		type = luaL_checkinteger(L, 1) & 0x3F;
		if (type == 0) type = 1;
	}

    lua_pushinteger(L, vm_gsm_sms_get_box_size(type));

	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
    return 1;
}

//===========================================
static int gsm_num_sms_received(lua_State *L)
{
    g_shell_result = -9;
	remote_CCall(L, &_gsm_num_sms_received);

	if (g_shell_result < 0) { // no response
		lua_pushinteger(L, -1);
	    g_shell_result = 1;
	}
	return g_shell_result;
}

//=============================================
static int _gsm_list_sms_received(lua_State *L)
{
	int stat = 1;

	if (lua_isnumber(L, 1)) {
		stat = luaL_checkinteger(L, 1) & 0x1F;
		if (stat == 0) stat = 1;
	}

	vm_gsm_sms_query_t query_data_t;

	query_data_t.sort_flag  = VM_GSM_SMS_SORT_TIMESTAMP;
	query_data_t.order_flag = VM_GSM_SMS_ORDER_ASCEND;
	query_data_t.status     = stat;  //VM_GSM_SMS_STATUS_READ;

	stat = vm_gsm_sms_get_message_id_list(&query_data_t, _gsm_text_callback, NULL);

	if (g_sms_list_cb_ref != LUA_NOREF) {
		lua_pushnumber(L, stat);
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
	}
	return 1;
}

//============================================
static int gsm_list_sms_received(lua_State *L)
{
    if (g_sms_list_cb_ref != LUA_NOREF) {
    	luaL_unref(L, LUA_REGISTRYINDEX, g_sms_list_cb_ref);
    	g_sms_list_cb_ref = LUA_NOREF;
    }
	if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
	    lua_pushvalue(L, -1);
	    g_sms_list_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

    g_shell_result = -9;
	remote_CCall(L, &_gsm_list_sms_received);

	if (g_shell_result < 0) { // no response
		lua_newtable(L);
	    g_shell_result = 1;
	}
	return g_shell_result;
}

//================================
static int _gsm_info(lua_State *L)
{
	VM_GSM_SIM_ID id = vm_gsm_sim_get_active_sim_card();
    g_shell_result = 1;
	if (id > 0) {
		int simn = vm_gsm_sim_get_card_count();
		if (simn > 0) {
			lua_pushinteger(L, vm_gsm_sim_get_card_status(id));
			lua_pushstring(L, vm_gsm_sim_get_imei(id));
			lua_pushstring(L, vm_gsm_sim_get_imsi(id));
			g_shell_result = 3;
		}
		else {
			lua_pushinteger(L, 0);
		}
	}
	else {
		lua_pushinteger(L, -2);
	}
	vm_signal_post(g_shell_signal);
	return 0;
}

//===============================
static int gsm_info(lua_State *L)
{
	remote_CCall(L, &_gsm_info);
	return g_shell_result;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

#ifdef INCLUDE_CALL_FUNCTIONS
const LUA_REG_TYPE gsm_map[] = {
		{LSTRKEY("call"), LFUNCVAL(gsm_call)},
		{LSTRKEY("answer"), LFUNCVAL(gsm_anwser)},
		{LSTRKEY("hang"), LFUNCVAL(gsm_hang)},
		{LSTRKEY("on_incoming_call"), LFUNCVAL(gsm_on_incoming_call)},
		{LNILKEY, LNILVAL}
};
#endif

const LUA_REG_TYPE sms_map[] = {
		{LSTRKEY("send"), LFUNCVAL(gsm_send)},
		{LSTRKEY("read"), LFUNCVAL(gsm_read)},
		{LSTRKEY("numrec"), LFUNCVAL(gsm_num_sms_received)},
		{LSTRKEY("delete"), LFUNCVAL(gsm_sms_delete)},
		{LSTRKEY("list"), LFUNCVAL(gsm_list_sms_received)},
		{LSTRKEY("siminfo"), LFUNCVAL(gsm_info)},
		{LSTRKEY("onmessage"), LFUNCVAL(gsm_on_new_message)},
		{LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_gsm(lua_State *L) {
  #ifdef INCLUDE_CALL_FUNCTIONS
  vm_gsm_tel_call_reg_listener(call_listener_func);
  luaL_register(L, "gsm", gsm_map);
  #endif

  luaL_register(L, "sms", sms_map);
  return 1;
}
