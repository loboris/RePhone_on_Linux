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
 *   reg_base_mt6261.h
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   Definition for chipset register base and global configuration registers
 *
 * Author:
 * -------
 *   
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __REG_BASE_MT6261_H__
#define __REG_BASE_MT6261_H__
#define VERSION_base                (0xA0000000) /*VERSION_CTRL ( map to 0x8000_0000)                                 */
#define CONFIG_base                 (0xA0010000) /*Configuration Registers(Clock, Power Down, Version and Reset)      */
#define GPIO_base                   (0xA0020000) /*General Purpose Inputs/Outputs                                     */
#define RGU_base                    (0xA0030000) /*Reset Generation Unit                                              */
#define EMI_base                    (0xA0050000) /*External Memory Interface                                          */
#define CIRQ_base                   (0xA0060000) /*Interrupt Controller                                               */
#define DMA_base                    (0xA0070000) /*DMA Controller                                                     */
#define UART1_base                  (0xA0080000) /*UART 1                                                             */
#define UART2_base                  (0xA0090000) /*UART 2  															  */
#define UART3_base                  (0xA00A0000) /*UART 3 add for MT6261                                              */
#define BTIF_base                   (0xA00B0000) /*BTIF                                                               */
#define GPT_base                    (0xA00C0000) /*General Purpose Timer                                              */
#define KP_base                     (0xA00D0000) /*Keypad Scanner                                                     */
#define PWM_base                    (0xA00E0000) /*Pulse-Width Modulation Outputs                                     */
#define SIM_base                    (0xA00F0000) /*SIM Interface                                                      */
#define SIM2_base                   (0xA0100000) /*SIM2 Interface                                                     */
#define SEJ_base                    (0xA0110000) /*SEJ                                                                */
#define I2C_base                    (0xA0120000) /*I2C                                                                */
#define MSDC_base                   (0xA0130000) /*MS/SD Controller                                                   */
#define SFI_base                    (0xA0140000) /*Serial Flash                                                       */
#define MIXED_base                  (0xA0170000) /*Analog Chip Interface Controller (PLL, CLKSQ, FH, CLKSW and SIMLS) */
#define PLL_base                    MIXED_base   /*Analog Chip Interface Controller (PLL, CLKSQ, FH, CLKSW and SIMLS) */
#define MCU_TOPSM_base              (0xA0180000) /*TOPSM0                                                             */
#define EFUSE_base                  (0xA01C0000) /*EFUSE                                                              */
#define SPI_base                    (0xA01E0000) /*SPI                                                                */
#define OSTIMER_base                (0xA01F0000) /*OSTIMER                                                            */
#define ANALOG_MAP__base            (0xA0210000) /*Analog                                                             */
#define MCU_MBIST_base              (0xA0220000) /*mcu_mbist_confg                                                    */
#define FSPI_MAS_base               (0xA0260000) /*FMSYS FSPI_MAS                                                     */
#define MSDC2_base                  (0xA0270000) /*MS/SD Controller 2                                                 */
#define PWM2_base                   (0xA0280000) /*Pulse-Width Modulation Outputs                                     */
#define SPI_SLAVE_base              (0xA0290000) /*SPI slave*/
#define I2C_18V_base                (0xA02A0000) /*I2C_18V*/
//MMSYS
#define ROT_DMA_base                (0xA0400000) /*ROT DMA                                                            */
#define CRZ_base                    (0xA0410000) /*Resizer                                                            */
#define CAMERA_base                 (0xA0420000) /*Camera Interface                                                   */
#define CAM_base                    CAMERA_base  /*Camera Interface                                                   */
#define SCAM_base                   (0xA0430000) /*SCAM  */ 
#define G2D_base					(0xA0440000) /*G2D*/                                                             
#define LCD_base                    (0xA0450000) /*LCD                                                                */
#define MMSYS_MBIST_base            (0xA0460000) /*MMSYS_MBIST_CONFIG                                                 */
#define MM_COLOR_base               (0xA0470000) /*MM COLOR                                                           */
#define MMSYS_CONFIG_base           (0xA0480000) /*MMSYS Config                                                       */
//ARMSYS
#define ARM_CONFG_base              (0xA0500000) /*arm_confg          */
#define BOOT_ENG_base               (0xA0510000) /*boot engine        */
#define CDCMP_base                  (0xA0520000) /*code decompression */
#define L1_CACHE_base               (0xA0530000) /*l1_cache           */
#define MPU_base                    (0xA0540000) /*l1_cache           */
//Analog
#define PMU_base                    (0xA0700000) /*PMU mixedsys                                                     */
#define RTC_base                    (0xA0710000) /*Real Time Clock                                                  */
#define ABBSYS_base                 (0xA0720000) /*Analog baseband (ABB) controller                                 */
#define ANA_CFGSYS_base             (0xA0730000) /*Analog die (MT6100) Configuration Registers (Clock, Reset, etc.) */
#define PWM_2CH_base                (0xA0740000) /*Pulse-Width Modulation (2 channel)                               */
#define ACCDET_base                 (0xA0750000) /*ACCDET                                                           */
#define ADIE_CIRQ_base              (0xA0760000) /*Interrupt Controller (16-bit)                                    */
#define AUXADC_base                 (0xA0790000) /*Auxiliary ADC Unit                                               */
//APSYS Bridge
#define USB_base                    (0xA0900000) /*USB */
#define USB_SIFSLV_base             (0xA0910000) /*USB SIFSLV */
#define DMA_AHB_base                (0xA0920000) /*DMA */
//Multimedia System
//BT System
#define BT_CONFG_base               (0xA3300000) /*BTSYS */
#define BT_CIRQ_base                (0xA3310000) /*BTSYS */
#define BT_DMA_base                 (0xA3320000) /*BTSYS */
#define BT_BTIF_base                (0xA3330000) /*BTSYS */
#define BT_PKV_base                 (0xA3340000) /*BTSYS */
#define BT_TIM_base                 (0xA3350000) /*BTSYS */
#define BT_RF_base                  (0xA3360000) /*BTSYS */
#define BT_MODEM_base               (0xA3370000) /*BTSYS */
#define BT_DBGIF_base               (0xA3380000) /*BTSYS */
#define BT_MBIST_CONFG__base        (0xA3390000) /*BTSYS */
//MODEMSYS APB
#define IDMA_base                   (0x82000000) /*MD2GSYS */
#define DPRAM_CPU_base              (0x82200000) /*MD2GSYS */
#define AHB2DSPIO_base              (0x82800000) /*MD2GSYS */
#define MD2GCONFG_base              (0x82C00000) /*MD2GSYS */
#define MD2G_MBIST_CONFG_base       (0x82C10000) /*MD2GSYS MBIST Config */
#define APC_base                    (0x82C30000) /*MD2GSYS */
#define CSD_ACC_base                (0x82C70000) /*MD2GSYS */
#define SHARE_base                  (0x82CA0000) /*MD2GSYS */
#define IRDMA_base                  (0x82CB0000) /*MD2GSYS */
#define PATCH_base                  (0x82CC0000) /*MD2GSYS */
#define AFE_base                    (0x82CD0000) /*MD2GSYS */
#define BFE_base                    (0x82CE0000) /*MD2GSYS */
//MODEMSYS APB
#define MDCONFIG_base               (0x83000000) /*MODEMSYS_PERI */
#define MODEM_MBIST_CONFIG_base     (0x83008000) /*MODEMSYS_PERI */
#define MODEM2G_TOPSM_base          (0x83010000) /*MODEMSYS_PERI */
#define TDMA_base                   (0x83020000) /*MODEMSYS_PERI */
#define SHAREG2_base                (0x83030000) /*MODEMSYS_PERI */
#define DIVIDER_base                (0x83040000) /*MODEMSYS_PERI */
#define FCS_base                    (0x83050000) /*MODEMSYS_PERI */
#define GCU_base                    (0x83060000) /*MODEMSYS_PERI */
#define BSI_base                    (0x83070000) /*MODEMSYS_PERI */
#define BPI_base                    (0x83080000) /*MODEMSYS_PERI */



#define VERSION_SD_base                (0xA0000000) /*VERSION_CTRL ( map to 0x8000_0000)                                 */
#define CONFIG_SD_base                 (0xA0010000) /*Configuration Registers(Clock, Power Down, Version and Reset)      */
#define GPIO_SD_base                   (0xA0020000) /*General Purpose Inputs/Outputs                                     */
#define RGU_SD_base                    (0xA0030000) /*Reset Generation Unit                                              */
#define EMI_SD_base                    (0xA0050000) /*External Memory Interface                                          */
#define CIRQ_SD_base                   (0xA0060000) /*Interrupt Controller                                               */
#define DMA_SD_base                    (0xA0070000) /*DMA Controller                                                     */
#define UART1_SD_base                  (0xA0080000) /*UART 1                                                             */
#define UART2_SD_base                  (0xA0090000) /*UART 2                                                             */
#define UART3_SD_base                  (0xA00A0000) /*UART 3 add for MT6261                                              */
#define BTIF_SD_base                   (0xA00B0000) /*BTIF                                                               */
#define GPT_SD_base                    (0xA00C0000) /*General Purpose Timer                                              */
#define KP_SD_base                     (0xA00D0000) /*Keypad Scanner                                                     */
#define PWM_SD_base                    (0xA00E0000) /*Pulse-Width Modulation Outputs                                     */
#define SIM_SD_base                    (0xA00F0000) /*SIM Interface                                                      */
#define SIM2_SD_base                   (0xA0100000) /*SIM2 Interface                                                     */
#define SEJ_SD_base                    (0xA0110000) /*SEJ                                                                */
#define I2C_SD_base                    (0xA0120000) /*I2C                                                                */
#define MSDC_SD_base                   (0xA0130000) /*MS/SD Controller                                                   */
#define SFI_SD_base                    (0xA0140000) /*Serial Flash                                                       */
#define MIXED_SD_base                  (0xA0170000) /*Analog Chip Interface Controller (PLL, CLKSQ, FH, CLKSW and SIMLS) */
#define PLL_SD_base                    MIXED_SD_base   /*Analog Chip Interface Controller (PLL, CLKSQ, FH, CLKSW and SIMLS) */
#define MCU_TOPSM_SD_base              (0xA0180000) /*TOPSM0                                                             */
#define EFUSE_SD_base                  (0xA01C0000) /*EFUSE                                                              */
#define SPI_SD_base                    (0xA01E0000) /*SPI                                                                */
#define OSTIMER_SD_base                (0xA01F0000) /*OSTIMER                                                            */
#define ANALOG_MAP__SD_base            (0xA0210000) /*Analog                                                             */
#define MCU_MBIST_SD_base              (0xA0220000) /*mcu_mbist_confg                                                    */
#define FSPI_MAS_SD_base               (0xA0260000) /*FMSYS FSPI_MAS                                                     */
#define MSDC2_SD_base                  (0xA0270000) /*MS/SD Controller 2                                                 */
#define PWM2_SD_base                   (0xA0280000) /*Pulse-Width Modulation Outputs                                     */
#define SPI_SLAVE_SD_base              (0xA0290000) /*SPI slave*/
#define I2C_18V_SD_base	               (0xA02A0000) /*I2C_18V*/

//MMSYS
#define ROT_DMA_SD_base                (0xA0400000) /*ROT DMA                                                            */
#define CRZ_SD_base                    (0xA0410000) /*Resizer                                                            */
#define CAMERA_SD_base                 (0xA0420000) /*Camera Interface                                                   */
#define CAM_SD_base                    CAMERA_SD_base  /*Camera Interface                                                   */
#define SCAM_SD_base                   (0xA0430000) /*SCAM                                                               */
#define G2D_SD_base						(0xA0440000)/*G2D*/
#define LCD_SD_base                    (0xA0450000) /*LCD                                                                */
#define MMSYS_MBIST_SD_base            (0xA0460000) /*MMSYS_MBIST_CONFIG                                                 */
#define MM_COLOR_SD_base               (0xA0470000) /*MM COLOR                                                           */
#define MMSYS_CONFIG_SD_base           (0xA0480000) /*MMSYS Config                                                       */
//ARMSYS
#define ARM_CONFG_SD_base              (0xA0500000) /*arm_confg          */
#define BOOT_ENG_SD_base               (0xA0510000) /*boot engine        */
#define CDCMP_SD_base                  (0xA0520000) /*code decompression */
#define L1_CACHE_SD_base               (0xA0530000) /*l1_cache           */
#define MPU_SD_base                    (0xA0540000) /*l1_cache           */
//Analog
#define PMU_SD_base                    (0xA0700000) /*PMU mixedsys                                                     */
#define RTC_SD_base                    (0xA0710000) /*Real Time Clock                                                  */
#define ABBSYS_SD_base                 (0xA0720000) /*Analog baseband (ABB) controller                                 */
#define ANA_CFGSYS_SD_base             (0xA0730000) /*Analog die (MT6100) Configuration Registers (Clock, Reset, etc.) */
#define PWM_2CH_SD_base                (0xA0740000) /*Pulse-Width Modulation (2 channel)                               */
#define ACCDET_SD_base                 (0xA0750000) /*ACCDET                                                           */
#define ADIE_CIRQ_SD_base              (0xA0760000) /*Interrupt Controller (16-bit)                                    */
#define AUXADC_SD_base                 (0xA0790000) /*Auxiliary ADC Unit                                               */
//APSYS Bridge
#define USB_SD_base                    (0xA0900000) /*USB */
#define USB_SIFSLV_SD_base             (0xA0910000) /*USB SIFSLV */
#define DMA_AHB_SD_base                (0xA0920000) /*DMA */
//Multimedia System
//BT System
#define BT_CONFG_SD_base               (0xA3300000) /*BTSYS */
#define BT_CIRQ_SD_base                (0xA3310000) /*BTSYS */
#define BT_DMA_SD_base                 (0xA3320000) /*BTSYS */
#define BT_BTIF_SD_base                (0xA3330000) /*BTSYS */
#define BT_PKV_SD_base                 (0xA3340000) /*BTSYS */
#define BT_TIM_SD_base                 (0xA3350000) /*BTSYS */
#define BT_RF_SD_base                  (0xA3360000) /*BTSYS */
#define BT_MODEM_SD_base               (0xA3370000) /*BTSYS */
#define BT_DBGIF_SD_base               (0xA3380000) /*BTSYS */
#define BT_MBIST_CONFG__SD_base        (0xA3390000) /*BTSYS */
//MODEMSYS APB
#define IDMA_SD_base                   (0x82000000) /*MD2GSYS */
#define AHB2DSPIO_SD_base              (0x82800000) /*MD2GSYS */
#define MD2GCONFG_SD_base              (0x82C00000) /*MD2GSYS */
#define MD2G_MBIST_CONFG_SD_base       (0x82C10000) /*MD2GSYS MBIST Config */
#define APC_SD_base                    (0x82C30000) /*MD2GSYS */
#define CSD_ACC_SD_base                (0x82C70000) /*MD2GSYS */
#define SHARE_SD_base                  (0x82CA0000) /*MD2GSYS */
#define IRDMA_SD_base                  (0x82CB0000) /*MD2GSYS */
#define PATCH_SD_base                  (0x82CC0000) /*MD2GSYS */
#define AFE_SD_base                    (0x82CD0000) /*MD2GSYS */
#define BFE_SD_base                    (0x82CE0000) /*MD2GSYS */
//MODEMSYS APB
#define MDCONFIG_SD_base               (0x83000000) /*MODEMSYS_PERI */
#define MODEM_MBIST_CONFIG_SD_base     (0x83008000) /*MODEMSYS_PERI */
#define MODEM2G_TOPSM_SD_base          (0x83010000) /*MODEMSYS_PERI */
#define TDMA_SD_base                   (0x83020000) /*MODEMSYS_PERI */
#define SHAREG2_SD_base                (0x83030000) /*MODEMSYS_PERI */
#define DIVIDER_SD_base                (0x83040000) /*MODEMSYS_PERI */
#define FCS_SD_base                    (0x83050000) /*MODEMSYS_PERI */
#define GCU_SD_base                    (0x83060000) /*MODEMSYS_PERI */
#define BSI_SD_base                    (0x83070000) /*MODEMSYS_PERI */
#define BPI_SD_base                    (0x83080000) /*MODEMSYS_PERI */


#endif  /* __REG_BASE_MT6261_H__ */
