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

#ifndef VMPWR_SDK_H
#define VMPWR_SDK_H
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/
#include "vmtype.h"
#include "vmdatetime.h"

/*  scheduled startup option */
typedef enum
{
    VM_PWR_STARTUP_DISABLE = 0,                 /* Disable scheduled startup */
    VM_PWR_STARTUP_ENABLE_CHECK_S = 1,          /* Enable Check Second. */
    VM_PWR_STARTUP_ENABLE_CHECK_MS = 2,         /* Enable Check Minute, Second. */
    VM_PWR_STARTUP_ENABLE_CHECK_HMS = 3,        /* Enable Check Hour, Minute, Second.*/
    VM_PWR_STARTUP_ENABLE_CHECK_DHMS = 4, 	    /* Enable Check Day, Hour, Minute, Second */
    VM_PWR_STARTUP_ENABLE_CHECK_WDHMS = 5, 	    /* Enable Check Week Day, Day, Hour, Minute, Second. */
    VM_PWR_STARTUP_ENABLE_CHECK_MDHMS = 6, 	    /* Enable Check Month, Week Day, Day, Hour, Minute, Second*/
    VM_PWR_STARTUP_ENABLE_CHECK_YMDHMS = 7 	    /* Enable Check Year, Month, Week Day, Day, Hour, Minute, Second*/
} VM_PWR_STARTUP_OPTION;


/*****************************************************************************
 * FUNCTION
 *  vm_pwr_scheduled_startup
 * DESCRIPTION
 *  this interface will set device auto power on option, include time and check rules. the option will not save always, you need set it again
 *  after device reboot.
 *  example : if current time is 2015/1/1 12:12:12, and you set reboot time is  2015/1/2 12:12:12.
 *          1, Use check rule VM_PWR_STARTUP_ENABLE_CHECK_S, then power off,  device will auto power on 2015/1/1 12:13:12.
 *          2, Use check rule VM_PWR_STARTUP_ENABLE_CHECK_MS, then power off,  device will auto power on 2015/1/1 13:12:12.
 *          3, Use check rule VM_PWR_STARTUP_ENABLE_CHECK_YMDHMS, then power off,  device will auto power on 2015/1/2 12:12:12.
 * PARAMETERS
 *  time  : [IN] auto power on time
 *  option  : [IN] check rules, refer VM_PWR_STARTUP_OPTION
 * RETURNS
 *  TRUE : set success
 *  FALSE : failed
 *****************************************************************************/
VMBOOL vm_pwr_scheduled_startup(const vm_date_time_t* time, VM_PWR_STARTUP_OPTION option);



/*****************************************************************************
 * FUNCTION
 *  vm_pwr_get_battery_level
 * DESCRIPTION
 *  Get a calculated battery level. This battery level is calculated based on
 *  the battery voltage and then feed to an algorithm to ensure the level
 *  is monotonically decreasing.
 * PARAMETERS
 *  void
 * RETURNS
 *  Battery level in percentage.
 *  Depend on the target config, return value maybe one of (0,25,50,75,100)
 *  or one of (0,33,66,100).
 *****************************************************************************/
VMINT vm_pwr_get_battery_level(void);

/*****************************************************************************
 * FUNCTION
 *  vm_pwr_is_charging
 * DESCRIPTION
 *  Get the current battery charging status.
 * PARAMETERS
 *  void
 * RETURNS
 *  VM_TRUE if battery is charging; VM_FALSE if not.
 *****************************************************************************/
VMBOOL vm_pwr_is_charging(void);

/*****************************************************************************
 * FUNCTION
 *  vm_pwr_reboot
 * DESCRIPTION
 *  Reboot the system. The system will restart after normal shutdown flow,
 *  there is no need to press the power key. The applcation does not receive
 *  VM_EVENT_QUIT event. Therefore developers should ensure vital information
 *  are stored properly before calling this function.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void vm_pwr_reboot(void);

/*****************************************************************************
 * FUNCTION
 *  vm_pwr_shutdown
 * DESCRIPTION
 *  Shutdown the system. The shutdown flow will uninitialize the LinkIt runtime
 *  directly. The applcation does not receive VM_EVENT_QUIT event. Therefore
 *  developers should ensure vital information are stored properly before
 *  calling this function.
 * PARAMETERS
 *  debug_id  : [IN] This id will be printed to monitor tool during shutdown for
 *                   debugging purpose.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_pwr_shutdown(VMUINT16 debug_id);

/*****************************************************************************
 * FUNCTION
 *  vm_pwr_lcd_sleep_in
 * DESCRIPTION
 *  LCD module enter sleep mode.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void vm_pwr_lcd_sleep_in(void);

/*****************************************************************************
 * FUNCTION
 *  vm_pwr_lcd_sleep_out
 * DESCRIPTION
 *  LCD module leave sleep mode.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void vm_pwr_lcd_sleep_out(void);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*VMPWR_SDK_H*/