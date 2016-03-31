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

#ifndef VMWLAN_SDK_H
#define VMWLAN_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"




/* Maximum length of WLAN SSID attribute in WLAN profile. */
#define VM_WLAN_SSID_MAX_LENGTH             (32)
/* Maximum length of IP address in WLAN profile. */
#define VM_WLAN_IP_ADDRESS_MAX_LENGTH       (4)

//#ifdef __WAPI_SUPPORT__
#define VM_WLAN_WAPI_PSK_STR_LENGTH         (32+1)            /* WAPI PSK string length. */
//#endif /* __WAPI_SUPPORT__ */
/* Maximum length of password attribute in WLAN profile. */
#define VM_WLAN_PROF_MAX_PW_LENGTH          (31)
/* Maximum length of username attribute in WLAN profile. */
#define VM_WLAN_USERNAME_MAX_LENGTH         (31)
/* Maximum length of password attribute of all kinds in WLAN profiles. */
#define VM_WLAN_PASSWORD_MAX_LENGTH         (64)
/* WLAN maximum length of passphrase. */
#define VM_WLAN_PASSPHRASE_MAX_LENGTH       (32)

/* Maximum length of WLAN profile passphrase attribute in WLAN profile. */
#define VM_WLAN_PROF_MAX_PASSPHRASE_LENGTH  (64)
/* Maximum length of WLAN profile PSK attribute in WLAN profile. */
#define VM_WLAN_PROF_MAX_PSK_LENGTH         (34)
/* Maximum length of WLAN profile WEP key attribute in WLAN profile. */
#define VM_WLAN_PROF_MAX_WEP_KEY_LENGTH     (26)
/* Maximum length of WLAN profile ID list. */
#define VM_MAX_WLAN_ACCOUNT_NUM             (30)

/* Length of the MAC address. */
#define VM_WLAN_MAC_ADDRESS_LENGTH          (6)

/* Maximum length of SSID list. */
#define VM_WLAN_MAX_SPEC_SSID_LIST_NUMBER   (7)

/* Maximum length of AP list. */
#define VM_WLAN_MAX_AP_LIST_NUMBER          (30)

/* Profile fields. */

/* Set SSID field in WLAN account profile. */
#define VM_WLAN_PROFILE_FIELD_SSID              (0x00000002)

/* Set authorization type field in WLAN account profile. */
#define VM_WLAN_PROFILE_FIELD_AUTHORIZE_TYPE    (0x00000400)

/* Set username field in WLAN account profile. */
#define VM_WLAN_PROFILE_FIELD_USER              (0x00080000)

/* Set password field in WLAN account profile. */
#define VM_WLAN_PROFILE_FIELD_PASSWORD          (0x00100000)

/* Set all fields. */
#define VM_WLAN_PROF_FIELD_ALL                  (0xFFFFFFFF)


/* WLAN network type enum. */
typedef enum
{
    VM_WLAN_NETWORK_TYPE_INFRA,             /* Infrastructure mode. */
    VM_WLAN_NETWORK_TYPE_ADHOC,             /* Ad-hoc mode. */
    VM_WLAN_NETWORK_TYPE_MAX = 0x7F         /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_NETWORK_TYPE;


/* WLAN authorization mode enum. */
typedef enum
{
    VM_WLAN_AUTHORIZE_MODE_OPEN,                /* Open Network. */
    VM_WLAN_AUTHORIZE_MODE_WEP,                 /* WEP. */
    VM_WLAN_AUTHORIZE_MODE_IEEE8021X,           /* IEEE802.1x. */
    VM_WLAN_AUTHORIZE_MODE_WPA_ONLY,            /* WPA Enterprise. */
    VM_WLAN_AUTHORIZE_MODE_WPA_ONLY_PSK,        /* WPA Only PSK. */
    VM_WLAN_AUTHORIZE_MODE_WAPICERT,            /* WAPICERT. */
    VM_WLAN_AUTHORIZE_MODE_WAPIPSK,             /* WAPIPSK. */

    VM_WLAN_AUTHORIZE_MODE_WPA2_ONLY,           /* WPA2 Enterprise. */
    VM_WLAN_AUTHORIZE_MODE_WPA_WPA2_MIX,        /* WPA WPA2 Mix Enterprise. */
    VM_WLAN_AUTHORIZE_MODE_WPA2_ONLY_PSK,       /* WPA2 Only PSK. */
    VM_WLAN_AUTHORIZE_MODE_WPA_WPA2_MIX_PSK,    /* WPA WPA2 Mix PSK. */
    VM_WLAN_AUTHORIZE_MODE_MAX = 0x7F           /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_AUTHORIZE_MODE;


/* WLAN encryption modes. */
typedef enum
{
    VM_WLAN_ENCRYPT_MODE_NONE,              /* Unencrypted mode. */
    VM_WLAN_ENCRYPT_MODE_WEP,               /* WEP mode. */
    VM_WLAN_ENCRYPT_MODE_TKIP,              /* TKIP mode. */
    VM_WLAN_ENCRYPT_MODE_AES,               /* AES mode. */
    VM_WLAN_ENCRYPT_MODE_WPI,               /* WPI mode. */
    VM_WLAN_ENCRYPT_MODE_TKIP_AES,          /* TKIP_CCMP. */
    VM_WLAN_ENCRYPT_MODE_MAX = 0x7F         /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_ENCRYPT_MODE;


/* WLAN WEP key formats. */
typedef enum
{
    VM_WLAN_WEP_KEY_FORMAT_HEX,                 /* HEX encoded. */
    VM_WLAN_WEP_KEY_FORMAT_ASCII,               /* ASCII encoded. */
    VM_WLAN_WEP_KEY_FORMAT_MAX = 0x7F           /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_WEP_KEY_FORMAT;


/* WLAN AP types. */
typedef enum
{
    VM_WLAN_AP_USER_DEFINE,     /* User defined. */
    VM_WLAN_AP_DEFAULT,         /* Default. */
    VM_WLAN_AP_UNKNOW,          /* Unknown. */
    VM_WLAN_AP_BLACK_LIST,      /* Black list. */
    VM_WLAN_AP_MAX = 0x7F       /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_AP_TYPE;


/* Connection error types. */
typedef enum
{
    VM_WLAN_TIMEOUT,                /* Connection timed out. */
    VM_WLAN_LOST_COVERAGE,          /* Lost the coverage. */
    VM_WLAN_NORMAL_REJ,             /* Normal rejection. */
    VM_WLAN_CONNECT_FAIL,           /* Connection failed. */
    VM_WLAN_AUTH_FAIL,              /* Authorization failed. */
    VM_WLAN_CAUSE_COUNT,            /* Cause counts. */
    VM_WLAN_UNSUPPORTED_EAP_TYPE,   /* Unsupported EAP. */
    VM_WLAN_WRONG_USER_INFO,        /* Incorrect user information. */
    VM_WLAN_LOW_RSSI,               /* Low RSSI, when RSSI < -85dBm. */
    VM_WLAN_INVALID_CAUSE,          /* Invalid cause. */
    VM_WLAN_CAUSE_MAX = 0x7F        /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_CAUSE; /* same as supc_abm_cause_enum */


/* Connection types. */
typedef enum
{
    VM_WLAN_ALL_PROFILE_ENABLE,                 /* All profiles enabled. */
    VM_WLAN_ONE_PROFILE_SELECT,                 /* One profile is selected. */
    VM_WLAN_START_WPS_PIN,                      /* WPS with PIN. */
    VM_WLAN_START_WPS_PBC,                      /* WPS with PBC (Push-Button Configuration). */
    VM_WLAN_CONNECTION_TYPE_MAX = 0x7F          /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_CONNECTION_TYPE; /* supc_connection_type_enum */


/* WLAN request result enum. */
typedef enum
{
    VM_WLAN_REQUEST_DONE            = 0,    /* Request completed successfully. */
    VM_WLAN_REQUEST_TERMINATED,             /* Request is terminated by newer request. */
    VM_WLAN_REQUEST_DUPLICATED,             /* Request is removed by newer same request. */
    VM_WLAN_REQUEST_ERROR_FAILED,           /* Request is ignored due to state error. */
    VM_WLAN_REQUEST_ADD_BY_SRV,             /* Request is added by the server. */
    VM_WLAN_REQUEST_UNKNOWN_ERROR,          /* Unknown error. */
    VM_WLAN_REQUEST_CANCELLED,              /* Cancelled. */
    VM_WLAN_REQUEST_BUSY,                   /* Busy. */
    VM_WLAN_REQUEST_RESULT_MAX = 0x7F       /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_REQUEST_RESULT;


/* WLAN scan result enum. */
typedef enum
{
    VM_WLAN_SCAN_SUCCESS        = 0,        /* WLAN scan success. */
    VM_WLAN_SCAN_ERROR          = -1,       /* WLAN scan failed. */
    VM_WLAN_SCAN_ERROR_ABORTED  = -2,       /* WLAN scan service is aborted. */
    VM_WLAN_SCAN_ERROR_DELETED  = -3,       /* WLAN scan delete in CONNECT_WAIT. */
    VM_WLAN_SCAN_RESULT_MAX = 0x7F          /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_SCAN_RESULT;


/* WLAN services result enum. */
typedef enum
{
    VM_WLAN_SUCCESS                 = 0,    /* Success. */
    VM_WLAN_ERROR_FAILED            = -1,   /* General failed case. */
    VM_WLAN_ERROR_PROCESSING        = -2,   /* Processing state implies event is being notified. */
    VM_WLAN_ERROR_STATE_ERROR       = -3,   /* Service status error. */
    VM_WLAN_ERROR_INVALID_INPUT     = -4,   /* Invalid input parameter. */
    VM_WLAN_ERROR_ABORTED           = -5,   /* The service is aborted. */
    VM_WLAN_ERROR_TERMINATED        = -6,   /* The service is terminated. */
    VM_WLAN_ERROR_ONE_STORE_EXISTS  = -7,   /* Currently only support one store. */
    VM_WLAN_ERROR_STORE_NOT_COMMIT  = -8,   /* Store was not configured and committed. */
    VM_WLAN_ERROR_INVALID_STORE     = -9,   /* Invalid store. */
    VM_WLAN_ERROR_STORE_FULL        = -10,  /* Reach maximum profile storage. */
    VM_WLAN_ERROR_READ_ONLY         = -11,  /* Cannot delete or update read_only profile. */
    VM_WLAN_ERROR_FIRST_BOOTUP      = -12,  /* The first boot up, return code for store open. */
    VM_WLAN_ERROR_RESET_FACTORY     = -13,  /* The data base SIM profile folder is deleted. */
    VM_WLAN_ERROR_FS_ERROR          = -14,  /* The database file system error. */
    VM_WLAN_ERROR_IGNORE            = -15,  /* Ignored. */
    VM_WLAN_RESULT_MAX = 0x7F  /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_RESULT;


/* WLAN set mode enum. */
typedef enum
{
    VM_WLAN_MODE_NONE,              /* Not set. */
    VM_WLAN_MODE_OFF,               /* OFF. */
    VM_WLAN_MODE_AP,                /* AP (Access Point) mode only. This mode is reserved and not available now. */
    VM_WLAN_MODE_STA,               /* STA (Station) mode only. Use this mode to join an access point. */
    VM_WLAN_MODE_AP_STA,            /* AP+STA. This mode is reserved and not available now. */
    VM_WLAN_MODE_MAX = 0x7F         /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_MODE;


/* WLAN mode status enum. */
typedef enum
{
    VM_WLAN_MODE_STATUS_OFF,              /* OFF. */
    VM_WLAN_MODE_STATUS_OFF_SWITCHING,    /* Switching to OFF mode. */
    VM_WLAN_MODE_STATUS_AP,               /* AP only. */
    VM_WLAN_MODE_STATUS_AP_SWITCHING,     /* Switching to AP mode. */
    VM_WLAN_MODE_STATUS_STA,              /* STA only. */
    VM_WLAN_MODE_STATUS_STA_SWITCHING,    /* Switching to STA mode. */
    VM_WLAN_MODE_STATUS_AP_STA,           /* AP+STA mode. */
    VM_WLAN_MODE_STATUS_AP_STA_SWITCHING, /* Switching to AP+STA mode. */
    VM_WLAN_MODE_STATUS_ERROR,            /* Error. */
    VM_WLAN_MODE_STATUS_MAX = 0x7F        /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_MODE_STATUS;


/* WLAN station mode status. */
typedef enum
{
    VM_WLAN_STATION_STATUS_INACTIVE     = 0x01, /* Inactive. */
    VM_WLAN_STATION_STATUS_ACTIVE       = 0x02, /* Active. */
    VM_WLAN_STATION_STATUS_CONNECTED    = 0x04, /* Connected. */
    VM_WLAN_STATION_STATUS_INVALID_MAC  = 0x08, /* Current MAC address is not valid. OR'd with ACTIVE and CONNECTED status. */
    VM_WLAN_STATION_STATUS_SNIFFER      = 0x10, /* Sniffer status. */
    VM_WLAN_STATION_STATUS_BUSY         = 0x20, /* Busy. Status is changing. */
    VM_WLAN_STATION_STATUS_MAX = 0x7FFFFFFF     /* Forces the compiler to use 4-byte integer for enum. */
} VM_WLAN_STATION_STATUS;


/* Sniffer ON Callback types. */
typedef enum
{
    VM_WLAN_SNIFFER_ON_RES,                             /* Resource type of callback. */
    VM_WLAN_SNIFFER_ON_AP_INFO,                         /* AP information type of callback. */
    VM_WLAN_SNIFFER_ON_CALLBACK_TYPE_MAX = 0x7F         /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_SNIFFER_ON_CALLBACK_TYPE;


/* Sniffer ON/OFF callback result Enum */
typedef enum
{
    VM_WLAN_SNIFFER_SUCCESS                              = 0,    /* Success. */
    VM_WLAN_SNIFFER_ERROR_INVALID_CHANNEL                = -1,   /* Invalid channel. */
    VM_WLAN_SNIFFER_ERROR_INVALID_SNIFFER_FILTER_OPTIONS = -2,   /* Invalid sniffer filter options. */
    VM_WLAN_SNIFFER_ERROR_INVALID_ON_OFF_SWITCH          = -3,   /* Invalid on/off switch. */
    VM_WLAN_SNIFFER_ERROR_NETWORK_IS_CONNECTED           = -4,   /* Network is connected, unable to sniff. */
    VM_WLAN_SNIFFER_ERROR_UNKNOWN                        = -5,   /* Unknown error. */
    VM_WLAN_SNIFFER_ERROR_STATE_ERROR                    = -6,   /* State error. */
    VM_WLAN_SNIFFER_ERROR_HW_ERROR                       = -7,   /* Hardware error. */
    VM_WLAN_SNIFFER_RESULT_MAX = 0x7F    /* Forces the compiler to use 1-byte integer for enum. */
} VM_WLAN_SNIFFER_RESULT;


/* Notification types. */
typedef enum
{
    VM_WLAN_NOTIFICATION_NONE,                  /* None. */
    VM_WLAN_NOTIFICATION_IP_AVAILABLE,          /* IP is available. */
    VM_WLAN_NOTIFICATION_PASSIVE_DISCONNECT,    /* Passively disconnect. */
    VM_WLAN_NOTIFICATION_TYPE_MAX = 0x7FFF      /* Forces the compiler to use 2-byte integer for enum. */
} VM_WLAN_NOTIFICATION_TYPE;


/* Connected AP info type */
typedef enum
{
    VM_WLAN_AP_INFO_BSSID,      /* Gets BSSID. */
    VM_WLAN_AP_INFO_PROF_ID,    /* Gets Profile ID. */
    VM_WLAN_AP_INFO_RSSI,       /* Gets RSSI. */
    VM_WLAN_AP_INFO_ALL,        /* Gets All information. */
    VM_WLAN_AP_INFO_TYPE_MAX = 0x7FFF   /* Forces the compiler to use 2-byte integer for enum. */
} VM_WLAN_AP_INFO_TYPE;


/* Scan parameters */
typedef enum
{
    VM_WLAN_SCAN_PARAMETER_DWELL_TIME,          /* Gets or sets the dwell time. */
    VM_WLAN_SCAN_PARAMETER_SPECIFIC_SSID_LIST,  /* Gets or Sets the SSID list. */
    VM_WLAN_SCAN_PARAMETER_MAX = 0x7FFF         /* Forces the compiler to use 2-byte integer for enum. */
} VM_WLAN_SCAN_PARAMETER;


/* WLAN parameters. */
typedef enum
{
    VM_WLAN_PARAMETER_AP_AUTHORIZE_MODE = 0x00000001, /* Authorization in AP mode. */
    VM_WLAN_PARAMETER_AP_SSID           = 0x00000002, /* SSID in AP mode. */
    VM_WLAN_PARAMETER_AP_PASSWORD       = 0x00000004, /* Password in AP mode. */

    VM_WLAN_PARAMETER_AP_ALL = VM_WLAN_PARAMETER_AP_AUTHORIZE_MODE |
                               VM_WLAN_PARAMETER_AP_SSID |
                               VM_WLAN_PARAMETER_AP_PASSWORD, /* All parameters. */

    VM_WLAN_PARAMETER_MAX = 0x7FFFFFFF   /* Forces the compiler to use 4-byte integer for enum. */
} VM_WLAN_PARAMETER;

/* Profile settings from Smart Connection broadcasts. Note that
 * the username field is not supported by current version
 * of Smart Connection.
 */
typedef struct
{
    VMUINT8  ssid[VM_WLAN_SSID_MAX_LENGTH + 1];         /* SSID string (ASCII). */
    VM_WLAN_AUTHORIZE_MODE authorize_mode;              /* Profile authorization mode. */
    VMUINT8  username[VM_WLAN_USERNAME_MAX_LENGTH + 1]; /* Username string (ASCII). This field is reserved and not usable now. */
    VMUINT8  password[VM_WLAN_PASSWORD_MAX_LENGTH + 1]; /* Password string (ASCII). */
    VMUINT32 profile_id;                                /* Profile ID. */
} vm_wlan_profile_t;


/* Sniffer ON structure , used in callback function. */
typedef struct
{
    VM_WLAN_SNIFFER_ON_CALLBACK_TYPE cb_type;   /* Callback type. */
    VM_WLAN_SNIFFER_RESULT           result;    /* Result. */
    vm_wlan_profile_t                profile;   /* Profile settings from Smart Connection broadcasts. */
} vm_wlan_sniffer_on_result_t;


/* Sniffer ON configuration structure, used as parameter. */
typedef struct
{
    VMUINT32    sniffer_filter_options; /* Sniffer filter options. */
    VMUINT8     channel_number;         /* Channel number. */
    VMINT32     min_rssi;               /* Minimum RSSI. */
    VMINT32     max_rssi;               /* Maximum RSSI. */
    VMUINT16    min_package_length;     /* Minimum package length. */
    VMUINT16    max_package_length;     /* Maximum package length. */
} vm_wlan_sniffer_on_config_t;


/* WLAN Notification structure. */
typedef struct
{
    VM_WLAN_NOTIFICATION_TYPE type; /* Notification type. */
    void* data;                     /* Notification data. */
} vm_wlan_notification_t;


/* MAC address query structure. */
typedef struct
{
    VMCHAR*  destination;           /* Destination buffer. */
    VMUINT16 destination_length;    /* Destination buffer length. */
    VMUINT16 request_length;        /* If this attribute is provided by
                                     * application (NOT NULL), WLAN SRV will hint
                                     * application the required buffer length. */
} vm_wlan_string_info_query_t;


/* WLAN scan AP info structure. */
typedef struct
{
    VMUINT8     bssid[VM_WLAN_MAC_ADDRESS_LENGTH];  /* MAC address. */
    VMUINT8     ssid [VM_WLAN_SSID_MAX_LENGTH];     /* SSID. */
    VMUINT16    ssid_len;                           /* SSID length. */
    VMINT32     rssi;                               /* Received signal strength in dBm. */
    VMUINT32    channel_number;                     /* AP Channel number. */
    VMUINT8     max_rate;                           /* AP maximum rate. */
    VMUINT8     privacy;                            /* WEP encryption requirement. */
    VM_WLAN_NETWORK_TYPE    network_type;           /* Profile network type. */
    VM_WLAN_AUTHORIZE_MODE  authorize_mode;         /* Profile authorization mode. */
    VM_WLAN_ENCRYPT_MODE    encrypt_mode;           /* Profile encrypt mode. */
} vm_wlan_scan_ap_info_t;


/* This struct is designed for WLAN SRV to send the scan result to APP. */
typedef struct
{
    VM_WLAN_SCAN_RESULT result;         /* Scan result enum. */
    VMUINT8             ap_list_number; /* The total number of searched AP list. */
    VMINT32             scan_job_id;    /* Scan job ID. 0xFFFFFF means scan result broadcasting. */
    vm_wlan_scan_ap_info_t ap_list[VM_WLAN_MAX_AP_LIST_NUMBER]; /* BSS info array. */
} vm_wlan_scan_result_t;


/* This struct is designed for user to pass the specialized SSID for scanning. */
typedef struct
{
    VMUINT8 ssid_len;                       /* SSID length. */
    VMUINT8 ssid[VM_WLAN_SSID_MAX_LENGTH];  /* SSID. */
} vm_wlan_ssid_t;


/* This struct is designed for WLAN SRV to send the connection result to APP. */
typedef struct
{
    VM_WLAN_RESULT  result; /* SUCCESS: connected; FAILED: connection failed;
                             * ABORTED: preemptied by a new connection request;
                             * TERMINATED: terminated by a new disconnect request;
                             * STATE_ERROR: cannot perform connection request. */
    VM_WLAN_CAUSE   cause;  /* Error cause. Used only when the result is FAILED. */
    VMUINT8         bssid[VM_WLAN_MAC_ADDRESS_LENGTH];  /* MAC address. */
    VMUINT8         ssid[VM_WLAN_SSID_MAX_LENGTH];      /* SSID. */
    VMUINT16        ssid_length;                        /* SSID length. */
    VMBYTE          init_by_socket;     /* Connection activated by socket flag. */
    VMUINT32        channel_number;     /* Channel info from chip. */
    VMUINT32        profile_id;         /* Connected profile ID. */
    VMUINT32        authorize_type;     /* Authorization type. */
    VMUINT32        eap_peap_authorize_type;    /* EAP PEAP authentication type. */
    VMUINT32        eap_ttls_authorize_type;    /* EAP TTLS authentication type. */
    VM_WLAN_CONNECTION_TYPE connection_type;    /* Connection type. */
    VMUINT16        encry_mode; /* Encryption mode. The default is WPS.
                                 * the encry_mode is not parsed in M2. */
    VMUINT8         passphrase[VM_WLAN_PASSPHRASE_MAX_LENGTH];  /* Pass phrase. */
    VMUINT8         passphrase_length;      /* Length of the pass phrase. */
    VMBYTE          need_login;             /* Need login. */
    VMBYTE          cancel_by_disconnect;   /* Cancelled due to disconnection. */
    VMINT           rssi;                   /* RSSI. */
} vm_wlan_connect_result_t;


/* This struct is designed for WLAN store query operation. */
typedef struct
{
    VMUINT32 ids[VM_MAX_WLAN_ACCOUNT_NUM];  /* Profile ID list. */
    VMUINT8  number_ids;                    /* The number of IDs in the list. */
} vm_wlan_profile_list_t;


/* Event structure for setting roaming parameter. */
typedef struct
{
    VMBYTE  is_roaming; /* param of roaming, 1 is roaming, 0 is non-roaming */ //[Seven] - make it as VMBOOL?
} vm_wlan_roaming_parameter_t;


/* This struct is designed for WLAN server to retrieve the IP information. */
typedef struct
{
    VMUINT8 ip_address[VM_WLAN_IP_ADDRESS_MAX_LENGTH];  /* IP address. */
    VMUINT8 netmask[VM_WLAN_IP_ADDRESS_MAX_LENGTH];     /* Netmask. */
    VMUINT8 gateway[VM_WLAN_IP_ADDRESS_MAX_LENGTH];     /* Gateway. */
    VMUINT8 dns1[VM_WLAN_IP_ADDRESS_MAX_LENGTH];        /* Primary DNS. */
    VMUINT8 dns2[VM_WLAN_IP_ADDRESS_MAX_LENGTH];        /* Secondary DNS. */
} vm_wlan_ip_info_t;


/* AP information. */
typedef struct
{
    VMUINT8 ssid[VM_WLAN_SSID_MAX_LENGTH + 1];          /* SSID of the AP. */
    VM_WLAN_AUTHORIZE_MODE authorize_mode;              /* Authorization mode. */
    VMUINT8 password[VM_WLAN_PASSWORD_MAX_LENGTH + 1];  /* Password of the AP. */
} vm_wlan_ap_info_t;


/* Connected AP information. */
typedef struct
{
    VMUINT8     bssid[VM_WLAN_MAC_ADDRESS_LENGTH + 1];  /* BSSID. */
    VMUINT32    profile_id;                             /* Profile ID. */
    VMINT32     rssi_out;                               /* RSSI out. */
} vm_wlan_connected_ap_info_t;


/* Part of scan parameters -- the specific SSID list. */
typedef struct
{
    VMUINT8 ssid_list_number;                                    /* The length of the ssid_list. */
    vm_wlan_ssid_t ssid_list[VM_WLAN_MAX_SPEC_SSID_LIST_NUMBER]; /* List of the specific SSID. */
} vm_wlan_specific_ssid_list_t;


/* AP mode information. */
typedef struct
{
    VM_WLAN_AUTHORIZE_MODE authorize_mode;      /* Authorization mode. */
    VMINT8 reserved;                            /* Reserved */
    VMINT8 ssid[VM_WLAN_SSID_MAX_LENGTH + 1];   /* SSID. */
    VMINT8 psk[VM_WLAN_PASSWORD_MAX_LENGTH];    /* PSK. */
} vm_wlan_parameter_t;


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_connect_callback
 * DESCRIPTION
 *  Prototype of WLAN connect result callback function
 * PARAMETERS
 *  user_data:      [IN]  Additional user data for the callback.
 *  connect_result: [IN]  Connection result.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_connect_callback) (void* user_data,
        vm_wlan_connect_result_t* connect_result);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_callback
 * DESCRIPTION
 *  Prototype of WLAN general callback function for no feedback arguments except
 *  the additional user data.
 * PARAMETERS
 *  user_data: [IN] Additional user data for the callback.
 *  result:    [IN] Request result.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_callback) (void* user_data, VM_WLAN_REQUEST_RESULT result);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_sniffer_on_callback
 * DESCRIPTION
 *  Prototype of WLAN sniffer on callback function. If successfully, this
 *  function will be called again with AP info.
 * PARAMETERS
 *  user_data: [IN]  Additional user data for the callback.
 *  result:    [IN] Sniffer ON result from WLAN server.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_sniffer_on_callback) (void* user_data,
        vm_wlan_sniffer_on_result_t* result);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_sniffer_off_callback
 * DESCRIPTION
 *  Prototype of WLAN sniffer OFF callback function.
 * PARAMETERS
 *  user_data: [IN]  Additional user data for the callback.
 *  result:    [IN]  Sniffer OFF result from the WLAN server.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_sniffer_off_callback) (void* user_data,
        VM_WLAN_SNIFFER_RESULT result);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_notification_callback
 * DESCRIPTION
 *  Prototype of notification callback function. When notification arrives, the
 *  corresponding callback function will be invoked if it has been registered.
 * PARAMETERS
 *  user_data:    [IN] Additional user data for the callback.
 *  notification: [IN] The notification that triggers the callback.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_notification_callback) (void* user_data,
        vm_wlan_notification_t* notification);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_scan_callback
 * DESCRIPTION
 *  WLAN scan result callback function.
 *  APPs need to copy the bss info byself.
 * PARAMETERS
 *  job_id:      [IN] The job ID returned from vm_wlan_scan().
 *  user_data:   [IN] Additional user data for the callback.
 *  scan_result: [IN] The scan result from the WLAN server.
 * RETURNS
 *  void
 *****************************************************************************/
typedef void (*vm_wlan_scan_callback) (VMUINT32 job_id, void* user_data,
                                       vm_wlan_scan_result_t* scan_result);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_connect
 * DESCRIPTION
 *  Connects to the AP, as specified by the ap_info. DA WLAN service can only
 *  support one connection at the same time due to chip constraint.
 *  If a new connection request is received, the connecting process of the current
 *  request will be aborted. If there is a connection established already, the
 *  connection will be disconnected, and then make a new connection.
 * PARAMETERS
 *  ap_info:   [IN] Specifies the AP to be connected to.
 *  callback:  [IN] Connection result callback to notify the application.
 *  user_data: [IN] Additional user data for the callback.
 * RETURN VALUES
 *  Refer to VM_WLAN_RESULT for return value and error code.
 * EXAMPLE
 *  <code>
 *  // STA status is ACTIVED before calling vm_wlan_connect().
 *  vm_wlan_ap_info_t ap_info = {0};
 *  strcpy(ap_info.ssid, "ssidexample");
 *  ap_info.authorize_mode = VM_WLAN_AUTHORIZE_MODE_WPA_ONLY_PSK;
 *  strcpy(ap_info.password, "ssidpwd");
 *  vm_wlan_connect(&ap_info, conn_cb, NULL);
 *  void conn_cb(void* user_data, vm_wlan_connect_result_t* res)
 *  {
 *       if (VM_WLAN_SUCCESS == res->result)
 *       {
 *           // Connection succeeds.
 *       }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_connect(vm_wlan_ap_info_t* ap_info,
                          vm_wlan_connect_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_disconnect
 * DESCRIPTION
 *  Disconnects the current WLAN connection. After the WLAN disconnecting process
 *  is complete, the specified callback will be invoked.
 * PARAMETERS
 *  callback:  [IN] The callback to notify the application that the disconnect
 *                  operation is complete.
 *  user_data: [IN] Additional user data for the callback.
 * RETURN VALUES
 *  Refer to VM_WLAN_RESULT for return value and error code.
 * EXAMPLE
 *  <code>
 *  // STA status is CONNECTED before calling vm_wlan_disconnect().
 *  vm_wlan_disconnect(disconn_cb, NULL);
 *  void disconn_cb(void* user_data, VM_WLAN_REQUEST_RESULT res)
 *  {
 *       if (VM_WLAN_REQUEST_DONE == res)
 *       {
 *           // Disconnection succeeds.
 *       }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_disconnect(vm_wlan_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_mode_set
 * DESCRIPTION
 *  Switches the mode.
 *  VM_WLAN_SET_MODE_OFF turns off both AP mode and STA mode.
 *  VM_WLAN_SET_MODE_AP turns only AP mode on.
 *  VM_WLAN_SET_MODE_STA turns only STA mode on.
 *  VM_WLAN_SET_MODE_AP_STA turns both AP mode and STA mode on.
 * PARAMETERS
 *  mode:      [IN] Specifies the mode to be switched on.
 *  callback:  [IN] The callback to notify the application about the completion
 *                  of the mode switch.
 *  user_data: [IN] Additional user data for the callback.
 * RETURN VALUES
 *  Refer to VM_WLAN_RESULT for return value and error code.
 * EXAMPLE
 *  <code>
 *  vm_wlan_mode_set(VM_WLAN_SET_MODE_AP, switch_mode_cb, NULL);
 *  void switch_mode_cb(void* user_data, VM_WLAN_REQUEST_RESULT res)
 *  {
 *      if (VM_WLAN_REQUEST_DONE == res)
 *      {
 *          // Switch mode successfully.
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_mode_set(VM_WLAN_MODE mode, vm_wlan_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_mode_get
 * DESCRIPTION
 *  Retrieves the WLAN mode.
 * PARAMETERS
 *  void
 * RETURNS
 *  VM_WLAN_MODE
 * EXAMPLE
 *  <code>
 *  VM_WLAN_MODE mode;
 *  mode = vm_wlan_mode_get();
 *  </code>
 *****************************************************************************/
VM_WLAN_MODE vm_wlan_mode_get(void);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_station_status
 * DESCRIPTION
 *  Retrieves the status of WLAN STA mode.
 * PARAMETERS
 *  void
 * RETURNS
 *  VM_WLAN_STATION_STATUS. The VM_WLAN_STATION_STATUS_BUSY means that the WiFi
 *  status is changing. The caller needs to wait for the status changing
 *  finishes and polls the status again.
 * EXAMPLE
 *  <code>
 *  VM_WLAN_STATION_STATUS sta_status;
 *  sta_status = vm_wlan_station_status();
 *  </code>
 *****************************************************************************/
VM_WLAN_STATION_STATUS vm_wlan_station_status(void);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_get_ip_info
 * DESCRIPTION
 *  Gets the IP address, Netmask, gateway IP and DNS, when they are available.
 *  The IP information will be available only when the WLAN server is in CONNECTED
 *  status.
 *  There are two situations due to the way of how to obtain the IP.
 *  If using static IP when connecting, the IP will be available once the
 *  connection succeeds.
 *  If using DHCP when connecting, the IP will be available only when ABM notifies
 *  the IP by MSG, which will take a little time after connection succeeds.
 * PARAMETERS
 *  ip_info: [OUT] Containing the IP information upon the return of this function.
 * RETURN VALUES
 *  Refer to VM_WLAN_RESULT for return value and error code.
 *  The VM_WLAN_ERROR_STATE_ERROR means it was at the wrong time to get IP, as
 *  the IP is not available yet.
 * EXAMPLE
 *  <code>
 *  vm_wlan_ip_info_t ip_info = {0};
 *  vm_wlan_get_ip_info(&ip_info);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_get_ip_info(vm_wlan_ip_info_t* ip_info);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_get_mac_address
 * DESCRIPTION
 *  Gets the MAC address.
 *  The size of the dest buffer must be equal to or greater than
 *  VM_WLAN_MAC_ADDRESS_LENGTH, which means the destination_length must be equal
 *  to or greater than VM_WLAN_MAC_ADDRESS_LENGTH. Otherwise the WLAN Server will
 *  fail, and the required length (VM_WLAN_MAC_ADDRESS_LENGTH) will be set in
 *  the request_length, back to the application.
 *
 *  If the dest is NULL and the request_length exists (not NULL), the function
 *  will fail with the required length set in request_length for the application.
 * PARAMETERS
 *  mac_address: [IN|OUT] Containing the MAC address upon the return of this function.
 * RETURN VALUES
 *  The zero means the function returns successfully. Any other value means failed.
 * EXAMPLE
 *  <code>
 *  CHAR dest[VM_WLAN_MAC_ADDRESS_LENGTH] = {0};
 *  vm_wlan_string_info_query_t mac_addr;
 *  mac_addr.destination = dest;
 *  mac_addr.destination_length = VM_WLAN_MAC_ADDRESS_LENGTH;
 *  vm_wlan_get_mac_address(&mac_addr);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_get_mac_address(vm_wlan_string_info_query_t* mac_address);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_get_connected_ap_info
 * DESCRIPTION
 *  Gets information of the connected AP, such as BSSID, RSSI and the profile ID.
 *
 *  The info_type specifies what kind of information to be retrieved. The caller
 *  should ensure enough space for the buffer to store the information as following:
 *  info_type                   info
 *  VM_WLAN_AP_INFO_BSSID      VMUINT8 array with length at least VM_WLAN_MAC_ADDRESS_LENGTH + 1.
 *  VM_WLAN_AP_INFO_PROF_ID    VMUINT32* points to a VMUINT32 variable.
 *  VM_WLAN_AP_INFO_RSSI       VMINT32* points to an VMINT32 variable.
 *  VM_WLAN_AP_INFO_ALL        vm_wlan_connected_ap_info_t* to an vm_wlan_connected_ap_info_t variable.
 *
 * PARAMETERS
 *  info       [OUT] Containing the AP information upon the return of this function.
 *  info_type: [IN]  Specifies the type of information to be retrieved.
 * RETURN VALUES
 *  The zero means the function returns successfully. Any other value means failed.
 * EXAMPLE
 *  <code>
 *  // Gets the BSSID.
 *  VMUNIT8 bssid[VM_WLAN_MAC_ADDRESS_LENGTH + 1] = {0};
 *  vm_wlan_get_connected_ap_info((void*)bssid, VM_WLAN_AP_INFO_BSSID);
 *
 *  // Gets the profile ID.
 *  VMUNIT32 prof_id;
 *  vm_wlan_get_connected_ap_info((void*)&prof, VM_WLAN_AP_INFO_PROF_ID);
 *
 *  // Gets the RSSI.
 *  VMINT32 rssi;
 *  vm_wlan_get_connected_ap_info((void*)&rssi, VM_WLAN_AP_INFO_RSSI);
 *
 *  // Gets all information.
 *  vm_wlan_connected_ap_info_t ap_info = {0};
 *  vm_wlan_get_connected_ap_info((void*)&ap_info, VM_WLAN_AP_INFO_ALL);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_get_connected_ap_info(void* info, VM_WLAN_AP_INFO_TYPE info_type);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_scan
 * DESCRIPTION
 *  Scans for the Access Points.
 * PARAMETERS
 *  callback:  [IN] The callback to be invoked upon the completion of the scan.
 *  user_data: [IN] Additional user data.
 * RETURN VALUES
 *  If it returns a non-negative number, the number is the job ID of the scan
 *  job that is created by this function successfully.
 *  If it returns a negative number, it means it cannot create a scan job.
 * EXAMPLE
 *  <code>
 *  VMUINT32 job_id;
 *  job_id = vm_wlan_scan(scan_cb, NULL);
 *
 *  void scan_cb(VMUINT32 job_id, void* user_data, vm_wlan_scan_result_t* scan_res)
 *  {
 *      if (VM_WLAN_SCAN_SUCCESS == scan_res->result)
 *      {
 *          // Scan successfully. Information of APs scaned is in scan_res.
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_scan(vm_wlan_scan_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_scan_abort
 * DESCRIPTION
 *  Aborts the scan job.
 * PARAMETERS
 *  job_id: [IN] The job ID to be aborted. It is the return value of calling vm_wlan_scan().
 * RETURNS
 *  If it is successful, it returns 0. If it fails, it returns non-zero.
 * EXAMPLE
 *  <code>
 *  // The job_id is returned by vm_wlan_scan().
 *  vm_wlan_scan_abort(job_id);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_scan_abort(VMUINT32 job_id);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_scan_set_parameter
 * DESCRIPTION
 *  Sets scan parameters, such as dwell time or specific SSID list.
 *  The scan parameters will be initialized to the default values everytime when
 *  the WLAN is turned on from inactive. Setting scan parameters is not useful
 *  if the WLAN is in INACTIVE state. So it is not allowed to call this function
 *  when the WLAN is in INACTIVE state.
 *  If the scan parameters are not set, the default will be used. The default
 *  dwell time is 60 seconds; the default spec_ssid_list is NULL.
 *
 *  The parameter_type specifies what kind of scan parameters that is expected
 *  to be set. The caller should provide proper parameter for each kind of scan
 *  parameter type as following:
 *
 *  parameter_type                              scan_parameter
 *  VM_WLAN_SCAN_PARAMETER_DWELL_TIME          VMUINT16* points to a VMUINT16 variable.
 *  VM_WLAN_SCAN_PARAMETER_SPECIFIC_SSID_LIST  vm_wlan_specific_ssid_list_t* points to an
 *                                             vm_wlan_specific_ssid_list_t variable.
 * PARAMETERS
 *  scan_parameter: [IN] The actual scan parameters to be set.
 *  parameter_type: [IN] Specifies what type of parameters to be set for scan.
 *  callback:       [IN] The callback to be invoked when the scan completes.
 *  user_data:      [IN] Addition user data.
 * RETURN VALUES
 *  VM_WLAN_SUCCESS:                Success.
 *  VM_WLAN_ERROR_FAILED:           Failed.
 *  VM_WLAN_ERROR_INVALID_INPUT:    Invalid input parameter(s).
 *  VM_WLAN_ERROR_STATE_ERROR:      Try to set scan parameter(s) in INACTIVE state.
 * EXAMPLE
 *  <code>
 *  // Sets dwell time.
 *  VMUINT16 dwell_time = 70;
 *  vm_wlan_scan_set_parameter((void*)&dwell_time, VM_WLAN_SCAN_PARAMETER_DWELL_TIME,
 *                             set_dwell_time_cb, NULL);
 *
 *  void set_dwell_time_cb(void* user_data, VM_WLAN_REQUEST_RESULT res)
 *  {
 *      if (VM_WLAN_REQUEST_DONE == res)
 *      {
 *          // Sets dwell time successfully.
 *      }
 *  }
 *
 *  // Sets parameter for the specific SSID list.
 *  vm_wlan_specific_ssid_list_t ssid_list = {0};
 *  ssid_list.ssid_list[0] = {5, "ssid1"};
 *  ssid_list.ssid_list[1] = {5, "ssid2"};
 *  ssid_list.ssid_list[2] = {5, "ssid3"};
 *  ssid_list.ssid_list[3] = {5, "ssid4"};
 *  ssid_list.ssid_list_number = 4;
 *  vm_wlan_scan_set_parameter((void*)&ssid_list,
 *                             VM_WLAN_SCAN_PARAMETER_SPECIFIC_SSID_LIST,
 *                             set_spec_ssid_list_cb, NULL);
 *
 *  void set_spec_ssid_list_cb(void* user_data, VM_WLAN_REQUEST_RESULT res)
 *  {
 *      if (VM_WLAN_REQUEST_DONE == res)
 *      {
 *          // Sets parameter for the specific SSID list successfully.
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_scan_set_parameter(void* scan_parameter,
                                     VM_WLAN_SCAN_PARAMETER parameter_type,
                                     vm_wlan_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_scan_get_parameter
 * DESCRIPTION
 *  Gets the scan parameters, such as the dwell time or the specific SSID list.
 *
 *  The parameter_type specifies what kind of scan parameters to be retrieved.
 *  The caller should provide proper parameter_type to each kind of scan
 *  parameter as following:
 *
 *  parameter_type                              scan_parameter
 *  VM_WLAN_SCAN_PARAMETER_DWELL_TIME          VMUINT16* points to a VMUINT16 variable.
 *  VM_WLAN_SCAN_PARAMETER_SPECIFIC_SSID_LIST  vm_wlan_specific_ssid_list_t* points to an
 *                                             vm_wlan_specific_ssid_list_t variable.
 * PARAMETERS
 *  scan_parameter: [OUT] The actual scan parameters retrieved upon the return
 *                        of this function.
 *  parameter_type: [IN]  Specifies what type of parameters to be retrieved.
 * RETURN VALUES
 *  VM_WLAN_RESULT_SUCCESS:        Success.
 *  VM_WLAN_RESULT_FAILED:         Failed.
 *  VM_WLAN_RESULT_INVALID_INPUT:  Invalid input parameter(s).
 * EXAMPLE
 *  <code>
 *  // Gets the dwell time.
 *  VMUINT16 dwell_time;
 *  vm_wlan_scan_get_parameter((void*)&dwell_time, VM_WLAN_SCAN_PARAMETER_DWELL_TIME);
 *
 *  // Gets the specific SSID list.
 *  vm_wlan_specific_ssid_list_t ssid_list;
 *  vm_wlan_scan_get_parameter((void*)&ssid_list,
                               VM_WLAN_SCAN_PARAMETER_SPECIFIC_SSID_LIST);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_scan_get_parameter(void* parameter_value,
                                     VM_WLAN_SCAN_PARAMETER parameter_type);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_register_notification_handler
 * DESCRIPTION
 *  Registers the callback for the notifications, such as IP available
 *  notification, or WiFi passive disconnect notification. When the related
 *  things happens, notifications will be activated and the callback will be
 *  invoked, if the callback has been registered.
 *
 *  Use vm_wlan_deregister_notification_handler() to un-register the callback
 *  that was registered prior.
 * PARAMETERS
 *  type:      [IN] The type of notifications the callback will listen to.
 *  callback:  [IN] The callback to be registered.
 *  user_data: [IN] Addition user data for the callback.
 * RETURN VALUES
 *  It returns 0, if the registration is successful. It returns non-zero when
 *  the registration fails.
 * EXAMPLE
 *  <code>
 *  vm_wlan_register_notification_handler(VM_WLAN_NOTIFICATION_IP_AVAILABLE,
 *                                        ip_available_hdl, NULL);
 *
 *  // When IP is available, ip_available_hdl() will be called.
 *  void ip_available_hdl(void* user_data, vm_wlan_noti_struct* notification)
 *  {
 *    if (notification->type == VM_WLAN_NOTIFICATION_IP_AVAILABLE)
 *    {
 *      //...
 *    }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_register_notification_handler(VM_WLAN_NOTIFICATION_TYPE type,
        vm_wlan_notification_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_deregister_notification_handler
 * DESCRIPTION
 *  De-registers the callback for the notifications that was registered prior.
 *  The type of notifications, callback and user_data should be the same as the
 *  those used at the registration by vm_wlan_register_notification_handler().
 * PARAMETERS
 *  type:      [IN] The type of notifications the callback was listening to.
 *  callback:  [IN] The callback to be de-registered.
 *  user_data: [IN] Addition user data for the callback.
 * RETURN VALUES
 *  It returns 0, if the de-registration is successful. It returns non-zero when
 *  the de-registration fails.
 * EXAMPLE
 *  <code>
 *  vm_wlan_register_notification_handler(VM_WLAN_NOTIFICATION_IP_AVAILABLE,
                                          ip_available_hdl, NULL);
 *
 *  // De-registers the same callback and user_data.
 *  vm_wlan_deregister_notification_handler(VM_WLAN_NOTIFICATION_IP_AVAILABLE,
                                            ip_available_hdl, NULL);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_deregister_notification_handler(VMINT type,
        vm_wlan_notification_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_sniffer_on
 * DESCRIPTION
 *  The function is used to open the sniffer in WLAN STA mode.
 * PARAMETERS
 *  config:    [IN] The configuration parameter of sniffer status.
 *  callback:  [IN] The callback to be invoked by the sniffer.
 *  user_data: [IN] Addition user data.
 * RETURN VALUES
 *  VM_WLAN_ERROR_SUCCESS:       Success.
 *  VM_WLAN_ERROR_FAILED:        Failed.
 *  VM_WLAN_ERROR_INVALID_INPUT: Invalid input parameter(s).
 *  VM_WLAN_ERROR_PROCESSING:    Waiting for result.
 * EXAMPLE
 *  <code>
 *  vm_wlan_sniffer_on(NULL, sniffer_on_cb, NULL);
 *
 *  void sniffer_on_cb(void* user_data, vm_wlan_sniffer_on_result_t* res)
 *  {
 *      switch (res->cb_type)
 *      {
 *          case VM_WLAN_SNIFFER_ON_RES:
 *          {
 *              if (res->result == VM_WLAN_SNIFFER_SUCCESS)
 *              {
 *                  // Opens sniffer successfully.
 *              }
 *              break;
 *          }
 *          case VM_WLAN_SNIFFER_ON_AP_INFO:
 *          {
 *              if (res->result == VM_WLAN_SNIFFER_SUCCESS)
 *              {
 *                  // Gets AP information successfully. The AP information is
 *                  // stored in res->profile.
 *              }
 *              break;
 *          }
 *          default:
 *              break;
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_sniffer_on(const vm_wlan_sniffer_on_config_t* config,
                             vm_wlan_sniffer_on_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_sniffer_off
 * DESCRIPTION
 *  The function is used to close the sniffer in WLAN STA mode.
 * PARAMETERS
 *  callback:  [IN] The callback to be invoked by the sniffer.
 *  user_data: [IN] Addition user data.
 * RETURN VALUES
 *  VM_WLAN_ERROR_SUCCESS:       Success.
 *  VM_WLAN_ERROR_FAILED:        Failed.
 *  VM_WLAN_ERROR_INVALID_INPUT: Invalid input parameter(s).
 *  VM_WLAN_ERROR_PROCESSING:    Waiting for result.
 * EXAMPLE
 *  <code>
 *  vm_wlan_sniffer_off(sniffer_off_cb, NULL);
 *
 *  void sniffer_off_cb(void* user_data, VM_WLAN_SNIFFER_RESULT res)
 *  {
 *      if (VM_WLAN_SNIFFER_SUCCESS == res)
 *      {
 *          // Exits sniffer successfully.
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_sniffer_off(vm_wlan_sniffer_off_callback callback, void* user_data);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_roaming_set
 * DESCRIPTION
 *  Sets the roaming parameter.
 * PARAMETERS
 *  is_roaming: [IN] Specifies if roaming should be enabled or not. VM_TRUE means
 *                   to enable the roaming; VM_FALSE means to disable the roaming.
 *  callback:   [IN] The callback to be invoked by the WLAN layer.
 *  user_data:  [IN] Addition user data.
 * RETURN VALUES
 *  VM_WLAN_ERROR_SUCCESS:    Success.
 *  VM_WLAN_ERROR_FAILED:     Failed.
 *  VM_WLAN_ERROR_PROCESSING: Waiting for result.
 * EXAMPLE
 *  <code>
 *  VMBOOL enable_roaming = VM_TRUE;
 *  vm_wlan_roaming_set(enable_roaming, set_roaming_cb, NULL);
 *
 *  void set_roaming_cb(void* user_data, VM_WLAN_REQUEST_RESULT res)
 *  {
 *      if (VM_WLAN_REQUEST_DONE == res)
 *      {
 *          // Set roaming successfully.
 *      }
 *  }
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_roaming_set(VMBOOL is_roaming, vm_wlan_callback callback,
                              void* user_data);

/*****************************************************************************
 * FUNCTION
 *  vm_wlan_roaming_get
 * DESCRIPTION
 *  Gets roaming parameter.
 * PARAMETERS
 *  is_roaming     :   [OUT]    WLAN roaming parameter
 * RETURN VALUES
 *  VM_WLAN_ERROR_SUCCESS: Success.
 *  VM_WLAN_ERROR_FAILED:  Failed.
 * EXAMPLE
 *  <code>
 *  VMBOOL is_roaming = VM_TRUE;
 *  vm_wlan_roaming_get(&is_roaming);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_roaming_get(VMBOOL* is_roaming);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_set_parameter
 * DESCRIPTION
 *  Set the WLAN parameters under different modes. The caller should prepare the
 *  corresponding variables with proper types for parameter and parameter_type
 *  of the input parameters.
 *
 *  Currently, only VM_WLAN_MODE_AP and VM_WLAN_MODE_AP_STA are supported for
 *  setting up the SSID, Authorize Mode and Password.
 *
 *  For Authorize mode, only OPEN and WPA2_ONLY_PSK are supported. The parameter(s)
 *  need to be set before the AP is turned on.
 *
 *  The default values will be used if the parameters are not set or the setting
 *  operation failed.
 *  The default SSID is MTK_XXXXXX, where the value of XXXXXX is the last three bytes
 *  of mac address. The default authorize mode is WPA2_ONLY_PSK. The default
 *  password is 12345678.
 *
 * PARAMETERS
 *  mode:           [IN] Specifies the WLAN mode.
 *  parameter:      [IN] Specifies the parameter of vm_wlan_parameter_t type in
 *                       AP mode.
 *  parameter_type: [IN] Specifies what parameter to configure,
 *                       VM_WLAN_PARAMETER for AP mode.
 * RETURNS
 *  VM_WLAN_ERROR_SUCCESS:       Success.
 *  VM_WLAN_ERROR_FAILED:        Failed.
 *  VM_WLAN_ERROR_STATE_ERROR:   Tried to set the parameter in a wrong state,
 *                               such as trying to set AP parameter when AP is on.
 *  VM_WLAN_ERROR_INVALID_INPUT: Invalid input parameter(s).
 * EXAMPLE
 *  <code>
 *  // Sets AP parameter before turning on AP mode.
 *  vm_wlan_parameter_t ap_param = {0};
 *  ap_param.authorize_mode = VM_WLAN_AUTHORIZE_MODE_WPA2_ONLY_PSK;
 *  strcpy(ap_param.ssid, "SSIDEXAMPLE");
 *  strcpy(ap_param.pwd, "PWDEXAMPLE")
 *  vm_wlan_set_para(VM_WLAN_SET_MODE_AP, (void*)&ap_param, VM_WLAN_CFG_AP_PARA_ALL);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_set_parameter(VM_WLAN_MODE mode, void* parameter,
                                VM_WLAN_PARAMETER parameter_type);


/*****************************************************************************
 * FUNCTION
 *  vm_wlan_get_parameter
 * DESCRIPTION
 *  Gets the parameter under different modes. Currently, only VM_WLAN_MODE_AP and
 *  VM_WLAN_MODE_AP_STA are supported to get AP's SSID, Auth mode and Password.
 *
 *  The caller should prepare the corresponding variables with proper types for
 *  parameter.
 *  The caller should prepare varible with proper type for "parameter" input parameter.
 * PARAMETERS
 *  mode:      [IN]  Specifies the WLAN mode.
 *  parameter: [OUT] Containing the parameter upon the return of this function.
 *                   vm_wlan_parameter_t for VM_WLAN_MODE_AP or VM_WLAN_MODE_AP_STA.
 * RETURNS
 *  VM_WLAN_ERROR_SUCCESS:       Success.
 *  VM_WLAN_ERROR_FAILED:        Failed.
 *  VM_WLAN_ERROR_INVALID_INPUT: Invalid input parameter(s).
 * EXAMPLE
 *  <code>
 *  vm_wlan_parameter_t ap_param = {0};
 *  vm_wlan_get_parameter(VM_WLAN_MODE_AP, (void*)&ap_param);
 *  </code>
 *****************************************************************************/
VM_RESULT vm_wlan_get_parameter(VM_WLAN_MODE mode, void* parameter);




#ifdef __cplusplus
}
#endif

#endif /* VMWLAN_SDK_H */
