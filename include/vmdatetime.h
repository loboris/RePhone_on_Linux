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

#ifndef VMDATETIME_SDK_H
#define VMDATETIME_SDK_H

#ifdef __cplusplus
extern "C" {
#endif/* __cplusplus */

#include "vmtype.h"

/* System time structure  */
typedef struct
{
    VMINT year;        /* Year, the value is from 1970. */
    VMINT month;       /* Month, the value is from 1 to 12. */
    VMINT day;	       /* Day, the value is from 1	to 31. */
    VMINT hour;        /* Hour, the value is from 0	to 23. */
    VMINT minute;	   /* Minute, the value is from 0 to 59. */
    VMINT second;	   /* Second, the value is from 0 to 59. */
} vm_date_time_t;


/*****************************************************************************
 * FUNCTION
 *    vm_time_set_date_time
 * DESCRIPTION
 *    Sets the system time.
 * PARAMETERS
 *    time: [IN] The time structure.
 * RETURNS
 *    If the return value is greater than or equal to 0, it means success; any
 *    other value means fail.
 * EXAMPLE
 * <code>
 *  ret_val = vm_time_set_date_time(&updated_time);
 * </code>
 *****************************************************************************/
VM_RESULT vm_time_set_date_time(vm_date_time_t* time);


/*****************************************************************************
 * FUNCTION
 *    vm_time_get_date_time
 * DESCRIPTION
 *    Gets the current system time.
 * PARAMETERS
 *    time: [OUT] The time structure.
 * RETURNS
 *    If the return value is greater than or equal to 0, it means success; any
 *    other value means fail.
 * EXAMPLE
 * <code>
 *  ret_val = vm_time_get_date_time(&current_time);
 * </code>
 *****************************************************************************/
VM_RESULT vm_time_get_date_time(vm_date_time_t* time);


/*****************************************************************************
 * FUNCTION
 *    vm_time_get_unix_time
 * DESCRIPTION
 *    Gets the time since the epoch in second.(00:00:00 UTC, January 1, 1970)
 * PARAMETERS
 *    rtc_time: [OUT] the RTC time(real-time clock) in seconds.
 * RETURNS
 *    If the return value is greater than or equal to 0, it means success; any
 *    other value means fail.
 * EXAMPLE
 * <code>
 *  ret_val = vm_time_get_unix_time(&rtc_time);
 * </code>
 *****************************************************************************/
VM_RESULT vm_time_get_unix_time(VMUINT* unix_time);


/* The count of microsecond timer */
typedef VMUINT32 VM_TIME_UST_COUNT;

/*****************************************************************************
 * FUNCTION
 *    vm_time_ust_get_count
 * DESCRIPTION
 *    Gets the count from the microsecond timer (ust), a hardware timer. This
 *    microsecond timer counts the microseconds passed from the time the system is
 *    powered on until now. It allows the caller to measure the durations in
 *    high precision. To calculate the duration between different timer counts,
 *    use vm_time_ust_get_duration(), instead of simply subtract values, because
 *    the timer counts may wrap around.
 * PARAMETERS
 *    void
 * RETURNS
 *    The microseconds from the system power on until now.
 * EXAMPLE
 * <code>
 *  start = vm_time_ust_get_count();
 * </code>
 *****************************************************************************/
VM_TIME_UST_COUNT vm_time_ust_get_count(void);


/*****************************************************************************
 * FUNCTION
 *    vm_time_ust_get_duration
 * DESCRIPTION
 *    Gets the duration from start_count to end_count, in microsecond. The caller
 *    should ensure that the end_count is equal or greater than the start_count.
 * PARAMETERS
 *    start_count: [IN] The count at the start.
 *    end_count:   [IN] The count at the end.
 * RETURNS
 *    The duration between start_count and end_count in microseconds.
 * EXAMPLE
 * <code>
 *  start_count = vm_time_ust_get_count();
 *  ...
 *  end_count = vm_time_ust_get_count();
 *  duration = vm_time_ust_get_duration(start_count, end_count);
 * </code>
 *****************************************************************************/
VMUINT32 vm_time_ust_get_duration(VM_TIME_UST_COUNT start_count,VM_TIME_UST_COUNT end_count);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*VMDATETIME_SDK_H*/
