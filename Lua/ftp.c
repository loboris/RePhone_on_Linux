/* ----------------------------------------------------------------------------
 * FTP module
 * GPRS (General packet radio services) 2.5G -- 35Kbps to 171kbps
 * ----------------------------------------------------------------------------*/

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "vmtype.h"
#include "vmlog.h"
#include "vmcmd.h"
#include "vmssl.h"
#include "vmtcp.h"
#include "vmfs.h"
#include "vmstdlib.h"
#include "vmdatetime.h"
#include "vmchset.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"


#define FTP_RECEIVE_TO_FILE		1
#define FTP_RECEIVE_TO_STRING	2
#define FTP_SEND_FROM_FILE		3
#define FTP_SEND_FROM_STRING	4


extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

typedef struct {
	VMINT handle;
	VMINT data_handle;
	int port;
	char data_ip[18];
	int data_port;
	int data_recv;
	int connected;
	int data_connected;
	int data_length;
	int ftp_response;
	int ftp_transfer;
	int file_handle;
	char command[128];
	VMCHAR *host;
	const char *user;
	const char *pass;
	char *buffer;
	int bufptr;
	int bufsize;
	int totable;
} ftp_t;

static ftp_t *ftp = NULL;

//-------------------------
static void _close(int res)
{
	if (ftp != NULL) {
		if (ftp->data_handle >= 0) {
			// Close data connection
			vm_tcp_close_sync(ftp->data_handle);
			ftp->data_handle = -1;
		}
		if ((ftp->handle >= 0) && (res < 9)) {
			if ((ftp->handle >= 0) && (res < 9)) {
				ftp->handle = -1;
			}
			vm_free(ftp);
			ftp = NULL;
		}
	}

	if (g_shell_result == -9) {
		if (res > 9) g_shell_result = -1 * res;
		if (res == 9) g_shell_result = 0;
		else g_shell_result = res;
		vm_signal_post(g_shell_signal);
	}
}

// Transfer data over data channel
//-------------------------
static void _transfer(void)
{
    VMINT read_size, write_size, res;

    if ((ftp->data_connected == 0) || (ftp->data_handle < 0)) {
    	// error, no data connection
    	_close(10);
    	return;
    }

	res = 9;
    if (ftp->ftp_transfer == FTP_SEND_FROM_STRING) {
    	// === send from string buffer
		if (ftp->buffer != NULL) {
			if (ftp->bufsize > 0) {
				ftp->data_length = vm_tcp_write_sync(ftp->data_handle, ftp->buffer, ftp->bufsize);
				if (ftp->data_length < 0) {
					ftp->data_length = 0;
					res = 11;
				}
			}
		}
		else res = 12;
	}
	else if (ftp->ftp_transfer == FTP_SEND_FROM_FILE) {
		// === send from file
		if (ftp->file_handle >= 0) {
			char *buf = vm_malloc(1024);
			do {
				read_size = vm_fs_read(ftp->file_handle, buf, 1024, &write_size);
				ftp->data_length += write_size;
				if (write_size > 0)	{
					write_size = vm_tcp_write_sync(ftp->data_handle, buf, write_size);
					if (write_size < 0) {
						ftp->data_length = 0;
						res = 13;
					}
				}
			} while (write_size > 0);
		}
		else res = 14;
	}
	else if (ftp->ftp_transfer == FTP_RECEIVE_TO_STRING) {
		// === receive to string buffer
		vm_free(ftp->buffer);
		ftp->buffer = vm_malloc(1024);
		ftp->bufsize = 1024;
		ftp->bufptr = 0;
		ftp->data_length = 0;
		do {
			if (ftp->buffer != NULL) {
				if ((ftp->bufsize - ftp->bufptr) < 256) {
					int new_size = ftp->bufsize + 256;
					char *new_buf = vm_realloc(ftp->buffer, new_size);
					if (new_buf != NULL) {
						ftp->buffer = new_buf;
						ftp->bufsize = new_size;
						read_size = vm_tcp_read_sync(ftp->data_handle, ftp->buffer+ftp->bufptr, ftp->bufsize - ftp->bufptr);
						if (read_size > 0) {
							ftp->bufptr += read_size;
							ftp->data_length += read_size;
						}
					}
					else {
						// dummy read
						char *buf[128];
						read_size = vm_tcp_read_sync(ftp->data_handle, buf, 128);
						if (read_size > 0) {
							ftp->data_length += read_size;
						}
					}
				}
				else {
					read_size = vm_tcp_read_sync(ftp->data_handle, ftp->buffer+ftp->bufptr, ftp->bufsize - ftp->bufptr);
					if (read_size > 0) {
						ftp->bufptr += read_size;
						ftp->data_length += read_size;
					}
				}
			}
			else {
				read_size = 0;
				res = 16;
			}
		} while (read_size > 0);
	}
	else if (ftp->ftp_transfer == FTP_RECEIVE_TO_FILE) {
		// === receive to file
		vm_free(ftp->buffer);
		ftp->buffer = vm_malloc(1024);
		ftp->bufsize = 1024;
		ftp->bufptr = 0;
		ftp->data_length = 0;
		do {
			if ((ftp->buffer != NULL) && (ftp->file_handle >= 0)) {
				read_size = vm_tcp_read_sync(ftp->data_handle, ftp->buffer, ftp->bufsize);
				if (read_size > 0) {
					res = vm_fs_write(ftp->file_handle, ftp->buffer, read_size, &write_size);
					if ((res < 0) || (read_size != write_size)) {
						read_size = 0;
						res = 17;
					}
					else ftp->bufptr += write_size;
				}
			}
			else {
				read_size = 0;
				res = 18;
			}
		} while (read_size > 0);
		vm_free(ftp->buffer);
		ftp->buffer = NULL;
	}
	else res = 19; // error, no file transfer active

    _close(res);
}

// Analize server reply and take action
//-------------------------------------
static void ftp_response(char *recvBuf)
{
	if (ftp->ftp_response <= 0) return;
	if (ftp == NULL) return;

    VMINT write_size = 0;
    VMCHAR write_buf[128] = {'\0'};
    int do_send = 1;

    switch (ftp->ftp_response) {
        case 220:
        	sprintf(write_buf, "USER %s\r\n", ftp->user);
            break;
        case 331:
        	sprintf(write_buf, "PASS %s\r\n", ftp->pass);
            break;
        case 230:
        	ftp->connected = 1;
        	sprintf(write_buf, "TYPE I\r\n");
            break;
        case 200:
            vm_log_debug("[FTP] Command okay");
        	_close(9);
            do_send = 0;
            break;
        case 221:
            vm_log_debug("[FTP] Request Logout");
            do_send = 0;
        	_close(0);
            break;
        case 226:
            vm_log_debug("[FTP] Transfer complete, closing data connection");
            do_send = 0;
            break;
        case 125:
            vm_log_debug("[FTP] Data connection already open; transfer starting");
           	_transfer();
            do_send = 0;
        	break;
        case 150:
            vm_log_debug("[FTP] Start transfer");
           	_transfer();
            do_send = 0;
            break;
        case 421:
            vm_log_debug("[FTP] Connection timeout");
            do_send = 0;
        	_close(0);
            break;
        case 227: {
        		vm_log_debug("[FTP] Entering passive mode");
            	sprintf(write_buf, "QUIT\r\n");
	            if (strlen(ftp->command) == 0) {
	                vm_log_error("[FTP] No transfer command found");
	            }
	            else {
					// get dataport IP & Port
					char *tStr = strtok(recvBuf, "(,");
					uint8_t array_pasv[6] = {0,0,0,0,0,0};
					for ( int i = 0; i < 6; i++) {
					  tStr = strtok(NULL, "(,");
					  if (tStr != NULL) {
						array_pasv[i] = atoi(tStr);
					  }
					  else {
						array_pasv[i] = 0;
					  }
					}
					ftp->data_port = (array_pasv[4] << 8) + (array_pasv[5] & 255);
					sprintf(ftp->data_ip, "%d.%d.%d.%d", array_pasv[0], array_pasv[1], array_pasv[2], array_pasv[3]);

					if ((ftp->data_port > 0) && (strstr(ftp->data_ip, "0.0.0.0.0") == NULL)) {
						// === Start data connection ===
						vm_log_debug("[FTP] Opening data connection to: %s:%d", ftp->data_ip, ftp->data_port);

						ftp->data_connected = 0;
						ftp->data_handle = vm_tcp_connect_sync(ftp->data_ip, ftp->data_port, gprs_bearer_type);
						if (ftp->data_handle < 0) {
							vm_log_error("[FTP] Error opening data connection");
						}
						else {
							// == Send transfer command ==
							vm_log_debug("[FTP] DATA connected");
							ftp->data_length = 0;
							ftp->data_connected = 1;
							sprintf(write_buf, "%s\r\n", ftp->command);
							ftp->command[0] = '\0';
						}
					}
					else {
						vm_log_error("[FTP] Cannot open data connection");
					}
	            }
        	}
            break;
        case 250:
            vm_log_debug("[FTP] Requested file operation okay; completed");
            _close(9);
            do_send = 0;
            break;
        case 257:
        	{
				vm_log_debug("[FTP] Pathname received");
				vm_free(ftp->buffer);
				ftp->buffer = vm_calloc(strlen(recvBuf)+2);
				if (ftp->buffer != NULL) {
					int i;
					for (i = 0; i<strlen(recvBuf); i++) {
						if (*(recvBuf+i) == '"') break;
					}
					if (i<strlen(recvBuf)) memmove(recvBuf, recvBuf+i+1, strlen(recvBuf)-i);
					else memmove(recvBuf, recvBuf+4, strlen(recvBuf)-3);
					for (i=strlen(recvBuf)-1; i>0; i--) {
						if ((*(recvBuf+i) == '"') || (*(recvBuf+i) == '"') || (*(recvBuf+i) == '"')) *(recvBuf+i) = '\0';
					}
					memcpy(ftp->buffer, recvBuf, strlen(recvBuf)+1);
					ftp->data_length = strlen(recvBuf);
					ftp->bufptr = strlen(recvBuf);
					_close(9);
				}
				else _close(10);
				do_send = 0;
        	}
            break;
        case 550:
        	recvBuf[strlen(recvBuf)-2] = '\0';
            vm_log_error("[FTP] ERROR: [%s]", recvBuf);
            do_send = 0;
            _close(10);
            break;
        case 530:
            vm_log_debug("[FTP] Not logged in");
        	sprintf(write_buf, "QUIT\r\n");
            break;
        default:
        	recvBuf[strlen(recvBuf)-2] = '\0';
            vm_log_debug("[FTP] unhandled response: [%s]", recvBuf);
            _close(10);
            do_send = 0;
    }
	if (do_send) {
    	write_size = vm_tcp_write(ftp->handle, write_buf, strlen(write_buf));
    	write_buf[strlen(write_buf)-2] = '\0';
        vm_log_debug("[FTP] >>> [%s]", write_buf);
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
        	ftp->handle = handle;
            vm_log_debug("[FTP] connected");
            break;

        case VM_TCP_EVENT_CAN_WRITE:
            vm_log_debug("[FTP] can write");
            break;

        case VM_TCP_EVENT_PIPE_CLOSED:
        	vm_log_debug("[FTP] The socket is closed.");
        	_close(-1);
        	break;

        case VM_TCP_EVENT_CAN_READ:
        	if (ftp == NULL) return;

        	// read response, get response code & send command
        	read_size = vm_tcp_read(handle, read_buf, 64);
        	read_buf[63] = '\0';
			if ((read_size > 0) && (read_size < 64)) read_buf[read_size] = '\0';
            while (read_size > 0) {
                read_size = vm_tcp_read(handle, dummy_buf, 64);
            }
            c = read_buf[3];
			read_buf[3] = '\0';
			ftp->ftp_response = vm_str_strtoi(read_buf);
			read_buf[3] = c;
            //vm_log_debug("[FTP] response: %d, state: %d %s", ftp->ftp_response, ftp->ftp_state, read_buf);
            ftp_response(read_buf);
            break;

        case VM_TCP_EVENT_PIPE_BROKEN:
        	vm_log_debug("[FTP] The socket has timed out or the connection has broken.");
        	_close(-1);
        	break;

        case VM_TCP_EVENT_HOST_NOT_FOUND:
        	vm_log_debug("[FTP] Cannot reach the host specified.");
        	_close(-1);
        	break;
    }
}

//----------------------------
static int check_connect(void)
{
	if ((ftp == NULL) || (ftp->connected == 0)) {
    	vm_log_error("Not connected.");
		g_shell_result = -1;
		vm_signal_post(g_shell_signal);
		return 1;
	}
	else return 0;
}

//============================
int _ftp_connect(lua_State* L)
{
	int len = 0;

	if ((ftp != NULL) && (ftp->connected > 0)) {
    	vm_log_error("Already connected.");
		g_shell_result = -1;
		vm_signal_post(g_shell_signal);
		return 0;
	}

	vm_free(ftp);

	ftp = vm_calloc(sizeof(ftp_t));
	if (ftp == NULL) {
		 vm_log_error("error allocating buffer" );
		 goto exit;
	}

	ftp->handle = -1;
	ftp->data_handle = -1;
	ftp->file_handle = -1;

	// === Get parameters
	lua_getfield(L, 1, "host");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    ftp->host = (VMCHAR *)luaL_checklstring( L, -1, &len );
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

	ftp->port = 21;
	lua_getfield(L, 1, "port");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isnumber(L, -1) ) {
	    ftp->port = luaL_checkinteger( L, -1 );
	  }
	}

	ftp->user = NULL;
	ftp->pass = NULL;
	lua_getfield(L, 1, "user");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    ftp->user = luaL_checklstring( L, -1, &len );
	  }
	}

	lua_getfield(L, 1, "pass");
	if (!lua_isnil(L, -1)) {
	  if ( lua_isstring(L, -1) ) {
	    ftp->pass = luaL_checklstring( L, -1, &len );
	  }
	}

    ftp->handle = vm_tcp_connect(ftp->host, ftp->port, gprs_bearer_type, NULL, tcp_connection_callback);

    if (ftp->handle < 0) {
    	vm_log_error("error starting connection %d", ftp->handle );
    	_close(-1);
    }

    return 0;

exit:
	if (ftp != NULL) {
		vm_free(ftp);
		ftp = NULL;
	}
	g_shell_result = -1;
	vm_signal_post(g_shell_signal);
	return 0;
}

//==================================
static int ftp_connect(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_error( L, "table arg expected" );
	}

    g_shell_result = -9;
	CCwait = 20000;

	remote_CCall(L, &_ftp_connect);

	if (g_shell_result < 0) {
		_close(9);
		lua_pushinteger(L, -1); // no response or error
	}
	else lua_pushinteger(L, 0);

	return 1;
}

//===============================
int _ftp_disconnect(lua_State* L)
{
	if (check_connect()) return 0;

   	int write_size = vm_tcp_write(ftp->handle, "QUIT\r\n", 6);
	return 0;
}

//=====================================
static int ftp_disconnect(lua_State *L)
{
    g_shell_result = -9;
	CCwait = 5000;

	remote_CCall(L, &_ftp_disconnect);

	if (g_shell_result < 0) lua_pushinteger(L, -1); // no response or error
	else lua_pushinteger(L, 0);

	return 1;
}

//==============================
int _ftp_getstring(lua_State* L)
{
	if ((ftp->buffer != NULL) && (ftp->bufptr > 0)) {
		if (ftp->totable == 0) {
			lua_pushinteger(L, ftp->bufptr);
			lua_pushlstring(L, ftp->buffer, ftp->bufptr);
		}
		else {
			int i, j, nlin;
			i = 0;
			j = 0;
			nlin = 0;
			while (i < ftp->bufptr) {
				if ((ftp->buffer[i] == '\r') || (ftp->buffer[i] == '\n')) {
			        nlin++;
			        i++;
			        j = i;
					while ((i < ftp->bufptr) && ((ftp->buffer[i] == '\r') || (ftp->buffer[i] == '\n'))) {
						i++;
						j++;
					}
				}
				else i++;
			}
			lua_pushinteger(L, nlin);

			i = 0;
			j = 0;
			nlin = 0;
			lua_newtable(L);
			while (i < ftp->bufptr) {
				if ((ftp->buffer[i] == '\r') || (ftp->buffer[i] == '\n')) {
			        nlin++;
					lua_pushlstring( L, ftp->buffer+j, i-j );
			        lua_rawseti(L, -2, nlin);
			        i++;
			        j = i;
					while ((i < ftp->bufptr) && ((ftp->buffer[i] == '\r') || (ftp->buffer[i] == '\n'))) {
						i++;
						j++;
					}
				}
				else i++;
			}
		}
		if (ftp->bufptr != ftp->data_length) {
	    	vm_log_error("Not all data received %d [%d]", ftp->bufptr, ftp->data_length);
		}
	}
	else {
		lua_pushinteger(L, -1);
		lua_pushstring(L, "");
	}
	vm_free(ftp->buffer);
	ftp->buffer = NULL;

	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 0;
}

//==============================
int _ftp_closefile(lua_State* L)
{
	vm_fs_close(ftp->file_handle);
	lua_pushinteger(L, ftp->data_length);

	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 0;
}

//==============================
int _ftp_closesend(lua_State* L)
{
	if (ftp->file_handle >= 0) {
		vm_fs_close(ftp->file_handle);
	}
	else {
		vm_free(ftp->buffer);
		ftp->buffer = NULL;
	}
	lua_pushinteger(L, ftp->data_length);

	g_shell_result = 1;
	vm_signal_post(g_shell_signal);
	return 0;
}

//=========================
int _ftp_list(lua_State* L)
{
	if (check_connect()) return 0;

	const char *fspec = luaL_checkstring(L, 1);

	if (lua_isstring(L, 2)) {
		size_t len = 0;
		const char *filename = luaL_checklstring(L, 2, &len);
		if ((len > 63) || (len < 1)) {
	    	vm_log_error("wrong file name");
			g_shell_result = -1;
			vm_signal_post(g_shell_signal);
			return 0;
		}
		if (strstr(filename, "*totable") != NULL) {
			ftp->totable = 1;
			ftp->file_handle = -1;
			ftp->ftp_transfer = FTP_RECEIVE_TO_STRING;
		}
		else {
			ftp->totable = 0;
			VMWCHAR ucs_name[128];
		    full_fname((char *)filename, ucs_name, 128);
			int res = vm_fs_open(ucs_name, VM_FS_MODE_CREATE_ALWAYS_WRITE, VM_TRUE);
			if (res < 0) {
				vm_log_error("error creating local file: %d", res);
				g_shell_result = -1;
				vm_signal_post(g_shell_signal);
				return 0;
			}
			ftp->file_handle = res;
			ftp->ftp_transfer = FTP_RECEIVE_TO_FILE;
		}
	}
	else {
		ftp->totable = 0;
		ftp->file_handle = -1;
		ftp->ftp_transfer = FTP_RECEIVE_TO_STRING;
	}
	vm_free(ftp->buffer);
	ftp->buffer = NULL;
	ftp->bufptr = 0;
	ftp->data_length = 0;
	ftp->data_length = 0;
	sprintf(ftp->command, "LIST %s", fspec);
    int write_size = vm_tcp_write(ftp->handle, "PASV\r\n", 6);

	return 0;
}

//===============================
static int ftp_list(lua_State *L)
{
	const char *fspec = luaL_checkstring(L, 1);

	g_shell_result = -9;
	CCwait = 10000;

	remote_CCall(L, &_ftp_list);

	if (g_shell_result < 0) {
		lua_pushinteger(L, g_shell_result); // no response or error
		return 1;
	}
	else {
		if (ftp->file_handle >= 0) {
			remote_CCall(L, &_ftp_closefile);
			return 1;
		}
		else {
			remote_CCall(L, &_ftp_getstring);
			return 2;
		}
	}
}

/* ftp.recv(rfile, [lfile], ["*tostring"])
 * Receive file from ftp server to local file or string
 */
//=========================
int _ftp_recv(lua_State* L)
{
	if (check_connect()) return 0;

	char *file = luaL_checkstring(L, 1);

	int tofile = 0;
	char lfilename[64];

	if (lua_isstring(L, 2)) {
		size_t len = 0;
		const char *filename = luaL_checklstring(L, 2, &len);
		if ((len > 63) || (len < 1)) {
	    	vm_log_error("wrong file name");
			g_shell_result = -1;
			vm_signal_post(g_shell_signal);
			return 0;
		}
		if (strstr(filename, "*tostring") == NULL) {
			tofile = 1;
			sprintf(lfilename, filename, strlen(filename));
		}
	}
	else {
		tofile = 1;
		sprintf(lfilename, file, strlen(file));
	}

	if (tofile) {
		VMWCHAR ucs_name[128];
	    full_fname(lfilename, ucs_name, 128);
		int res = vm_fs_open(ucs_name, VM_FS_MODE_CREATE_ALWAYS_WRITE, VM_TRUE);
		if (res < 0) {
	    	vm_log_error("error creating local file: %d", res);
			g_shell_result = -1;
			vm_signal_post(g_shell_signal);
			return 0;
		}
		ftp->file_handle = res;
		ftp->ftp_transfer = FTP_RECEIVE_TO_FILE;
	}
	else {
		ftp->file_handle = -1;
		ftp->ftp_transfer = FTP_RECEIVE_TO_STRING;
	}

	vm_free(ftp->buffer);
	ftp->totable = 0;
	ftp->buffer = NULL;
	ftp->bufptr = 0;
	ftp->data_length = 0;
	ftp->data_length = 0;
	sprintf(ftp->command, "RETR %s", file);
    int write_size = vm_tcp_write(ftp->handle, "PASV\r\n", 6);

	return 0;
}

//===============================
static int ftp_recv(lua_State *L)
{
	char *file = luaL_checkstring(L, 1);

	g_shell_result = -9;
	CCwait = 30000;
	remote_CCall(L, &_ftp_recv);

	if (g_shell_result < 0) {
		lua_pushinteger(L, g_shell_result); // no response or error
		return 1;
	}
	else {
		if (ftp->file_handle >= 0) {
			remote_CCall(L, &_ftp_closefile);
			return 1;
		}
		else {
			remote_CCall(L, &_ftp_getstring);
			return 2;
		}
	}
}

//-------------------------------
static int _ftp_pwd(lua_State* L)
{
	if (check_connect()) return 0;

	ftp->totable = 0;
   	int write_size = vm_tcp_write(ftp->handle, "PWD\r\n", 5);

   	return 0;
}

//==============================
static int ftp_pwd(lua_State *L)
{
	g_shell_result = -9;
	CCwait = 5000;

	remote_CCall(L, &_ftp_pwd);

	if (g_shell_result < 0) {
		lua_pushinteger(L, g_shell_result); // no response or error
		return 1;
	}
	else {
		remote_CCall(L, &_ftp_getstring);
		return 2;
	}
}

/* ftp.chdir([dir])
 * Set or get current remote directory
 */
//========================
int _ftp_cwd(lua_State* L)
{
	if (check_connect()) return 0;

	char cmd[128];
	const char *dir = luaL_checkstring(L, 1);
	sprintf(cmd, "CWD %s\r\n", dir);

	ftp->totable = 0;
   	int write_size = vm_tcp_write(ftp->handle, cmd, strlen(cmd));

   	return 0;
}

//==============================
static int ftp_cwd(lua_State *L)
{
	const char *dir = luaL_checkstring(L, 1);

	g_shell_result = -9;
	CCwait = 5000;

	remote_CCall(L, &_ftp_cwd);

	if (g_shell_result < 0) {
		lua_pushinteger(L, g_shell_result); // no response or error
		return 1;
	}
	else {
		lua_pushinteger(L, 0);
		return 1;
	}
}

/* ftp.send(file|string, [rfile], ["*append"])
 * Send string or local file to ftp server
 */
//=========================
int _ftp_send(lua_State* L)
{
	if (check_connect()) return 0;

	size_t len = 0;
	size_t slen = 0;
	int appnd = 0;
	int fromfile = 0;
	char rfile[64] = {'\0'};
	int fhndl = 0;

	// === check local file
	const char *source = luaL_checklstring(L, 1, &slen);
	if ((slen < 64) && (slen > 0)) {
		VMWCHAR ucs_name[128];
	    full_fname((char *)source, ucs_name, 128);
		fhndl = vm_fs_open(ucs_name, VM_FS_MODE_READ, VM_TRUE);
		if (fhndl >= 0) fromfile = 1;
	}

	if (lua_gettop(L) > 2) {
		// 3rd arg can only be append modifier
		if (lua_isstring(L, 3)) {
			const char *sappnd = luaL_checklstring(L, 1, &len);
			if ((len == 7) && (strstr(sappnd, "*append") != NULL)) appnd = 1;
		}
		// 2nd arg must be remote file name
		if (lua_isstring(L, 2)) {
			// remote file name
			const char *remotefile = luaL_checkstring(L, 2);
			sprintf(rfile, remotefile, strlen(remotefile));
		}
	}
	else if (lua_gettop(L) > 1) {
		// 2nd arg can be remote file name or append modifier
		if (lua_isstring(L, 2)) {
			const char *sappnd = luaL_checklstring(L, 1, &len);
			if ((len == 7) && (strstr(sappnd, "*append") != NULL)) appnd = 1;
			else {
				// remote file name
				const char *remotefile = luaL_checkstring(L, 2);
				sprintf(rfile, remotefile, strlen(remotefile));
			}
		}
	}

	if (strlen(rfile) == 0) {
		// no remote file name is given
		if (fromfile == 0) {
	    	vm_log_error("No remote file name.");
			g_shell_result = -1;
			vm_signal_post(g_shell_signal);
			return 0;
		}
		else {
			// remote file name same as local file name
			sprintf(rfile, source, strlen(source));
		}
	}

	vm_free(ftp->buffer);
	ftp->buffer = NULL;
	ftp->bufsize = 0;
	ftp->bufptr = 0;
	ftp->data_length = 0;
	ftp->data_length = 0;
	ftp->file_handle = -1;

	if (fromfile == 0) {
		ftp->ftp_transfer = FTP_SEND_FROM_STRING;
		ftp->buffer = vm_malloc(slen);
		ftp->bufsize = slen;
		if (ftp->buffer == NULL) {
			g_shell_result = -1;
			vm_signal_post(g_shell_signal);
			return 0;
		}
		memcpy(ftp->buffer, source, slen);
	}
	else {
		ftp->file_handle = fhndl;
		ftp->ftp_transfer = FTP_SEND_FROM_FILE;
	}

	if (appnd) sprintf(ftp->command, "APPE %s", rfile);
	else sprintf(ftp->command, "STOR %s", rfile);
    int write_size = vm_tcp_write(ftp->handle, "PASV\r\n", 6);

	return 0;
}

//===============================
static int ftp_send(lua_State *L)
{
	char *localfile = luaL_checkstring(L, 1);

	g_shell_result = -9;
	CCwait = 30000;
	remote_CCall(L, &_ftp_send);

	if (g_shell_result < 0) {
		if (ftp->file_handle >= 0) vm_fs_close(ftp->file_handle);
		lua_pushnil(L); // no response or error
	}
	else remote_CCall(L, &_ftp_closesend);

	return 1;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE ftp_map[] = {
		{LSTRKEY("connect"), LFUNCVAL(ftp_connect)},
		{LSTRKEY("disconnect"), LFUNCVAL(ftp_disconnect)},
		{LSTRKEY("list"), LFUNCVAL(ftp_list)},
		{LSTRKEY("recv"), LFUNCVAL(ftp_recv)},
		{LSTRKEY("send"), LFUNCVAL(ftp_send)},
		{LSTRKEY("chdir"), LFUNCVAL(ftp_cwd)},
		{LSTRKEY("getdir"), LFUNCVAL(ftp_pwd)},
        {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_ftp(lua_State *L) {

  luaL_register(L, "ftp", ftp_map);
  return 1;
}

