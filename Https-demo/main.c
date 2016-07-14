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
#include <string.h>

#include "vmtype.h" 
#include "vmboard.h"
#include "vmsystem.h"
#include "vmcmd.h" 
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_i2c.h"
#include "vmthread.h"

#include "vmhttps.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"
#include "vmwdt.h"
#include "vmpwr.h"
#include "vmlog.h"

#include "vmsock.h"
#include "vmbearer.h"


//-----------------------------------------------------------------------------
void _log_printf(int type, const char *file, int line, const char *msg, ...)
{
  //if ((retarget_target != retarget_device_handle) && (retarget_target != retarget_uart1_handle)) return;
/*
  va_list ap;
  char *pos, message[256];
  int sz;
  int nMessageLen = 0;

  memset(message, 0, 256);
  pos = message;

  sz = 0;
  va_start(ap, msg);
  nMessageLen = vsnprintf(pos, 256 - sz, msg, ap);
  va_end(ap);

  if( nMessageLen<=0 ) return;

  if (type == 1) printf("\n[FATAL]");
  else if (type == 2) printf("\n[ERROR]");
  else if (type == 3) printf("\n[WARNING]");
  else if (type == 4) printf("\n[INFO]");
  else if (type == 5) printf("\n[DEBUG]");
  if (type > 0) printf(" %s:%d ", file, line);
*/
  printf("%s\n", msg);
}


#define CUST_APN    "web.htgprs"      /* !!!=== The APN of your test SIM ===!!! */

#define USING_PROXY VM_FALSE          /* Whether your SIM uses proxy */
#define PROXY_ADDRESS   "10.0.0.172"  /* The proxy address */
#define PROXY_PORT  8080              /* The proxy port */

#define VMHTTPS_TEST_DELAY 60000      /* 60 seconds */
#define VMHTTPS_TEST_URL "https://amp.dhz.hr"
//#define VMHTTPS_TEST_URL "https://www.howsmyssl.com/a/check"

#define CONNECT_ADDRESS "82.196.4.208"
#define CONNECT_PORT 80
#define MAX_BUF_LEN 512

static VMUINT8 g_channel_id;
static VMINT g_read_seg_num;
static int cnt = 0;
static VM_BEARER_HANDLE g_bearer_hdl;
static VM_THREAD_HANDLE g_thread_handle;
static VMINT g_soc_sockname;
static VMINT g_soc_client;

extern void retarget_setup();

#define I2C_SCL_PIN_NAME	43
#define I2C_SDA_PIN_NAME	44

static VM_DCL_HANDLE g_i2c_handle = VM_DCL_HANDLE_INVALID;

// Lua: i2c.setup(address [, speed])
//=================================
static void i2c_setup(void)
{
    vm_dcl_i2c_control_config_t conf_data;
    int result;
    VMINT8 address = 0x76;
    VMUINT32 speed = 100; // default speed - 100kbps

	vm_log_debug("=== configuring i2c ===");
    if (g_i2c_handle == VM_DCL_HANDLE_INVALID) {
    	// SPI_MOSI gpio config
    	/*vm_log_debug("configuring i2c pins");
    	VM_DCL_HANDLE gpio_handle = vm_dcl_open(VM_DCL_GPIO, I2C_SCL_PIN_NAME);
        //if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
        //vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_1, NULL);
    	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
    	gpio_handle = vm_dcl_open(VM_DCL_GPIO, I2C_SDA_PIN_NAME);
        //if (gpio_handle == VM_DCL_HANDLE_INVALID) goto exit;
        //vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_1, NULL);
    	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);*/

    	//vm_log_debug("configuring SCL %d", vm_dcl_config_pin_mode(I2C_SCL_PIN_NAME, VM_DCL_PIN_MODE_I2C));
    	//vm_log_debug("configuring SDA %d", vm_dcl_config_pin_mode(I2C_SDA_PIN_NAME, VM_DCL_PIN_MODE_I2C));

    	vm_log_debug("opening i2c");
        g_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
        if (g_i2c_handle < 0) {
        	vm_log_error("error opening i2c %d", g_i2c_handle);
        	g_i2c_handle = VM_DCL_HANDLE_INVALID;
        }
        else {
        	vm_log_error("OK, handle = %d", g_i2c_handle);
        }
    }
    if (g_i2c_handle != VM_DCL_HANDLE_INVALID) {
        if (speed > 400) {
        	conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_HIGH_SPEED_MODE;
    		conf_data.fast_mode_speed = 0;
    		conf_data.high_mode_speed = speed;
        }
        else {
        	conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
    		conf_data.fast_mode_speed = 0;
    		conf_data.high_mode_speed = speed;
        }
		conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
		conf_data.get_handle_wait = 0;
		conf_data.reserved_1 = 0;
		conf_data.delay_length = 0;
		conf_data.slave_address = (address << 1);
    	vm_log_debug("configuring i2c %d", g_i2c_handle);
		result = vm_dcl_control(g_i2c_handle, VM_DCL_I2C_CMD_CONFIG, (void *)&conf_data);
    	vm_log_debug("result %d", result);

    }
}



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

#define REQUEST "GET http://loboris.eu/ HTTP/1.1\r\nHOST:loboris.eu\r\n\r\n"

//================================================================================================================

static VMINT32 soc_sub_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    SOCKADDR_IN addr_in = {0};
    char buf[MAX_BUF_LEN] = {0};
    int len = 0;
    int ret;

    g_soc_client = socket(PF_INET, SOCK_STREAM, 0);
    printf("Create socket: %d\n", g_soc_client);
    addr_in.sin_family = PF_INET;
    addr_in.sin_addr.S_un.s_addr = inet_addr(CONNECT_ADDRESS);
    addr_in.sin_port = htons(CONNECT_PORT);

    ret = connect(g_soc_client, (SOCKADDR*)&addr_in, sizeof(SOCKADDR));
    printf("Connect %d\n", ret);
    strcpy(buf, REQUEST);
    ret = send(g_soc_client, buf, strlen(REQUEST), 0);
    printf("Send &d\n", ret);
    ret = recv(g_soc_client, buf, MAX_BUF_LEN, 0);
    if(0 == ret)
    {
        printf("Received FIN from server\n");
    }
    else
    {
        printf("Received %d bytes data\n", ret);
    }
    buf[200] = 0;
    printf("First 200 bytes of the data:\n%s\n", buf);
    closesocket(g_soc_client);
    return 0;
}

static void bearer_callback(VM_BEARER_HANDLE handle, VM_BEARER_STATE event, VMUINT data_account_id, void *user_data)
{
    if (VM_BEARER_WOULDBLOCK == g_bearer_hdl)
    {
        g_bearer_hdl = handle;
    }
    if (handle == g_bearer_hdl)
    {
        switch (event)
        {
            case VM_BEARER_DEACTIVATED:
                break;
            case VM_BEARER_ACTIVATING:
                break;
            case VM_BEARER_ACTIVATED:
                g_thread_handle = vm_thread_create(soc_sub_thread, NULL, 0);

                break;
            case VM_BEARER_DEACTIVATING:
                break;
            default:
                break;
        }
    }
}


void start_doing(VM_TIMER_ID_NON_PRECISE tid, void* user_data)
{
    vm_timer_delete_non_precise(tid);
    g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, NULL, bearer_callback, VM_BEARER_IPV4);
}

//================================================================================================================

void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
    case VM_EVENT_CREATE:
        set_custom_apn();
        //vm_timer_create_non_precise(VMHTTPS_TEST_DELAY, https_send_request, NULL);
        //vm_timer_create_non_precise(20000, start_doing, NULL);
        i2c_setup();
        break;

    case VM_EVENT_QUIT:
        break;
    }
}

void vm_main(void) 
{
    retarget_setup();
    printf("\n=== TEST ===\n");

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
