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

#ifndef VMDCL_ADC_SDK_H
#define VMDCL_ADC_SDK_H

#include "vmdcl.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * This enum type represents the device of ADC (Analog Digital Convert) module.
 * The vm_dcl_open() uses the enum values to create a handle to access the
 * device. Applications should use the DCL (Driver Common Layer) APIs to
 * control the ACD module. Different analog input pins are defined in
 * VM_DCL_ADC_CHANNEL.
 *****************************************************************************/
typedef enum
{
    VM_DCL_ADC_START = VM_DCL_ADC_GROUP_START,
    VM_DCL_ADC   /* The ADC module. */
} VM_DCL_ADC_DEVICE;


/* The control commands of ADC module. */
typedef enum
{
    VM_DCL_ADC_COMMAND_CREATE_OBJECT = 0x901,   /* Get the logic channel. */
    VM_DCL_ADC_COMMAND_MODIFY_PARAMETER,        /* Set the measure parameters. */
    VM_DCL_ADC_COMMAND_SEND_START,              /* Start to measure. */
    VM_DCL_ADC_COMMAND_SEND_STOP,               /* Stop measure. */
    VM_DCL_ADC_COMMAND_GET_CHANNEL              /* Get the physical channel. */
} VM_DCL_ADC_CONTROL_COMMAND;


/* ADC module events. */
typedef enum
{
    /* The measurement result of a logical channel. The result parameter are
       stored in type vm_dcl_adc_measure_done_confirm_t. */
    VM_DCL_ADC_GET_RESULT = VM_DCL_ADC_EVENTS_START
} VM_DCL_ADC_EVENT;

/* The structure of ADC control command VM_DCL_ADC_COMMAND_CREATE_OBJECT. */
typedef struct
{
    VMUINT32 period;				        /* Measurement period, the uint is in
                                         tick. */
    VMUINT32 sap_id;                   /* Reserved. */
    VM_DCL_OWNER_ID owner_id;          /* Use vm_dcl_get_owner_id to retrieve
                                         owner id. */
    VMUINT8 channel;			           /* The physical ADC channel to be
                                      measured. Channels are defined in
                                      VM_DCL_ADC_CHANNEL. Use
                                      the macro PIN2CHANNEL to
                                      convert pin names, e.g. A0 and A1,
                                      to the corresponding
                                      physical channel id. */
    VMUINT8 evaluate_count;		        /* Number of measurement to take. */
    VMBOOL send_message_primitive;	  /* Set to VM_TRUE to receive
                                         VM_DCL_ADC_GET_RESULT. Set to VM_FALSE
                                         to ignore the */
} vm_dcl_adc_control_create_object_t;


/* The structure of ADC control command VM_DCL_ADC_COMMAND_MODIFY_PARAMETER. */
typedef struct
{
    VMUINT32 period;				           /* Measurement period, the uint is in tick. */
    VMUINT8  evaluate_count;		        /* Measurement count. */
} vm_dcl_adc_control_modify_parameter_t;


/* The structure of ADC control command VM_DCL_ADC_COMMAND_SEND_START. */
typedef struct
{
    VM_DCL_OWNER_ID owner_id;				/* Indicate the ADC module driver to notify the result. */
} vm_dcl_adc_control_send_start_t;


/* The structure of ADC control command VM_DCL_ADC_COMMAND_SEND_STOP. */
typedef struct
{
    VM_DCL_OWNER_ID owner_id;				/* Indicate the ADC module driver to notify the result. */
} vm_dcl_adc_control_send_stop_t;


/* The structure of ADC measure result for event VM_DCL_ADC_GET_RESULT. */
typedef struct
{
    VMUINT8         reference_count;    /* Reserved. */
    VMUINT16        message_length;     /* Reserved. */
    VMDOUBLE        value;              /* Indicate the measure raw data */
    VMINT32         handle;             /* Reserved. */
    VMINT32         volt_value;         /* Indicate the measure data(volt) */
    VMUINT8         scheduler_id;       /* Reserved. */
} vm_dcl_adc_measure_done_confirm_t;


/* The channel name for VM_DCL_ADC_COMMAND_GET_CHANNEL command.
   Note that not all of the channels are available.
   Refer to the HDK pin table and pin-out diagram for the
   corresponding GPIO pin number, or, use the macro
   PIN2CHANNEL in vmdcl.h to convert pin name to corresponding
   VM_DCL_ADC_CHANNEL. */
typedef enum
{
    VM_DCL_ADC_VBAT_CHANNEL = 0,     /* Battery voltage input */
    VM_DCL_ADC_VISENSE_CHANNEL = 1,  /*  */
    VM_DCL_ADC_VBATTMP_CHANNEL = 2,  /*  */
    VM_DCL_ADC_BATTYPE_CHANNEL = 3,  /*  */
    VM_DCL_ADC_VCHARGER_CHANNEL = 4, /*  */
    VM_DCL_ADC_PCBTMP_CHANNEL = 5,   /*  */
    VM_DCL_ADC_AUX_CHANNEL = 6,      /*  */
    VM_DCL_ADC_CHR_USB_CHANNEL = 7,    /*  */
    VM_DCL_ADC_OTG_VBUS_CHANNEL = 8,   /*  */
    VM_DCL_ADC_RFTMP_CHANNEL = 9,      /*  */
    VM_DCL_ADC_XP_CHANNEL = 10,         /*  */
    VM_DCL_ADC_YP_CHANNEL = 11,         /*  */
    VM_DCL_ADC_YM_CHANNEL = 12,         /*  */
    VM_DCL_ADC_XM_CHANNEL = 13,         /*  */
    VM_DCL_ADC_MAX_CHANNEL = 0x7FFF
} VM_DCL_ADC_CHANNEL;


/* The structure of ADC control command VM_DCL_ADC_COMMAND_GET_CHANNEL. */
typedef struct
{
    VMUINT8    physical_channel;    /*  */
    VMUINT16   name;                /*  */
} vm_dcl_adc_control_get_physical_channel_t;

#ifdef __cplusplus
}
#endif

#endif   /* VMDCL_ADC_SDK_H */



