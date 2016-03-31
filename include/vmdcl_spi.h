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

#ifndef VMDCL_SPI_SDK_H
#define VMDCL_SPI_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

/* DOM-NOT_FOR_SDK-BEGIN */
/********************USER SHOULD KEEP THIS RULE*************************************************
* 1) Please use port : VM_DCL_SPI_PORT1 ( the default starts with VM_DCL_SPI_PORT1 and when a new SPI HW address exists,
*     VM_DCL_SPI_PORT2 will be added )
* 2) The AP SPI works in DMA mode. Therefore, please SET USER read/write/readwrite BUFFER as NON-Cacheable.
*     (ex: VM_READ_BUFFER* spi_read_data;
*          VM_WRITE_BUFFER* spi_write_data;
*          spi_write_data = (VM_WRITE_BUFFER*)vm_malloc_dma(sizeof(VM_WRITE_BUFFER));
*	       spi_read_data = (VM_READ_BUFFER*)vm_malloc_dma(sizeof(VM_READ_BUFFER));)
***************************************************************************************************/
/* DOM-NOT_FOR_SDK-END */


/*****************************************************************************
 * DESCRIPTION
 *   VM_DCL_SPI_DEVICE is the ENUM SPI type, you can select one of the
 *   devices to use with DCL.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1, 0);
 * if(VM_DCL_HANDLE_INVALID != spi_handle)
 * {
 *     // To do something.
 * }
 * </code>
 *****************************************************************************/
typedef enum
{
    VM_DCL_SPI_PORT_RESERVED = VM_DCL_SPI_GROUP_START,  /* This is for start SPI 1 only. Do Not Use! */
    VM_DCL_SPI_PORT1                            /* Use from SPI 1. */
} VM_DCL_SPI_DEVICE;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * This is the SPI IO control definition.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 *  spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 *  if(VM_DCL_HANDLE_INVALID == spi_handle) return;
 *	  config_data.clock_high_time = 5;
 *	  config_data.clock_low_time = 5;
 *	  config_data.cs_hold_time = 15;
 *	  config_data.cs_idle_time = 15;
 *	  config_data.cs_setup_time= 15;
 *	  config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	  config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *    config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	  config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	  config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	  config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_CONTROL_GET_PARAMETER = 0,         /* Gets the current SPI configuration parameters.*/
    VM_DCL_SPI_CONTROL_QUERY_CAPABILITY = 1,      /* Gets the capability of current SPI hardware. */
    VM_DCL_SPI_CONTROL_QUERY_MODE_SUPPORT = 2,    /* Queries supported SPI modes. */
    VM_DCL_SPI_CONTROL_QUERY_CLOCK = 3,           /* Queries SPI base clock. */
    VM_DCL_SPI_CONTROL_SET_MODE = 4,              /* Sets and configures SPI mode. */
    VM_DCL_SPI_CONTROL_GET_MODE = 5,              /* Gets SPI mode settings. */
    VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER = 8,  /* Sets SPI driving current. */
    VM_DCL_SPI_CONTROL_WRITE_AND_READ = 9         /* Sets SPI write and read. */
} VM_DCL_SPI_CONTROL_COMMAND;


/*************************************************************************************************************************************
  * DESCRIPTION
  * VM_DCL_SPI_MODE indicates which mode and what functions SPI Hardware supports. Generally users use DEASSERT or PAUSE mode.
  * ULTRA_HIGH and SLOW_DOWN is not often used as these are used to adjust the SPI ratio of EMI bandwidth. GET_TICK adjusts the
  * clock timing in order to sample SPI's master clock correctly when the speed is very high.
  *************************************************************************************************************************************/

typedef enum
{
    VM_DCL_SPI_MODE_DEASSERT,     /* The de-assert mode. The SPI CS pin will be switched to low and high for each byte during transmission. */
    VM_DCL_SPI_MODE_PAUSE,        /* The pause mode. The SPI CS pin is switched to low and kept low until the specific amount of transfers have been finished. */
    VM_DCL_SPI_MODE_ULTRA_HIGH,   /* The ultra high mode. It raises DMA priority during DMA transmission. */
    VM_DCL_SPI_MODE_SLOW_DOWN,    /* The slow down mode. It slows down SPI DMA speed during DMA transmission. */
    VM_DCL_SPI_MODE_GET_TICK      /* The get tick delay mode. This mode is used for tuning SPI timing. */
} VM_DCL_SPI_MODE;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * VM_DCL_SPI_GET_TICK is the ENUM which indicates for how many clocks the SPI should be delayed.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_mode_t spi_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1, 0);
 * if(VM_DCL_HANDLE_INVALID == spi_handle) return;
 * spi_data.mode = VM_DCL_SPI_MODE_GET_TICK;
 * spi_data.is_enable = KAL_TRUE;
 * spi_data.param = VM_DCL_SPI_GET_TICK_DELAY1;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_MODE,(void *)&spi_data);
 * //To do something.
 * </code>
*************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_GET_TICK_DELAY_1 = 1,     /* The delay SPI clock for one clock. */
    VM_DCL_SPI_GET_TICK_DELAY_2 = 2      /* The delay SPI clock for two clocks. */
} VM_DCL_SPI_GET_TICK;


/*************************************************************************************************************************************
  *  DESCRIPTION
  * VM_DCL_SPI_MSBF is the ENUM which indicates the SPI transmission bit order.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_LSB_FIRST = 0,    /* The transfer bit order is LSB first. */
    VM_DCL_SPI_MSB_FIRST = 1     /* The transfer bit order is MSB first. */
} VM_DCL_SPI_MSBF;


/*************************************************************************************************************************************
  *  DESCRIPTION
  * VM_DCL_SPI_CLOCK_POLARITY is the ENUM which indicates the SPI clock polarity.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_CLOCK_POLARITY_0 = 0,    /* The clock polarity is 0. */
    VM_DCL_SPI_CLOCK_POLARITY_1 = 1     /* The clock polarity is 1. */
}  VM_DCL_SPI_CLOCK_POLARITY;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * VM_DCL_SPI_CLOCK_PHASE is the ENUM which indicates the SPI clock phase.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_CLOCK_PHASE_0 = 0,    /* The clock phase is 0. */
    VM_DCL_SPI_CLOCK_PHASE_1 = 1     /* The clock phase is 1. */
} VM_DCL_SPI_CLOCK_PHASE;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * VM_DCL_SPI_ENDIAN is the ENUM which indicates the SPI data transfer byte order.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef enum
{
    VM_DCL_SPI_ENDIAN_LITTLE = 0,    /* Use little endian. */
    VM_DCL_SPI_ENDIAN_BIG = 1        /* Use big endian. */
} VM_DCL_SPI_ENDIAN;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_config_parameter_t holds the SPI configuration parameters.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    /* The CS setup time. Unit in SPI base clock count. Range(0-255).
       The chip select setup time = (cs_setup_time+1)*clock_PERIOD, where clock_PERIOD is the cycle time of the clock that the SPI engine adopts. */
    VMUINT32 cs_setup_time;
    /* The CS hold time. Unit in SPI base clock count. Range(0-255).
       The chip select hold time = (cs_hold_time+1)*clock_PERIOD. */
    VMUINT32 cs_hold_time;
    /* The CS idle time. Unit in SPI base clock count. Range(0-255).
       The chip select idle time between consecutive transaction = (cs_idle_time+1)*clock_PERIOD. */
    VMUINT32 cs_idle_time;
    /* The SCK clock low time. Unit in SPI base clock count Range(0-255). */
    VMUINT32 clock_low_time;
    /* The SCK clock high time. Unit in SPI base clock count. Range(0-255). */
    VMUINT32 clock_high_time;
    /* The bit order setting for SPI output. */
    VM_DCL_SPI_MSBF tx_msbf;
    /* The bit order setting for SPI input. */
    VM_DCL_SPI_MSBF rx_msbf;
    /* The byte order setting for SPI output. */
    VM_DCL_SPI_ENDIAN tx_endian;
    /* The byte order setting for SPI input. */
    VM_DCL_SPI_ENDIAN rx_endian;
    /* The SPI clock polarity. */
    VM_DCL_SPI_CLOCK_POLARITY clock_polarity;
    /* The SPI clock format. */
    VM_DCL_SPI_CLOCK_PHASE clock_phase;
} vm_dcl_spi_config_parameter_t;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_control_write_and_read_t holds the SPI write and read operation parameters.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_config_parameter_t config_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT0,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	config_data.clock_high_time = 5;
 *	config_data.clock_low_time = 5;
 *	config_data.cs_hold_time = 15;
 *	config_data.cs_idle_time = 15;
 *	config_data.cs_setup_time= 15;
 *	config_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
 *	config_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
 *  config_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
 *	config_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
 *	config_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;
 *  vm_dcl_spi_control_write_and_read_t  write_and_read;
 *  write_and_read.read_data_ptr=in_buff;
 *  write_and_read.read_data_length=10;
 *  write_and_read.write_data_ptr=out_buff;
 *  write_and_read.write_data_length=1;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&config_data);
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&write_and_read);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    VMUINT8* read_data_ptr;      /* The pointer to the read data. */
    VMUINT32 read_data_length;   /* The read data length. */
    VMUINT8* write_data_ptr;     /* The pointer to the write data. */
    VMUINT32 write_data_length;  /* The write data length. */
} vm_dcl_spi_control_read_write_t;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_capability_t exposes SPI capabilities which can be queried. This structure is
 * used as a parameter of VM_DCL_SPI_CONTROL_QUERY_CAPABILITY.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_capability_t spi_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_QUERY_CAPABILITY,(void *)&spi_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    /* The minimum value for cs setup time. */
    VMUINT32 cs_setup_time_min;
    /* The maximum value for cs setup time. */
    VMUINT32 cs_setup_time_max;
    /* The minimum value for cs hold time. */
    VMUINT32 cs_hold_time_min;
    /* The maximum value for cs hold time. */
    VMUINT32 cs_hold_time_max;
    /* The minimum value for cs idle time. */
    VMUINT32 cs_idle_time_min;
    /* The maximum value for cs idle time. */
    VMUINT32 cs_idle_time_max;
    /* The minimum value for clock low time. */
    VMUINT32  clock_low_time_min;
    /* The maximum value for clock low time. */
    VMUINT32  clock_low_time_max;
    /* The minimum value for clock high time. */
    VMUINT32  clock_high_time_min;
    /* The maximum value for clock high time. */
    VMUINT32  clock_high_time_max;
    /* The minimum value for transfer length. */
    VMUINT32  transfer_length_min;
    /* The maximum value for transfer length. */
    VMUINT32  transfer_length_max;
    /* The minimum value for transfer count. */
    VMUINT32  transfer_count_min;
    /* The maximum value for transfer count. */
    VMUINT32  transfer_count_max;
    /* The minimum value for slow down threshold. */
    VMUINT32  slow_down_threshold_min;
    /* The maximum value for slow down threshold. */
    VMUINT32  slow_down_threshold_max;
    /* The minimum value for ultra high threshold. */
    VMUINT32  ultra_high_threshold_min;
    /* The maximum value for ultra high threshold. */
    VMUINT32  ultra_high_threshold_max;
    /* The minimum value for driving current. */
    VMUINT32  driving_current_min;
    /* The maximum value for driving current. */
    VMUINT32  driving_current_max;
} vm_dcl_spi_capability_t;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_query_mode_t exposes SPI supported modes. This structure is used as a parameter
 * of VM_DCL_SPI_CONTROL_QUERY_MODE_SUPPORT.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_query_mode_t spi_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 * spi_data.mode=VM_DCL_SPI_MODE_PAUSE;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_QUERY_MODE_SUPPORT,(void *)&spi_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    VM_DCL_SPI_MODE mode;           /* [IN]-  Specifies the SPI mode. */
    VMBOOL is_supported;            /* [OUT]- Returns whether the specific mode is supported or not. */
} vm_dcl_spi_query_mode_t;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_query_mode_t exposes the SPI's clock speed. This structure is used as parameter of VM_DCL_SPI_CONTROL_QUERY_CLOCK.
 *  It contains the clock frequency which is used to calculate the SPI's timing parameters.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_clock_t spi_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_QUERY_CLOCK,(void *)&spi_data);
 * //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    VMUINT32 clock;       /* [OUT]- The SPI base clock in MHz. */
} vm_dcl_spi_clock_t;


/*************************************************************************************************************************************
 *  DESCRIPTION
 * vm_dcl_spi_mode_t exposes if the SPI's mode can be set and or get. This structure is used as the parameter of
 * VM_DCL_SPI_CONTROL_SET_MODE and VM_DCL_SPI_CONTROL_GET_MODE.
 * EXAMPLE
 * <code>
 *	VM_DCL_HANDLE spi_handle;
 *  vm_dcl_spi_mode_t spi_data;
 * spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1,0);
 * if(VM_DCL_HANDLE_INVALID==spi_handle) return;
 *  spi_data.mode = SPI_MODE_DEASSERT;
 *  spi_data.is_enable = KAL_TRUE;
 *	vm_dcl_control(spi_handle,VM_DCL_SPI_CONTROL_SET_MODE,(void *)&spi_data);
 *  //To do something.
 * </code>
  *************************************************************************************************************************************/
typedef struct
{
    /* [IN]- Specifies the SPI mode. */
    VM_DCL_SPI_MODE mode;
    /* [IN/OUT]- Specifies whether the mode is enabled.
       For VM_DCL_SPI_CONTROL_SET_MODE, it is an input parameter.
       For VM_DCL_SPI_CONTROL_GET_MODE, it is an output parameter.
    */
    VMBOOL is_enabled;
    /* [IN/OUT]- The parameter for the specific mode.
       The meaning of this parameter depends on the mode to be set and or get.
    */
    VMUINT32 parameter;
} vm_dcl_spi_mode_t;

#ifdef __cplusplus
}
#endif

#endif  /* VMDCL_SPI_SDK_H */
