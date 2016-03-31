/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005-2015
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef VMSOCK_SDK_H
#define VMSOCK_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

#include "vmgsm_gprs.h"

/* The maximum length of the network socket address. */
#define VM_SOC_ADDRESS_MAX_LENGTH    28
/* Socket address struct. */
typedef struct
{
    VMINT16     address_len; /* Address length. */
    VMUINT16    port;        /* Port number. */
    VMUINT8     address[VM_SOC_ADDRESS_MAX_LENGTH]; /* Address. */
} vm_soc_address_t;

/* Socket return codes. The negative values stand for errors. */
typedef enum
{
    VM_SOC_SUCCESS           = 0,     /* Success. */
    VM_SOC_ERROR             = -1,    /* Error. */
    VM_SOC_WOULDBLOCK        = -2,    /* Not done yet. */
    VM_SOC_LIMIT_RESOURCE    = -3,    /* Limited resource. */
    VM_SOC_INVALID_SOCKET    = -4,    /* Invalid socket. */
    VM_SOC_INVALID_ACCOUNT   = -5,    /* Invalid account ID. */
    VM_SOC_NAMETOOLONG       = -6,    /* Address too long. */
    VM_SOC_ALREADY           = -7,    /* Operation already in progress. */
    VM_SOC_OPNOTSUPP         = -8,    /* Operation not support. */
    VM_SOC_CONNABORTED       = -9,    /* Software caused connection abort. */
    VM_SOC_INVAL             = -10,   /* Invalid argument. */
    VM_SOC_PIPE              = -11,   /* Broken pipe. */
    VM_SOC_NOTCONN           = -12,   /* Socket is not connected. */
    VM_SOC_MSGSIZE           = -13,   /* Message is too long. */
    VM_SOC_BEARER_FAIL       = -14,   /* Bearer is broken. */
    VM_SOC_CONNRESET         = -15,   /* TCP half-write close, i.e., FINED. */
    VM_SOC_DHCP_ERROR        = -16,   /* DHCP error. */
    VM_SOC_IP_CHANGED        = -17,   /* IP has changed. */
    VM_SOC_ADDRINUSE         = -18,   /* Address already in use. */
    VM_SOC_CANCEL_ACT_BEARER = -19,   /* Cancel the activation of bearer. */
    VM_SOC_RESULT_MAX = 0x7FFFFFFF    /* Forces the compiler to use 4-byte integer for enum. */
} VM_SOC_RESULT;



/* Protocol family. */
#define PF_INET     (0)  /* IPv4. */
#define PF_INET6    (1)  /* IPv6. */

/* Address family */
#define AF_INET     PF_INET /* IPv4. */

/* Socket type */
#define SOCK_STREAM (0)  /* Stream socket, TCP. */
#define SOCK_DGRAM  (1)  /* Datagram socket, UDP. */


/*
 * <GROUP Macro_Consts>
 *
 * Internet Control Message Protocol. Currently, applications are only allowed
 * to set this type when RAW_SOCKET is used
 */
#define IPPROTO_ICMP    (1)

/* for SOC/TCP/IP internal use, application shall NOT use */

/*
 * <GROUP  Macro_Consts>
 *
 * Dummy for IP.
 */
#define IPPROTO_IP      (0)

/*
 * <GROUP  Macro_Consts>
 *
 * IPv6 hop-by-hop options.
 */
#define IPPROTO_HOPOPTS (0)

/*
 * <GROUP  Macro_Consts>
 *
 * Internet Group Management Protocol.
 */
#define IPPROTO_IGMP    (2)

/*
 * <GROUP  Macro_Consts>
 *
 * IPv4 encapsulation.
 */
#define IPPROTO_IPV4    (4)

/*
 * <GROUP  Macro_Consts>
 *
 * For compatibility.
 */
#define IPPROTO_IPIP    IPPROTO_IPV4

/*
 * <GROUP  Macro_Consts>
 *
 * TCP protocol.
 */
#define IPPROTO_TCP     (6)

/*
 * <GROUP  Macro_Consts>
 *
 * User Datagram Protocol.
 */
#define IPPROTO_UDP     (17)

/*
 * <GROUP  Macro_Consts>
 *
 * Raw IP packet.
 */
#define IPPROTO_RAW     (255)

/* Generic Socket Address structure. */
typedef struct sockaddr
{
    VMUINT16 sa_family;     /* Address family. */
    VMINT8   sa_data[14];   /* Direct address (up to 14 bytes). */
} SOCKADDR;

/* Generic IPv4 Address structure. */
typedef struct in_addr
{
    union
    {
        struct
        {
            VMUINT8 s_b1;
            VMUINT8 s_b2;
            VMUINT8 s_b3;
            VMUINT8 s_b4;     /* IP address, byte access. */
        } S_un_b;
        struct
        {
            VMUINT16 s_w1;
            VMUINT16 s_w2;    /* IP address, short int access. */
        } S_un_w;
        VMUINT32 s_addr;    /* IP address in network byte order. */
    } S_un;
} IN_ADDR;


/* IPv4 Socket Address structure. */
typedef struct sockaddr_in
{
    VMINT16  sin_family;        /* Socket domain. */
    VMUINT16 sin_port;          /* Port. */
    IN_ADDR  sin_addr;          /* IP address. */
    VMINT8   sin_zero[8];       /* Reserved. */
} SOCKADDR_IN;

/* Generic IPv6 Address structure. */
typedef struct in6_addr
{
    union
    {
        VMUINT8  s6_u8[16];
        VMUINT32 s6_u32[4];
        VMUINT64 s6_u64[2];
    } S_all;
} IN6_ADDR;

/* IPv6 Socket Address structure. */
typedef struct sockaddr_in6
{
    VMINT16   sin_family;           /* Socket domain. */
    VMUINT16  sin_port;             /* Port. */
    VMUINT32  sin6_flowinfo;        /* IPv6 traffic class & flow info. */
    IN6_ADDR  sin6_addr;            /* IP address. */
    VMUINT32  sin6_scop_id;         /* Set of interfaces for a scope. */
} SOCKADDR_IN6;


/*****************************************************************************
 * FUNCTION
 *  vm_soc_socket
 * DESCRIPTION
 *  Creates a socket.
 *  Note: It is advised to use BSD socket API all in one sub-thread.
 * PARAMETERS
 *  family:   [IN] Specifies the address family, PF_INET or PF_INET6.
 *  type:     [IN] Specifies the type for the new socket.
 *                 SOCK_STREAM = connection based type.
 *                 SOCK_DGRAM  = datagram connectionless type.
 *  protocol: [IN] Particular protocol to be used with the socket that is specific
 *                 to the indicated address family.
 * RETURN VALUES
 *  Non-negative number:     Socket ID for the newly created socket.
 *  VM_SOC_INVAL:            Wrong parameters, domain, type, or protocol.
 *  VM_SOC_LIMIT_RESOURCE:   No socket available.
 *  VM_SOC_BEARER_FAIL:      Bearer broken.
 *****************************************************************************/
int vm_soc_socket(int family, int type, int protocol);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_socket_ex
 * DESCRIPTION
 *  Creates a socket with a particular bearer, as specified by the data_account_id.
 *  In contrast, the vm_soc_socket() creates a socket with the latest opened bearer.
 *  Note: It is advised to use BSD socket API all in one sub-thread.
 * PARAMETERS
 *  family:          [IN] Specifies the address family, PF_INET or PF_INET6.
 *  type:            [IN] Specifies the type for the new socket.
 *                        SOCK_STREAM = connection based type.
 *                        SOCK_DGRAM  = datagram connectionless type.
 *  protocol:        [IN] Particular protocol to be used with the socket that is
 *                        specific to the indicated address family.
 *  data_account_id: [IN] Data account for network which was obtained from the
 *                        callback parameter of vm_bearer_open().
 * RETURN VALUES
 *  Non-negative number:     Socket ID for the newly created socket.
 *  VM_SOC_INVAL:            Wrong parameters, domain, type, or protocol.
 *  VM_SOC_LIMIT_RESOURCE:   No socket available.
 *  VM_SOC_BEARER_FAIL:      Bearer broken.
 *****************************************************************************/
int vm_soc_socket_ex(int family, int type, int protocol, unsigned int data_account_id);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_close_socket
 * DESCRIPTION
 *  Closes the socket and release the resource.
 * PARAMETERS
 *  sock: [IN] The descriptor that identifies an unbound socket.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket descriptor.
 *****************************************************************************/
int vm_soc_close_socket(int socket);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_bind
 * DESCRIPTION
 *  Binds a local address with a socket.
 * PARAMETERS
 *  sock:    [IN] The descriptor that identifies an unbound socket.
 *  addr:    [IN] The local address to be bound to the socket.
 *  addrlen: [IN] The size of the address structure, in bytes.
 *  type:    [IN] Specifies the type for the new socket.
 *                SOCK_STREAM = connection based type.
 *                SOCK_DGRAM  = datagram connectionless type.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_ERROR:          Unspecified error.
 *  VM_SOC_ADDRINUSE:      The local port has been bound with other socket.
 *****************************************************************************/
int vm_soc_bind(int sock, const SOCKADDR* addr, int addrlen, int type);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_listen
 * DESCRIPTION
 *  Makes the specified socket listen to the incoming connections.
 * PARAMETERS
 *  sock:    [IN] Specifies the bound socket to listen to the incoming connections.
 *  backlog: [IN] The maximum number of connection requests that can be queued.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_OPNOTSUPP:      Only TCP can be listened.
 *  VM_SOC_ALREADY:        Already listened.
 *  VM_SOC_ERROR:          Unspecified error.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *****************************************************************************/
int vm_soc_listen(int sock, int backlog);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_accept
 * DESCRIPTION
 *  Accepts a connection on a socket.
 * PARAMETERS
 *  sock:    [IN] The descriptor that identifies a bound socket.
 *  addr:    [IN|OUT] The socket address structure, to be filled with the peer
 *                    socket address.
 *  addrlen: [IN|OUT] The size of the socket address structure. On return, it will
 *                    contain the actual size of the peer address.
 * RETURN VALUES
 *  Non-negative number:   Socket descriptor for accepted socket.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_CONNABORTED:    Server socket cannot receive anymore.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_ERROR:          Unspecified error.
 *****************************************************************************/
int vm_soc_accept(int sock, SOCKADDR* addr, int* addrlen);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_connect
 * DESCRIPTION
 *  Connects a socket to a remote host.
 * PARAMETERS
 *  sock:    [IN] The descriptor of the socket to be connected.
 *  addr:    [IN] The socket address to be connected to the socket specified by sock.
 *  addrlen: [IN] The size of the socket address.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_CONNABORTED:    Server socket cannot receive anymore.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_ERROR:          Unspecified error.
 *****************************************************************************/
int vm_soc_connect(int sock, SOCKADDR* addr, int addrlen);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_send
 * DESCRIPTION
 *  Sends data on a connected socket.
 * PARAMETERS
 *  sock:  [IN] The socket descriptor.
 *  buf:   [IN] The buffer that contains the data to be sent.
 *  len:   [IN] The length of data (in bytes).
 *  flags: [IN]  Reserved. Not in use.
 * RETURN VALUES
 *  Non-negative number:   The actual number of bytes that has been sent.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_CONNABORTED:    Server socket cannot receive anymore.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_ERROR:          Unspecified error.
 *****************************************************************************/
int vm_soc_send(int sock, const char* buf, int len, int flags);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_sendto
 * DESCRIPTION
 *  Sends data to the endpoint node.
 * PARAMETERS
 *  sock:  [IN] The socket descriptor.
 *  buf:   [IN] The buffer that contains the data to be sent.
 *  len:   [IN] The length of data (in bytes).
 *  flags: [IN]  Reserved. Not in use.
 *  to:    [IN] The remote socket address.
 *  tolen: [IN] The size of the remote socket address.
 * RETURN VALUES
 *  Non-negative number:   The actual number of bytes that has been sent.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_NOTCONN:        Socket is not connected in case of TCP.
 *  VM_SOC_PIPE:           Socket is already shutdown.
 *  VM_SOC_MSGSIZE:        Message is too long.
 *  VM_SOC_ERROR:          Unspecified error.
 *  VM_SOC_NOTBIND:        Unbound in case of sending ICMP Echo Request, shall bind before
 *****************************************************************************/
int vm_soc_sendto(int sock, const char* buf, int len, int flags, const SOCKADDR* to, int tolen);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_recv
 * DESCRIPTION
 *  Receives data on a connected socket.
 * PARAMETERS
 *  sock:  [IN]  The socket descriptor.
 *  buf:   [OUT] The buffer containing the data received from the socket upon
 *               the return of this function.
 *  len:   [IN]  The length of data (in bytes).
 *  flags: [IN]  Reserved. Not in use.
 * RETURN VALUES
 *  Positive number:       The actual number of bytes received.
 *  Zero:                  Received the FIN from the server.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_NOTCONN:        Socket is not connected in case of TCP.
 *  VM_SOC_PIPE:           Socket is already shutdown.
 *  VM_SOC_ERROR:          Unspecified error.
 *****************************************************************************/
int vm_soc_recv(int sock, char* buf, int len, int flags);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_recvfrom
 * DESCRIPTION
 *  Receives data from the endpoint node.
 * PARAMETERS
 *  sock:    [IN]     The socket descriptor.
 *  buf:     [OUT]    The buffer containing the data received from the socket upon
 *                    the return of this function.
 *  len:     [IN]     The length of data (in bytes).
 *  flags:   [IN]     Reserved. Not in use.
 *  from:    [OUT]    The remote socket address.
 *  fromlen: [IN|OUT] The size of the remote socket address. On return, it will
 *                    contain the actual size of the remote socket address.
 * RETURN VALUES
 *  Positive number:       The actual number of bytes received.
 *  Zero:                  Received the FIN from the server.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_NOTCONN:        Socket is not connected in case of TCP.
 *  VM_SOC_PIPE:           Socket is already shutdown.
 *  VM_SOC_ERROR:          Unspecified error.
 *****************************************************************************/
int vm_soc_recvfrom(int sock, char* buf, int len, int flags, SOCKADDR* from, int* fromlen);

/* FD SET SIZE */
#define VM_FD_SETSIZE 25

/* Macros for SOC_SELECT */
typedef struct
{
    VMUINT8    fds_bits[VM_FD_SETSIZE]; /* Select set based on socket ID. */
} vm_fd_set;

#ifdef fd_set
#undef fd_set
#endif
#define fd_set  vm_fd_set   /* Sets the socket ID that you want to select.  */

/*
 * <GROUP  Macro_Funcs>
 *
 * Sets the socket ID that you want to select.
 */
#ifdef FD_SET
#undef FD_SET
#endif
#define FD_SET(s, p)    ((p)->fds_bits[s] |= 0x01)  /* Sets the socket ID that you want to select.  */

/*
 * <GROUP  Macro_Funcs>
 *
 * Clears the socket ID that you do not want to select.
 */

#ifdef FD_CLR
#undef FD_CLR
#endif
#define FD_CLR(s, p)    ((p)->fds_bits[s] &= ~(0x01))

/*
 * <GROUP  Macro_Funcs>
 *
 * Checks if the socket ID is selected.
 */
#ifdef FD_ISSET
#undef FD_ISSET
#endif
#define FD_ISSET(s, p)  ((p)->fds_bits[s] & 0x02)

/*
 * <GROUP  Macro_Funcs>
 *
 * Clears the select set.
 */
#ifdef FD_ZERO
#undef FD_ZERO
#endif
#define FD_ZERO(p)              memset(p, 0x00, sizeof(*(p)))


/* Selects timeout structure. */
typedef struct
{
    VMUINT32 tv_sec;  /* Select timeout, the number of seconds. */
    VMUINT32 tv_usec; /* Select timeout, the number of microsecond. */
} timeval;

/*****************************************************************************
 * <GROUP  Berkley_API>
 *
 * FUNCTION
 *  vm_soc_select
 * DESCRIPTION
 *  Allows the application to query the status of multiple sockets.
 *  If timeout is null, vm_soc_select() will wait indefinitely until the first
 *  socket is ready. If both tv_sec and tv_msec are zero, vm_soc_select() will
 *  timeout immediately for polling the sockets in the set.
 *  in:
 *  1. If listen has been called and a connection is pending, accept will succeed.
 *  2. Data is available for reading (includes OOB data if VM_SOC_OOBINLINE is enabled).
 *  3. Connection has been closed/reset/terminated
 *  out:
 *  1. If processing a connect request (non-blocking mode), connection has succeeded.
 *  2. Data can be sent.
 *  ex:
 *  If processing a connect request (non-blocking mode), connection attempt failed.
 *  OOB data is available for reading (only if VM_SOC_OOBINLINE is disabled).
 *  Useful Macros:
 *   FD_ZERO(fd_set)        Reset fd_set.
 *   FD_SET(s, fd_set)      Add a particular socket with ID of s to fd_set.
 *   FD_CLR(s, fd_set)      Remove a particular socket with ID of s from fd_set.
 *   FD_ISSET(s, fd_set)    Non-zero if s is a member of the fd_set (s is ready).
 *                          Otherwise, zero.
 * PARAMETERS
 *  nfds:      [IN] The highest-numbered socket descriptor in any of the three sets,
 *                  plus 1.
 *  readfds:   [IN] Set of socket descriptors for read.
 *  writefds:  [IN] Set of socket descriptors for write.
 *  exceptfds: [IN] Set of socket descriptors for exceptions.
 *  timeout:   [IN] Specifies the timeout value.
 *                  timeout == NULL: block until ready.
 *                  timeout != NULL && value == 0: return immediately.
 *                  timeout != NULL && value != 0: return after timeout.
 * RETURN VALUES
 *  Non-negative number:   The number of ready sockets.
 *  Zero:                  The timeout expires before anything interesting happens.
 *  VM_SOC_INVALID_SOCKET: Invalid socket ID.
 *  VM_SOC_INVAL:          Socket is not calling soc_listen.
 *  VM_SOC_WOULDBLOCK:     Non-blocking.
 *  VM_SOC_BEARER_FAIL:    Bearer broken.
 *  VM_SOC_ERROR:          Unspecified error.
 * EXAMPLE
 * <code>
 *  timeval timeout;
 *  fd_set writefds;
 *  timeout.tv_sec = 0;
 *  timeout.tv_usec = 2000;
 *  FD_ZERO(&writefds);
 *  FD_SET((bf_u32)sockIds[sock].id, &writefds);
 *  if (vm_soc_select(sockIds[sock].id+1, 0, &writefds, 0, &timeout) >= 0)
 *  {
 *     if (FD_ISSET(sockIds[sock].id, &writefds))
 *     {
 *         //socket is ready for writting data
 *     }
 *  }
 * </code>
 *****************************************************************************/
int vm_soc_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
                  const timeval* timeout);


/* Shutdown(2), specified by Posix.1g. */
typedef enum
{
    VM_SOC_SHUT_RD   = 0x01,         /* Shutdown the reading side. */
    VM_SOC_SHUT_WR   = 0x02,         /* Shutdown the writing side. */
    VM_SOC_SHUT_RDWR = VM_SOC_SHUT_RD | VM_SOC_SHUT_WR,   /* Shutdown both reading and writing sides. */
} vm_soc_shutdown_enum;

/*****************************************************************************
 * FUNCTION
 *  vm_soc_shutdown
 * DESCRIPTION
 *  Shutdowns the connection in specific side, such as reading side, writing side,
 *  or both.
 *  In a half read situation, vm_soc_shutdown() discards the received data that
 *  the application has not yet read and any further incoming data.
 *  In a half write situation of TCP, the remaining data will be sent first,
 *  then the FIN will be sent to close the connection.
 * PARAMETERS
 *  sock: [IN] Socket descriptor.
 *  how:  [IN] Specifies shutdown read, write or both.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket descriptor.
 *  VM_SOC_INVAL:          Invalid value, specified in how.
 *  VM_SOC_ERROR:          Unspecified error.
 * EXAMPLE
 * <code>
 * INT handle;
 * vm_soc_shutdown((kal_int8) handle, VM_SOC_SHUT_WR);
 * </code>
 *****************************************************************************/
int vm_soc_shutdown(int sock,  int how);


/* Socket Options */
typedef enum
{
    VM_SOC_OOBINLINE        = 0x01 << 0,    /* Not supported. */
    VM_SOC_LINGER           = 0x01 << 1,    /* Linger on close. */
    VM_SOC_NBIO             = 0x01 << 2,    /* Non-blocking. */
    VM_SOC_ASYNC            = 0x01 << 3,    /* Asynchronous notification. */

    VM_SOC_NODELAY          = 0x01 << 4,    /* Disable Nagle algorithm or not. */
    VM_SOC_KEEPALIVE        = 0x01 << 5,    /* Enable/disable the keepalive. */
    VM_SOC_RCVBUF           = 0x01 << 6,    /* Set the socket receive buffer size. */
    VM_SOC_SENDBUF          = 0x01 << 7,    /* Set the socket send buffer size. */

    VM_SOC_NREAD            = 0x01 << 8,    /* The number of bytes for read, only for soc_getsockopt. */
    VM_SOC_PKT_SIZE         = 0x01 << 9,    /* Datagram maximum packet size. */
    VM_SOC_SILENT_LISTEN    = 0x01 << 10,   /* SOC_SOCK_SMS property. */
    VM_SOC_QOS              = 0x01 << 11,   /* Set the socket QoS. */

    VM_SOC_TCP_MAXSEG       = 0x01 << 12,   /* Set the maximum segment size. */
    VM_SOC_IP_TTL           = 0x01 << 13,   /* Set the IP TTL value. */
    VM_SOC_LISTEN_BEARER    = 0x01 << 14,   /* Enable listen bearer. */
    VM_SOC_UDP_ANY_FPORT    = 0x01 << 15,   /* Enable UDP any foreign port. */

    VM_SOC_WIFI_NOWAKEUP    = 0x01 << 16,   /* Send packet in power saving mode. */
    VM_SOC_UDP_NEED_ICMP    = 0x01 << 17,   /* Deliver NOTIFY(close) for ICMP error. */
    VM_SOC_IP_HDRINCL       = 0x01 << 18,   /* IP header included for raw sockets. */
    VM_SOC_IPSEC_POLICY     = 0x01 << 19,   /* IP security policy. */
    VM_SOC_TCP_ACKED_DATA   = 0x01 << 20,   /* TCP/IP acknowledgement data. */
    VM_SOC_TCP_DELAYED_ACK  = 0x01 << 21,   /* TCP delayed acknowledgement. */
    VM_SOC_TCP_SACK         = 0x01 << 22,   /* TCP selective acknowledgement. */
    VM_SOC_TCP_TIME_STAMP   = 0x01 << 23,   /* TCP timestamp. */
    VM_SOC_TCP_ACK_MSEG     = 0x01 << 24,   /* TCP ACK multiple segments. */
} VM_SOC_OPTION;

/*****************************************************************************
 * FUNCTION
 *  vm_soc_setsockopt
 * DESCRIPTION
 *  This function sets the socket options.
 * PARAMETERS
 *  sock:     [IN] Socket descriptor.
 *  option:   [IN] Specifies the option type, as defined in VM_SOC_OPTION.
 *  val:      [IN] The option value.
 *  val_size: [IN] The size of the option value.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket descriptor.
 *  VM_SOC_INVAL:          Invalid value, specified in how.
 *  VM_SOC_ERROR:          Unspecified error.
 * SEE ALSO
 *   vm_soc_getsockopt()
 * EXAMPLE
 * VM_SOC_NBIO:
 * This option must to be set if you want to use the non-blocking mode or
 * non-blocking plus async mode.
 * val contains KAL_TRUE indicates non-blocking shall be adopted,
 * KAL_FALSE indicates blocking mode shall be adopted.
 * For example, set a socket to non-blocking mode:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  SOC_NBIO, &val, sizeof(kal_bool));
 *
 * VM_SOC_SILENT_LISTEN:
 * val contains KAL_TRUE indicates the silent listen shall be adopted,
 * KAL_FALSE indicates non-silent listen shall be adopted.
 * This is SOC_SOCK_SMS property. For example, if you want to let the socket be
 * the passive to activate the bear, you can set a socket to silent listen mode:
 * For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_SILENT_LISTEN, &val, sizeof(kal_bool))
 *
 * VM_SOC_UDP_NEED_ICMP:
 * This option is used to deliver NOTIFY(close) for ICMP error.
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * For example, if you want to let set a socket to UDP NEED ICMP mode:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_UDP_NEED_ICMP, &val, sizeof(kal_bool))
 *
 * VM_SOC_ASYNC:
 * this option needs to be set if you want to use the non-blocking plus asyn mode.
 * val contains the events application wants to receive.
 * For example, if application wants to receive
 * READ/WRITE/CLOSE/CONNECT events,
 * it shall call soc_setsockopt like the following:
 * kal_uint8 val = 0;
 * val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
 * vm_soc_setsockopt(sock,  VM_SOC_ASYNC, &val, sizeof(kal_uint8));
 * P.S. if you are using the server socket, please remember to set the SOC_ACCPET event.
 *
 * VM_SOC_TCP_MAXSEG:
 * A process can only decrease the MSS. When a TCP socket is created,
 * the default MSS is set to 512. The MSS is updated according to the value announced
 * from peer TCP after the socket completes TCP three-way handshake.
 * Therefore, after a socket is created but before connected, a process can only decrease
 * the MSS from its default of 512. After a connection is established,
 * the process can decrease the MSS from the new updated value.
 *
 * The value of TCP MSS is reduced to fit the interface MTU if the application set an
 * unacceptable large value.
 * For example:
 * kal_uint8 val = 512;
 * vm_soc_setsockopt(sock,  VM_SOC_TCP_MAXSEG, &val, sizeof(kal_uint8));
 *
 * VM_SOC_LISTEN_BEARER:
 * val contains bear type indicates which kind of bear we want to receive packets from.
 * For example, set a socket to receive the WiFi bear:
 * kal_uint8 val = SOC_BEARER_WIFI;
 * vm_soc_setsockopt(sock,  VM_SOC_LISTEN_BEARER, &val, sizeof(kal_uint8));
 *
 * VM_SOC_WIFI_NOWAKEUP:
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * This option is to make WNDRV sends outgoing packet to in power saving mode.
 * For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_WIFI_NOWAKEUP, &val, sizeof(kal_bool))
 *
 * VM_SOC_IP_HDRINCL:
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled. If this option is set for a raw IP socket,
 * we must build our own IP header for all the datagrams that we send on the raw socket.
 * We can only set this socket option if the socket type in soc_create is
 * SOC_SOCK_RAW(raw socket) and protocol in soc_create is not SOC_IPPROTO_ICMP.
 * For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_IP_HDRINCL, &val, sizeof(kal_bool))
 *
 * VM_SOC_UDP_ANY_FPORT:
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled. For a connected UDP socket,
 * this option makes socket deliver incoming packet from the same remote
 * address but different remote port.
 * For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_UDP_ANY_FPORT, &val, sizeof(kal_bool))
 *
 * VM_SOC_IP_TTL:
 * val contains the TTL values . If you want to change the default TTL (64),
 * you can set this option. For example:
 * kal_uint8 val = 128;
 * vm_soc_setsockopt(sock,  VM_SOC_IP_TTL, &val, sizeof(kal_uint8))
 *
 * VM_SOC_QOS:
 * val contains the TOS values . If you want to set the TOS values, you can set this option.
 * For example:
 * kal_uint8 val = 0x80;
 * vm_soc_setsockopt(sock,  VM_SOC_QOS, &val, sizeof(kal_uint8))
 *
 * VM_SOC_KEEPALIVE:
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * You can use this option to enable or disable the keep-alive timer in TCP.
 * For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_KEEPALIVE, &val, sizeof(kal_bool))
 *
 * VM_SOC_NODELAY:
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * If you want to send the packet directly without waiting a packet size larger than MSS,
 * you can set this option. For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_NODELAY, &val, sizeof(kal_bool))
 *
 * VM_SOC_SENDBUF:
 * val contains the value of send buffer's high water mark.
 * If you want to set the high water mark, you can set this option.
 * If the value you set is smaller than the MSS, the mss would be adjusted to this value.
 * The value must be greater than 0 and smaller than the SSPDU + SSPDU_GROW.
 * For example:
 * kal_uint32 val = 1024;
 * vm_soc_setsockopt(sock,  VM_SOC_SENDBUF, &val, sizeof(kal_uint32))
 *
 * VM_SOC_RECVBUF:
 * val contains the value of receive buffer's high water mark.
 * If you want to set the high water mark, you can set this option.
 * This value must be greater than 0 and smaller than the SRPDU size.
 * For example:
 * kal_uint32 val = 4096;
 * vm_soc_setsockopt(sock,  VM_SOC_RECVBUF, &val, sizeof(kal_uint32))
 *
 * VM_SOC_LINGER:
 * val contains the value of enable/diable LINGER option and the value of LINGER time.
 * This option is used in linger on close.
 * For example:
 * soc_linger_t val;
 * val.onoff = KAL_TRUE; val.
 * linger_time = 3; (sec)
 * vm_soc_setsockopt(sock,  VM_SOC_LINGER, &val, sizeof(soc_linger_t_val));
 *
 * VM_SOC_IPSEC_POLICY:
 * val contains sadb_x_policy structure content.
 * You can set this option to set the IPSEC policy.
 * If you want to get the IPSEC policy,
 * you can set this option. For example:
 * struct sadb_x_policy policy;
 * policy.sadb_x_policy_len = PFKEY_UNIT64(sizeof(policy));
 * policy.sadb_x_policy_exttype = SADB_X_EXT_POLICY;
 * policy.sadb_x_policy_type = 4;  // IPSEC_POLICY_BYPASS;
 * policy.sadb_x_policy_dir = 1;   // IPSEC_DIR_INBOUND;
 * vm_soc_setsockopt(sock,  VM_SOC_IPSEC_POLICY, &policy, sizeof(sadb_x_policy))
 *
 * VM_SOC_TCP_DELAYED_ACK:
 * The VM_SOC_TCP_DELAYED_ACK function is enable by default. That is to say,
 * if you don't set this option. The default VM_SOC_TCP_DELAYED_ACK value is TRUE.
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * If you want to enable the "TCP no delay" function,
 * you can set this option. For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_TCP_DELAYED_ACK, &val, sizeof(kal_bool))
 *
 * VM_SOC_TCP_SACK:
 * The VM_SOC_TCP_SACK function is enable by default. That is to say,
 * if you don't set this option. The default VM_SOC_TCP_SACK value is TRUE.
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * If you want to enable the "TCP SACK" function,
 * you can set this option. For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_TCP_SACK, &val, sizeof(kal_bool))
 *
 * VM_SOC_TCP_TIME_STAMP:
 * The VM_SOC_TCP_TIME_STAMP function is enable by default. That is to say,
 * if you don't set this option. The default VM_SOC_TCP_TIME_STAMP value is TRUE.
 * val contains KAL_TRUE indicates this option shall be enabled,
 * KAL_FALSE indicates this option shall be disabled.
 * If you want to enable the "TCP TIME STAMP" function,
 * you can set this option. For example:
 * kal_bool val = KAL_TRUE;
 * vm_soc_setsockopt(sock,  VM_SOC_TCP_TIME_STAMP, &val, sizeof(kal_bool))
 *
 *****************************************************************************/
int vm_soc_setsockopt(int sock,  unsigned int option, void* val, unsigned int val_size);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_getsockopt
 * DESCRIPTION
 *  Gets the socket options.
 * PARAMETERS
 *  sock:     [IN]     Socket descriptor.
 *  option:   [IN]     Specifies the option type, as defined in VM_SOC_OPTION.
 *  val:      [OUT]    The buffer to contain the option value upon the return of
 *                     this function.
 *  val_size: [IN|OUT] The size of the buffer, and the actual size of the option
 *                     value upon the return of this function.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:        Success.
 *  VM_SOC_INVALID_SOCKET: Invalid socket descriptor.
 *  VM_SOC_INVAL:          Invalid value, specified in how.
 *  VM_SOC_ERROR:          Unspecified error.
 * SEE ALSO
 *   vm_soc_setsockopt()
 * EXAMPLE
 *
 * VM_SOC_TCP_DELAYED_ACK:
 * val contains kal_bool to hold the option behavior.
 * If you don't set this option, the default value you get is KAL_TRUE.
 * If you to get this option, you can do:
 * <code>
 * kal_bool val;
 * getsockopt(sock,  SOC_TCP_DELAYED_ACK, &val, sizeof(kal_bool))
 * </code>
 *
 * VM_SOC_TCP_SACK:
 * val contains kal_bool to hold the option behavior.
 * If you don't set this option, the default value you get is KAL_TRUE.
 * If you to get this option, you can do:
 * <code>
 * kal_bool val;
 * getsockopt(sock,  SOC_TCP_SACK, &val, sizeof(kal_bool))
 * </code>
 *
 * VM_SOC_TCP_TIME_STAMP:
 * val contains kal_bool to hold the option behavior.
 * If you don't set this option, the default value you get is KAL_TRUE.
 * If you to get this option, you can do:
 *  <code>
 *  kal_bool val;
 *  getsockopt(sock,  SOC_TCP_TIME_STAMP, &val, sizeof(kal_bool))
 *  </code>
 *****************************************************************************/
int vm_soc_getsockopt(int sock,  unsigned int option, void* val, unsigned int val_size);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_getsockname
 * DESCRIPTION
 *  Retrieves the local IP address and port number.
 * PARAMETERS
 *  sock: [IN]  Socket descriptor.
 *  addr: [OUT] Structure that will receive IP address and port number upon the return.
 *  len:  [OUT] Length of SOCKADDR structure.
 * RETURNS
 *  Refer to VM_SOC_RESULT for return value.
 *****************************************************************************/
int vm_soc_getsockname(int s, SOCKADDR* addr, int* len);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_getpeername
 * DESCRIPTION
 *  Retrieves the IP address and port number of the endpoint node.
 * PARAMETERS
 *  sock: [IN]  Socket descriptor.
 *  addr: [OUT] Structure that will receive IP address and port number upon the return.
 *  len:  [OUT] Length of SOCKADDR structure.
 * RETURNS
 *  Refer to VM_SOC_RESULT for return value.
 *****************************************************************************/
int vm_soc_getpeername(int s, SOCKADDR* addr, int* len);


/*****************************************************************************
 * FUNCTION
 *  vm_soc_htonl
 * DESCRIPTION
 *  Converts long integer (32 bits) from host byte order to network byte order.
 * PARAMETERS
 *  a: [IN] The long integer to be converted.
 * RETURNS
 *  Converted result in unsigned integer.
 * EXAMPLE
 * <code>
 * loopback = vm_soc_htonl(0x7f000001);
 * </code>
 *****************************************************************************/
unsigned int vm_soc_htonl(unsigned int a);


/*****************************************************************************
 * FUNCTION
 *  vm_soc_htons
 * DESCRIPTION
 *  Converts short integer (16 bits) from host byte order to network byte order
 * PARAMETERS
 *  a: [IN] A short integer to be converted.
 * RETURNS
 *  Converted result in unsigned integer.
 * EXAMPLE
 * <code>
 * sockaddr sock_addr;
 * ((struct SOCKADDR_IN *) sock_addr)->sin_port = vm_soc_htons(port);
 * </code>
 *****************************************************************************/
unsigned short vm_soc_htons(unsigned short a);


/*****************************************************************************
 * FUNCTION
 *  vm_soc_inet_addr
 * DESCRIPTION
 *  Converts the internet address in a dotted-decimal string notation into an
 *  internet network address in the IN_ADDR structure.
 * PARAMETERS
 *  addr: [IN] The internet address in a dotted-decimal string notation.
 * RETURNS
 *  The internet network address in IN_ADDR type.
 *****************************************************************************/
unsigned int vm_soc_inet_addr(const char* addr);

/*****************************************************************************
 * FUNCTION
 *  vm_soc_inet_ntoa
 * DESCRIPTION
 *  Converts an internet network address into a dotted-decimal string format.
 * PARAMETERS
 *  in: [IN] The internet network address to be converted.
 * RETURNS
 *  A string in dotted-decimal notation.
 *****************************************************************************/
char* vm_soc_inet_ntoa(IN_ADDR in);



/*****************************************************************************
 * FUNCTION
 *  vm_soc_inet_pton
 * DESCRIPTION
 *  Converts an IPv4 or IPv6 address in its standard text presentation form into
 *  its numeric binary form in network byte order.
 *  The format of 'x:x:x:x:x:x:d.d.d.d' is not supported.
 * PARAMETERS
 *  af:   [IN]  Specifies the protocol family, 0 for IPv4 and 1 for IPv6.
 *  src:  [IN]  IP address in standard text presentation form ends with '\0'.

 *  dst:  [OUT] IP address in numeric binary form in network byte order
 *              For IPv4, it is a kal_uint8 array with size of at least 4 bytes.
 *              For IPv6, it is an ipv6_addr_t variable or compatible kal_uint8
 *              array with size of at least 16 bytes.
 * RETURNS
 *  The converted IP address in standard text representation, if the conversion is
 *  successful. If the conversion failed, the NULL is returned.
 * EXAMPLE
 * <code>
 * // IPv4
 * kal_char ipv4[] = {"192.168.1.1"};
 * kal_uint8 ipv4_dst[4] = {0};
 * kal_bool ret = KAL_FALSE;
 * ret = vm_soc_inet_pton(0, ipv4, ipv4_dst);
 * if (ret)
 * {
 *      // successfully
 * }
 *
 *
 * // IPv6
 * VMCHAR ipv6[] = {"FF::EE"};
 * VMUINT8 ipv6_dst[16] = {0};
 * VMINT ret;
 * ret = vm_soc_inet_pton(1, ipv6, ipv6_dst);
 * if (ret)
 * {
 *      // successfully
 * }
 * </code>
 *****************************************************************************/
int vm_soc_inet_pton(int af, const char* src, void* dst);


/*****************************************************************************
 * FUNCTION
 *  vm_soc_inet_ntop
 * DESCRIPTION
 *  Converts an IPv4 or IPv6 address in its numeric binary form in network byte
 *  order into its standard text representation form.
 *  The format of 'x:x:x:x:x:x:d.d.d.d' is not supported.
 * PARAMETERS
 *  af:   [IN]  Specifies the protocol family, 0 for IPv4 and 1 for IPv6.
 *  src:  [IN]  IP address in numeric binary form in network byte order
 *              For IPv4, it is a kal_uint8 array with size of at least 4 bytes.
 *              For IPv6, it is an ipv6_addr_t variable or compatible kal_uint8
 *              array with size of at least 16 bytes. And src should obey 4-byte
 *              alignment.
 *  dst:  [OUT] IP address in standard text presentation form ends with '\0'.
 *              The size of the dst buffer should be big enough to accommodate the
 *              whole IP address. Otherwise, conversion will fail.
 *              For IPv4, the safe size is 16 bytes.
 *              For IPv6, the safe size is 40 bytes.
 *  size: [IN]  The size of dst buffer in bytes.
 * RETURNS
 *  The converted IP address in standard text representation, if the conversion is
 *  successful. If the conversion failed, the NULL is returned.
 * EXAMPLE
 * <code>
 * // IPv4
 * VMUINT8 ipv4[] = {192, 168, 0, 1};
 * VMCHAR ipv4_addr[16] = {0}, *addr = NULL;
 * addr = vm_soc_inet_ntop(0, ipv4, ipv4_addr, 16);
 * if (addr)
 * {
 *      // successfully
 * }
 *
 *
 * // IPv6
 * VMUINT8 ipv6[] = {0xff, 0xee, 0xdd, 0xcc, 0xaa, 99, 88, 77, 66, 55, 44, 33, 22, 11, 00, 00};
 * VMCHAR ipv6_addr[16] = {0};
 * const VMCHAR *addr = NULL;
 * addr = vm_soc_inet_ntop(1, ipv6, ipv6_addr, 40);
 * if (addr)
 * {
 *      // successfully
 * }
 * </code>
 *****************************************************************************/
const char* vm_soc_inet_ntop(int af, const void* src, char* dst, unsigned int size);


/*****************************************************************************
 * FUNCTION
 *  vm_soc_get_account_localip
 * DESCRIPTION
 *  Gets the local IP held by the socket as specified.
 * PARAMETERS
 *  sock:     [IN]  Socket descriptor.
 *  local_ip: [OUT] Contains the IP address on return of this function.
 * RETURN VALUES
 *  VM_SOC_SUCCESS:         Get IP address successfully.
 *  VM_SOC_INVAL:           Invalid argument, e.g. local_ip is NULL.
 *  VM_SOC_BEARER_FAIL:     Bearer is not activated.
 *  VM_SOC_INVALID_SOCKET:  Invalid socket.
 *  VM_SOC_INVALID_ACCOUNT: Invalid account.
 * EXAMPLE
 * <code>
 * static VMUINT8 ip[4];
 * VMINT result;
 * result = vm_soc_get_account_localip(fd, ip);
 * if (result == VM_SOC_SUCCESS)
 * {
 *     // Gets the local IP address successfully.
 * }
 * </code>
 *****************************************************************************/
VM_RESULT vm_soc_get_account_localip(VMINT8 sock, VMUINT8* local_ip);


/* Refer to vm_socket. */
#define socket(family, type, protocol)                  vm_soc_socket(family, type, protocol)
/* Refer to vm_socket_ex. */
#define socket_ex(family, type, protocol, account)      vm_soc_socket_ex(family, type, protocol, account)
/* Refer to vm_closesocket. */
#define closesocket(s)                                  vm_soc_close_socket(s)
/* Refer to vm_bind. */
#define bind(s, addr, len, type)                        vm_soc_bind(s, addr, len, type)
/* Refer to vm_listen. */
#define listen(s, bl)                                   vm_soc_listen(s, bl)
/* Refer to vm_accept. */
#define accept(s, addr, len)                            vm_soc_accept(s, addr, len)
/* Refer to vm_connect. */
#define connect(s, addr, len)                           vm_soc_connect(s, addr, len)
/* Refer to vm_send. */
#define send(s, buf, len, flags)                        vm_soc_send(s, buf, len, flags)
/* Refer to vm_sendto. */
#define sendto(s, buf, len, flags, to, tlen)            vm_soc_sendto(s, buf, len, flags, to, tlen)
/* Refer to vm_recv. */
#define recv(s, buf, len, flags)                        vm_soc_recv(s, buf, len, flags)
/* Refer to vm_recvfrom. */
#define recvfrom(s, buf, len, flags, from, flen)        vm_soc_recvfrom(s, buf, len, flags, from, flen)
/* Refer to vm_select. */
#define select(fds, reads, writes, except, timeout)     vm_soc_select(fds, reads, writes, except, timeout)
/* Refer to vm_shutdown. */
#define shutdowns(s, how)                               vm_soc_shutdown(s, how)
/* Refer to vm_setsockopt. */
#define setsockopt(s, opt, val, size)                   vm_soc_setsockopt(s, opt, val, size)
/* Refer to vm_getsockopt. */
#define getsockopt(s, opt, val, size)                   vm_soc_getsockopt(s, opt, val, size)
/* Refer to vm_getsockname. */
#define getsockname(s, addr, len)                       vm_soc_getsockname(s, addr, len)
/* Refer to vm_getpeername. */
#define getpeername(s, addr, len)                       vm_soc_getpeername(s, addr, len)

/* Refer to vm_htonl. */
#define htonl(a)                    vm_soc_htonl(a)
/* Refer to vm_htons. */
#define htons(a)                    vm_soc_htons(a)
/* Refer to vm_htonl. */
#define ntohl(a)                    vm_soc_htonl(a)
/* Refer to vm_htons. */
#define ntohs(a)                    vm_soc_htons(a)

/* Refer to vm_inet_addr. */
#define inet_addr(a)             vm_soc_inet_addr(a)
/* Refer to vm_inet_ntoa. */
#define inet_ntoa(in)            vm_soc_inet_ntoa(in)
/* Refer to vm_inet_pton. */
#define inet_pton(af, src, dst)                         vm_soc_inet_pton(af, src, dst)
/* Refer to vm_inet_ntop. */
#define inet_ntop(af, src, dst, size)                   vm_soc_inet_ntop(af, src, dst, size)

#ifdef __cplusplus
}
#endif

#endif /* VMSOCK_SDK_H */
