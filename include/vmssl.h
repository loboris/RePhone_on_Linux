/*
 * FIPS-197 compliant AES implementation
 *
 * Copyright  2006-2007 Christophe Devine
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of XySSL nor the names of its contributors may be
 * used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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

#ifndef VMSSL_SDK_H
#define VMSSL_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"

/* Handle to a SSL connection. */
typedef VMINT VM_SSL_HANDLE;

/* Verification type used for socket connection. */
typedef enum
{
    VM_SSL_VERIFY_NONE = 0,     /* Peer certificate is not checked (default),
                                   This is insecure and should be avoided. */
    VM_SSL_VERIFY_OPTIONAL = 1, /* Peer certificate is checked, however the
                                   handshake continues even if verification
                                   failed. */
    VM_SSL_VERIFY_REQUIRED = 2, /* Peer *must* present a valid certificate,
                                   handshake is aborted if
                                   verification failed. */
    VM_SSL_VERIFY_MAX = 0x7FFFFFFF
} VM_SSL_VERIFY_TYPE;

/* Events passing to vm_ssl_connection_callback. */
typedef enum
{
    VM_SSL_EVENT_CONNECTED = 1,        /* The socket is connected. */
    VM_SSL_EVENT_CAN_WRITE = 2,        /* The socket is now ready to be
                                          written to. */
    VM_SSL_EVENT_CAN_READ = 3,         /* There are data available for read */
    VM_SSL_EVENT_PIPE_BROKEN = 4,      /* The socket has timed out or the
                                          connection has broken. */
    VM_SSL_EVENT_HOST_NOT_FOUND = 5,   /* Cannot reach the host specified. */
    VM_SSL_EVENT_PIPE_CLOSED = 6,      /* The socket is closed. */
    VM_SSL_EVENT_HANDSHAKE_FAILED = 7, /* SSL handshaking has failed. */
    VM_SSL_EVENT_CERTIFICATE_VALIDATION_FAILED = 8, /* Certificate validation
                                                       has failed. */
    VM_SSL_EVENT_MAX = 0x7FFFFFFF
} VM_SSL_EVENT;

/* Error values for connection related APIs,
   e.g. vm_ssl_read and vm_ssl_write. */
typedef enum
{
    VM_SSL_ERROR_FEATURE_UNAVAILABLE = -0x1000,
    VM_SSL_ERROR_BAD_INPUT_DATA = -0x1800,
    VM_SSL_ERROR_INVALID_MAC = -0x2000,
    VM_SSL_ERROR_INVALID_RECORD = -0x2800,
    VM_SSL_ERROR_INVALID_MODULUS_SIZE = -0x3000,
    VM_SSL_ERROR_UNKNOWN_CIPHER = -0x3800,
    VM_SSL_ERROR_NO_CIPHER_CHOSEN = -0x4000,
    VM_SSL_ERROR_NO_SESSION_FOUND = -0x4800,
    VM_SSL_ERROR_NO_CLIENT_CERTIFICATE = -0x5000,
    VM_SSL_ERROR_CERTIFICATE_TOO_LARGE = -0x5800,
    VM_SSL_ERROR_CERTIFICATE_REQUIRED = -0x6000,
    VM_SSL_ERROR_BAD_HS_SERVER_HELLO_DONE = -0xC000,
    VM_SSL_ERROR_BAD_HS_CLIENT_KEY_EXCHANGE = -0xC800,
    VM_SSL_ERROR_BAD_HS_CERTIFICATE_VERIFY = -0xD000,
    VM_SSL_ERROR_BAD_HS_CHANGE_CIPHER_SPEC = -0xD800,
    VM_SSL_ERROR_BAD_HS_FINISHED = -0xE000,
    VM_SSL_ERROR_MAX = 0x7FFFFFFF
} VM_SSL_ERROR;

/* Error values for vm_ssl_get_verify_result. */
typedef enum
{
    VM_SSL_VERIFY_SUCCESS = 0,
    VM_SSL_VERIFY_ERROR = -1,                   /* Unknown error */
    VM_SSL_BAD_CERTIFICATE_EXPIRED = 0x01,      /* The certificate validity
                                                   has expired. */
    VM_SSL_BAD_CERTIFICATE_REVOKED = 0x02,      /* The certificate has been
                                                   revoked (is on a CRL). */
    VM_SSL_BAD_CERTIFICATE_CN_MISMATCH = 0x04,  /* Certificate Common Name(CN)
                                                   does not match with the
                                                   expected CN. */
    VM_SSL_BAD_CERTIFICATE_NOT_TRUSTED = 0x08,  /* The certificate is not
                                                   correctly signed by a
                                                   trusted CA. */
    VM_SSL_BAD_CERTIFICATE_MAX = 0x7FFFFFFF
} VM_SSL_VERIFY_RESULT;

/* Error values for vm_ssl_load_ca_chain_certificate. */
typedef enum
{
    VM_SSL_ERROR_X509_FEATURE_UNAVAILABLE = -0x0020,
    VM_SSL_ERROR_X509_CERT_INVALID_PEM = -0x0040,
    VM_SSL_ERROR_X509_CERT_INVALID_FORMAT = -0x0060,
    VM_SSL_ERROR_X509_CERT_INVALID_VERSION = -0x0080,
    VM_SSL_ERROR_X509_CERT_INVALID_SERIAL = -0x00A0,
    VM_SSL_ERROR_X509_CERT_INVALID_ALG = -0x00C0,
    VM_SSL_ERROR_X509_CERT_INVALID_NAME = -0x00E0,
    VM_SSL_ERROR_X509_CERT_INVALID_DATE = -0x0100,
    VM_SSL_ERROR_X509_CERT_INVALID_PUBKEY = -0x0120,
    VM_SSL_ERROR_X509_CERT_INVALID_SIGNATURE = -0x0140,
    VM_SSL_ERROR_X509_CERT_INVALID_EXTENSIONS = -0x0160,
    VM_SSL_ERROR_X509_CERT_UNKNOWN_VERSION = -0x0180,
    VM_SSL_ERROR_X509_CERT_UNKNOWN_SIG_ALG = -0x01A0,
    VM_SSL_ERROR_X509_CERT_UNKNOWN_PK_ALG = -0x01C0,
    VM_SSL_ERROR_X509_CERT_SIG_MISMATCH = -0x01E0,
    VM_SSL_ERROR_X509_CERT_VERIFY_FAILED = -0x0200,
    VM_SSL_ERROR_X509_KEY_INVALID_PEM = -0x0220,
    VM_SSL_ERROR_X509_KEY_INVALID_VERSION = -0x0240,
    VM_SSL_ERROR_X509_KEY_INVALID_FORMAT = -0x0260,
    VM_SSL_ERROR_X509_KEY_INVALID_ENC_IV = -0x0280,
    VM_SSL_ERROR_X509_KEY_UNKNOWN_ENC_ALG = -0x02A0,
    VM_SSL_ERROR_X509_KEY_PASSWORD_REQUIRED = -0x02C0,
    VM_SSL_ERROR_X509_KEY_PASSWORD_MISMATCH = -0x02E0,
    VM_SSL_ERROR_X509_POINT_ERROR = -0x0300,
    VM_SSL_ERROR_X509_VALUE_TO_LENGTH = -0x0320,
    VM_SSL_X509_ERROR_MAX = 0x7FFFFFFF
} VM_SSL_X509_ERROR;

/* Error values for vm_ssl_base64_encode and vm_ssl_base64_decode */
typedef enum
{
    VM_SSL_ERROR_BASE64_BUFFER_TOO_SMALL = -0x0010,
    VM_SSL_ERROR_BASE64_INVALID_CHARACTER = -0x0012,
    VM_SSL_BASE64_ERROR_MAX = 0x7FFFFFFF
} VM_SSL_BASE64_ERROR;

/* Callback function for connection events. This is configured in
   vm_ssl_context_t. */
typedef void (*vm_ssl_connection_callback)(VM_SSL_HANDLE handle,
        VM_SSL_EVENT event);

/* SSL configuration context */
typedef struct
{
    /* NULL-terminated IP address of the remote host to connect to. */
    VMCHAR* host;

    /* Port at which connection will be made. */
    VMINT port;

    /* Callback function for connection events, e.g data available for reading.
       Refer to VM_SSL_EVENT for details. */
    vm_ssl_connection_callback connection_callback;

    /* Verification type used. See VM_SSL_VERIFY_TYPE. */
    VM_SSL_VERIFY_TYPE authorize_mode;

    /* User agent string to pass to host. */
    VMCHAR* user_agent;
} vm_ssl_context_t;


/*****************************************************************************
 * FUNCTION
 *   vm_ssl_connect
 * DESCRIPTION
 *   Creates SSL connection to remote host.
 * PARAMETERS
 *  context: [IN] Configuration context, refer to vm_ssl_context_t for
 *                details on each field.
 * RETURNS
 *  Non-negative VM_SSL_HANDLE if successfully,
 *  and error code VM_SSL_ERROR if failed to create.
 *****************************************************************************/
VM_SSL_HANDLE vm_ssl_connect(const vm_ssl_context_t* context);


/*****************************************************************************
 * FUNCTION
 *   vm_ssl_load_ca_chain_certificate
 * DESCRIPTION
 *   Load certificate agency certificate in X.509 formate (.cer files)
 * PARAMETERS
 *  handle : [IN]  ssl handle
 *  certificate_buffer : [IN]  cert data, the raw data get from resource of app
 *  buffer_length : [IN]  length of certificate data.
 * RETURNS
 *  VMRESULT, refer to VM_SSL_X509_ERROR for detailed error values.
 *****************************************************************************/
VM_RESULT vm_ssl_load_ca_chain_certificate(VM_SSL_HANDLE handle,
        const void* certificate_buffer,
        VMINT buffer_length);

/*****************************************************************************
 * FUNCTION
 *  vm_ssl_read
 * DESCRIPTION
 *  read data from ssl connection.
 * PARAMETERS
 *  handle : [IN] ssl handle.
 *  buffer : [OUT] data buffer.
 *  length : [IN] data length.
 * RETURNS
 *  read data size.
 * RETURN VALUES
 *  Read size if read succesfully, VM_SSL_ERROR otherwise.
 *****************************************************************************/
VMINT vm_ssl_read(VM_SSL_HANDLE handle, VMUINT8* buffer, VMINT length);


/*****************************************************************************
 * FUNCTION
 *  vm_ssl_write
 * DESCRIPTION
 *  write data from ssl connection.
 * PARAMETERS
 *  handle : [IN] ssl handle.
 *  buffer : [OUT] data buffer.
 *  length : [IN] data length.
 * RETURNS
 *  >=0: Write data size.
 *  < 0: error code.
 * RETURN VALUES
 *  Written size if write succesfully, VM_SSL_ERROR otherwise.
 *****************************************************************************/
VMINT vm_ssl_write(VM_SSL_HANDLE handle, const VMUINT8* buffer, VMINT length);


/*****************************************************************************
 * FUNCTION
 *   vm_ssl_close
 * DESCRIPTION
 *  Closes the connection.
 * PARAMETERS
 *  handle: [IN] A valid SSL handle.
*****************************************************************************/
VM_RESULT vm_ssl_close(VM_SSL_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *   vm_ssl_get_verify_result
 * DESCRIPTION
 *  Get the verify result.
 * PARAMETERS
 *  handle : [IN] ssl handle.
 * RETURNS
 *  result
 * RETURN VALUES
 *  VM_SSL_VERIFY_RESULT
 *****************************************************************************/
VM_SSL_VERIFY_RESULT vm_ssl_get_verify_result(VM_SSL_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *  vm_ssl_base64_encode
 * DESCRIPTION
 *  This utility function encodes a buffer with base64 encoding.
 *  Call this function with *destination_length = 0 to obtain the
 *  required buffer size in *destination_length.
 * PARAMETERS
 *  destination:        [OUT] destination buffer.
 *  destination_length: [OUT] size of the buffer. This parameter is always
 *                            updated to reflect the amount of data that
 *                            has (or would have) been written, even if
 *                            the function fails.
 *  source:             [IN] source buffer.
 *  source_length:      [IN] amount of data to be encoded, in bytes.
 * RETURN VALUES
 *  0 if successful, otherwise VM_SSL_BASE64_ERROR is returned.
 *****************************************************************************/
VM_RESULT vm_ssl_base64_encode(VMBYTE* destination, VMINT* destination_length,
                               const VMBYTE* source, VMINT source_length);

/*****************************************************************************
* FUNCTION
*  vm_ssl_base64_decode
* DESCRIPTION
*  This utility function decodes a buffer with base64 encoding.
*  Call this function with *destination_length = 0 to obtain the
*  required buffer size in *destination_length.
* PARAMETERS
*  destination:        [OUT] destination buffer.
*  destination_length: [OUT] size of the buffer. This parameter is always
*                            updated to reflect the amount of data that
*                            has (or would have) been written, even if
*                            the function fails.
*  source:              [IN] source buffer.
*  source_length:       [IN] amount of data to be decoded, in bytes.
* RETURN VALUES
*  0 if successful, otherwise VM_SSL_BASE64_ERROR is returned.
*****************************************************************************/
VM_RESULT vm_ssl_base64_decode(VMBYTE* destination, VMINT* destination_length,
                               const VMBYTE* source, VMINT source_length);

/*****************************************************************************
* FUNCTION
*   vm_ssl_sha1_hmac
* DESCRIPTION
*  This utility function hashes input buffer using the HMAC algorithm
*  and the SHA-1 hash function to a 160 bits (20-bytes) hash value.
* PARAMETERS
*  key:          [IN] HMAC secret key.
*  key_length:   [IN] Length of the HMAC key.
*  input:        [IN] Buffer holding the  data.
*  input_length: [IN] Length of the input data.
*  output:       [OUT] HMAC-SHA-1 hash result.
* RETURNS
*  Always VM_OK as this function never fails.
*****************************************************************************/
VM_RESULT vm_ssl_sha1_hmac(VMBYTE* key, VMINT key_length,
                           const VMBYTE* input, VMINT input_length,
                           VMBYTE output[20]);

#ifdef __cplusplus
}
#endif

#endif /*VMSSL_SDK_H*/

