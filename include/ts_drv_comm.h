

#ifndef __TS_DRV_COMM_H__
#define __TS_DRV_COMM_H__

//#include"kal_release.h"

#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))
#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
#define DRV_WriteReg32(addr,data)     ((*(volatile kal_uint32 *)(addr)) = (kal_uint32)(data))
#define DRV_Reg32(addr)               (*(volatile kal_uint32 *)(addr))
#define DRV_WriteReg8(addr,data)     ((*(volatile kal_uint8 *)(addr)) = (kal_uint8)(data))
#define DRV_Reg8(addr)               (*(volatile kal_uint8 *)(addr))

#define DRV_ClearBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData(addr, bitmask, value)     {\
   kal_uint16 temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg(addr,temp);\
}

#define DRV_ClearBits32(addr,data)     {\
   kal_uint32 temp;\
   temp = DRV_Reg32(addr);\
   temp &=~(data);\
   DRV_WriteReg32(addr,temp);\
}

#define DRV_SetBits32(addr,data)     {\
   kal_uint32 temp;\
   temp = DRV_Reg32(addr);\
   temp |= (data);\
   DRV_WriteReg32(addr,temp);\
}

#define DRV_SetData32(addr, bitmask, value)     {\
   kal_uint32 temp;\
   temp = (~(bitmask)) & DRV_Reg32(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg32(addr,temp);\
}

#define DRV_ClearBits8(addr,data)     {\
   kal_uint8 temp;\
   temp = DRV_Reg8(addr);\
   temp &=~(data);\
   DRV_WriteReg8(addr,temp);\
}

#define DRV_SetBits8(addr,data)     {\
   kal_uint8 temp;\
   temp = DRV_Reg8(addr);\
   temp |= (data);\
   DRV_WriteReg8(addr,temp);\
}

#define DRV_SetData8(addr, bitmask, value)     {\
   kal_uint8 temp;\
   temp = (~(bitmask)) & DRV_Reg8(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg8(addr,temp);\
}


#ifdef __cplusplus
extern "C"
{
#endif

//extern kal_uint32 SaveAndSetIRQMask(void);
//extern void RestoreIRQMask(kal_uint32);

#ifdef __cplusplus
}
#endif



#ifdef DRV_DEBUG
extern void dbg_print(char *fmt,...);
extern void dbg_printWithTime(char *fmt,...);
#endif   /*DRV_DEBUG*/



// MoDIS parser skip end
#endif   /*__DRV_COMM_H__*/

