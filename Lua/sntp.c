/**
******************************************************************************
* @file    sntp.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   Create a NTP client thread, and synchronize RTC with NTP server.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/
#include <time.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#include "sntp.h"
#include "vmtype.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmudp.h"
#include "vmdatetime.h"
#include "vmtimer.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"
#include "shell.h"

extern VM_BEARER_DATA_ACCOUNT_TYPE gprs_bearer_type;

#define NTP_PACKET_SIZE 48
#define NTP_TIME_OUT 60

static VM_UDP_HANDLE g_udp;
vm_soc_address_t g_address = {4, 123, {132, 163, 4, 101}};
VMUINT8 g_packetBuffer[NTP_PACKET_SIZE];

static VM_TIMER_ID_PRECISE sntp_timer_id = -1;

static VMUINT8 ntp_time_set = 0;
static VMUINT8 ntp_can_write = 0;
static VMUINT8 ntp_showlog = 0;
static int ntp_time_zone = 0;
static int ntp_timeout = 0;
int ntp_cb_ref = LUA_NOREF;
static cb_func_param_int_t ntp_cb_params;

struct NtpPacket
{
	int flags;
	int stratum;
	int poll;
	int precision;
	int root_delay;
	int root_dispersion;
	int referenceID;
	int ref_ts_sec;
	int ref_ts_frac;
	int origin_ts_sec;
	int origin_ts_frac;
	int recv_ts_sec;
	int recv_ts_frac;
	int trans_ts_sec;
	int trans_ts_frac;
};

//----------------------------------------------------------------------------
static void sntp_timer_callback(VM_TIMER_ID_PRECISE timer_id, void* user_data)
{
	VMINT nwrite;

	if (ntp_time_set == 0) {
		//printf("waiting for ntp...\n");
		ntp_timeout++;
		if (ntp_timeout > NTP_TIME_OUT) {
			ntp_timeout = 0;
    		ntp_can_write = 0;
			vm_udp_close(g_udp);

        	if (ntp_cb_ref != LUA_NOREF) {
        		ntp_cb_params.par = -1;
        		ntp_cb_params.cb_ref = ntp_cb_ref;
                remote_lua_call(CB_FUNC_INT, &ntp_cb_params);
    			ntp_cb_ref = LUA_NOREF;
        	}
        	else if (ntp_showlog) {
        		vm_log_error("NTP ERROR, time not set.");
        	}
			vm_timer_delete_precise(sntp_timer_id);
			sntp_timer_id = -1;
		}
		else {
        	if (ntp_can_write) {
        		ntp_can_write = 0;
				//printf("ntp request\n");
				if (vm_udp_send(g_udp, g_packetBuffer, NTP_PACKET_SIZE, &g_address) < 0) {
					if (ntp_showlog) {
						vm_log_error("NTP write error: %d", nwrite);
					}
	        		ntp_can_write = 1;
				}
        	}
		}
	}
	else {
		vm_timer_delete_precise(sntp_timer_id);
		sntp_timer_id = -1;
	}
}


//-----------------------------------------------------------------
static void sntp_callback(VM_UDP_HANDLE handle, VM_UDP_EVENT event)
{
    VMCHAR buf[100] = {0};
    VMINT ret = 0;
    VMINT nwrite;
    struct tm *ntpTime;
    int contry = 0;
    vm_date_time_t new_time;

    switch (event)
    {
        case VM_UDP_EVENT_READ:
        	ntp_can_write = 0;
        	//printf("NTP read\n");
            ret = vm_udp_receive(g_udp, &buf, 100, &g_address);
            if (ret > 0) {
                vm_timer_delete_precise(sntp_timer_id);
    			sntp_timer_id = -1;
            	//printf("received %d\n", ret);
                //the timestamp starts at byte 40 of the received packet and is four bytes,
                // or two words, long. First, esxtract the two words:
                //unsigned long highWord = (buf[32] * 256) + buf[33];
                //unsigned long lowWord = (buf[34] * 256) + buf[35];
                // combine the four bytes (two words) into a long integer
                // this is NTP time (seconds since Jan 1 1900):
                //unsigned long secsSince1900 = (highWord << 16) | lowWord;
                unsigned long timestamp;
                memcpy(&timestamp, buf+32, sizeof(unsigned long));
                unsigned long secsSince1900 = ntohl(timestamp);
                // now convert NTP time into everyday time:
                // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
                // subtract seventy years:
                const unsigned long seventyYears = 2208988800UL;
                unsigned long epoch = secsSince1900 - seventyYears + (ntp_time_zone*3600);
                // print Unix time:

                ntpTime = gmtime(&epoch);
                new_time.day = ntpTime->tm_mday;
                new_time.hour = ntpTime->tm_hour;
                new_time.minute = ntpTime->tm_min;
                new_time.second = ntpTime->tm_sec;
                new_time.month = ntpTime->tm_mon + 1;
                new_time.year = ntpTime->tm_year + 1900;

                vm_time_set_date_time(&new_time);
                ntp_time_set = 1;
            	if (ntp_cb_ref != LUA_NOREF) {
            		ntp_cb_params.par = 0;
            		ntp_cb_params.cb_ref = ntp_cb_ref;
                    remote_lua_call(CB_FUNC_INT, &ntp_cb_params);
        			ntp_cb_ref = LUA_NOREF;
            	}
            	else if (ntp_showlog) {
                    vm_log_debug("Time Synchronized: tz=%d epoch=%lu: %s", ntp_time_zone, epoch, asctime(ntpTime));
            	}
                vm_udp_close(g_udp);
            }
            else {
            	//printf("no response\n");
            	ntp_can_write = 1;
            }
            break;

        case VM_UDP_EVENT_WRITE:
        	//printf("NTP write\n");
        	ntp_can_write = 1;
            break;

        default:
        	//printf("NTP event %d\n", event);
            break;
    }

}

//----------------------------------------
void sntp_gettime(int tz, uint8_t showlog)
{
  ntp_time_set = 0;
  ntp_time_zone = tz;
  ntp_timeout = 0;
  ntp_can_write = 0;
  ntp_showlog = showlog;

  memset(g_packetBuffer, 0, NTP_PACKET_SIZE);
  g_packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  /*g_packetBuffer[1] = 0;     // Stratum, or type of clock
  g_packetBuffer[2] = 6;     // Polling Interval
  g_packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  g_packetBuffer[12]  = 49;
  g_packetBuffer[13]  = 0x4E;
  g_packetBuffer[14]  = 49;
  g_packetBuffer[15]  = 52;*/

  if (sntp_timer_id >= 0) {
      if (ntp_showlog) {
    	  vm_log_debug("ntp request already running");
      }
      return;
  }

  sntp_timer_id = vm_timer_create_precise(1000, sntp_timer_callback, NULL);
  if (sntp_timer_id < 0) {
      if (ntp_showlog) {
    	  vm_log_error("Error creating ntp timer: %d", sntp_timer_id);
      }
  }

  g_udp = vm_udp_create(1000, gprs_bearer_type, sntp_callback, 0);
  if (g_udp < 0) {
      vm_timer_delete_precise(sntp_timer_id);
	  sntp_timer_id = -1;
      if (ntp_showlog) {
    	  vm_log_error("Error creating udp socket: %d", g_udp);
      }
	  return;
  }

  vm_udp_send(g_udp, g_packetBuffer, NTP_PACKET_SIZE, &g_address);
}

