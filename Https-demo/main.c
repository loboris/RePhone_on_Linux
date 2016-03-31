/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This sample Connect securely to https://www.howsmyssl.com/a/check and retrieve resulting and print to vm_log
  
  It calls the API vm_https_register_context_and_callback() to register the callback functions,
  then set the channel by vm_https_set_channel(), after the channel is established,
  it will send out the request by vm_https_send_request() and read the response by
  vm_https_read_content().

  You can change the url by modify macro VMHTTPS_TEST_URL.
  Before run this example, please set the APN information first by modify macros.
*/
#include <string.h>
#include <stdio.h>

#include "vmtype.h" 
#include "vmboard.h"
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmcmd.h" 
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmthread.h"

#include "vmhttps.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"
#include "vmwdt.h"
#include "vmpwr.h"


#define CUST_APN    "web.htgprs"      /* !!!=== The APN of your test SIM ===!!! */

#define USING_PROXY VM_FALSE          /* Whether your SIM uses proxy */
#define PROXY_ADDRESS   "10.0.0.172"  /* The proxy address */
#define PROXY_PORT  8080              /* The proxy port */

#define VMHTTPS_TEST_DELAY 60000      /* 60 seconds */
#define VMHTTPS_TEST_URL "https://amp.dhz.hr"
//#define VMHTTPS_TEST_URL "https://www.howsmyssl.com/a/check"

static VMUINT8 g_channel_id;
static VMINT g_read_seg_num;
//static VM_WDT_HANDLE sys_wdt_id = -1;
//static VM_TIMER_ID_PRECISE sys_timer_id = 0;
static int cnt = 0;

extern void retarget_setup();

static void https_send_request_set_channel_rsp_cb(VMUINT32 req_id, VMUINT8 channel_id, VMUINT8 result)
{
    VMINT ret = -1;

    
    ret = vm_https_send_request(
        0,                           /* Request ID */
        VM_HTTPS_METHOD_GET,         /* HTTP Method Constant */
        VM_HTTPS_OPTION_NO_CACHE,    /* HTTP request options */
        VM_HTTPS_DATA_TYPE_BUFFER,   /* Reply type (wps_data_type_enum) */
        100,                     	 /* bytes of data to be sent in reply at a time.
                                        If data is more that this, multiple response would be there */
        (VMUINT8 *)VMHTTPS_TEST_URL, /* The request URL */
        strlen(VMHTTPS_TEST_URL),    /* The request URL length */
        NULL,                        /* The request header */
        0,                           /* The request header length */
        NULL,
        0);

    printf("[HTTPS] callback - send request: %d\n", ret);
    if (ret != 0) {
        vm_https_unset_channel(channel_id);
    }
}

static void https_unset_channel_rsp_cb(VMUINT8 channel_id, VMUINT8 result)
{
    printf("[HTTPS] https_unset_channel_rsp_cb()\n");
}
static void https_send_release_all_req_rsp_cb(VMUINT8 result)
{
    printf("[HTTPS] https_send_release_all_req_rsp_cb()\n");
}
static void https_send_termination_ind_cb(void)
{
    printf("[HTTPS] https_send_termination_ind_cb()\n");
}
static void https_send_read_request_rsp_cb(VMUINT16 request_id, VMUINT8 result, 
                                         VMUINT16 status, VMINT32 cause, VMUINT8 protocol, 
                                         VMUINT32 content_length,VMBOOL more,
                                         VMUINT8 *content_type, VMUINT8 content_type_len,  
                                         VMUINT8 *new_url, VMUINT32 new_url_len,
                                         VMUINT8 *reply_header, VMUINT32 reply_header_len,  
                                         VMUINT8 *reply_segment, VMUINT32 reply_segment_len)
{
    VMINT ret = -1;
    char rply[102];
    memset(rply, 0x00, 102);
    strncpy(rply, reply_segment, reply_segment_len);
    printf("[HTTPS] https_send_request_rsp_cb()\n");
    printf("[HTTPS]      protocol: %d\n", protocol);
    printf("[HTTPS]  content type: %s\n", content_type);
    printf("[HTTPS]       new url: %s\n", new_url);
    printf("[HTTPS]        header: \n-----------------\n%s\n-----------------\n", reply_header);
    printf("[HTTPS] reply_content: %s\n", rply);
    printf("[HTTPS] === https_send_request_rsp_cb() ===================\n\n");
    if (result != 0) {
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
    }
    else {
        
        ret = vm_https_read_content(request_id, ++g_read_seg_num, 100);
        if (ret != 0) {
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
        }
    }
}
static void https_send_read_read_content_rsp_cb(VMUINT16 request_id, VMUINT8 seq_num, 
                                                 VMUINT8 result, VMBOOL more, 
                                                 VMUINT8 *reply_segment, VMUINT32 reply_segment_len)
{
    VMINT ret = -1;
    char rply[102];
    memset(rply, 0x00, 102);
    strncpy(rply, reply_segment, reply_segment_len);
    printf("[HTTPS] reply_content: n%s\n", rply);
    if (more > 0) {
        printf("[HTTPS] has more\n");
        ret = vm_https_read_content(
            request_id,               /* Request ID */
            ++g_read_seg_num,         /* Sequence number (for debug purpose) */
            100);                     /* The suggested segment data length of replied data in the peer buffer of
                                         response. 0 means use reply_segment_len in MSG_ID_WPS_HTTP_REQ or
                                         read_segment_length in previous request. */
        if (ret != 0) {
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
        }
    }
    else {
        printf("[HTTPS] no more\n");
        /* don't want to send more requests, so unset channel */
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
        g_channel_id = 0;
        g_read_seg_num = 0;

    }
}
static void https_send_cancel_rsp_cb(VMUINT16 request_id, VMUINT8 result)
{
    printf("[HTTPS] https_send_cancel_rsp_cb()\n");
}
static void https_send_status_query_rsp_cb(VMUINT8 status)
{
    printf("[HTTPS] https_send_status_query_rsp_cb()\n");
}

void set_custom_apn(void)
{
    vm_gsm_gprs_apn_info_t apn_info;

    memset(&apn_info, 0, sizeof(apn_info));
    strcpy(apn_info.apn, CUST_APN);
    strcpy(apn_info.proxy_address, PROXY_ADDRESS);
    apn_info.proxy_port = PROXY_PORT;
    apn_info.using_proxy = USING_PROXY;
    vm_gsm_gprs_set_customized_apn_info(&apn_info);
}


static void https_send_request(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
     /*----------------------------------------------------------------*/
     /* Local Variables                                                */
     /*----------------------------------------------------------------*/
     VMINT ret = -1;
     //VMINT apn = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN;
     VMINT apn = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN;
     vm_https_callbacks_t callbacks = {
    	(vm_https_set_channel_response_callback)https_send_request_set_channel_rsp_cb,
     	(vm_https_unset_channel_response_callback)https_unset_channel_rsp_cb,
 		(vm_https_release_all_request_response_callback)https_send_release_all_req_rsp_cb,
 		(vm_https_termination_callback)https_send_termination_ind_cb,
        (vm_https_send_response_callback)https_send_read_request_rsp_cb,
 		(vm_https_read_content_response_callback)https_send_read_read_content_rsp_cb,
        (vm_https_cancel_response_callback)https_send_cancel_rsp_cb,
		(vm_https_status_query_response_callback)https_send_status_query_rsp_cb
     };
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    cnt++;
	printf("\n[HTTPS] ** http send request [%d]**\n", cnt);
	//vm_timer_delete_non_precise(timer_id);
	ret = vm_https_register_context_and_callback(apn, &callbacks);

	if (ret != 0) {
		printf("ERROR %d", ret);
		return;
	}

	/* set network profile information */
	ret = vm_https_set_channel(
		0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0,
		0, 0
	);
}

/*
//-------------------------------------------------------------------------------
static void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
	  vm_wdt_reset(sys_wdt_id);
}
*/

void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
    case VM_EVENT_CREATE:
    	//sys_wdt_id = vm_wdt_start(2000);
        //sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
        set_custom_apn();
        vm_timer_create_non_precise(VMHTTPS_TEST_DELAY, https_send_request, NULL);
        break;

    case VM_EVENT_QUIT:
        break;
    }
}

void vm_main(void) 
{
    retarget_setup();
    printf("\n=== https example ===\n");
    
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}

