/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *    wdt_sw.h
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file is intends for WDT driver.
 *
 * Author:
 * -------
 * -------
 ****************************************************************************/
#ifndef WDT_SW_H
#define WDT_SW_H

//#include "drv_features_wdt.h"
//#include "drv_comm.h"
#include "wdt_hw.h"
//#include "kal_non_specific_general_types.h"

typedef enum{
	
	HIBERNATION_EXCEPTION = 1,	 /* WDT RST is caused by hibernation exception happened */
	TINY_SYSTEM_HANG = 2,       /*WDT RST is caused by tiny system hang  */
	AP_TIMEOUT_RST = 3,       /*WDT RST is caused by  AP  system hang  */
	AP_SW_RST = 4,            /* WDT RST is caused by  AP  systen software reset */
    FIRST_BOOT_UP = 5,        /* WDT RST should not be once  happend  when fisrt boot up */
	AP_SW_AND_TIMEOUT_RST=6,   /*WDT  RST is caused by AP software and timeout reset*/
	COLD_BOOT=7,                  /*WDT RST is caused by hibernation to ap resume */
	WARM_BOOT=8,                 /* JUMP TO MAUI INI by hibernation to ap resume */
	SUSPEND_RESUME=9,           /*SUSPNED STATUS */
	INVLIAD_BOOT_UP = 0xFF	  /* WDT RST shoud not be happened */
}BOOT_UP_SOURCE;




/*Must include "drv_comm.h*/
#define WDT_RSTINTERVAL_VALUE    0xffa
#define WDT_Restart()  DRV_WriteReg(WDT_RESTART,WDT_RESTART_KEY)
/*
void WDT_SetValue(kal_uint16 value);
void WDT_Enable(kal_bool en);
void WDT_EnableInterrupt(kal_bool enable);
void WDT_SetExtpol(IO_level extpol);
void WDT_SetExten(kal_bool en);
void WDT_Config(IO_level extpol, kal_bool exten);
void WDT_Enable_Debug_Mode(kal_bool en);
void WDT_init(void);
void DRV_RESET(void);
void DRV_ABN_RESET(void);
void WDT_Restart2(void);
extern BOOT_UP_SOURCE	AP_check_boot_up_source();
extern kal_bool  Is_warm_boot_before_region_init();
extern kal_bool  Is_first_bootup_or_wdt_reset();
#if defined(DRV_WDT_RETN_REG)
void WDT_Write_RETN_FLAG(kal_uint8 flag);
void WDT_SET_RETN_FLAG(kal_uint8 flag);
void WDT_CLR_RETN_FLAG(kal_uint8 flag);
kal_uint8 WDT_Read_RETN_FLAG(void);
void WDT_Write_RETN_DAT0(kal_uint32 value);
kal_uint32 WDT_Read_RETN_DAT0(void);
#endif
*/


#define DRV_WDT_WriteReg(addr,data)              DRV_WriteReg(addr,data)
#define DRV_WDT_Reg(addr)                        DRV_Reg(addr)                      
#define DRV_WDT_WriteReg32(addr,data)            DRV_WriteReg32(addr,data)          
#define DRV_WDT_Reg32(addr)                      DRV_Reg32(addr)                    
#define DRV_WDT_WriteReg8(addr,data)             DRV_WriteReg8(addr,data)           
#define DRV_WDT_Reg8(addr)                       DRV_Reg8(addr)                     
#define DRV_WDT_ClearBits(addr,data)             DRV_ClearBits(addr,data)           
#define DRV_WDT_SetBits(addr,data)               DRV_SetBits(addr,data)             
#define DRV_WDT_SetData(addr, bitmask, value)    DRV_SetData(addr, bitmask, value)  
#define DRV_WDT_ClearBits32(addr,data)           DRV_ClearBits32(addr,data)         
#define DRV_WDT_SetBits32(addr,data)             DRV_SetBits32(addr,data)           
#define DRV_WDT_SetData32(addr, bitmask, value)  DRV_SetData32(addr, bitmask, value)
#define DRV_WDT_ClearBits8(addr,data)            DRV_ClearBits8(addr,data)          
#define DRV_WDT_SetBits8(addr,data)              DRV_SetBits8(addr,data)            
#define DRV_WDT_SetData8(addr, bitmask, value)   DRV_SetData8(addr, bitmask, value) 

#endif

