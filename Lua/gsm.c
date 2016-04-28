

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

#define MAX_SMS_CONTENT_LEN  160*2

enum VoiceCall_Status { IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };

static int g_gsm_incoming_call_cb_ref = LUA_NOREF;
static int g_gsm_new_message_cb_ref = LUA_NOREF;
static int g_sms_list_cb_ref = LUA_NOREF;
static int g_sms_read_cb_ref = LUA_NOREF;
static int g_sms_delete_cb_ref = LUA_NOREF;
static int g_sms_send_cb_ref = LUA_NOREF;

//static vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;

static vm_gsm_tel_id_info_t g_uid_info;
static VMINT8 g_call_status = IDLE_CALL;
static VMINT8 g_incoming_number[42];

extern lua_State *L;


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
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_gsm_incoming_call_cb_ref);
        lua_pushstring(L, g_incoming_number);
        lua_call(L, 1, 0);
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
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    if (g_gsm_incoming_call_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_gsm_incoming_call_cb_ref);
	    lua_pushvalue(L, 1);
	    g_gsm_incoming_call_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	    lua_pushinteger(L, 0);
	}
	else {
		lua_pushinteger(L, -1);
	}
    return 1;
}

//---------------------------------------------------------------
static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

//-------------------------------------------------------------------
static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}


// The callback for SMS functions
//--------------------------------------------------------------------
static void _gsm_text_callback(vm_gsm_sms_callback_t* callback_data) {

    if (callback_data->action == VM_GSM_SMS_ACTION_SEND) {
    	// send message status
        if (g_sms_send_cb_ref != LUA_NOREF) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, g_sms_send_cb_ref);
            lua_pushinteger(L, callback_data->result);
            lua_call(L, 1, 0);
        }
        else {
        	vm_log_debug("send sms callback, result = %d", callback_data->result);
        }
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_QUERY) {
    	// query message id's list
    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
    		if(!callback_data->action_data) {
    			vm_log_debug("SMS gets message id list action_data is NULL.");
                return;
            }
    		vm_gsm_sms_query_message_callback_t* query_result = (vm_gsm_sms_query_message_callback_t*)callback_data->action_data;
            if (!query_result->message_id_list) {
            	vm_log_debug("message_id_list == NULL.");
                return;
            }
            if (query_result->message_number <= 0) {
            	vm_log_debug("not find the message_id_list.");
            }
            else {
				int i;
				int j = query_result->message_number;
                if (g_sms_send_cb_ref != LUA_NOREF) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, g_sms_list_cb_ref);
                    lua_newtable( L );
					for (i=0; i < j; i++) {
				        lua_pushinteger( L, (VMUINT16)query_result->message_id_list[i]);
				        lua_rawseti(L,-2,i + 1);
					}
                    lua_call(L, 1, 0);
                }
                else {
					vm_log_debug("Number of messages id's is %d", query_result->message_number);
					printf("Message id: [");
					for (i=0; i < j; i++) {
						printf(" %d", (VMUINT16)query_result->message_id_list[i]);
					}
					printf("]\n");
                }
                //vm_free(query_result->message_id_list);
            }
    	}
        else {
        	vm_log_debug("Query message failed.");
        }
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_READ) {
    	// read message
    	vm_gsm_sms_read_message_data_callback_t* read_message;
    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
    		if(!callback_data->action_data) {
    			vm_log_debug("action_data is NULL.");
                return;
            }
            read_message = (vm_gsm_sms_read_message_data_callback_t*)callback_data->action_data;
            char msgdata[320] = {0};
            vm_date_time_t timestamp;
            struct tm sms_time;

            timestamp = read_message->message_data->timestamp;
            sms_time.tm_hour = timestamp.hour;
            sms_time.tm_min = timestamp.minute;
            sms_time.tm_sec = timestamp.second;
            sms_time.tm_mon = timestamp.month-1;
            sms_time.tm_mday = timestamp.day;
            sms_time.tm_year = timestamp.year-1900;
            sms_time.tm_isdst = -1;
            time_t time = mktime(&sms_time);

            if (g_sms_read_cb_ref != LUA_NOREF) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, g_sms_read_cb_ref);
				vm_chset_ucs2_to_ascii((VMSTR)msgdata, 320, (VMCWSTR)(read_message->message_data->number));
				lua_pushstring(L, msgdata);
			    lua_createtable(L, 0, 9);      // 9 = number of fields
			    setfield(L, "sec", sms_time.tm_sec);
			    setfield(L, "min", sms_time.tm_min);
			    setfield(L, "hour", sms_time.tm_hour);
			    setfield(L, "day", sms_time.tm_mday);
			    setfield(L, "month", sms_time.tm_mon+1);
			    setfield(L, "year", sms_time.tm_year+1900);
			    setfield(L, "wday", sms_time.tm_wday+1);
			    setfield(L, "yday", sms_time.tm_yday+1);
			    setboolfield(L, "isdst", sms_time.tm_isdst);
				vm_chset_ucs2_to_ascii((VMSTR)msgdata, 320, (VMCWSTR)(read_message->message_data->content_buffer));
				lua_pushstring(L, msgdata);
                lua_call(L, 3, 0);
            }
            else {
				vm_chset_ucs2_to_ascii((VMSTR)msgdata, 320, (VMCWSTR)(read_message->message_data->number));
				printf("\nMESSAGE in %d, from %s received on %s", read_message->message_data->storage_type, msgdata, asctime(&sms_time));

				vm_chset_ucs2_to_ascii((VMSTR)msgdata, 320, (VMCWSTR)(read_message->message_data->content_buffer));
				printf("[%s]\n", msgdata);
            }

            vm_free(read_message->message_data->content_buffer);
            vm_free(read_message->message_data);
    	}
        else {
        	vm_log_debug("read message failed.");
        }
    }
    else if (callback_data->action == VM_GSM_SMS_ACTION_DELETE) {
    	// delete message
    	if (callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR) {
            if (g_sms_delete_cb_ref != LUA_NOREF) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, g_sms_delete_cb_ref);
				if (!callback_data->action_data) lua_pushinteger(L, 0);
				else lua_pushinteger(L, 1);
                lua_call(L, 1, 0);
            }
            else {
				if (!callback_data->action_data) {
					vm_log_debug("action_data is NULL.");
				}
				else {
					vm_log_debug("delete message success, result = %d, cause = %d",	callback_data->result, callback_data->cause);
				}
            }
        }
        else {
        	vm_log_debug("delete message failed.");
        }
    }
}

//--------------------------------------------------------------
static int _gsm_on_new_message(vm_gsm_sms_event_t* event_data) {
    vm_gsm_sms_event_new_sms_t * event_new_message_ptr;
    vm_gsm_sms_new_message_t * new_message_ptr = NULL;
    char content[320];
    /* Checks if this event is for new SMS message. */
    if (event_data->event_id == VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE){
        /* Gets the event info. */
        event_new_message_ptr = (vm_gsm_sms_event_new_sms_t *)event_data->event_info;

        /* Gets the message data. */
        new_message_ptr  =  event_new_message_ptr->message_data;

        /* Converts the message content to ASCII. */
        vm_chset_ucs2_to_ascii((VMSTR)content, 320, (VMWSTR)event_new_message_ptr->content);

        if (g_gsm_new_message_cb_ref != LUA_NOREF) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, g_gsm_new_message_cb_ref);
            lua_pushinteger(L, new_message_ptr->message_id);
            lua_pushstring(L, new_message_ptr->number);
            lua_pushstring(L, content);
            lua_call(L, 3, 0);
        }
        else {
        	vm_log_info("New message from: %s\n", (char *)new_message_ptr->number);
        	vm_log_info("[%s]\n", content);
        }

        return 0;
    }
    else {
    	vm_log_debug("SMS event: %d", event_data->event_id);
        return 0;
    }
}

//===============================
static int gsm_text(lua_State *L)
{
    VMWCHAR number[42];
    VMWCHAR content[MAX_SMS_CONTENT_LEN];

    const char *phone_number = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);

	if ((lua_type(L, 3) == LUA_TFUNCTION) || (lua_type(L, 3) == LUA_TLIGHTFUNCTION)) {
	    if (g_sms_send_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_sms_send_cb_ref);
	    lua_pushvalue(L, 3);
	    g_sms_send_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

    vm_chset_ascii_to_ucs2(content, MAX_SMS_CONTENT_LEN*2, message);
	vm_chset_ascii_to_ucs2(number, 42*2, phone_number);

    lua_pushnumber(L, vm_gsm_sms_send(number, content, _gsm_text_callback, NULL));

    return 1;
}

//===============================
static int gsm_read(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);
    vm_gsm_sms_read_message_data_t * message_data = NULL;
    VMWCHAR * content_buff;
    VMINT res;

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	    if (g_sms_read_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_sms_read_cb_ref);
	    lua_pushvalue(L, 2);
	    g_sms_read_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

    message_data = vm_calloc(sizeof(vm_gsm_sms_read_message_data_t));
    if(message_data == NULL) {
    	vm_log_debug("message_data vm_calloc failed.");
    }
    content_buff = vm_calloc((500+1)*sizeof(VMWCHAR));
    if(content_buff == NULL) {
    	vm_free(message_data);
        vm_log_debug("content_buff vm_calloc failed.");
    }
    message_data->content_buffer = content_buff;
    message_data->content_buffer_size = 500;

    lua_pushnumber(L, vm_gsm_sms_read_message(message_id, VM_TRUE, message_data, _gsm_text_callback, NULL));

    return 1;
}

//=====================================
static int gsm_sms_delete(lua_State *L)
{
    int message_id = luaL_checkinteger(L, 1);

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	    if (g_sms_delete_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_sms_delete_cb_ref);
	    lua_pushvalue(L, 2);
	    g_sms_delete_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	lua_pushnumber(L, vm_gsm_sms_delete_message(message_id, _gsm_text_callback, NULL));

    return 1;
}

//=========================================
static int gsm_on_new_message(lua_State *L)
{
	if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
	    if (g_gsm_new_message_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_gsm_new_message_cb_ref);
	    lua_pushvalue(L, 1);
	    g_gsm_new_message_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	    lua_pushnumber(L, vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE, _gsm_on_new_message, NULL));
	}
	else {
		lua_pushinteger(L, -1);
	}
    return 1;
}


//===========================================
static int gsm_num_sms_received(lua_State *L)
{
    int type = luaL_checkinteger(L, 1);

    lua_pushinteger(L, vm_gsm_sms_get_box_size(type));

    return 1;
}

//============================================
static int gsm_list_sms_received(lua_State *L)
{
    int stat = luaL_checkinteger(L, 1);
	vm_gsm_sms_query_t query_data_t;

	if ((lua_type(L, 2) == LUA_TFUNCTION) || (lua_type(L, 2) == LUA_TLIGHTFUNCTION)) {
	    if (g_sms_list_cb_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, g_sms_list_cb_ref);
	    lua_pushvalue(L, 2);
	    g_sms_list_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	query_data_t.sort_flag  = VM_GSM_SMS_SORT_TIMESTAMP;
	query_data_t.order_flag = VM_GSM_SMS_ORDER_ASCEND;
	query_data_t.status     = stat;  //VM_GSM_SMS_STATUS_READ;

	lua_pushnumber(L, vm_gsm_sms_get_message_id_list(&query_data_t, _gsm_text_callback, NULL));
    return 1;
}

//===============================
static int gsm_info(lua_State *L)
{
	VM_GSM_SIM_ID id = vm_gsm_sim_get_active_sim_card();
	if (id > 0) {
		lua_pushinteger(L, vm_gsm_sim_get_card_status(id));
		lua_pushstring(L, vm_gsm_sim_get_imei(id));
		lua_pushstring(L, vm_gsm_sim_get_imsi(id));
		return 3;
	}
	else {
		lua_pushnil(L);
		return 1;
	}
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE gsm_map[] = {{LSTRKEY("call"), LFUNCVAL(gsm_call)},
                                {LSTRKEY("answer"), LFUNCVAL(gsm_anwser)},
                                {LSTRKEY("hang"), LFUNCVAL(gsm_hang)},
                                {LSTRKEY("on_incoming_call"), LFUNCVAL(gsm_on_incoming_call)},
                                {LSTRKEY("sms_send"), LFUNCVAL(gsm_text)},
                                {LSTRKEY("sms_read"), LFUNCVAL(gsm_read)},
                                {LSTRKEY("sms_numrec"), LFUNCVAL(gsm_num_sms_received)},
                                {LSTRKEY("sms_delete"), LFUNCVAL(gsm_sms_delete)},
                                {LSTRKEY("sms_list"), LFUNCVAL(gsm_list_sms_received)},
                                {LSTRKEY("sim_info"), LFUNCVAL(gsm_info)},
                                {LSTRKEY("on_new_message"), LFUNCVAL(gsm_on_new_message)},
                                {LNILKEY, LNILVAL}};

LUALIB_API int luaopen_gsm(lua_State *L) {
  vm_gsm_tel_call_reg_listener(call_listener_func);

  luaL_register(L, "gsm", gsm_map);
  return 1;
}
