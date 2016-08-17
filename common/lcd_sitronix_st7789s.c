/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example will show an image in the screen.

  In this example, it will load an image in the resource and show it in the screen.
  Launch application, it will show image in the screen
*/

#include "vmtype.h"
#include "vmdrv_lcd.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmstdlib.h"
#include "vmdcl.h"
#include "vmdcl_pwm.h"
#include "lcd_sitronix_st7789s.h"

//-----------------------
#define LCD_GPIO_RESET 38
#define LCD_GPIO_CS 39
//-----------------------

volatile VMUINT32 lcd_delay;

VMUINT8 _VM_LCD_DRIVER_ = 1;
VMUINT8 _TS_VER_ = _TOUCH_SCREEN_V1_0_;

VM_DCL_HANDLE lcd_reset_handle;
VM_DCL_HANDLE lcd_cs_handle;

extern vm_drv_lcd_function_list_t lcd_func_ST7789S;

#define LCD_ENABLE_CS vm_dcl_control(lcd_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL)
#define LCD_DISABLE_CS vm_dcl_control(lcd_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL)

#define LCD_CtrlWrite_ST7789S(_data) \
{\
LCD_ENABLE_CS;\
vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, _data);\
LCD_DISABLE_CS;\
}

#define LCD_DataWrite_ST7789S(_data) \
{\
LCD_ENABLE_CS;\
vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, _data);\
LCD_DISABLE_CS;\
}

//-------------------------
void lcd_st7789s_init(void)
{
	volatile VMUINT32* VSIM1_CON2 = (VMUINT32*)0xA0700188;
	volatile VMUINT32* VSIM1_CON0 = (VMUINT32*)0xA0700180;

	lcd_reset_handle = vm_dcl_open(VM_DCL_GPIO, LCD_GPIO_RESET);
	lcd_cs_handle = vm_dcl_open(VM_DCL_GPIO, LCD_GPIO_CS);

	// Settings set in WFD_lcd_initialize
	//LCD_SET_LCD_ROI_CTRL_OUTPUT_FORMAT(LCD_16BIT_16_BPP_RGB565_1);

	*VSIM1_CON2 = *VSIM1_CON2 | 0x0002;  // VSIM1 LDO Control Register 2, SIM1 controled by VSIM1_CON0
	*VSIM1_CON0 = *VSIM1_CON0 | 0x0001;  // VSIM1 LDO Control Register 0, VSIM1 Enable

	//Serial Clock:
	vm_drv_lcd_set_serial_clock(0, LCD_SERIAL_CLOCK_132MHZ);

	vm_drv_lcd_operation(VM_DRV_LCD_DISABLE_SERIAL0_SINGLE_A0);
	vm_drv_lcd_operation(VM_DRV_LCD_DISABLE_SERIAL0_CS_STAY_LOW);
	vm_drv_lcd_operation(VM_DRV_LCD_ENABLE_SERIAL0_3WIRE);
	//LCD_DISABLE_SERIAL0_SDI;
	//vm_drv_lcd_operation(VM_DRV_LCD_DISABLE_SERIAL0_SINGLE_A0);
	vm_drv_lcd_operation(VM_DRV_LCD_ENABLE_HW_CS);

	vm_drv_lcd_operation2(VM_DRV_LCD_SET_SERIAL0_IF_SIZE,LCD_SCNF_IF_WIDTH_8);
	//Timing need tuning
	vm_drv_lcd_init_timing(0, 0, 0, 0, 0, 7, 7);

	//Set Driving current
	//set_lcd_driving_current_centralize(LCD_DRIVING_12MA);

	vm_drv_lcd_set_serial0_1v8_driving(LCD_DRIVING_12MA);
	vm_drv_lcd_setup_driver(&lcd_func_ST7789S);
}

//--------------------------------
void lcd_enter_sleep_st7789s(void)
{

    LCD_CtrlWrite_ST7789S(0x10);
    vm_drv_lcd_delay_ms(120); 
}

//-------------------------------
void lcd_exit_sleep_st7789s(void)
{

    LCD_CtrlWrite_ST7789S(0x11);
    vm_drv_lcd_delay_ms(10);
    LCD_CtrlWrite_ST7789S(0x29);
    vm_drv_lcd_delay_ms(120); 
}

//------------------------------------
VMUINT8 lcd_partial_line_st7789s(void)
{
  return 1;   // partial display in 1 line alignment
}

//-----------------------------------------------------------------------------------------
void lcd_block_clear_st7789s(VMUINT16 x1,VMUINT16 y1,VMUINT16 x2,VMUINT16 y2,VMUINT16 data)
{
  VMUINT16 LCD_x;
  VMUINT16 LCD_y;
  /*
  VMUINT8 r_color,g_color,b_color;
  // transfer to RGB666
  r_color=((data&0xF800)>>10) | (data>>15);
  g_color=((data>>5)&0x3F);
  b_color=((data&0x1F)<<1) | ((data>>4)&0x1);
  */
  LCD_CtrlWrite_ST7789S(0x2A);
  LCD_DataWrite_ST7789S((x1 & 0xFF00) >> 8);
  LCD_DataWrite_ST7789S(x1 & 0xFF);
  LCD_DataWrite_ST7789S((x2 & 0xFF00) >> 8);
  LCD_DataWrite_ST7789S(x2 & 0xFF);
  
  LCD_CtrlWrite_ST7789S(0x2B);
  LCD_DataWrite_ST7789S((y1 & 0xFF00) >> 8);
  LCD_DataWrite_ST7789S(y1 & 0xFF);
  LCD_DataWrite_ST7789S((y2 & 0xFF00) >> 8);
  LCD_DataWrite_ST7789S(y2 & 0xFF);
  
  LCD_CtrlWrite_ST7789S(0x2C);

  LCD_ENABLE_CS;
  for (LCD_y=y1;LCD_y<=y2;LCD_y++)
	{
	  for(LCD_x=x1;LCD_x<=x2;LCD_x++)
	  {
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, ((data & 0xFF00) >> 8));
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, (data & 0xFF));
	  }
	}
  LCD_DISABLE_CS;
}

//--------------------------------------
void LCD_ClearAll_ST7789S(VMUINT16 data)
{
   lcd_block_clear_st7789s(0,0,240-1,240-1,data);
}

//-------------------------------------------------------
void lcd_init_st7789s(VMUINT32 bkground, void **buf_addr)
{  
  // Do HW Reset
	vm_dcl_control(lcd_reset_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	vm_drv_lcd_delay_ms(1);
	vm_dcl_control(lcd_reset_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	vm_drv_lcd_delay_ms(10);
	vm_dcl_control(lcd_reset_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	vm_drv_lcd_delay_ms(120);
  
	LCD_CtrlWrite_ST7789S(0x11);
	vm_drv_lcd_delay_ms(120);

	LCD_ENABLE_CS;

	if (_VM_LCD_DRIVER_) {
		vm_drv_lcd_operation(VM_DRV_LCD_ENABLE_SERIAL0_2PIN);
		vm_drv_lcd_operation2(VM_DRV_LCD_SET_SERIAL0_IF_2PIN_SIZE, LCD_SCNF_IF_2PIN_WIDTH_16);

		// 2 data lane
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xE7);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x10);
	}

	if (_TS_VER_ == _TOUCH_SCREEN_V1_0_) {
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x36);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);// C0 40 60

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x35);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00); //te on

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x2a);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xef);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x2b);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xef);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x3A);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x55);
		
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xB2);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1C);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1C);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x01);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xFF);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x33);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xB3);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x10);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xFF);	  // 0x05
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0F);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xB4);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0B);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xB5);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x9F);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xB7);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x35);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xBB);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x28);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xBC);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xEC);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xBD);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xFE);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xC0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x2C);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xC2);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x01);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xC3);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1E);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xC4);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x20);

		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xC6);		  // Normal mode frame rate
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1E);	// 0x0f 60Hz while FPA and BPA = 0x0C
	
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xD0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xA4);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xA1);
	
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xE0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xD0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x07);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x05);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x29);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x54);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x41);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x3C);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x17);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x15);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1A);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x20);
	
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xE1);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xD0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x07);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x04);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x29);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x44);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x42);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x3B);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x16);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x15);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1B);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1F);

	}
	else {
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x36);	// Memory Data Access Control
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x3a);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x05);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x21);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x2a);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xef);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x2b);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xef);
		//--------------------------------ST7789V Frame rate setting----------------------------------//
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xb2);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0c);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0c);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x00);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x33);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x33);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xb7);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x35);
		//---------------------------------ST7789V Power setting--------------------------------------//
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xbb);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x1f);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xc0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x2c);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xc2);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x01);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xc3);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x12);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xc4);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x20);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xc6);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0f);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xd0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xa4);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xa1);
		//--------------------------------ST7789V gamma setting--------------------------------------//
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xe0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xd0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x11);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0c);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x15);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x39);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x33);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x50);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x36);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x13);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x14);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x29);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x2d);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0xe1);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0xd0);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x10);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x08);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x06);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x06);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x39);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x44);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x51);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x0b);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x16);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x14);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x2f);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, 0x31);
		vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, 0x29);
	}

	LCD_DISABLE_CS;

	// clear the screen with black color
	//LCD_CtrlWrite_ST7789S(0x2C);
	LCD_ClearAll_ST7789S(0);

	// display on
	LCD_CtrlWrite_ST7789S(0x29);
	vm_drv_lcd_delay_ms(120);

}

//-------------------------------------------
void lcd_power_on_st7789s(VM_DRV_LCD_BOOL on)
{
   if (on) lcd_exit_sleep_st7789s();
   else lcd_enter_sleep_st7789s();
}

//-------------------------------------
void lcd_on_st7789s(VM_DRV_LCD_BOOL on)
{
   if (on) lcd_exit_sleep_st7789s();
   else lcd_enter_sleep_st7789s();
}

//---------------------------------------------------------------------------------------
void lcd_block_write_st7789s(VMUINT16 startx,VMUINT16 starty,VMUINT16 endx,VMUINT16 endy)
{  
  LCD_CtrlWrite_ST7789S(0x2A);
  LCD_DataWrite_ST7789S((startx&0xFF00)>>8);
  LCD_DataWrite_ST7789S(startx&0xFF);
  LCD_DataWrite_ST7789S((endx&0xFF00)>>8);
  LCD_DataWrite_ST7789S(endx&0xFF);
  
  LCD_CtrlWrite_ST7789S(0x2B);
  LCD_DataWrite_ST7789S((starty&0xFF00)>>8);
  LCD_DataWrite_ST7789S(starty&0xFF);
  LCD_DataWrite_ST7789S((endy&0xFF00)>>8);
  LCD_DataWrite_ST7789S(endy&0xFF);
  
  LCD_CtrlWrite_ST7789S(0x2C);
  
  LCD_ENABLE_CS;
  vm_drv_lcd_operation(VM_DRV_LCD_ENABLE_LCD_CMD_COMPLETE_INT);
  vm_drv_lcd_operation(VM_DRV_LCD_DISABLE_LCD_ROI_CTRL_CMD_FIRST);
  vm_drv_lcd_operation(VM_DRV_LCD_START_LCD_TRANSFER);
}

//---------------------------------------------------------------------
void lcd_size_st7789s(VMUINT16 *out_LCD_width,VMUINT16 *out_LCD_height)
{
   *out_LCD_width = 240;
   *out_LCD_height = 240;
}

//-------------------------------------------------------------------------------------
VM_DRV_LCD_IOCTRL_STATUS lcd_io_ctrl_st7789s(VM_DRV_LCD_IOCTRL_ID ID, void* Parameters)
{
   switch (ID)
   {
       case VM_DRV_LCD_IOCTRL_SET_FRAME_RATE:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
            
       case VM_DRV_LCD_IOCTRL_QUERY_FRAME_MARKER:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
       
       case VM_DRV_LCD_IOCTRL_SET_FRAME_MARKER:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;

       case VM_DRV_LCD_IOCTRL_QUERY_SUPPORT_H_V_SIGNAL_FUNC:
       case VM_DRV_LCD_IOCTRL_QUERY_SUPPORT_V_PULSE_WIDTH:
       case VM_DRV_LCD_IOCTRL_QUERY_SUPPORT_H_PULSE_WIDTH:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
            
       case VM_DRV_LCD_IOCTRL_QUERY_BACK_PORCH:
            *(VMUINT32 *)(Parameters) = 12;
            return VM_DRV_LCD_IOCTRL_OK;
            
       case VM_DRV_LCD_IOCTRL_QUERY_FRONT_PORCH:
            *(VMUINT32 *)(Parameters) = 12;
            return VM_DRV_LCD_IOCTRL_OK;

       case VM_DRV_LCD_IOCTRL_SET_BACK_PORCH:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
       
       case VM_DRV_LCD_IOCTRL_SET_FRONT_PORCH:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
   
       case VM_DRV_LCD_IOCTRL_QUERY_TE_EDGE_ATTRIB:
            //*(VMUINT32 *)(Parameters) = 0;
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
            
       case VM_DRV_LCD_IOCTRL_QUERY_SUPPORT_READBACK_FUNC:
       case VM_DRV_LCD_IOCTRL_QUERY_SCANLINE_REG:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
            
       case VM_DRV_LCD_IOCTRL_QUERY_IF_CS_NUMBER:
            *(VMUINT32 *)(Parameters) = 3;
            return VM_DRV_LCD_IOCTRL_OK;

       case VM_DRV_LCD_IOCTRL_QUERY_FRAME_RATE:
            *((unsigned long*)(Parameters)) = 40; // 90
            return VM_DRV_LCD_IOCTRL_OK;
            
       case VM_DRV_LCD_IOCTRL_QUERY_LCM_WIDTH:
            *((unsigned long*)(Parameters)) = 240;// physical, not use app's VM_DRV_LCD_WIDTH
            return VM_DRV_LCD_IOCTRL_OK;

       case VM_DRV_LCD_IOCTRL_QUERY_LCM_HEIGHT:
            *((unsigned long*)(Parameters)) = 240;//320;// physical, not use app's VM_DRV_LCD_HEIGHT
            return VM_DRV_LCD_IOCTRL_OK;
    
       case VM_DRV_LCD_IOCTRL_QUERY_SYNC_MODE:
            *(VMUINT32 *)(Parameters) = 1;
            return VM_DRV_LCD_IOCTRL_OK;
			
       default:
            return VM_DRV_LCD_IOCTRL_NOT_SUPPORT;
   }
}

//---------------------------------------------
vm_drv_lcd_function_list_t lcd_func_ST7789S = {
   lcd_init_st7789s,
   lcd_power_on_st7789s,
   0,
   lcd_on_st7789s,
   lcd_block_write_st7789s,
   lcd_size_st7789s,
   lcd_enter_sleep_st7789s,
   lcd_exit_sleep_st7789s,
   0,
   0,
   lcd_partial_line_st7789s,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0, 
   0,
   0,
   0,
   lcd_io_ctrl_st7789s
};

//-----------------------------------------------
void _sitronix_LcdTransfer(uint8_t *buf, int len)
{

  VM_DCL_STATUS status = 0;
  int count = len;
  int bptr = 0;
  uint16_t ndata;
  uint32_t rep, i, n;

  // --- Activate chip select ---
  LCD_ENABLE_CS;
  while (count > 0) {
	// *** New command ***
	if (buf[bptr] == 0) break;

	// --- Activate chip select ---
	//LCD_ENABLE_CS;

	// --- send command byte ---
	vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_LOW_ADDR, buf[bptr]);
	// --- Deactivate chip select ?? ---
	//LCD_DISABLE_CS;

	count--;
    bptr++;  // point to ndata
	ndata = (uint16_t)(buf[bptr++] << 8);
	ndata += (uint16_t)(buf[bptr++]);
	rep = (uint32_t)(buf[bptr++] << 24);
	rep += (uint32_t)(buf[bptr++] << 16);
	rep += (uint32_t)(buf[bptr++] << 8);
	rep += (uint32_t)(buf[bptr++]);
	count -= 6;
	if ((count > 0) && (ndata > 0)) {
	  // --- Activate chip select ?? ---

	  // --- Send data
	  for (i=0; i<rep; i++) {
		  for (n=0; n < ndata; n++) {
		    //LCD_ENABLE_CS;
			vm_drv_lcd_operation2(VM_DRV_LCD_WRITE_SERIAL0_A0_HIGH_ADDR, buf[bptr+n]);
			//LCD_DISABLE_CS;
		  }
	  }
	  count -= ndata;
	  bptr += ndata;
	  // --- Deactivate chip select -------------------
	}
  }
  // --- Deactivate chip select ---
  LCD_DISABLE_CS;
}

//-------------------------------------
void _set_backlight(uint8_t brightness)
{
    VM_DCL_HANDLE pwm_handle;
    vm_dcl_pwm_set_clock_t pwm_clock;
    vm_dcl_pwm_set_counter_threshold_t pwm_config_adv;
    vm_dcl_config_pin_mode(3, VM_DCL_PIN_MODE_PWM);
    pwm_handle = vm_dcl_open(PIN2PWM(3), vm_dcl_get_owner_id());
    vm_dcl_control(pwm_handle, VM_PWM_CMD_START, 0);
    pwm_config_adv.counter = 100;
    pwm_config_adv.threshold = brightness;
    pwm_clock.source_clock = 0;
    pwm_clock.source_clock_division = 3;
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_CLOCK, (void*)(&pwm_clock));
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD, (void*)(&pwm_config_adv));
    vm_dcl_close(pwm_handle);
}

