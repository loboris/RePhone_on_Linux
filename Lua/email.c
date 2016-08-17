/* ----------------------------------------------------------------------------
 * email module
 * ----------------------------------------------------------------------------*/

#include <string.h>
#include <time.h>
#include "vmtype.h"
#include "vmlog.h"
#include "vmcmd.h"
#include "vmssl.h"
#include "vmtcp.h"
#include "vmfs.h"
#include "vmstdlib.h"
#include "vmdatetime.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

typedef struct {
	VMINT handle;
	int port;
	int smtp_response;
	int smtp_state;
	VMCHAR *host;
	const char *user;
	const char *pass;
	const char *from;
	const char *from_name;
	const char *to;
	const char *subject;
	const char *msg;
} smtp_t;

static smtp_t *smtp = NULL;
static VM_TIMER_ID_PRECISE smtp_timer_id = -1;


//-------------------------
static void _close(int res)
{
	if (smtp_timer_id >= 0) vm_timer_delete_precise(smtp_timer_id);

	if (smtp != NULL) {
		if (smtp->handle >= 0) {
			if (smtp->port != 25) vm_ssl_close(smtp->handle);
			else vm_tcp_close(smtp->handle);
		}
		smtp->handle = -1;
	}

	if (smtp != NULL) {
		vm_free(smtp);
		smtp = NULL;
	}

	if (g_shell_result == -9) {
		g_shell_result = res;
		vm_signal_post(g_shell_signal);
	}
}

//----------------------------------------------------------------------------
static void smtp_timer_callback(VM_TIMER_ID_PRECISE timer_id, void* user_data)
{
    vm_log_debug("[SMTP] timeout");
	_close(-1);
}

//----------------------------
static void send_message(void)
{
	if (smtp == NULL) return;

	VMINT write_size;
    VMCHAR *write_buf;
    int bptr = 0;
    time_t t;
    VMUINT rtct;
	vm_time_get_unix_time(&rtct);
	t = rtct;
	struct tm *stm;
	stm = localtime(&t);

	write_buf = vm_calloc(512);
	if (write_buf == NULL) vm_log_error("error allocating header buffer");
	else {
		// compose header
		if (smtp->from_name != NULL) sprintf(write_buf,"From: %s<%s>\r\n", smtp->from_name, smtp->from);
		else sprintf(write_buf,"From: %s\r\n", smtp->from);
		bptr = strlen(write_buf);
		sprintf(write_buf+bptr,"To: %s\r\n", smtp->to);
		bptr = strlen(write_buf);
		if (smtp->subject != NULL) sprintf(write_buf+bptr,"Subject: %s\r\n", smtp->subject);
		else sprintf(write_buf+bptr,"Subject: RePhone msg\r\n");
		bptr = strlen(write_buf);
		sprintf(write_buf+bptr,"Date: %s", asctime(stm));
		bptr = strlen(write_buf)-1;
		sprintf(write_buf+bptr,"\r\n\r\n");

	    if (smtp->port != 25) {
	    	write_size = vm_ssl_write(smtp->handle, write_buf, strlen(write_buf));
	    }
	    else {
	    	write_size = vm_tcp_write(smtp->handle, write_buf, strlen(write_buf));
	    }

        vm_log_debug("[SMTP] HEADER:\n%s===", write_buf);

        // send message body
		sprintf(write_buf,"\r\n.\r\n");
	    if (smtp->msg != NULL) {
		    if (smtp->port != 25) {
		    	write_size = vm_ssl_write(smtp->handle, smtp->msg, strlen(smtp->msg));
		    	write_size = vm_ssl_write(smtp->handle, write_buf, strlen(write_buf));
		    }
		    else {
		    	write_size = vm_tcp_write(smtp->handle, (char *)smtp->msg, strlen(smtp->msg));
		    	write_size = vm_tcp_write(smtp->handle, write_buf, strlen(write_buf));
		    }
	    }
	    else {
		    if (smtp->port != 25) {
		    	write_size = vm_ssl_write(smtp->handle, write_buf, strlen(write_buf));
		    }
		    else {
		    	write_size = vm_tcp_write(smtp->handle, write_buf, strlen(write_buf));
		    }
	    }

	    vm_free(write_buf);
	}
}

//-------------------------
static void __smtp_send(void)
{
	if (smtp->smtp_response <= 0) return;
	if (smtp == NULL) return;

    VMINT write_size = 0;
    VMCHAR write_buf[128] = {'\0'};
    VMCHAR b64_write_buf[96] = {'\0'};
    int do_send = 1;

    switch (smtp->smtp_response) {
        case 220:
        	sprintf(write_buf, "HELO %s\r\n", smtp->host);
            break;
        case 250:
        	if (smtp->smtp_state == 0) {
        		if ((smtp->user != NULL) && (smtp->pass != NULL)) {
					sprintf(write_buf, "AUTH LOGIN\r\n");
					smtp->smtp_state = 1;
        		}
        		else {
            		sprintf(write_buf, "MAIL FROM:<%s>\r\n", smtp->from);
            		smtp->smtp_state = 4;
        		}
        	}
        	else if (smtp->smtp_state == 3) {
        		sprintf(write_buf, "MAIL FROM:<%s>\r\n", smtp->from);
        		smtp->smtp_state = 4;
        	}
        	else if (smtp->smtp_state == 4) {
        		sprintf(write_buf, "RCPT TO:<%s>\r\n", smtp->to);
        		smtp->smtp_state = 5;
        	}
        	else if (smtp->smtp_state == 5) {
        		sprintf(write_buf, "DATA\r\n");
        		smtp->smtp_state = 6;
        	}
        	else if (smtp->smtp_state == 7) {
        		sprintf(write_buf, "QUIT\r\n");
        		smtp->smtp_state = 8;
        	}
            break;
        case 334:
        	// user & pass
            write_size = 0;
        	if (smtp->smtp_state == 1) {
        		vm_ssl_base64_encode(b64_write_buf, &write_size, smtp->user, strlen(smtp->user));
        		vm_ssl_base64_encode(b64_write_buf, &write_size, smtp->user, strlen(smtp->user));
            	b64_write_buf[write_size] = '\0';
                vm_log_debug("[SMTP] encode user: %d [%s] [%s]", write_size, smtp->user, b64_write_buf);
        		smtp->smtp_state = 2;
        	}
        	else if (smtp->smtp_state == 2) {
        		vm_ssl_base64_encode(b64_write_buf, &write_size, smtp->pass, strlen(smtp->pass));
        		vm_ssl_base64_encode(b64_write_buf, &write_size, smtp->pass, strlen(smtp->pass));
            	b64_write_buf[write_size] = '\0';
                vm_log_debug("[SMTP] encode pass: %d [%s] [%s]", write_size, smtp->pass, b64_write_buf);
        		smtp->smtp_state = 3;
        	}
        	sprintf(write_buf, "%s\r\n", b64_write_buf);
            break;
        case 354:
        	send_message();
        	do_send = 0;
        	smtp->smtp_state = 7;
            break;
        case 235:
        	// Accepted (user&pass)
    		sprintf(write_buf, "NOOP\r\n");
            break;
        case 221:
        	// Disconnect
        	if (smtp->smtp_state > 6) {
                vm_log_debug("[SMTP] finished");
                _close(0);
        	}
        	else {
                vm_log_debug("[SMTP] disconnected");
                _close(-1);
        	}
        	return;
            break;
        default:
            vm_log_debug("[SMTP] unhandled response: %d", smtp->smtp_response);
            if (smtp->smtp_response >= 400) {
        		sprintf(write_buf, "QUIT\r\n");
            }
            else {
        		sprintf(write_buf, "NOOP\r\n");
            }
    }
	if (do_send) {
	    if (smtp->port != 25) {
	    	write_size = vm_ssl_write(smtp->handle, write_buf, strlen(write_buf));
	    }
	    else {
	    	write_size = vm_tcp_write(smtp->handle, write_buf, strlen(write_buf));
	    }
        vm_log_debug("[SMTP] write %d: %s", strlen(write_buf), write_buf);
	}
}

//------------------------------------------------------------
static void ssl_connection_callback(VMINT handle, VMINT event)
{
    VMINT read_size;
    VM_SSL_VERIFY_RESULT verify_result;
    VMCHAR read_buf[64] = {'\0'};
    VMCHAR dummy_buf[64] = {'\0'};
    VMCHAR c = ' ';;

    switch(event)
    {
        case VM_SSL_EVENT_CONNECTED:
        	smtp->handle = handle;
            vm_log_debug("[SMTP] connected");
            break;

        case VM_SSL_EVENT_CAN_WRITE:
            vm_log_debug("[SMTP] can write");
            break;

        case VM_SSL_EVENT_CAN_READ:
        	if (smtp == NULL) return;

        	// read response, get response code & send command
        	read_size = vm_ssl_read(handle, read_buf, 64);
        	read_buf[63] = '\0';
			if ((read_size > 0) && (read_size < 64)) read_buf[read_size] = '\0';
            while (read_size > 0) {
                read_size = vm_ssl_read(handle, dummy_buf, 64);
            }
            c = read_buf[3];
			read_buf[3] = '\0';
			smtp->smtp_response = vm_str_strtoi(read_buf);
			read_buf[3] = c;
            vm_log_debug("[SMTP] response: %d, state: %d %s", smtp->smtp_response, smtp->smtp_state, read_buf);
            __smtp_send();
            break;

        case VM_SSL_EVENT_CERTIFICATE_VALIDATION_FAILED:
        case VM_SSL_EVENT_HANDSHAKE_FAILED:
            verify_result = vm_ssl_get_verify_result(smtp->handle);
            vm_log_debug("[SMTP] handshake failed %d", verify_result);
        	if ((smtp != NULL) && (smtp->smtp_state > 6)) _close(0);
        	else _close(-1);
            break;

        case VM_SSL_EVENT_PIPE_BROKEN:
        	vm_log_debug("[SMTP] The socket has timed out or the connection has broken.");
        	if (smtp->smtp_state > 6) _close(0);
        	else _close(-1);
        	break;

        case VM_SSL_EVENT_HOST_NOT_FOUND:
        	vm_log_debug("[SMTP] Cannot reach the host specified.");
        	_close(-1);
        	break;

        case VM_SSL_EVENT_PIPE_CLOSED:
        	vm_log_debug("[SMTP] The socket is closed.");
        	if ((smtp != NULL) && (smtp->smtp_state > 6)) _close(0);
        	else _close(-1);
        	break;
    }
}

//--------------------------------------------------------------------------------------------
static void tcp_connection_callback(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
    VMINT read_size;
    VM_SSL_VERIFY_RESULT verify_result;
    VMCHAR read_buf[64] = {'\0'};
    VMCHAR dummy_buf[64] = {'\0'};
    VMCHAR c = ' ';;

    switch(event)
    {
        case VM_TCP_EVENT_CONNECTED:
        	smtp->handle = handle;
            vm_log_debug("[SMTP] connected");
            break;

        case VM_TCP_EVENT_CAN_WRITE:
            vm_log_debug("[SMTP] can write");
            break;

        case VM_TCP_EVENT_PIPE_CLOSED:
        	vm_log_debug("[SMTP] The socket is closed.");
        	if ((smtp != NULL) && (smtp->smtp_state > 6)) _close(0);
        	else _close(-1);
        	break;

        case VM_TCP_EVENT_CAN_READ:
        	if (smtp == NULL) return;

        	// read response, get response code & send command
        	read_size = vm_tcp_read(handle, read_buf, 64);
        	read_buf[63] = '\0';
			if ((read_size > 0) && (read_size < 64)) read_buf[read_size] = '\0';
            while (read_size > 0) {
                read_size = vm_tcp_read(handle, dummy_buf, 64);
            }
            c = read_buf[3];
			read_buf[3] = '\0';
			smtp->smtp_response = vm_str_strtoi(read_buf);
			read_buf[3] = c;
            vm_log_debug("[SMTP] response: %d, state: %d %s", smtp->smtp_response, smtp->smtp_state, read_buf);
            __smtp_send();
            break;

        case VM_TCP_EVENT_PIPE_BROKEN:
        	vm_log_debug("[SMTP] The socket has timed out or the connection has broken.");
        	if ((smtp != NULL) && (smtp->smtp_state > 6)) _close(0);
        	else _close(-1);
        	break;

        case VM_TCP_EVENT_HOST_NOT_FOUND:
        	vm_log_debug("[SMTP] Cannot reach the host specified.");
        	_close(-1);
        	break;
    }
}

//==========================
int _smtp_send(lua_State* L)
{
	int len = 0;
	if (smtp != NULL) {
		vm_free(smtp);
		smtp = NULL;
	}

	smtp = vm_calloc(sizeof(smtp_t));
	if (smtp == NULL) {
		 vm_log_error("error allocating buffer" );
		 goto exit;
	}

	smtp->handle = -1;
	smtp->smtp_response = 0;
	smtp->smtp_state = 0;

	// === Get parameters
	lua_getfield(L, 1, "host");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->host = (VMCHAR *)luaL_checklstring( L, -1, &len );
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

	smtp->port = 25;
	lua_getfield(L, 1, "port");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    smtp->port = luaL_checkinteger( L, -1 );
	  }
	}

	lua_getfield(L, 1, "from");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->from = luaL_checklstring( L, -1, &len );
	  }
	  else {
 		 vm_log_error("wrong arg type: from" );
 		 goto exit;
	  }
	}
	else {
		vm_log_error("FROM address missing" );
		goto exit;
	}

	lua_getfield(L, 1, "from_name");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->from_name = luaL_checklstring( L, -1, &len );
	  }
	  else {
 		 vm_log_error("wrong arg type: from_name" );
 		 goto exit;
	  }
	}
	else smtp->from_name = smtp->from;

	lua_getfield(L, 1, "to");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->to = luaL_checklstring( L, -1, &len );
	  }
	  else {
 		 vm_log_error("wrong arg type: to" );
 		 goto exit;
	  }
	}
	else {
		vm_log_error("TO address missing" );
		goto exit;
	}

	smtp->subject = NULL;
	smtp->msg = NULL;
	lua_getfield(L, 1, "subject");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->subject = luaL_checklstring( L, -1, &len );
	  }
	}

	lua_getfield(L, 1, "msg");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->msg = luaL_checklstring( L, -1, &len );
	  }
	}

	if ((smtp->subject == NULL) && (smtp->msg == NULL)) {
		vm_log_error("subject or message must be set" );
		goto exit;
	}

	smtp->user = NULL;
	smtp->pass = NULL;
	lua_getfield(L, 1, "user");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->user = luaL_checklstring( L, -1, &len );
	  }
	}

	lua_getfield(L, 1, "pass");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    smtp->pass = luaL_checklstring( L, -1, &len );
	  }
	}

	if (((smtp->user != NULL) && (smtp->pass == NULL)) || ((smtp->user == NULL) && (smtp->pass != NULL))) {
		vm_log_error("both user and pass must be set" );
		goto exit;
	}

	// create timeout timer
    smtp_timer_id = vm_timer_create_precise(25000, smtp_timer_callback, NULL);

    if (smtp->port != 25) {
        vm_ssl_context_t context;

        context.host = smtp->host;
		context.port = smtp->port;
		context.connection_callback = (vm_ssl_connection_callback)ssl_connection_callback;
		context.authorize_mode = VM_SSL_VERIFY_NONE;
		context.user_agent = NULL;

		smtp->handle = vm_ssl_connect(&context);
    }
    else {
        smtp->handle = vm_tcp_connect(smtp->host, smtp->port, gprs_bearer_type, NULL, tcp_connection_callback);
    }

    if (smtp->handle < 0) {
    	vm_log_error("error starting connection %d", smtp->handle );
    	_close(-1);
    }

    return 0;

exit:
	if (smtp != NULL) {
		vm_free(smtp);
		smtp = NULL;
	}
	g_shell_result = -1;
	vm_signal_post(g_shell_signal);
	return 0;
}

//================================
static int smtp_send(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}

    g_shell_result = -9;
	CCwait = 30000;

	remote_CCall(L, &_smtp_send);

	if (g_shell_result < 0) lua_pushinteger(L, -1); // no response or error
	else lua_pushinteger(L, 0);

	return 1;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE email_map[] = {
		{LSTRKEY("send"), LFUNCVAL(smtp_send)},
        {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_email(lua_State *L) {

  luaL_register(L, "email", email_map);
  return 1;
}

