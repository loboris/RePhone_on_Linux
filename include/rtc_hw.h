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
 *    rtc_hw.h
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file is intends for RTC driver.
 *
 * Author:
 * -------
 * -------
 ****************************************************************************/
#ifndef _RTC_HW_H
#define _RTC_HW_H
#include "reg_base.h"

#define DRV_RTC_REG_COMM
#define DRV_RTC_CII_HALF_SEC
#define DRV_RTC_PWRON_BBPU_SW
#define DRV_RTC_INIT_POLL
#define DRV_RTC_LATCH_PWR_POLL
#define DRV_RTC_BBPU_AS_6260
#define DRV_RTC_XOSC_UPDATE
//#define DRV_RTC_XOSC_DEF             (0x7)
#define DRV_RTC_HW_CALI
#define DRV_RTC_PROTECT1
#define DRV_RTC_PROTECT2
#define DRV_RTC_LOW_POWER_DETECT
#define DRV_RTC_PDN_EXTEND
#define DRV_RTC_GPIO
#define DRV_RTC_REG_SPAR
#define DRV_RTC_REG_NEW_SPAR
#define DRV_RTC_INTERNAL_32K_AS_6261
#define DRV_RTC_LPD_AS_6261
#define DRV_RTC_EXTEND_TO_KEEP_RTC_TIME
#define DRV_RTC_SET_AUTO_AFTER_ALARM

#if !defined(DRV_RTC_OFF)
/*****************
 * RTC Registers *
 *****************/
#define RTC_BBPU        (RTC_base+0x0000) /*Baseband Power-up ctrl  */
#define RTC_IRQ_STATUS  (RTC_base+0x0004) /*IRQ Status              */
#define RTC_IRQ_EN      (RTC_base+0x0008) /*IRQ Enable              */
#define RTC_CII_EN      (RTC_base+0x000C) /*Counter increment IRQ   */
#define RTC_AL_MASK     (RTC_base+0x0010)/*Alarm mask control      */
#define RTC_TC_SEC      (RTC_base+0x0014)/*Second time counter     */
#define RTC_TC_MIN      (RTC_base+0x0018)/*Minute time counter     */
#define RTC_TC_HOU      (RTC_base+0x001C)/*Hour time counter       */
#define RTC_TC_DOM      (RTC_base+0x0020)/*Day of Mth time counter */
#define RTC_TC_DOW      (RTC_base+0x0024)/*Day of Wk time counter  */
#define RTC_TC_MTH      (RTC_base+0x0028)/*Month time counter      */
#define RTC_TC_YEA      (RTC_base+0x002C)/*Year time counter       */
#define RTC_AL_SEC      (RTC_base+0x0030)/*Second alarm            */
#define RTC_AL_MIN      (RTC_base+0x0034)/*Minute alarm            */
#define RTC_AL_HOU      (RTC_base+0x0038)/*Hour alarm              */
#define RTC_AL_DOM      (RTC_base+0x003C)/*Day of Month alarm      */
#define RTC_AL_DOW      (RTC_base+0x0040)/*Day of Week alarm       */
#define RTC_AL_MTH      (RTC_base+0x0044)/*Month alarm             */
#define RTC_AL_YEA      (RTC_base+0x0048)/*Year alarm              */
#define RTC_XOSCCAL     (RTC_base+0x004C)
#define RTC_POWERKEY1   (RTC_base+0x0050)
#define RTC_POWERKEY2   (RTC_base+0x0054)
#if defined(DRV_RTC_REG_COMM)
   #define RTC_INFO1    (RTC_base+0x0058)
   #define RTC_INFO2    (RTC_base+0x005c)
#if defined(DRV_RTC_W_FLAG)
   #define RTC_W_FLAG   (RTC_base+0x0060)   
#endif
#endif   /*DRV_RTC_REG_COMM*/

#define RTC_AL_RTC_AL_SEC_MASK      (0x003F)
#define RTC_AL_RTC_AL_MIN_MASK      (0x003F)
#define RTC_AL_RTC_AL_HOU_MASK      (0x001F)
#define RTC_AL_RTC_AL_DOM_MASK      (0x001F)
#define RTC_AL_RTC_AL_DOW_MASK      (0x0007)
#define RTC_AL_RTC_AL_MTH_MASK      (0x000F)
#define RTC_AL_RTC_AL_YEA_MASK      (0x007F)

#if defined(DRV_RTC_HW_CALI)
#define RTC_SPAR0       (RTC_base+0x0060)
#define RTC_SPAR1       (RTC_base+0x0064)
#define RTC_PROT        (RTC_base+0x0068)
#define RTC_DIFF        (RTC_base+0x006C)
#define RTC_CALI        (RTC_base+0x0070)
#define RTC_WRTGR       (RTC_base+0x0074)
#endif /* DRV_RTC_HW_CALI */

#define RTC_NEW_SPAR0               (RTC_AL_HOU)
#define RTC_NEW_SPAR1               (RTC_AL_DOM)
#define RTC_NEW_SPAR2               (RTC_AL_DOW)
#define RTC_NEW_SPAR3               (RTC_AL_MTH)
#define RTC_NEW_SPAR4               (RTC_AL_YEA)
#define RTC_NEW_SPAR_MASK           (0xFF00)
#define RTC_NEW_SPAR_OFFSET         (8)

#if defined(DRV_RTC_GPIO)
#define RTC_GPIO       (RTC_base+0x0078)
#endif /* DRV_RTC_GPIO */

//RTC_IRQ_STATUS
#define RTC_IRQ_STATUS_AL_STAT		0x0001
#define RTC_IRQ_STATUS_TC_STAT		0x0002
#if defined(DRV_RTC_LOW_POWER_DETECT)
#define RTC_IRQ_STATUS_LP_STAT		0x0008
#endif /* DRV_RTC_GPIO */

//RTC_IRQ_EN
#define	RTC_IRQ_EN_AL		   0x0001
#define	RTC_IRQ_EN_TC		   0x0002
#define	RTC_IRQ_EN_ONESHOT	   0x0004
#if defined(DRV_RTC_LOW_POWER_DETECT)
#define RTC_IRQ_EN_LPD         0x0008
#endif
#define	RTC_IRQ_EN_ALLOFF	   0x0000

//RTC_CII_EN
#define	RTC_CII_EN_SEC		0x0001
#define	RTC_CII_EN_MIN		0x0002
#define	RTC_CII_EN_HOU		0x0004
#define	RTC_CII_EN_DOM		0x0008
#define	RTC_CII_EN_DOW		0x0010
#define	RTC_CII_EN_MTH		0x0020
#define	RTC_CII_EN_YEA		0x0040
#define	RTC_CII_EN_ALLOFF	0x0000
#if defined(DRV_RTC_CII_HALF_SEC)
   #define	RTC_CII_EN_1_2S		0x0080
   #define	RTC_CII_EN_1_4S		0x0100
   #define	RTC_CII_EN_1_8S	   0x0200
#endif   /*DRV_RTC_CII_HALF_SEC*/

//RTC_AL_MASK, mask ==> 1 close intr, 0 open intr.
#define	RTC_AL_MASK_SEC		0x0001
#define	RTC_AL_MASK_MIN		0x0003
#define	RTC_AL_MASK_HOU		0x0007
#define	RTC_AL_MASK_DOM		0x000f
#define	RTC_AL_MASK_DOW		0x0017
#define	RTC_AL_MASK_MTH		0x002f
#define	RTC_AL_MASK_YEA		0x006f
#define	RTC_AL_MASK_ALLOFF	0x0000
#define RTC_AL_MASK_NORMAL	   (RTC_AL_MASK_HOU | RTC_AL_MASK_MIN)

//RTC_POWERKEY
#define RTC_POWERKEY1_KEY	0xa357
#define RTC_POWERKEY2_KEY	0x67d2

#define RTC_PROTECT1 0x586a
#define RTC_PROTECT2 0x9136

#if defined(DRV_RTC_INFO_MASK)
   #define RTC_INFO1_RESETDTIME  0x000f
   #define RTC_INFO1_INFO_MASK   0x00f0
   #define RTC_INFO2_INFO_MASK   0x00ff
#endif   /*DRV_RTC_INFO_MASK*/

#if defined(DRV_RTC_REG_COMM)
#if !defined(DRV_RTC_PDN_EXTEND)
#define RTC_PDN1_MASK 0x00f1
#endif /*!defined(DRV_RTC_PDN_EXTEND)*/
#endif   /*DRV_RTC_REG_COMM*/


#if defined(DRV_RTC_W_FLAG)

#define  RTC_POWERKEY_BUSY  0x3
#define  RTC_BBPU_BUSY  0x4
#define  RTC_TIME_BUSY  0x8000

#endif

#if defined(DRV_RTC_HW_CALI)
#define RTC_DIFF_MASK   0x0fff
#define RTC_CALI_MASK   0x007f
#define RTC_WRTGR_WRTGR 0x0001
#endif

#if defined(DRV_RTC_INTERNAL_32K_AS_6255)
#define RTC_GPIO_VBAT_LPSTA_RAW     0x0001
#define RTC_GPIO_EMBCK_SWITCH_FAIL  0x0002
#elif defined(DRV_RTC_INTERNAL_32K_AS_6261)
#define RTC_GPIO_VBAT_LPSTA_RAW         0x0001

#define RTC_DIFF_CALI_RD_SEL_MASK       0x8000
#define RTC_DIFF_CALI_RD_SEL_NORMAL     0x0000
#define RTC_DIFF_CALI_RD_SEL_EOSC32     0x8000

#define RTC_CALI_K_EOSC32_OVERFLOW_MASK     0x8000
#define RTC_CALI_K_EOSC32_OVERFLOW_ABSENT   0x0000
#define RTC_CALI_K_EOSC32_OVERFLOW_PRESENT  0x8000

#define RTC_CALI_EOSC32_CALI_WEN_MASK   0x4000
#define RTC_CALI_EOSC32_CALI_WEN_NORMAL 0x0000
#define RTC_CALI_EOSC32_CALI_WEN_EOSC32 0x4000
#endif

#if defined(DRV_RTC_GPIO)
#if defined(DRV_RTC_LPD_AS_6261)
#define RTC_CON_POWEROFF_SEQ_EN 0x1000

#define RTC_GPIO_LPD_OPT_MASK   0x0010
#define RTC_GPIO_LPD_OPT_XOSC32 0x0000
#define RTC_GPIO_LPD_OPT_EOSC32 0x0010

#define RTC_GPIO_XOSC32_LPEN    0x0002
#define RTC_GPIO_EOSC32_LPEN    0x0004
#define RTC_GPIO_LPEN           (RTC_CON_POWEROFF_SEQ_EN|RTC_GPIO_LPD_OPT_EOSC32|RTC_GPIO_XOSC32_LPEN|RTC_GPIO_EOSC32_LPEN)
#else
#define RTC_GPIO_LPEN 			0x0004
#endif
#define RTC_GPIO_LPRST			0x0008
#define RTC_GPIO_CDBO			0x0010
#define RTC_GPIO_F32KOB		    0x0020
#define RTC_GPIO_GPO			0x0040
#define RTC_GPIO_GOE			0x0080
#define RTC_GPIO_GSR			0x0100
#define RTC_GPIO_GSMT			0x0200
#define RTC_GPIO_GPEN			0x0400
#define RTC_GPIO_GPU			0x0800
#define RTC_GPIO_GE4			0x1000
#define RTC_GPIO_GE8			0x2000
#define RTC_GPIO_GPI			0x4000
#define RTC_GPIO_LPSTA_RAW	0x8000
#endif

#endif /*!defined(DRV_RTC_OFF)*/

#endif   /*_RTC_HW_H*/

