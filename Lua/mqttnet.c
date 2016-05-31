/* mqttnet.c
 *
 * Copyright (C) 2006-2016 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 *
 * wolfMQTT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfMQTT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */


//#include <time.h>

#include "mqtt_client.h"
#include "mqttnet.h"

#include "vmtype.h"
#include "vmsock.h"
#include "vmlog.h"


// Setup defaults
#define SOCKET_T        int
#define SOERROR_T       int
#define SELECT_FD(fd)   ((fd) + 1)


/* Local context for Net callbacks */
typedef struct _SocketContext {
    SOCKET_T fd;
} SocketContext;


/* Private functions */
static void setup_timeout(timeval* tv, int timeout_ms)
{
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms % 1000) * 1000;

    /* Make sure there is a minimum value specified */
    if (tv->tv_sec < 0 || (tv->tv_sec == 0 && tv->tv_usec <= 0)) {
        tv->tv_sec = 0;
        tv->tv_usec = 100;
    }
}

#ifdef FD_SET
#undef FD_SET
#endif
#define FD_SET(s, p)    ((p)->fds_bits[s] |= 0x01)  // Sets the socket ID that you want to select.

#ifdef FD_CLR
#undef FD_CLR
#endif
#define FD_CLR(s, p)    ((p)->fds_bits[s] &= ~(0x01))

#ifdef FD_ISSET
#undef FD_ISSET
#endif
#define FD_ISSET(s, p)  ((p)->fds_bits[s] & 0x02)

#ifdef FD_ZERO
#undef FD_ZERO
#endif
#define FD_ZERO(p)      memset(p, 0x00, sizeof(*(p)))


//---------------------------------------------------------------------------------
static int NetConnect(void *context, const char* host, word16 port, int timeout_ms)
{
    SocketContext *sock = (SocketContext*)context;
    SOCKADDR_IN address = {0};
    int rc;
    SOERROR_T so_error = 0;

	// Default to error
	rc = -1;

    address.sin_family = PF_INET;
    address.sin_addr.S_un.s_addr = vm_soc_inet_addr(host);
    address.sin_port = vm_soc_htons(port);

	// Create socket
	sock->fd = vm_soc_socket(AF_INET, SOCK_STREAM, 0);
	if (sock->fd >= 0) {
		// connect
		so_error = vm_soc_connect(sock->fd, (SOCKADDR*)&address, sizeof(SOCKADDR));
		if (so_error == 0) rc = 0; // Success
	}

    // Show error
    if (rc != 0) {
        vm_log_debug("[Mqtt] Socket_Connect Error: Fd=%d, Rc=%d, SoErr=%d", sock->fd, rc, so_error);
    }

    return rc;
}

//------------------------------------------------------------------------------
static int NetWrite(void *context, const byte* buf, int buf_len, int timeout_ms)
{
    SocketContext *sock = (SocketContext*)context;
    int rc = -99;
    timeval tv;

    if (context == NULL || buf == NULL || buf_len <= 0) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }

    // Setup timeout
    setup_timeout(&tv, timeout_ms);
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock->fd, &writefds);
    if (vm_soc_select((int)SELECT_FD(sock->fd), 0, &writefds, 0, &tv) >= 0) {
       if (FD_ISSET(sock->fd, &writefds)) {
           //socket is ready for writting data
    	   rc = vm_soc_send(sock->fd, buf, buf_len, 0);
       }
    }

    if (rc < 0) {
    	vm_log_debug("[Mqtt] Socket_NetWrite: Error %d", rc);
    }

    return rc;
}

//-----------------------------------------------------------------------
static int NetRead(void *context, byte* buf, int buf_len, int timeout_ms)
{
    SocketContext *sock = (SocketContext*)context;
    int rc = -1, bytes = 0;
    SOERROR_T so_error = 0;
    vm_fd_set recvfds, errfds;
    timeval tv;

    if (context == NULL || buf == NULL || buf_len <= 0) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }

    // Setup timeout and FD's
    setup_timeout(&tv, timeout_ms);
    FD_ZERO(&recvfds);
    FD_SET(sock->fd, &recvfds);
    FD_ZERO(&errfds);
    FD_SET(sock->fd, &errfds);

    // Loop until buf_len has been read, error or timeout
    while (bytes < buf_len)
    {
        // Wait for rx data to be available
        rc = vm_soc_select((int)SELECT_FD(sock->fd), &recvfds, NULL, &errfds, &tv);
        if (rc > 0) {
            // Check if rx or error
            if (FD_ISSET(sock->fd, &recvfds)) {
                // Try and read number of buf_len provided, minus what's already been read */
                rc = vm_soc_recv(sock->fd, &buf[bytes], buf_len - bytes, 0);
                if (rc <= 0) {
                    rc = -1;
                    break; // Error
                }
                else {
                    bytes += rc; // Data
                }
            }
            if (FD_ISSET(sock->fd, &errfds)) {
                rc = -1;
                break;
            }
        }
        else {
            break; // timeout or signal
        }
    }

    if (rc < 0) {
        // Get error
        if (so_error == 0 && !FD_ISSET(sock->fd, &recvfds)) {
            rc = 0; // Handle signal
        }
        else {
            vm_log_debug("MqttSocket_NetRead: Error %d", so_error);
        }
    }
    else {
        rc = bytes;
    }

    return rc;
}

//-------------------------------------
static int NetDisconnect(void *context)
{
    SocketContext *sock = (SocketContext*)context;
    if (sock) {
        if (sock->fd >= 0) {
        	vm_soc_close_socket(sock->fd);
            sock->fd = -1;
        }
    }

    return 0;
}


// ** Public Functions **

//----------------------------------
int MqttClientNet_Init(MqttNet* net)
{
    if (net) {
        XMEMSET(net, 0, sizeof(MqttNet));
        net->connect = NetConnect;
        net->read = NetRead;
        net->write = NetWrite;
        net->disconnect = NetDisconnect;
        net->context = WOLFMQTT_MALLOC(sizeof(SocketContext));
    }
    return 0;
}

//------------------------------------
int MqttClientNet_DeInit(MqttNet* net)
{
    if (net) {
        if (net->context) {
            WOLFMQTT_FREE(net->context);
        }
        XMEMSET(net, 0, sizeof(MqttNet));
    }
    return 0;
}

