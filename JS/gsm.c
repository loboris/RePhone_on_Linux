

#include <string.h>

#include "vmlog.h"
#include "vmgsm_tel.h"
#include "vmchset.h"
#include "vmgsm_sim.h"
#include "vmgsm_sms.h"

#include "v7.h"

#define INCOMING_CALL_CB "incoming_call_cb"
#define NEW_MESSAGE_CB "new_message_cb"

extern struct v7* v7;

enum VoiceCall_Status { IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };

vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;

vm_gsm_tel_id_info_t g_uid_info;
VMINT8 g_call_status = IDLE_CALL;
VMINT8 g_incoming_number[42];

void call_listener_func(vm_gsm_tel_call_listener_data_t* data)
{
    vm_log_info("call_listener_func");

    if(data->call_type == VM_GSM_TEL_INDICATION_INCOMING_CALL) {
        vm_gsm_tel_call_info_t* ind = (vm_gsm_tel_call_info_t*)data->data;
        g_uid_info.call_id = ind->uid_info.call_id;
        g_uid_info.group_id = ind->uid_info.group_id;
        g_uid_info.sim = ind->uid_info.sim;
        strcpy(g_incoming_number, (char*)ind->num_uri);
        g_call_status = RECEIVINGCALL;

        vm_log_info("incoming call");

        {
            v7_val_t cb = v7_get(v7, v7_get_global(v7), INCOMING_CALL_CB, sizeof(INCOMING_CALL_CB) - 1);

            if(v7_is_function(cb)) {
                v7_val_t res;
                v7_val_t args = v7_create_array(v7);
                v7_val_t number = v7_create_string(v7, g_incoming_number, strlen(g_incoming_number), 1);

                v7_array_push(v7, args, number);
                if(v7_apply(v7, &res, cb, v7_create_undefined(), args) != V7_OK) {
                    /* TODO(mkm): make it print stack trace */
                    fprintf(stderr, "cb threw an exception\n");
                }
            }
        }

    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_OUTGOING_CALL) {
        vm_gsm_tel_call_info_t* ind = (vm_gsm_tel_call_info_t*)data->data;
        g_uid_info.call_id = ind->uid_info.call_id;
        g_uid_info.group_id = ind->uid_info.group_id;
        g_uid_info.sim = ind->uid_info.sim;
        strcpy(g_incoming_number, (char*)ind->num_uri);
        g_call_status = CALLING;

        vm_log_info("calling");
    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_CONNECTED) {
        vm_gsm_tel_connect_indication_t* ind = (vm_gsm_tel_connect_indication_t*)data->data;
        g_uid_info.call_id = ind->uid_info.call_id;
        g_uid_info.group_id = ind->uid_info.group_id;
        g_uid_info.sim = ind->uid_info.sim;
        g_call_status = TALKING;

        vm_log_info("connected");
    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_CALL_ENDED) {
        g_call_status = IDLE_CALL;
        vm_log_info("endded");
    }

    else {
        vm_log_info("bad operation type");
    }

    vm_log_info("g_call_status is %d", g_call_status);
}

static void call_voiceCall_callback(vm_gsm_tel_call_actions_callback_data_t* data)
{
    vm_log_info("call_voiceCall_callback");

    if(data->type_action == VM_GSM_TEL_CALL_ACTION_DIAL) {

        if(data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) {
            g_call_status = CALLING;
        } else {
            g_call_status = IDLE_CALL;
        }
    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_ACCEPT) {
        if(data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) {
            g_call_status = TALKING;
        } else {
            g_call_status = IDLE_CALL;
        }
    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_HOLD) {

    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_END_SINGLE) {
        g_call_status = IDLE_CALL;
    } else {
    }
}

int _gsm_call(const char* phone_number)
{
    vm_gsm_tel_dial_action_request_t req;
    vm_gsm_tel_call_actions_data_t data;

    vm_chset_ascii_to_ucs2((VMWSTR)req.num_uri, VM_GSM_TEL_MAX_NUMBER_LENGTH, (VMSTR)phone_number);
    req.sim = VM_GSM_TEL_CALL_SIM_1;
    req.is_ip_dial = 0;
    req.module_id = 0;
    req.phonebook_data = NULL;

    data.action = VM_GSM_TEL_CALL_ACTION_DIAL;
    data.data_action = (void*)&req;
    data.user_data = NULL;
    data.callback = call_voiceCall_callback;

    return vm_gsm_tel_call_actions(&data);
}

static v7_val_t gsm_call(struct v7* v7)
{
    v7_val_t numberv = v7_arg(v7, 0);
    const char* number;
    size_t len;

    if(!v7_is_string(numberv)) {
        return v7_create_undefined();
    }

    number = v7_to_string(v7, &numberv, &len);

    return v7_create_number(_gsm_call(number));
}

static v7_val_t gsm_answer(struct v7* v7)
{
    int result;
    vm_gsm_tel_single_call_action_request_t req;
    vm_gsm_tel_call_actions_data_t data;

    req.action_id.sim = g_uid_info.sim;
    req.action_id.call_id = g_uid_info.call_id;
    req.action_id.group_id = g_uid_info.group_id;

    data.action = VM_GSM_TEL_CALL_ACTION_ACCEPT;
    data.data_action = (void*)&req;
    data.user_data = NULL;
    data.callback = call_voiceCall_callback;

    result = vm_gsm_tel_call_actions(&data);

    return v7_create_number(result);
}

static v7_val_t gsm_hang(struct v7* v7)
{
    int result = 0;
    vm_gsm_tel_single_call_action_request_t req;
    vm_gsm_tel_call_actions_data_t data;

    if(IDLE_CALL != g_call_status) {
        req.action_id.sim = g_uid_info.sim;
        req.action_id.call_id = g_uid_info.call_id;
        req.action_id.group_id = g_uid_info.group_id;
        // req.action_id.sim = 1;
        // req.action_id.call_id = 1;
        // req.action_id.group_id = 1;

        data.action = VM_GSM_TEL_CALL_ACTION_END_SINGLE;
        data.data_action = (void*)&req;
        data.user_data = NULL;
        data.callback = call_voiceCall_callback;

        result = vm_gsm_tel_call_actions(&data);
    }

    return v7_create_number(result);
}

static v7_val_t gsm_on_incoming_call(struct v7* v7)
{
    v7_val_t cb = v7_arg(v7, 0);
    if(!v7_is_function(cb)) {
        return v7_create_boolean(0);
    };

    v7_set(v7, v7_get_global(v7), INCOMING_CALL_CB, sizeof(INCOMING_CALL_CB) - 1, 0, cb);

    return v7_create_boolean(1);
}

/* The callback of sending SMS, for checking if an SMS is sent successfully. */
void _gsm_text_callback(vm_gsm_sms_callback_t* callback_data)
{
    if(callback_data->action == VM_GSM_SMS_ACTION_SEND) {
        vm_log_debug("send sms callback, result = %d", callback_data->result);
    }
}

static v7_val_t gsm_text(struct v7* v7)
{
    VMWCHAR number[42];
    VMWCHAR content[100];
    v7_val_t numberv = v7_arg(v7, 0);
    v7_val_t messagev = v7_arg(v7, 1);
    const char* phone_number;
    const char* message;
    size_t len, message_len;

    if(!v7_is_string(numberv) || !v7_is_string(messagev)) {
        return v7_create_undefined();
    }

    phone_number = v7_to_string(v7, &numberv, &len);
    message = v7_to_string(v7, &messagev, &message_len);

    vm_chset_ascii_to_ucs2(content, 100 * 2, message);
    vm_chset_ascii_to_ucs2(number, 42 * 2, phone_number);

    return v7_create_number(vm_gsm_sms_send(number, content, _gsm_text_callback, NULL));
}

int _gsm_on_new_message(vm_gsm_sms_event_t* event_data)
{
    vm_gsm_sms_event_new_sms_t* event_new_message_ptr;
    vm_gsm_sms_new_message_t* new_message_ptr = NULL;
    char content[100];
    /* Checks if this event is for new SMS message. */
    if(event_data->event_id == VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE) {
        /* Gets the event info. */
        event_new_message_ptr = (vm_gsm_sms_event_new_sms_t*)event_data->event_info;

        /* Gets the message data. */
        new_message_ptr = event_new_message_ptr->message_data;

        /* Converts the message content to ASCII. */
        vm_chset_ucs2_to_ascii((VMSTR)content, 100, (VMWSTR)event_new_message_ptr->content);

        {
            v7_val_t cb = v7_get(v7, v7_get_global(v7), NEW_MESSAGE_CB, sizeof(NEW_MESSAGE_CB) - 1);

            if(v7_is_function(cb)) {
                v7_val_t res;
                v7_val_t args = v7_create_array(v7);
                v7_val_t number = v7_create_string(v7, new_message_ptr->number, strlen(new_message_ptr->number), 1);
                v7_val_t message = v7_create_string(v7, content, strlen(content), 1);
                
                v7_array_push(v7, args, number);
                v7_array_push(v7, args, message);
                if(v7_apply(v7, &res, cb, v7_create_undefined(), args) != V7_OK) {
                    /* TODO(mkm): make it print stack trace */
                    fprintf(stderr, "cb threw an exception\n");
                }
            }
        }
        return 1;
    } else {
        return 0;
    }
}

static v7_val_t gsm_on_new_message(struct v7* v7)
{
    v7_val_t cb = v7_arg(v7, 0);
    if(!v7_is_function(cb)) {
        return v7_create_boolean(0);
    };

    v7_set(v7, v7_get_global(v7), NEW_MESSAGE_CB, sizeof(NEW_MESSAGE_CB) - 1, 0, cb);
    
    vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE, _gsm_on_new_message, NULL);

    return v7_create_boolean(1);
}

void js_init_gsm(struct v7* v7)
{
    vm_gsm_tel_call_reg_listener(call_listener_func);

    v7_val_t gsm = v7_create_object(v7);
    v7_set(v7, v7_get_global(v7), "gsm", 3, 0, gsm);
    v7_set_method(v7, gsm, "call", gsm_call);
    v7_set_method(v7, gsm, "answer", gsm_answer);
    v7_set_method(v7, gsm, "hang", gsm_hang);
    v7_set_method(v7, gsm, "on_incoming_call", gsm_on_incoming_call);
    v7_set_method(v7, gsm, "text", gsm_text);
    v7_set_method(v7, gsm, "on_new_message", gsm_on_new_message);
}
