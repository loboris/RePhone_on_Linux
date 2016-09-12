/* Lua mqtt.c
 *
 */

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

#include "vmbearer.h"
#include "vmlog.h"
#include "vmdns.h"
#include "vmsock.h"
#include "vmthread.h"

#include "mqtt_client.h"
#include "mqttnet.h"

#define LUA_MQTT   "mqtt"

/* Configuration */
//#define MAX_MQTT_CLIENTS        3
#define DEFAULT_MAX_BUFFER      1024
#define DEFAULT_MAX_PACKET      (int)(DEFAULT_MAX_BUFFER + sizeof(MqttPacket) + XSTRLEN(DEFAULT_TOPIC_NAME) + MQTT_DATA_LEN_SIZE)
#define DEFAULT_HASH_TYPE       WC_HASH_TYPE_SHA256
#define DEFAULT_SIG_TYPE        WC_SIGNATURE_TYPE_ECC
#define MAX_PACKET_ID           ((1 << 16) - 1)
#define DEFAULT_MQTT_QOS        MQTT_QOS_0
#define DEFAULT_KEEP_ALIVE_SEC  300
#define DEFAULT_CMD_TIMEOUT_MS  4000
#define DEFAULT_CON_TIMEOUT_MS  5000
#define DEFAULT_CLIENT_ID       "RephoneMQTTClient"
#define TEST_MESSAGE            "test from RePhone"

static VM_BEARER_HANDLE	g_bearer_hdl = -1;


//-------------------------------------------------------------------------------------------------
static int mqttclient_message_cb(MqttClient *client, MqttMessage *msg, byte msg_new, byte msg_done)
{
	mqtt_info_t *p = (mqtt_info_t *)client->parrent;

    // Verify this message is for the subscribed topic
	if (msg_new) {
		// Print incoming message info
		if (p->mRecBuf != NULL) {
			WOLFMQTT_FREE(p->mRecBuf);
			p->mRecBuf = NULL;
		}

		int isSubscribed = 0;
		for (int i=0; i<MAX_MQTT_TOPICS;i++) {
			if (memcmp(msg->topic_name, p->topics[i].topic_filter, msg->topic_name_len) == 0) {
				isSubscribed = 1;
				snprintf(p->mRecTopic, MAX_MQTT_TOPIC_LEN, "%s", msg->topic_name);
				p->mRecTopic[MAX_MQTT_TOPIC_LEN-1] = '\0';
				if (msg->topic_name_len < MAX_MQTT_TOPIC_LEN) p->mRecTopic[msg->topic_name_len] = '\0';
				break;
			}
		}
		if (isSubscribed) {
			// Allocate buffer for entire message
			p->mRecBuf = (byte*)WOLFMQTT_MALLOC(msg->total_len+1);
			if (p->mRecBuf == NULL) {
				return MQTT_CODE_ERROR_OUT_OF_BUFFER;
			}
		}
		else {
		    // Return negative to termine publish processing !
		    return -1;
		}
	}

	if (p->mRecBuf != NULL) {
		XMEMCPY(&p->mRecBuf[msg->buffer_pos], msg->buffer, msg->buffer_len);

		// === Process message if done ===
		if (msg_done) {
			p->mRecBuf[msg->total_len] = 0;
			p->mRecBuf_len = msg->total_len;
			if (p->cb_ref_message == LUA_NOREF) {
        		if (g_shell_result == -9) {
        			g_shell_result = 0;
        			vm_signal_post(g_shell_signal);
        		}
        		else {
					printf("\n[MQTT MESSAGE] topic: '%s'\n%s\n", p->mRecTopic, p->mRecBuf);

					// Free
					WOLFMQTT_FREE(p->mRecBuf);
					p->mRecBuf = NULL;
        		}
			}
			else {
				remote_lua_call(CB_FUNC_MQTT_MESSAGE, p);
			}
		}
	}

    // Return negative to termine publish processing !
    return MQTT_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------
static VM_RESULT get_host_callback(VM_DNS_HANDLE handle, vm_dns_result_t* result, void *user_data)
{
	if (g_shell_result == -9) {
		mqtt_info_t *p = (mqtt_info_t *)user_data;

		sprintf(p->host_IP, "%d.%d.%d.%d", (result->address[0]) & 0xFF, ((result->address[0]) & 0xFF00)>>8,
				((result->address[0]) & 0xFF0000)>>16, ((result->address[0]) & 0xFF000000)>>24);

		g_shell_result = 0;
		vm_signal_post(g_shell_signal);
	}
	return VM_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------
static void bearer_callback(VM_BEARER_HANDLE handle, VM_BEARER_STATE event, VMUINT data_account_id, void *user_data)
{
	mqtt_info_t *p = (mqtt_info_t *)user_data;

	if (VM_BEARER_WOULDBLOCK == g_bearer_hdl) {
		vm_log_info("[mqtt] Bearer [%d] would block", handle);
		g_bearer_hdl = handle;
    }

    if (handle == g_bearer_hdl) {
        switch (event) {
            case VM_BEARER_DEACTIVATED:
    			vm_log_debug("[mqtt] Bearer [%d] deactivated.", handle);
    	    	g_bearer_hdl = -1;
        		if (g_shell_result == -9) {
        			g_shell_result = 0;
        			vm_signal_post(g_shell_signal);
        		}
                break;
            case VM_BEARER_ACTIVATING:
                break;
            case VM_BEARER_ACTIVATED: {
            		vm_log_debug("[mqtt] Bearer [%d] activated.", handle);
            		if (g_shell_result == -9) {
            			g_shell_result = 0;
            			vm_signal_post(g_shell_signal);
            		}
            	}
                break;
            case VM_BEARER_DEACTIVATING:
                break;
            default:
                break;
        }
    }
}

//==================================
static int _mqtt_getIP(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	vm_dns_result_t dnsres;
	VM_DNS_HANDLE g_handle;
	g_handle = vm_dns_get_host_by_name(VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, p->pServer, &dnsres, get_host_callback, p);

	return 0;
}

//======================================
static int _mqtt_getBearer(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, p, bearer_callback, VM_BEARER_IPV4);

	return 0;
}

//-----------------------------
int _mqtt_check(mqtt_info_t *p)
{
	if (!p->connected) return -1;

	int rc;

	p->keep_alive_cnt += p->check_sec;

    // === Check for message
	// Try to read packet, if read successfuly it will fire msg callback function
	rc = MqttClient_WaitMessage(&p->client, 50);

	if (rc == MQTT_CODE_ERROR_TIMEOUT) {
		// Keep Alive
		if (p->keep_alive_cnt > (p->keep_alive_sec / 2)) {
			p->keep_alive_cnt = 0;
			rc = MqttClient_Ping(&p->client);
			if (rc != MQTT_CODE_SUCCESS) {
				vm_log_debug("[MQTT] Ping Keep Alive Error: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
				rc = MqttClient_Disconnect(&p->client);
				rc = MqttClient_NetDisconnect(&p->client);
				//rc = vm_timer_delete_non_precise(p->timerHandle);
				//p->timerHandle = -1;
				p->connected = 0;
				if (p->cb_ref_disconnect != LUA_NOREF) {
					remote_lua_call(CB_FUNC_MQTT_MESSAGE, p);
				}

			}
		}
	}
	else if (rc != MQTT_CODE_SUCCESS) {
		// There was an error
		vm_log_debug("[MQTT] Wait message Error: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
		rc = MqttClient_Disconnect(&p->client);
	    rc = MqttClient_NetDisconnect(&p->client);
	    //rc = vm_timer_delete_non_precise(p->timerHandle);
	    //p->timerHandle = -1;
		p->connected = 0;
		if (p->cb_ref_disconnect != LUA_NOREF) {
			remote_lua_call(CB_FUNC_MQTT_MESSAGE, p);
		}
	}

	return rc;
}

//===================================
static int __mqtt_check(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));

	int rc = _mqtt_check(p);
	if (rc < 0) {
		g_shell_result = 1;
		vm_signal_post(g_shell_signal);
	}
	return 0;
}

//=================================
static int mqtt_check(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));

	g_shell_result = -9;
	CCwait = 2000;
	remote_CCall(L, &__mqtt_check);
	if (g_shell_result != 0) { // no response or error
		g_shell_result = 0;
		lua_pushnil(L);
		return 1;
	}

	lua_pushstring(L, p->mRecTopic);
	lua_pushstring(L, p->mRecBuf);

	// Free msg buffer
	WOLFMQTT_FREE(p->mRecBuf);
	p->mRecBuf = NULL;

	return 2;
}

//-------------------------------------------------------------------------------------
static void _mqtt_timer_callback(VM_TIMER_ID_NON_PRECISE sys_timer_id, void* user_data)
{
	mqtt_info_t *p = (mqtt_info_t *)user_data;
	if (p->connected) {
		remote_lua_call(CB_FUNC_MQTT_TIMER, p);
	}
}

//====================================
static int _timer_create(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	if (p->timerHandle < 0)
		p->timerHandle = vm_timer_create_non_precise(p->check_sec * 1000, _mqtt_timer_callback, p);

	g_shell_result = 0;
	vm_signal_post(g_shell_signal);
    return 0;
}

/* Connect to MQTT broker
 * mqtt.connect(mqtt, check_int, ka_int)
 */
//===================================
static int mqtt_connect(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

    int rc = -9;
    int tmrint = 30;
    int kaint = 60;

    if (lua_isnumber(L, 2)) tmrint = luaL_checkinteger(L, 2);
    if (lua_isnumber(L, 3)) kaint = luaL_checkinteger(L, 3);

   	if (kaint < 30) tmrint = 30;
   	if (kaint > 600) tmrint = 600;
   	if (tmrint < 5) tmrint = 5;
   	if (tmrint > 300) tmrint = 300;
   	if (tmrint > (kaint/2)) tmrint = kaint/2;

	if (p->connected) {
		rc = 0;
		goto exit;
	}

	if (g_bearer_hdl < 0) {
		g_shell_result = -9;
		CCwait = 8000;
		remote_CCall(L, &_mqtt_getBearer);
		if (g_shell_result < 0) { // no response
			g_shell_result = 1;
		    vm_log_debug("[MQTT] Error obtaining bearer");
			goto exit;
		}
	}

	if (p->host_IP[0] == '\0') {
		// Activate bearer and get host IP
		g_shell_result = -9;
		CCwait = 2000;
		remote_CCall(L, &_mqtt_getIP);

		if (g_shell_result < 0) { // no response
			g_shell_result = 1;
		    vm_log_debug("[MQTT] Error obtaining IP");
			goto exit;
		}
	}

	// === Now we have host IP, connect to MQTT broker ===
    p->clean_session = 1;
    p->check_sec = tmrint;
    p->keep_alive_sec = kaint;
    p->cmd_timeout_ms = DEFAULT_CMD_TIMEOUT_MS;
    p->keep_alive_cnt = 0;

    // **** Start test MQTT Client ****

    // ==== Initialize Network ====
    rc = MqttClientNet_Init(&p->net);
    if (rc != MQTT_CODE_SUCCESS) {
        vm_log_debug("[MQTT] Net Init: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
    	goto exit;
    }

    // ==== Initialize MqttClient structure ====
    rc = MqttClient_Init(&p->client, &p->net, mqttclient_message_cb, p->tx_buf, MAX_BUFFER_SIZE, p->rx_buf, MAX_BUFFER_SIZE, p->cmd_timeout_ms);
	p->client.parrent = p;
    if (rc != MQTT_CODE_SUCCESS) {
        vm_log_debug("[MQTT] Client Init: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
    	goto exit;
    }

    // ==== Connect to broker ====
    rc = MqttClient_NetConnect(&p->client, p->host_IP, p->port, DEFAULT_CON_TIMEOUT_MS, p->use_tls, NULL);
    if (rc != MQTT_CODE_SUCCESS) {
        vm_log_debug("[MQTT] Net Connect: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
    	goto exit;
    }

    // ==== Define connect parameters ====
	MqttConnect connect;
	XMEMSET(&connect, 0, sizeof(MqttConnect));
	connect.keep_alive_sec = p->keep_alive_sec;
	connect.clean_session = p->clean_session;
	connect.client_id = p->client_id;

	// ==== Optional authentication ====
	if (p->username[0] != '\0') connect.username = p->username;
	if (p->password[0] != '\0') connect.password = p->password;

	/// Send 'Connect' and wait for Connect Ack ====
	rc = MqttClient_Connect(&p->client, &connect);
	if (rc == MQTT_CODE_SUCCESS) {
		// ---- Validate Connect Ack info ----
		vm_log_debug("[MQTT] Connect Ack: Return Code %u, Session Present %d",
			connect.ack.return_code,
			(connect.ack.flags & MQTT_CONNECT_ACK_FLAG_SESSION_PRESENT) ? 1 : 0 );
		p->connected = 1;
		// === Create timer for checking new messages ===
		remote_CCall(L, &_timer_create);
	}
	else {
		vm_log_debug("[MQTT] Client Connect: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
	}

exit:
	lua_pushinteger(L, rc);
	return 1;
}

//====================================
static int _timer_delete(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));
	int res;

	if (p->timerHandle >= 0) {
		res = vm_timer_delete_non_precise(p->timerHandle);
		p->timerHandle = -1;
	}

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
    return 0;
}


//======================================
static int mqtt_disconnect(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));

	if (p->timerHandle >= 0) remote_CCall(L, &_timer_delete);

	int rc = MqttClient_Disconnect(&p->client);
    vm_log_debug("[MQTT] Disconnect: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);

    rc = MqttClient_NetDisconnect(&p->client);
	vm_log_debug("[MQTT] Socket Disconnect: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);

	p->connected = 0;

	lua_pushinteger(L, rc);
	return 1;
}

//---------------------------------------------------
static word16 mqttclient_get_packetid(mqtt_info_t *p)
{
    p->mPacketIdLast = (p->mPacketIdLast >= MAX_PACKET_ID) ? 1 : p->mPacketIdLast + 1;
    return (word16)p->mPacketIdLast;
}

/* Add topic
 * mqtt.subscribe(mqt, topic, qos)
 *   mqtt: mqtt client created with mqtt.create()
 *  topic: topic name
 */
//=====================================
static int mqtt_addtopic(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	int len;
	const char* topic_name = luaL_checklstring(L, 2, &len);
    if ((len <= 0) || (len >= MAX_MQTT_TOPIC_LEN)) {
		return luaL_error( L, "topic name error" );
    }
	int qos = p->qos;
	if (lua_isnumber(L, 3)) {
		qos = luaL_checkinteger(L, 3);
	    if ((qos < 0) || (qos > 2)) qos = p->qos;
	}

	int ntopics = 0;
	for (int i=0; i<MAX_MQTT_TOPICS;i++) {
		if ((p->topic_filters[i][0] == '\0') || (p->topics[i].topic_filter == NULL)) {
			snprintf(p->topic_filters[i], len+1, "%s", topic_name);
			p->topics[i].topic_filter = p->topic_filters[i];
			p->topics[i].qos = qos;
			p->topics[i].isSubscribed = 0;
			ntopics++;
	        break;
		}
	}

    if (ntopics == 0) {
		return luaL_error( L, "max topics num exceeded" );
    }
	return 0;
}

/* Subscribe to list of topics
 * mqtt.subscribe(mqt)
 *   mqtt: mqtt client created with mqtt.create()
 */
//=====================================
static int mqtt_subscribe(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	int ntopics = 0;

	for (int i=0; i<MAX_MQTT_TOPICS;i++) {
		if ((p->topic_filters[i][0] != '\0') && (p->topics[i].topic_filter != NULL)) {
			ntopics++;
			p->topics[i].isSubscribed = 1;
		}
		else break;
	}

    if (ntopics == 0) {
		return luaL_error( L, "no topics configured" );
    }

	if (!p->connected) {
		lua_pushinteger(L, -1);
		return 1;
	}

    MqttSubscribe subscribe;
    MqttTopic *topic;
    int i, rc;

    XMEMSET(&subscribe, 0, sizeof(MqttSubscribe));
    subscribe.packet_id = mqttclient_get_packetid(p);
    subscribe.topics = p->topics;
    subscribe.topic_count = ntopics;

    // ==== Subscribe Topic ====

    rc = MqttClient_Subscribe(&p->client, &subscribe);
    vm_log_debug("[MQTT] Subscribe: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);
    if (rc == MQTT_CODE_SUCCESS) {
		for (i = 0; i < subscribe.topic_count; i++) {
			topic = &subscribe.topics[i];
			vm_log_debug("       Topic %s, Qos %u, Return Code %u", topic->topic_filter, topic->qos, topic->return_code);
		}
    }

	lua_pushinteger(L, rc);
	return 1;
}

/* Unsubscribe all or specified topic
 * mqtt.subscribe(mqt, [topic])
 *   mqtt: mqtt client created with mqtt.create()
 *  topic: optional; if given unsubscribe only that topic
 */
//=======================================
static int mqtt_unsubscribe(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));

	if (!p->connected) {
		lua_pushinteger(L, -1);
		return 1;
	}

	const char *topic = NULL;
	if (lua_isstring(L, 2)) {
	    topic = luaL_checkstring(L, 2);
	}

	MqttUnsubscribe unsubscribe;
    int rc = 0;
    int ntopics = 0;

    XMEMSET(&unsubscribe, 0, sizeof(MqttUnsubscribe));
	unsubscribe.packet_id = mqttclient_get_packetid(p);

	if (topic == NULL) {
		for (int i=0; i<MAX_MQTT_TOPICS;i++) {
			if ((p->topic_filters[i][0] != '\0') && (p->topics[i].topic_filter != NULL)) ntopics++;
		}

		if (ntopics > 0) {
			// Unsubscribe Topics
			unsubscribe.topic_count = ntopics;
			unsubscribe.topics = p->topics;

			rc = MqttClient_Unsubscribe(&p->client, &unsubscribe);
			vm_log_debug("[MQTT] Unsubscribe all: %s (%d)", MqttClient_ReturnCodeToString(rc), rc);

			for (int i=0; i<MAX_MQTT_TOPICS;i++) {
				p->topic_filters[i][0] = '\0';
				p->topics[i].topic_filter = NULL;
				p->topics[i].isSubscribed = 0;
			}
		}
    }
    else {
		for (int i=0; i<MAX_MQTT_TOPICS;i++) {
			if ((p->topic_filters[i][0] != '\0') && (p->topics[i].topic_filter != NULL)) {
				if (strcmp(p->topic_filters[i], topic) == 0) {
					ntopics++;
					// Unsubscribe Topic
					unsubscribe.topic_count = ntopics;
					unsubscribe.topics = (MqttTopic  *)&p->topics[i];
					if (p->topics[i].isSubscribed) {
						rc = MqttClient_Unsubscribe(&p->client, &unsubscribe);
					}
					p->topic_filters[i][0] = '\0';
					p->topics[i].topic_filter = NULL;
					p->topics[i].isSubscribed = 0;
					vm_log_debug("[MQTT] Unsubscribe [%s]: %s (%d)", topic, MqttClient_ReturnCodeToString(rc), rc);
					break;
				}
			}
		}
		// Order topics
		MqttTopic *t = NULL;
		for (int i=0; i<MAX_MQTT_TOPICS;i++) {
			t = (MqttTopic  *)&p->topics[i];
			if ((p->topic_filters[i][0] == '\0') || (p->topics[i].topic_filter == NULL)) {
				for (int j=i+1; j<MAX_MQTT_TOPICS;j++) {
					if ((p->topic_filters[j][0] != '\0') && (p->topics[j].topic_filter != NULL)) {
						memcpy(t, &p->topics[j], sizeof(MqttTopic));
						p->topic_filters[j][0] = '\0';
						p->topics[j].topic_filter = NULL;
						break;
					}
				}
			}
		}
    }

	lua_pushinteger(L, rc);
	return 1;
}

/* Publish message to topic
 * mqtt.publish(mqtt, topic, message, [QoS])
 *      mqtt: mqtt client created with mqtt.create()
 *     topic: topis name
 *   message: message to publish
 */
//===================================
static int mqtt_publish(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, 1, LUA_MQTT));
    const char *topic = luaL_checkstring(L, 2);
    const char *message = luaL_checkstring(L, 3);

	if (!p->connected) {
		lua_pushinteger(L, -1);
		return 1;
	}

    MqttPublish publish;
    int rc;
    int qos = p->qos;
    if (lua_isnumber(L, -1)) {
    	qos = luaL_checkinteger(L, -1);
    	if ((qos < 0) || (qos > 2)) qos = p->qos;
    }

    // ==== Publish Topic ====
    XMEMSET(&publish, 0, sizeof(MqttPublish));
    publish.retain = 0;
    publish.qos = qos;
    publish.duplicate = 0;
    publish.topic_name = topic;
    publish.packet_id = mqttclient_get_packetid(p);
    publish.buffer = (byte*)message;
    publish.total_len = (word16)strlen(message);

    rc = MqttClient_Publish(&p->client, &publish);
    vm_log_debug("[MQTT] Publish: Topic %s, %s (%d)", publish.topic_name, MqttClient_ReturnCodeToString(rc), rc);

	lua_pushinteger(L, rc);
	return 1;
}

/* Create MQTT client
 * mqtt.create(config)
 * config: Lua table with mqtt config parameters
 */
//==================================
static int mqtt_create(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}

	mqtt_info_t *p;
	int len = 0;
	int port = 0;  //1883;
	MqttQoS qos = DEFAULT_MQTT_QOS;
	int use_tls = 0;
	const char *host = NULL;
	const char *user = NULL;
	const char *pass = NULL;
	const char *cid = NULL;
	int cb_msg = LUA_NOREF;
	int cb_dis = LUA_NOREF;

	lua_getfield(L, 1, "host");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    host = luaL_checklstring( L, -1, &len );
	    if (len > 63) host = NULL;
	  }
	  else {
 		 vm_log_error("wrong arg type: host" );
 		 goto exit;
	  }
	}
	else {
		vm_log_error("host name missing" );
		goto exit;
	}

	lua_getfield(L, 1, "user");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    user = luaL_checklstring( L, -1, &len );
	    if (len > 31) user = NULL;

	    lua_getfield(L, 1, "pass");
		if (!lua_isnil(L, -1)) {
		  if ( lua_isstring(L, -1) ) {
		    pass = luaL_checklstring( L, -1, &len );
		    if (len > 63) pass = NULL;
		  }
		}
		if (pass == NULL) user = NULL;
	  }
	}

	lua_getfield(L, 1, "clientid");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    cid = luaL_checklstring( L, -1, &len );
	    if (len > 31) cid = NULL;
	  }
	}

	lua_getfield(L, 1, "tls");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    use_tls = luaL_checkinteger( L, -1 );
	    if (use_tls != 0) use_tls = 1;
	  }
	}

	lua_getfield(L, 1, "port");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    port = luaL_checkinteger( L, -1 );
	    if ((port < 0) || (port > 65536)) port = 0;
	  }
	}

	lua_getfield(L, 1, "qos");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    qos = luaL_checkinteger( L, -1 );
	    if ((qos < 0) || (qos > 2)) qos = DEFAULT_MQTT_QOS;
	  }
	}

	lua_getfield(L, 1, "onmessage");
	if (!lua_isnil(L, -1)) {
		if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
			lua_pushvalue(L, -1);
			cb_msg = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}

	lua_getfield(L, 1, "ondisconnect");
	if (!lua_isnil(L, -1)) {
		if ((lua_type(L, -1) == LUA_TFUNCTION) || (lua_type(L, -1) == LUA_TLIGHTFUNCTION)) {
			lua_pushvalue(L, -1);
			cb_dis = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}

	// Create userdata for this mqtt client
	p = (mqtt_info_t *)lua_newuserdata(L, sizeof(mqtt_info_t));
	luaL_getmetatable(L, LUA_MQTT);
	lua_setmetatable(L, -2);

    XMEMSET(p, 0,sizeof(mqtt_info_t));

	p->client.parrent = p;
	strcpy(p->pServer, host);
	if (user) strcpy(p->username, user);
	if (pass) strcpy(p->password, pass);
    p->port = port;
    p->qos = qos;
	p->use_tls = use_tls;
	p->timerHandle = -1;
	if (cid) strcpy(p->client_id, cid);
	else strcpy(p->client_id,DEFAULT_CLIENT_ID);

	for (int i=0; i<MAX_MQTT_TOPICS;i++) {
		//p->topic_filters[i][0] = '\0';
		p->topics[i].topic_filter = NULL;
	}
	p->cb_ref_disconnect = cb_dis;
	p->cb_ref_message = cb_msg;

	vm_log_debug("[MQTT] Client data structure created, size=%d", sizeof(mqtt_info_t));
	return 1;

exit:
	lua_pushnil(L);
	return 1;
}


//-------------------------------
static int _mqtt_gc(lua_State *L)
{
	// check argument
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));
	int res;

	if (p->timerHandle >= 0) res = vm_timer_delete_non_precise(p->timerHandle);

	int rc = MqttClient_NetDisconnect(&p->client);

    // *** Cleanup network ***
    MqttClientNet_DeInit(&p->net);

    /*
    if (g_bearer_hdl >= 0) {
    	vm_bearer_close(g_bearer_hdl);
    	g_bearer_hdl = -1;
    }
    */
    vm_log_debug("gc: mqtt client [%s] deleted", p->pServer);

    g_shell_result = 0;
	vm_signal_post(g_shell_signal);
    return 0;
}

//==============================
static int mqtt_gc(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));

	g_shell_result = -9;
	CCwait = 2000;
    remote_CCall(L, &_mqtt_gc);
	if (g_shell_result < 0) { // no response
	    g_shell_result = 0;
	}

    if (p->cb_ref_disconnect != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref_disconnect);
		p->cb_ref_disconnect = LUA_NOREF;
	}
    if (p->cb_ref_message != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, p->cb_ref_message);
		p->cb_ref_message = LUA_NOREF;
	}

	return g_shell_result;
}

//------------------------------------
static int mqtt_tostring(lua_State *L)
{
	mqtt_info_t *p = ((mqtt_info_t *)luaL_checkudata(L, -1, LUA_MQTT));
    char state[16];
    if (p->connected) sprintf(state,"Connected");
    else sprintf(state,"Disconnected");
    lua_pushfstring(L, "mqtt (%s): host='%s:%d' [%s], user='%s:%s', QoS=%d, useTLS=%d",
    		state, p->pServer, p->port, p->host_IP, p->username, p->password, p->qos, p->use_tls);
    return 1;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE mqtt_map[] = {
		{LSTRKEY("create"), LFUNCVAL(mqtt_create)},
		{LSTRKEY("connect"), LFUNCVAL(mqtt_connect)},
		{LSTRKEY("disconnect"), LFUNCVAL(mqtt_disconnect)},
		{LSTRKEY("addtopic"), LFUNCVAL(mqtt_addtopic)},
		{LSTRKEY("subscribe"), LFUNCVAL(mqtt_subscribe)},
		{LSTRKEY("unsubscribe"), LFUNCVAL(mqtt_unsubscribe)},
		{LSTRKEY("publish"), LFUNCVAL(mqtt_publish)},
		{LSTRKEY("check"), LFUNCVAL(mqtt_check)},
        {LNILKEY, LNILVAL}
};


const LUA_REG_TYPE mqtt_table[] = {
  {LSTRKEY("__gc"), LFUNCVAL(mqtt_gc)},
  {LSTRKEY("__tostring"), LFUNCVAL(mqtt_tostring)},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_mqtt(lua_State *L) {

  luaL_newmetatable(L, LUA_MQTT);		// create metatable for mqtt handles
  lua_pushvalue(L, -1);					// push metatable
  lua_setfield(L, -2, "__index");		// metatable.__index = metatable
  luaL_register(L, NULL, mqtt_table);	// mqtt methods

  luaL_register(L, "mqtt", mqtt_map);
  return 1;
}

