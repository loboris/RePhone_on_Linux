/*****************************************************************************
*  Copyright Statement:
*  ____________________
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2006
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS_IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
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

#ifndef VMDRV_TP_SDK_H
#define VMDRV_TP_SDK_H

#ifdef _cplusplus
extern "C" {
#endif

#include "vmtype.h"
typedef enum 
{
  VM_DRV_TP_FALSE,
  VM_DRV_TP_TRUE
}VM_DRV_TP_BOOL;

typedef enum {
	VM_DRV_TP_ACTIVE_MODE,
	VM_DRV_TP_IDLE_MODE,
	VM_DRV_TP_SLEEP_MODE,
	VM_DRV_TP_GESTURE_DETECTION_MODE,
	VM_DRV_TP_MULTIPLE_POINT_MODE,
	VM_DRV_TP_FIRMWARE_UPDATE,
	VM_DRV_TP_FM_ENABLE,
	VM_DRV_TP_FM_DISABLE
}VM_DRV_TP_DEVICE_MODE;

typedef enum {
	VM_DRV_TP_COMMAND_GET_VERSION,
	VM_DRV_TP_COMMAND_GET_CONFIG,
	VM_DRV_TP_COMMAND_LOAD_INT_CONFIG,
	VM_DRV_TP_COMMAND_LOAD_EXT_CONFIG,
	VM_DRV_TP_COMMAND_GET_DIFF_DATA,
	VM_DRV_TP_COMMAND_GET_FW_BUFFER,
	VM_DRV_TP_COMMAND_DO_FW_UPDATE
}VM_DRV_TP_CTRL_COMMAND;

typedef struct
{
   VMUINT16 event; 
   VMUINT16 x;  
   VMUINT16 y;
   VMUINT16 z;
}vm_drv_tp_single_event_t;

typedef struct
{
   VMUINT16   model; // Single/Dual/Triple/Four/Five/All gesture 
   VMUINT16   padding; //currently use for check the structure format correctness, 0xAA
   VMUINT32  time_stamp;
   vm_drv_tp_single_event_t points[5];
}vm_drv_tp_multiple_event_t;

typedef struct
{
	VMUINT16 resolution;
	VMUINT16 threshold;
	VMUINT16 report_interval;
	VMUINT16 idle_time_interval;
	VMUINT16 sleep_time_interval;
	VMUINT16 gesture_active_distance;
	VMUINT16 ms_calibration[128];
}vm_drv_tp_parameters_t;

typedef enum {
     VM_DRV_TP_UP,
     VM_DRV_TP_DOWN
}VM_DRV_TP_PEN_STATE;

#define VM_DRV_TP_PATTERN 0xAA
#define VM_DRV_TP_CONFIG_LENGTH 106

typedef struct{
	VM_DRV_TP_BOOL (*vm_drv_tp_init_ptr)(void);
	VM_DRV_TP_BOOL (*vm_drv_tp_set_device_mode_ptr)(VM_DRV_TP_DEVICE_MODE);
	VM_DRV_TP_PEN_STATE (*vm_drv_tp_hisr_ptr)(void); 
	VM_DRV_TP_BOOL (*vm_drv_tp_get_data_ptr)(vm_drv_tp_multiple_event_t *);
	VM_DRV_TP_BOOL (*vm_drv_tp_parameters_ptr)(vm_drv_tp_parameters_t *, VMUINT32, VMUINT32);
	void (*vm_drv_tp_power_on_ptr)(VM_DRV_TP_BOOL);
	VMUINT32 (*vm_drv_tp_command_ptr)(VMUINT32, void *, void *);
}vm_drv_tp_function_list_t;

VMINT vm_drv_tp_setup_driver(vm_drv_tp_function_list_t* func_list);
VMUINT32 vm_drv_tp_get_time_stamp(void);

#ifdef _cplusplus
}
#endif

#endif
