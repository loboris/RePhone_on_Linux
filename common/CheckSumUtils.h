/**
  ******************************************************************************
  * @file    CheckSumUtils.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This header contains function prototypes which aid in checksum calculations.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */ 


#ifndef __CheckSumUtils_h__
#define __CheckSumUtils_h__

#include <stdint.h>

typedef struct
{
  uint16_t crc;
} CRC16_Context;

void CRC16_Init( CRC16_Context *inContext );

void CRC16_Update( CRC16_Context *inContext, const void *inSrc, int inLen );

void CRC16_Final( CRC16_Context *inContext, uint16_t *outResult );

uint8_t mico_CRC8_Table(uint8_t crc8_ori, uint8_t *p, uint32_t counter);



#endif //__CheckSumUtils_h__


