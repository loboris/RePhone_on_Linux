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

#ifndef VMHTTPS_SDK_H
#define VMHTTPS_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmbearer.h"


/*************************************************************************
 * HTTPS SERVICE  -- MACRO's And Typedef's structure definition
 ************************************************************************/

#define VM_HTTPS_MAX_CONTENT_TYPE_LEN        256


/* HTTPS method constants */
typedef enum
{
    VM_HTTPS_METHOD_GET = 0x40,
    VM_HTTPS_METHOD_HEAD = 0x42,
    VM_HTTPS_METHOD_POST = 0x60,
    VM_HTTPS_METHOD_PUT = 0x61,
    VM_HTTPS_METHOD_DELETE = 0x43,
    VM_HTTPS_METHOD_MAX = 0x7F
} VM_HTTPS_METHOD;

/* HTTPS request options */
typedef enum
{
    VM_HTTPS_OPTION_NO_CACHE = 0x00000001,                              /* The no cache option. */
    VM_HTTPS_OPTION_DO_NOT_CACHE_REPLY = 0x00000002,                    /* The don't cache reply option. */
    VM_HTTPS_OPTION_ALWAYS_CACHE = 0x00000004,                          /* The always cache option. */
    VM_HTTPS_OPTION_CACHE_ONLY = 0x00000008,                            /* The cache only option. */
    VM_HTTPS_OPTION_STALE_CACHE = 0x00000010,                           /* The stale cache option. */
    VM_HTTPS_OPTION_DO_NOT_HANDLE_SERVER_AUTHORIZATION = 0x00008000,    /* The don't handle server authorization option. */
    VM_HTTPS_OPTION_DO_NOT_HANDLE_PROXY_AUTHORIZATION = 0x00010000,     /* The don't handle proxy authorization option. */
    VM_HTTPS_OPTION_DO_NOT_REDIRECT = 0x00020000,                       /* The don't redirect option. */
    VM_HTTPS_OPTION_USE_LARGE_PIPE = 0x40000000,                        /* The use large pipe option. */
    VM_HTTPS_OPTION_FS_COMMIT = 0x80000000,                             /* The file system commit option. This will commit the changes for each "file write" operation */
    VM_HTTPS_OPTION_NO_ADD_COOKIE = 0x00400000,                         /* The don't add cookie option. */
    VM_HTTPS_OPTION_MAX = 0x7FFFFFFF                                    /* To ensure that compiler treats this enum as 32-bit integer option. */
} VM_HTTPS_OPTION;

/* Protocol constants */
typedef enum
{
    VM_HTTPS_PROTOCOL_WSP_CL = 1,       /* The CL-WSP protocol constant. */ 
    VM_HTTPS_PROTOCOL_WSP_CL_WTLS = 2,  /* The CL-WSP WTLS protocol constant. */
    VM_HTTPS_PROTOCOL_WSP_CO = 3,       /* The CO-WSP protocol constant. */
    VM_HTTPS_PROTOCOL_WSP_CO_WTLS = 4,  /* The CO-WSP WTLS protocol constant. */
    VM_HTTPS_PROTOCOL_HTTP = 5,         /* The HTTP protocol constant. */
    VM_HTTPS_PROTOCOL_HTTPS_TLS = 6,     /* The TLS HTTP protocol constant. */
    VM_HTTPS_PROTOCOL_HTTPS_SSL = 7,     /* The SSL HTTP protocol constant. */
    VM_HTTPS_PROTOCOL_CACHE = 8,        /* The cache protocol constant. */
    VM_HTTPS_PROTOCOL_FILE = 9,         /* The file protocol constant. */
    VM_HTTPS_PROTOCOL_OTHER = 10,       /* The others protocol constant. */
    VM_HTTPS_PROTOCOL_MAX = 0x7F        /* The ensure that compiler treats this enum as 32-bit integer protocol constant.*/
} VM_HTTPS_PROTOCOL;

/* This enum indicates HTTPS ready status */
typedef enum
{
    VM_HTTPS_READY_STATUS_NULL = 0,     /* The MMI HTTPS is not ready to service status. */
    VM_HTTPS_READY_STATUS_OK = 1,       /* The MMI HTTPS is ready to service status. */
    VM_HTTPS_READY_STATUS_MAX = 0x7F    /* The ensure that compiler treats this enum as 32-bit integer status. */
} VM_HTTPS_READY_STATUS;

/* This enum indicates the result and error code of a request. */
typedef enum
{
    VM_HTTPS_OK = 0,                                /* The success. */
    VM_HTTPS_ERROR_UNKNOWN = -1,                    /* The unknown error. */
    VM_HTTPS_ERROR_SEND = -2,                       /* The fail to write content. */
    VM_HTTPS_ERROR_INVALID_PARAMETER = -3,          /* The invalid parameters. */
    VM_HTTPS_ERROR_INVALID_HEADER = -4,             /* The invalid headers. */
    VM_HTTPS_ERROR_INVALID_CHANNEL_ID = -5,         /* The invalid channel ID. */
    VM_HTTPS_ERROR_INVALID_REQUEST_ID = -6,         /* The invalid request ID. */
    VM_HTTPS_ERROR_INVALID_SEQUENCE_NUMBER = -7,    /* The incorrect sequence number in vm_https_read_content and vm_https_post_callback. */
    VM_HTTPS_ERROR_INVALID_DEFAULT_PROFILE = -8,    /* The invalid default profile. */   
    VM_HTTPS_ERROR_INACTIVE_CHANNEL = -9,           /* The inactive channel. */
    VM_HTTPS_ERROR_INVALID_CONNECTION_TYPE = -10,   /* The invalid connection type. */
    VM_HTTPS_ERROR_NO_RESOURCE = -11,               /* The resource limitation. */
    VM_HTTPS_ERROR_ACCESS_DENY = -12,               /* The access deny. */
    VM_HTTPS_ERROR_PENDING_REQUEST_EXIST = -13,     /* The pending request exists. */
    VM_HTTPS_ERROR_FILE_ACCESS = -14,               /* The file system operation error. */
    VM_HTTPS_ERROR_REQUEST_ABORT = -15,             /* The request aborted. */
    VM_HTTPS_ERROR_NOT_FOUND = -16,                 /* The not found. */
    VM_HTTPS_ERROR_REQUEST_TOO_LARGE = -17,         /* The message header is too large. */
    VM_HTTPS_ERROR_REQUEST_NOT_SUPPORT = -18,       /* The request is not supported. */
    VM_HTTPS_ERROR_DNS_RESOLVE = -19,               /* The DNS error. */
    VM_HTTPS_ERROR_USE_PROXY = -20,                 /* The proxy error. */
    VM_HTTPS_ERROR_NOT_READY = -21,                 /* The HTTPS is not ready yet. */
    VM_HTTPS_ERROR_NO_CONTEXT_AVAILABLE = -22,      /* The HTTPS free context not available, please retry after some time.*/
    VM_HTTPS_ERROR_CONTEXT_NOT_REGISTERED = -23,    /* The when availing HTTPS service without register context.*/
    VM_HTTPS_RESULT_MAX = 0x7F                      /* The ensure that compiler treats this enum as 32-bit integer request.*/
}VM_HTTPS_RESULT;


/* This enum specifies the data format in a given HTTPS request or response. */
typedef enum
{
    VM_HTTPS_DATA_TYPE_BUFFER,              /* This stores replied content in buffer */
    VM_HTTPS_DATA_TYPE_FILE,                /* This stores replied content in  specified file path directly with downloading 
                                            progress indication while this is specified as reply_type of 
                                            vm_https_send_request. */

       
    VM_HTTPS_DATA_TYPE_FILE_NO_PROG_IND,    /* This stores replied content in specified file path directly without downloading 
                                            progress indication while this is specified as reply_type of 
                                            vm_https_send_request. */

    VM_HTTPS_DATA_TYPE_FILE_RESUME,         /* This resumes a download previously. HTTPS will append the content in the file 
                                            specified in the request. */

    VM_HTTPS_DATA_TYPE_MULTIPART,           /* This is multipart/form-data, pass to post_type when do posting for multipart/form-data */
    VM_HTTPS_DATA_TYPE_MAX = 0x7F           /* The ensure that compiler treats this enum as 32-bit integer request. */
}VM_HTTPS_DATA_TYPE;

/* This enum indicates the HTTPS character set. */
typedef enum
{
    VM_HTTPS_CHARSET_UNKNOWN = 0,       /* The unknown character set. */
    VM_HTTPS_CHARSET_ASCII  = 3,        /* The ASCII character set. */
    VM_HTTPS_CHARSET_ISO_8859_1 = 4,    /* The ISO-8859-1 character set. */
    VM_HTTPS_CHARSET_LATIN_1 = 4,       /* The Latin-1 character set, same with ISO-8859-1. */
    VM_HTTPS_CHARSET_UCS_2 = 1000,      /* The UCS2 character set. */
    VM_HTTPS_CHARSET_UTF_16BE = 1013,   /* The UTF-16 with Big-endian byte order. */
    VM_HTTPS_CHARSET_UTF_16LE = 1014,   /* The UTF-16 with Little-endian byte order. */
    VM_HTTPS_CHARSET_UTF_16 = 1015,     /* The UTF-16 with a byte-order mark. */
    VM_HTTPS_CHARSET_UTF_8 = 106,       /* The UTF-8 character set. */
    VM_HTTPS_CHARSET_MAX = 0x7F         /* The ensure that compiler treats this enum as 32-bit integer character set. */
} VM_HTTPS_CHARSET;


/* The vm_http_content_t is the unit of multipart. */
typedef struct
{
  VMUINT32    content_type_length;      /* The content type length. */
  VMSTR       content_type;             /* The fields and their corresponding headers: 
                                        *  Content-Type: contentType; charset="charset"  
                                        *  In case of multipart/form-data, each part will have its own header as shown. */

  VMUINT32    name_length;              /* The name length. */
  VMSTR       name;                     /* The fields used in multipart/form-data with their corresponding parts:                                         
                                        *  Content-Disposition: form-data; name="name"; filename="fileName" */

  VMUINT32    filename_length;          /* The fileName name length. */
  VMSTR       filename;                 /* The Content-Disposition attribute - not the multipart file name. */


  VM_HTTPS_CHARSET    charset;          /* The character set. */
  VMUINT32    data_type;                 /* The VM_HTTPS_DATA_TYPE_BUFFER or VM_HTTPS_DATA_TYPE_FILE. */
  VMUINT32    data_length;                /* The data length. */
  VMUINT32    file_path_name_length;     /* The fileName name length. */
  VMWSTR      file_path_name;            /* The absolute multipart filepath with its name. */
} vm_https_content_t;


/* This is the structure used for vm_https_send_req_ext. */
typedef struct
{
    VMUINT32 url_length;            /* The request URL length. */
    VMSTR url;                  /* The request URL. */
    VMUINT32 header_length;         /* The request header length. */
    VMSTR header;               /* The request header. */


    VMUINT32 post_segment_length;   /* The post segment buffer length when post_type is specified as
                                    VM_HTTPS_DATA_TYPE_BUFFER. This is mandatory if the method is POST/PUT and the post_type
                                    is VM_HTTPS_DATA_TYPE_BUFFER. */


    VMSTR post_segment;         /* The post data segment. It is mandatory if method is POST/PUT and
                                    post_type is VM_HTTPS_DATA_TYPE_BUFFER. */



    VMUINT32 number_entries;        /*In order to support posting of multipart data, specify the number of multipart entries*/
    vm_https_content_t* content;    /*The application should allocate memory for each vm_https_content_t and specify all the information required*/
} vm_https_request_context_t;


/*************************************************************************
 * HTTPS SERVICE  -- The Callback function prototype.
 ************************************************************************/


/*****************************************************************************
* FUNCTION
*  vm_https_set_channel_response_callback
* DESCRIPTION
*  Response to the channel configuration request. If the result is
*  successful then the channel ID will be used in the HTTP/WSP request.
*  This function responds to the channel configuration request. If the result is 
*  successful, then the channel ID will be used in the HTTP/WSP request.
* PARAMETERS
*  request_id:       [IN] The request ID.
*  channel_id:       [IN] The channel ID.
*  result:           [IN] The result.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_set_channel_response_callback)(VMUINT32 request_id,
        VMUINT8  channel_id,
        VM_HTTPS_RESULT  result);

/*****************************************************************************
* FUNCTION
*  vm_https_unset_channel_response_callback
* DESCRIPTION
*  Response to channel remove request. If the channel ID requested is
*  invalid or some pending request of this channel exists, an error
*  will come back as a result.
*  This function responds to channel removal request. If the channel ID requested is 
*  invalid or if there's a pending request of this channel, an error  
*  will incurr.
* PARAMETERS
*  channel_id:       [IN] The channel ID.
*  result:           [IN] The result.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_unset_channel_response_callback)(VMUINT8 channel_id,
        VM_HTTPS_RESULT result);

/*****************************************************************************
* FUNCTION
*  vm_https_release_all_request_response_callback
* DESCRIPTION
*  This function responds to release of all requests.
* PARAMETERS
*  result:           [IN] The result.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_release_all_request_response_callback)(VM_HTTPS_RESULT result);


/*****************************************************************************
* FUNCTION
*  vm_https_termination_callback
* DESCRIPTION
*  Notifying the application about an HTTPS agent termination.
*  The application should clear all information at its end, and start again.
*  This function sends notification on an HTTPS agent termination to the application. 
*  Upon receiving the notification, the application should clear all information at termination, and start again.
* PARAMETERS
*  void
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_termination_callback)(void);


/*****************************************************************************
* FUNCTION
*  vm_https_send_response_callback
* DESCRIPTION
*  Response to the HTTPS request. New_url is used to hold the new URL
*  that was used to retrieve this reply if the original request was
*  This function responds to the HTTPS request. New_url is used to hold the new URL 
*  that was used to retrieve this reply if the original request was 
*  redirected. If the reply_type is VM_HTTPS_DATA_TYPE_BUFFER,
*  then the more flag is used to indicate if the application needs to use
*  vm_https_read_content to retrieve the remaining content.
* PARAMETERS
*  request_id:       [IN] The request ID.
*  result:           [IN] The HTTPS result.
*  status:           [IN] The HTTPS response code.
*  cause:            [IN] The Error cause.
*  protocol:         [IN] The network protocol used to retrieve content.
*  content_length:   [IN] The length of the replied content.
*  more:             [IN] This indicates if there is more data that needs to be retrieved.
*  content_type:     [IN] The type of the replied content. Maximum is 256 characters including NULL.
*  content_type_length: [IN] The length of content_type.
*  new_url:          [IN] The new redirected URL.
*  new_url_length:   [IN] The length of new redirected URL.
*  reply_header:     [IN] The replied header.
*  reply_header_length: [IN] The length of replied header.
*  reply_segment:    [IN]: The reply segment buffer.
*  reply_segment_length: [IN] The reply segment buffer length when reply_type is specified as VM_HTTPS_DATA_TYPE_BUFFER. This is mandatory when reply_type is VM_HTTPS_DATA_TYPE_BUFFER.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_send_response_callback)( VMUINT16 request_id,
        VM_HTTPS_RESULT  result,
        VMUINT16 status,
        VMINT32  cause,
        VM_HTTPS_PROTOCOL  protocol,
        VMUINT32 content_length,
        VMBOOL  more,
        VMSTR content_type,
        VMUINT8  content_type_length,
        VMSTR new_url,
        VMUINT32 new_url_length,
        VMSTR reply_header,
        VMUINT32 reply_header_length,
        VMSTR reply_segment,
        VMUINT32 reply_segment_length);

/*****************************************************************************
* FUNCTION
*  vm_https_read_content_response_callback
* DESCRIPTION
*  Response to the vm_https_read_content. The sequence_number must be the same
*  as the one in the corresponding vm_https_read_content. The more
*  flag is used to indicate if the application needs to use
*  This function responds to the vm_https_read_content. The sequence_number must be the same 
*  as the one in the corresponding vm_https_read_content. The more 
*  flag is used to indicate if the application needs to use 
*  vm_https_read_content to retrieve the remaining content.
* PARAMETERS
*  request_id:           [IN] The request ID.
*  sequence_number:      [IN] The sequence number.
*  result:               [IN] The result.
*  more:                 [IN] This indicates if more data needs to be retrieved.
*  reply_segment:        [IN] The reply segment buffer.
*  reply_segment_length: [IN] The reply segment buffer length.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_read_content_response_callback)( VMUINT16 request_id,
        VMUINT8  sequence_number,
        VM_HTTPS_RESULT  result,
        VMBOOL  more,
        VMWSTR reply_segment,
        VMUINT32 reply_segment_length);

/*****************************************************************************
* FUNCTION
*  vm_https_cancel_response_callback
* DESCRIPTION
*  This function responds to the cancel request.
* PARAMETERS
*  request_id:           [IN] The request ID.
*  result:               [IN] The result.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_cancel_response_callback)(VMUINT16 request_id,
        VM_HTTPS_RESULT  result);


/*****************************************************************************
* FUNCTION
*  vm_https_status_query_response_callback
* DESCRIPTION
*  This function responds to status query request. It's reserved for future development.
* PARAMETERS
*  status:           [IN] The HTTPS status.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_status_query_response_callback)(VMUINT8 status);

/*****************************************************************************
* FUNCTION
*  vm_https_post_callback
* DESCRIPTION
*  This function responds to the post request.
* PARAMETERS
*  request_id:           [IN] The HTTPS status.
*  sequence_number:      [IN] The sequence number.
*  result:               [IN] The result.
* RETURNS
*  void
*****************************************************************************/
typedef void (*vm_https_post_callback)(VMUINT16 request_id, VMUINT8 sequence_number, VM_HTTPS_RESULT result);

/* HTTPS callbacks struct */
typedef struct
{
    vm_https_set_channel_response_callback set_channel_callback;                    /* This sets channel callback */
    vm_https_unset_channel_response_callback unset_channel_callback;                /* This unsets channel callback */
    vm_https_release_all_request_response_callback release_all_request_callback;    /* This releases all request callback */
    vm_https_termination_callback termination_callback;                             /* This is the terminal req callback */
    vm_https_send_response_callback send_callback;                                  /* This is the HTTP request callback */
    vm_https_read_content_response_callback read_content_callback;                  /* This reads content callback */
    vm_https_cancel_response_callback cancel_callback;                              /* This cancels req callback */
    vm_https_status_query_response_callback status_query_callback;                  /* This is status query callback */
} vm_https_callbacks_t;

/*****************************************************************************
 * FUNCTION
 *  vm_https_register_context_and_callback
 * DESCRIPTION
 *  This function registers a callback function.
 * PARAMETERS
 *  apn:        [IN] The APN enum, VM_TCP_APN_NON_PROXY/VM_TCP_APN_PROXY/VM_TCP_APN_WIFI.
 *  callbacks:  [IN] The callback functions.
 * RETURNS
 *  refer to VM_HTTPS_RESULT
 *****************************************************************************/
VM_RESULT vm_https_register_context_and_callback(
    VM_BEARER_DATA_ACCOUNT_TYPE data_account_type,
    vm_https_callbacks_t* callbacks);


/*****************************************************************************
 * FUNCTION
 *  vm_https_set_channel
 * DESCRIPTION
 *  Configure a dedicated channel for HTTP/WSP requests. An channel can be
 *  configured to use a proxy while use_proxy is true. When use_proxy is true,
 *  This function configures a dedicated channel for HTTP/WSP requests. A channel can be 
 *  configured to use a proxy while use_proxy is true. When use_proxy is true, 
 *  prx_ip, prx_port, username, and password will be used to communicate with the
 *  proxy server, thus prx_ip cannot be 0.0.0.0 and if the prx_port value is 0
 *  it means the default proxy port is 8080. When the conn_type is
 *  HTTPS_CONN_TYPE_WSP_CO or HTTPS_CONN_TYPE_WSP_CL, the use_proxy is true by
 *  default. However, if the application simply wants to use the current
 *  setting used in WAP, then just set use_default as true and ignore all
 *  proxy server, thus prx_ip cannot be 0.0.0.0 and if the prx_port value is 0 
 *  it means the default proxy port is 8080. 
 *	
 *  When the conn_type is HTTPS_CONN_TYPE_WSP_CO or HTTPS_CONN_TYPE_WSP_CL, then by default use_proxy is true. 
 *  However, if the application simply wants to use the current  
 *  setting in WAP, then set use_default as true and ignore all 
 *  other parameters.
 *  Static_header (if necessary) must be filled in
 *  WAP_PROF_CONN_TYPE_WSP_CO and the first WSP request of some dedicated
 *  channel with connection type WSP_CONN_TYPE_WSP_CL. If the application doesn't
 *  specify static_header, then HTTPS will use the value currently used by WAP.
 *
 * PARAMETERS
 *  request_id:                         [IN] The request ID.
 *  use_proxy:                          [IN] This indicates whether it uses proxy or not.
 *  proxy_ip:                           [IN] The proxy IP address.
 *  proxy_port:                         [IN] The proxy port number. 0 means default proxy 8080.
 *  username:                           [IN] The username for the proxy. Only ASCII is allowed and NULL terminated.
 *  username_length:                    [IN] The length should not exceed 40 characters excluding NULL character.
 *  password:                           [IN] The password for the proxy. Only ASCII is allowed and it is NULL terminated.
 *  password_length:                    [IN] The length should not exceed 40 character excluding NULL character.
 *  server_authorize_username:          [IN] The username for the origin server. Only ASCII is allowed and it is NULL terminated.
 *  server_authorize_username_lengt:    [IN] The length should not exceed 128 character excluding NULL character.
 *  server_authorize_password:          [IN] The length should not exceed 128 character excluding NULL character.
 *  server_authorize_password_lengt:    [IN] The length should not exceed 40 character excluding NULL character.
 *  static_header:                      [IN] The static header.
 *  static_header_length:               [IN] The length of static header.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_set_channel(VMUINT16 request_id,
                               VMBOOL use_proxy, VMUINT8 proxy_ip[4], VMUINT16 proxy_port,
                               VMSTR username, VMUINT8 username_length,
                               VMSTR password, VMUINT8 password_length,
                               VMSTR server_authorize_username, VMUINT8 server_authorize_username_length,
                               VMSTR server_authorize_password, VMUINT8 server_authorize_password_length,
                               VMSTR static_header, VMUINT32 static_header_length);


/*****************************************************************************
 * FUNCTION
 *  vm_https_unset_channel
 * DESCRIPTION
 *  This function sends unset channel request information to HTTPS.
 * PARAMETERS
 *  channel_id:  [IN] channel ID
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_unset_channel(VMUINT8 channel_id);

/*****************************************************************************
 * FUNCTION
 *  vm_https_send_request
 * DESCRIPTION
 *  Sends an HTTP/WSP request.
 * PARAMETERS
 *  request_id:                 [IN] Request ID.
 *  method:                     [IN] HTTP Method Constant (VM_HTTPS_METHOD).
 *  option:                     [IN] HTTP request options.
 *  reply_type:                 [IN] Reply type (wps_data_type_enum)
 *                                   The file path used to store replied content when reply_type is specified
 *                                   as HTTPS_DATA_TYPE_FILE, HTTPS_DATA_TYPE_FILE_NO_PROG_IND or
 *                                   HTTPS_DATA_TYPE_FILE_RESUME.
 *                                   Or the maximum buffer size that reading side suggests when reply_type is specified
 *                                   as VM_HTTPS_DATA_TYPE_BUFFER.
 *  request_id:                 [IN] The request ID.
 *  method:                     [IN] The HTTP Method Constant (VM_HTTPS_METHOD).
 *  option:                     [IN] The HTTP request options. 
 *  reply_type:                 [IN] The reply type (wps_data_type_enum) 
 *                                   The file path used to store replied content when reply_type is specified 
 *                                   as HTTPS_DATA_TYPE_FILE, HTTPS_DATA_TYPE_FILE_NO_PROG_IND or 
 *                                   HTTPS_DATA_TYPE_FILE_RESUME. 
 *                                   Or the maximum buffer size that reading side suggests when reply_type is specified 
 *                                   as VM_HTTPS_DATA_TYPE_BUFFER. 
 *  reply_segment_length:       [IN] The maximum length of the reply_segment.
 *  request_url_length:         [IN] The request URL length.
 *  request_url:                [IN] The request URL.
 *  request_header_length:      [IN] The request header length.
 *  request_header:             [IN] The request header.
 *                                   The post segment buffer length when post_type is specified as
 *                                   VM_HTTPS_DATA_TYPE_BUFFER. It is mandatory is method is POST/PUT and post_type
 *                                   is VM_HTTPS_DATA_TYPE_BUFFER.
 *  post_segment:               [IN] The post segment.
 *  post_segment_length:        [IN] The post segment length.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_send_request(VMUINT16 request_id,
                                VM_HTTPS_METHOD method, VM_HTTPS_OPTION option, VM_HTTPS_DATA_TYPE reply_type,
                                VMUINT32 reply_segment_length,
                                VMSTR request_url, VMUINT32 request_url_length,
                                VMSTR request_header, VMUINT32 request_header_length,
                                VMSTR post_segment, VMUINT32 post_segment_length);


/*****************************************************************************
 * FUNCTION
 *  vm_https_read_content
 * DESCRIPTION
 *  This function sends a request to continue to read HTTP/WSP reply content.
 * PARAMETERS
 *  request_id:         [IN] Request ID
 *  sequence_number:    [IN] Sequence number (for debug purpose)
 *  read_segment_length:[IN] Segment Length
 *  request_id:         [IN] The request ID    
 *  sequence_number:    [IN] The sequence number (for debug purpose)    
 *  read_segment_length:[IN] The segment Length       
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_read_content(VMUINT16 request_id, VMUINT8 sequence_number, VMUINT32 read_segment_length);

/*****************************************************************************
 * FUNCTION
 *  vm_https_cancel
 * DESCRIPTION
 *  This function sends a request to cancel an incomplete WSP/HTTP request.
 * PARAMETERS
 *  request_id: [IN] Request ID
 *  request_id: [IN] The request ID.         
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_cancel(VMUINT16 request_id);

/*****************************************************************************
 * FUNCTION
 *  vm_https_request_ext
 * DESCRIPTION
 *  This function sends a HTTP/WSP request. It's an extended version of the function vm_https_send_request.
 * CALLS
 *
 * PARAMETERS
 *  request_id :            [IN] The request ID.
 *  method :                [IN] The HTTP Method Constant.
 *  option :                [IN] The HTTP request options.
 *  reply_type :            [IN] The reply type (VM_HTTPS_DATA_TYPE).
 *  reply_segment_length :  [IN] The length of reply segment.
 *  more_post :             [IN] This indicates whether there is still more data that needs to be posted next time, please set it to true.
 *  post_type :             [IN] Please refer to VM_HTTPS_DATA_TYPE.
 *  request :               [IN] The request data, please refer to vm_httPs_http_req_var_t.
 *  callback :              [IN] The callback of post status.
 * RETURNS
 *  VM_RESULT
 *****************************************************************************/
VM_RESULT vm_https_request_ext(VMUINT16 request_id, VM_HTTPS_METHOD method, VM_HTTPS_OPTION option, VM_HTTPS_DATA_TYPE reply_type,
                               VMUINT32 reply_segment_length, VMBOOL more_post, VM_HTTPS_DATA_TYPE post_type,
                               const vm_https_request_context_t* request, vm_https_post_callback callback);

/*****************************************************************************
* FUNCTION
*  vm_https_post
* DESCRIPTION
*  This function handles continuous content post of a HTTPS request.
* PARAMETERS
*  request_id :         [IN] Request ID.
*  sequence_number :    [IN] Sequence id.
*  more :               [IN] Need more send or not.
*  post_segment_length: [IN] Post segment.
*  post_segment :       [IN] Post segment length.
*  request_id :         [IN] The request ID.
*  sequence_number :    [IN] The sequence id.
*  more :               [IN] This indicates whether it needs more send or not.
*  post_segment_length: [IN] The post segment.
*  post_segment :       [IN] The post segment length.     
* RETURNS
*  VM_RESULT
*****************************************************************************/
VM_RESULT vm_https_post(VMUINT16 request_id, VMUINT8 sequence_number, VMBOOL more,
                        VMSTR post_segment, VMUINT32 post_segment_length);

#ifdef __cplusplus
}
#endif

#endif /* VMHTTPS_SDK_H */
