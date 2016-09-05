/**
 * oleddisp.c
 */

#include <math.h>
#include <string.h>
#include <stdlib.h> 
#include <fcntl.h>
#include "tjpgd.h"

#include "vmdcl.h"
#include "vmtype.h"
#include "vmlog.h"
#include "vmmemory.h"
#include "vmfs.h"
#include "vmstdlib.h"
#include "vmchset.h"
#include "vmtouch.h"
#include "vmdcl_gpio.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "shell.h"

#include "lcd_sitronix_st7789s.h"
#include "tp_goodix_gt9xx.h"


#define LCD_SOFT_RESET // if not using RST pin

#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define XADOW_WIDTH  240
#define XADOW_HEIGHT 240

#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB 0x2

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define TFT_INVOFF     0x20
#define TFT_INVONN     0x21
#define TFT_DISPOFF    0x28
#define TFT_DISPON     0x29
#define TFT_CASET      0x2A
#define TFT_RASET      0x2B
#define TFT_RAMWR      0x2C
#define TFT_RAMRD      0x2E
#define TFT_MADCTL	   0x36
#define TFT_PTLAR 	   0x30

#define ST7735_COLMOD  0x3A

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_GAMMASET 0x26

#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

#define ILI9341_POWERA	0xCB
#define ILI9341_POWERB	0xCF
#define ILI9341_POWER_SEQ       0xED
#define ILI9341_DTCA	0xE8
#define ILI9341_DTCB	0xEA
#define ILI9341_PRC	0xF7
#define ILI9341_3GAMMA_EN	0xF2


// Color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
#define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define TFT_PINK        0xF81F

#define INVERT_ON		1
#define INVERT_OFF		0

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

#define PORTRAIT	0
#define LANDSCAPE	1
#define PORTRAIT_FLIP	2
#define LANDSCAPE_FLIP	3

#define LASTX	-1
#define LASTY	-2
#define CENTER	-3
#define RIGHT	-4
#define BOTTOM	-4

#define DEFAULT_FONT	0
#define FONT_7SEG		1
#define USER_FONT		2

#define bitmapdatatype uint16_t *

extern uint8_t lcd_DefaultFont[];

static uint8_t *userfont = NULL;

extern int g_graphics_ready;
extern VM_DCL_HANDLE g_spi_handle;
extern t_spi_params g_spi;
extern int _setup_spi();
extern int _spi_wrrd_data(VMUINT8 *temp_buf, int len);
extern int _LcdSpiTransfer(uint8_t *buf, int len);
extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);

static int TFT_type = -1;
static uint16_t _width = XADOW_WIDTH;
static uint16_t _height = XADOW_HEIGHT;

#ifndef LCD_SOFT_RESET
  uint8_t lcd_pinRST  = 255;
  #define LCD_RST1  vm_dcl_control(lcd_reset_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL)
  #define LCD_RST0  vm_dcl_control(lcd_reset_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL)
#endif
  
static int colstart = 0;
static int rowstart = 0;				// May be overridden in init func
static uint8_t orientation = PORTRAIT;	// screen orientation
static uint8_t rotation = 0;			// font rotation
static uint8_t tp_initialized = 0;		// touch panel initialized flag
static VM_DCL_HANDLE tp_cs_handle = VM_DCL_HANDLE_INVALID;

typedef struct {
	uint8_t 	*font;
	uint8_t 	x_size;
	uint8_t 	y_size;
	uint8_t	    offset;
	uint16_t	numchars;
    uint8_t     bitmap;
	uint16_t    color;
} Font;

typedef struct {
      uint8_t charCode;
      int adjYOffset;
      int width;
      int height;
      int xOffset;
      int xDelta;
      uint16_t dataPtr;
} propFont;

typedef struct {
	uint16_t        x1;
	uint16_t        y1;
	uint16_t        x2;
	uint16_t        y2;
} dispWin_t;

static dispWin_t dispWin = {
  .x1 = 0,
  .y1 = 0,
  .x2 = ST7735_WIDTH,
  .y2 = ST7735_HEIGHT,
};

static Font cfont;
static propFont fontChar;
static uint8_t _transparent = 0;
static uint8_t _wrap = 0; // carracter wrapping to new line
static uint8_t _forceFixed = 0;
static uint16_t _fg = TFT_GREEN;
static uint16_t _bg = TFT_BLACK;

#define CCBUF_SIZE 768  // set the size 16 * n

static uint8_t ccbuf[CCBUF_SIZE];
static uint16_t ccbufPtr = 0;

static int TFT_X  = 0;
static int TFT_Y  = 0;
static int TFT_OFFSET  = 0;

static int g_touch_cb_ref = LUA_NOREF;
static cb_func_param_touch_t touch_cb_params;

#define swap(a, b) { int16_t t = a; a = b; b = t; }


//--------------------------------------------------------------------
static void handle_touch_event(VM_TOUCH_EVENT event, VMINT x, VMINT y)
{
    // VM_TOUCH_EVENT_TAP
    // VM_TOUCH_EVENT_RELEASE

	if ((event == VM_TOUCH_EVENT_TAP) ||
		(event == VM_TOUCH_EVENT_LONG_TAP) ||
		(event == VM_TOUCH_EVENT_MOVE) ||
		(event == VM_TOUCH_EVENT_DOUBLE_CLICK)) {
		if (g_touch_cb_ref != LUA_NOREF) {
			if (touch_cb_params.busy == 0) {
				touch_cb_params.busy = 1;
				touch_cb_params.event = event;
				if ((orientation & 1)) {
					touch_cb_params.x = y;
					touch_cb_params.y = 240 - x;
				}
				else {
					touch_cb_params.x = x;
					touch_cb_params.y = y;
				}
				touch_cb_params.cb_ref = g_touch_cb_ref;
				remote_lua_call(CB_FUNC_TOUCH, &touch_cb_params);
			}
		}
		else {
			//vm_log_debug("touch: ev=%d, x=%d, y=%d\n", event, x, y);
		}
	}
}

//-------------------------------
static void send_to_display(void)
{	// use main thread to send data to sisplay
    if (TFT_type > 0) {
		if (TFT_type == 2) g_fcall_message.message_id = CCALL_MESSAGE_SIT_LCDWR;
		else g_fcall_message.message_id = CCALL_MESSAGE_LCDWR;
		g_CCparams.cpar1 = ccbuf;
		g_CCparams.ipar1 = ccbufPtr;
		vm_thread_send_message(g_main_handle, &g_fcall_message);
		// wait for call to finish...
		vm_signal_wait(g_shell_signal);
    }
}

//---------------------------
static void initccbuf(void) {
  ccbufPtr = 0;
  ccbuf[0] = 0;
}

//-----------------------------------------
static void ccbufPushByte( uint8_t data ) {
  ccbuf[ccbufPtr++] = data;
}

//--------------------------------------------
static void ccbufPushUint16( uint16_t data ) {
  ccbuf[ccbufPtr++] = (uint8_t)(data >> 8);
  ccbuf[ccbufPtr++] = (uint8_t)(data & 0x00FF);
}

//------------------------------------------
static void ccbufPushLong( uint32_t data ) {
  ccbuf[ccbufPtr++] = (uint8_t)(data >> 24);
  ccbuf[ccbufPtr++] = (uint8_t)(data >> 16);
  ccbuf[ccbufPtr++] = (uint8_t)(data >> 8);
  ccbuf[ccbufPtr++] = (uint8_t)(data & 0x000000FF);
}

//-----------------------------------
static void ccbufFlash( uint8_t n ) {
  if (ccbufPtr > (CCBUF_SIZE-n)) {
    ccbuf[ccbufPtr] = 0;
    send_to_display();
    initccbuf();
  }
}

//-----------------------------
static void ccbufSend( void ) {
  if (ccbufPtr > 0) {
    ccbuf[ccbufPtr] = 0;
    send_to_display();
    initccbuf();
  }
}

//-----------------------------------------------------------------------------------
static void _TFT_pushAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  ccbufFlash(22);
  
  ccbufPushByte(TFT_CASET);     // Column addr set
  ccbufPushUint16(4);           // ndata
  ccbufPushLong(1);             // repeat
  ccbufPushUint16(x0+colstart); // XSTART 
  ccbufPushUint16(x1+colstart); // XEND

  ccbufPushByte(TFT_RASET);     // Row addr set
  ccbufPushUint16(4);           // ndata
  ccbufPushLong(1);             // repeat
  ccbufPushUint16(y0+rowstart); // Y-START
  ccbufPushUint16(y1+rowstart); // Y-END
}

//----------------------------------------------------
static void _TFT_pushColorRep(uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1,
                       uint16_t color, uint32_t rep) {
  ccbufFlash(32);

  _TFT_pushAddrWindow(x0,y0,x1,y1);

  ccbufPushByte(TFT_RAMWR); // write to RAM command
  ccbufPushUint16(2);       // ndata
  ccbufPushLong(rep);       // repeat
  ccbufPushUint16(color);   // color

  ccbufFlash(32);
}

// Push control command to buffer
//----------------------------------------------------
static void TFT_sendCmd(uint8_t cmd, uint16_t ndata) {
  ccbufPushByte(cmd);
  ccbufPushUint16(ndata);
  ccbufPushLong(1);
}


// === LCD INITIALIZATION ===================================================
// Rather than a bazillion TFT_sendCmd() and TFT_sendData() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.

#define DELAY 0x80

// Initialization commands for 7735B screens
// -----------------------------------------
static const uint8_t Bcmd[] = {
  18,				// 18 commands in list:
  ST7735_SWRESET,   DELAY,	//  1: Software reset, no args, w/delay
  50,				//     50 ms delay
  ST7735_SLPOUT ,   DELAY,	//  2: Out of sleep mode, no args, w/delay
  255,				//     255 = 500 ms delay
  ST7735_COLMOD , 1+DELAY,	//  3: Set color mode, 1 arg + delay:
  0x05,				//     16-bit color 5-6-5 color format
  10,				//     10 ms delay
  ST7735_FRMCTR1, 3+DELAY,	//  4: Frame rate control, 3 args + delay:
  0x00,				//     fastest refresh
  0x06,				//     6 lines front porch
  0x03,				//     3 lines back porch
  10,				//     10 ms delay
  TFT_MADCTL , 1      ,	//  5: Memory access ctrl (directions), 1 arg:
  0x08,				//     Row addr/col addr, bottom to top refresh
  ST7735_DISSET5, 2      ,	//  6: Display settings #5, 2 args, no delay:
  0x15,				//     1 clk cycle nonoverlap, 2 cycle gate
  //     rise, 3 cycle osc equalize
  0x02,				//     Fix on VTL
  ST7735_INVCTR , 1      ,	//  7: Display inversion control, 1 arg:
  0x0,				//     Line inversion
  ST7735_PWCTR1 , 2+DELAY,	//  8: Power control, 2 args + delay:
  0x02,				//     GVDD = 4.7V
  0x70,				//     1.0uA
  10,				//     10 ms delay
  ST7735_PWCTR2 , 1      ,	//  9: Power control, 1 arg, no delay:
  0x05,				//     VGH = 14.7V, VGL = -7.35V
  ST7735_PWCTR3 , 2      ,	// 10: Power control, 2 args, no delay:
  0x01,				//     Opamp current small
  0x02,				//     Boost frequency
  ST7735_VMCTR1 , 2+DELAY,	// 11: Power control, 2 args + delay:
  0x3C,				//     VCOMH = 4V
  0x38,				//     VCOML = -1.1V
  10,				//     10 ms delay
  ST7735_PWCTR6 , 2      ,	// 12: Power control, 2 args, no delay:
  0x11, 0x15,
  ST7735_GMCTRP1,16      ,	// 13: Magical unicorn dust, 16 args, no delay:
  0x09, 0x16, 0x09, 0x20,	//     (seriously though, not sure what
  0x21, 0x1B, 0x13, 0x19,	//      these config values represent)
  0x17, 0x15, 0x1E, 0x2B,
  0x04, 0x05, 0x02, 0x0E,
  ST7735_GMCTRN1,16+DELAY,	// 14: Sparkles and rainbows, 16 args + delay:
  0x0B, 0x14, 0x08, 0x1E,	//     (ditto)
  0x22, 0x1D, 0x18, 0x1E,
  0x1B, 0x1A, 0x24, 0x2B,
  0x06, 0x06, 0x02, 0x0F,
  10,				//     10 ms delay
  TFT_CASET  , 4      , 	// 15: Column addr set, 4 args, no delay:
  0x00, 0x02,			//     XSTART = 2
  0x00, 0x81,			//     XEND = 129
  TFT_RASET  , 4      , 	// 16: Row addr set, 4 args, no delay:
  0x00, 0x02,			//     XSTART = 1
  0x00, 0x81,			//     XEND = 160
  ST7735_NORON  ,   DELAY,	// 17: Normal display on, no args, w/delay
  10,				//     10 ms delay
  TFT_DISPON ,   DELAY,  	// 18: Main screen turn on, no args, w/delay
  255				//     255 = 500 ms delay
};

// Init for 7735R, part 1 (red or green tab)
// -----------------------------------------
static const uint8_t  Rcmd1[] = {                 
  15,				// 15 commands in list:
  ST7735_SWRESET,   DELAY,	//  1: Software reset, 0 args, w/delay
  150,				//     150 ms delay
  ST7735_SLPOUT ,   DELAY,	//  2: Out of sleep mode, 0 args, w/delay
  255,				//     500 ms delay
  ST7735_FRMCTR1, 3      ,	//  3: Frame rate ctrl - normal mode, 3 args:
  0x01, 0x2C, 0x2D,		//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
  ST7735_FRMCTR2, 3      ,	//  4: Frame rate control - idle mode, 3 args:
  0x01, 0x2C, 0x2D,		//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
  ST7735_FRMCTR3, 6      ,	//  5: Frame rate ctrl - partial mode, 6 args:
  0x01, 0x2C, 0x2D,		//     Dot inversion mode
  0x01, 0x2C, 0x2D,		//     Line inversion mode
  ST7735_INVCTR , 1      ,	//  6: Display inversion ctrl, 1 arg, no delay:
  0x07,				//     No inversion
  ST7735_PWCTR1 , 3      ,	//  7: Power control, 3 args, no delay:
  0xA2,
  0x02,				//     -4.6V
  0x84,				//     AUTO mode
  ST7735_PWCTR2 , 1      ,	//  8: Power control, 1 arg, no delay:
  0xC5,				//     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
  ST7735_PWCTR3 , 2      ,	//  9: Power control, 2 args, no delay:
  0x0A,				//     Opamp current small
  0x00,				//     Boost frequency
  ST7735_PWCTR4 , 2      ,	// 10: Power control, 2 args, no delay:
  0x8A,				//     BCLK/2, Opamp current small & Medium low
  0x2A,  
  ST7735_PWCTR5 , 2      ,	// 11: Power control, 2 args, no delay:
  0x8A, 0xEE,
  ST7735_VMCTR1 , 1      ,	// 12: Power control, 1 arg, no delay:
  0x0E,
  TFT_INVOFF , 0      ,	// 13: Don't invert display, no args, no delay
  TFT_MADCTL , 1      ,	// 14: Memory access control (directions), 1 arg:
  0xC0,				//     row addr/col addr, bottom to top refresh, RGB order
  ST7735_COLMOD , 1+DELAY,	//  15: Set color mode, 1 arg + delay:
  0x05,				//     16-bit color 5-6-5 color format
  10				//     10 ms delay
};

// Init for 7735R, part 2 (green tab only)
// ---------------------------------------
static const uint8_t Rcmd2green[] = {
  2,				//  2 commands in list:
  TFT_CASET  , 4      ,	        //  1: Column addr set, 4 args, no delay:
  0x00, 0x02,			//     XSTART = 0
  0x00, 0x7F+0x02,		//     XEND = 129
  TFT_RASET  , 4      ,	        //  2: Row addr set, 4 args, no delay:
  0x00, 0x01,			//     XSTART = 0
  0x00, 0x9F+0x01		//     XEND = 160
};

// Init for 7735R, part 2 (red tab only)
// -------------------------------------
static const uint8_t Rcmd2red[] = {
  2,				//  2 commands in list:
  TFT_CASET  , 4      ,	        //  1: Column addr set, 4 args, no delay:
  0x00, 0x00,			//     XSTART = 0
  0x00, 0x7F,			//     XEND = 127
  TFT_RASET  , 4      ,	        //  2: Row addr set, 4 args, no delay:
  0x00, 0x00,			//     XSTART = 0
  0x00, 0x9F			//     XEND = 159
};

// Init for 7735R, part 3 (red or green tab)
// -----------------------------------------
static const uint8_t Rcmd3[] = {
  4,				//  4 commands in list:
  ST7735_GMCTRP1, 16      ,	//  1: Magical unicorn dust, 16 args, no delay:
  0x02, 0x1c, 0x07, 0x12,
  0x37, 0x32, 0x29, 0x2d,
  0x29, 0x25, 0x2B, 0x39,
  0x00, 0x01, 0x03, 0x10,
  ST7735_GMCTRN1, 16      ,	//  2: Sparkles and rainbows, 16 args, no delay:
  0x03, 0x1d, 0x07, 0x06,
  0x2E, 0x2C, 0x29, 0x2D,
  0x2E, 0x2E, 0x37, 0x3F,
  0x00, 0x00, 0x02, 0x10,
  ST7735_NORON  ,    DELAY,	//  3: Normal display on, no args, w/delay
  10,				//     10 ms delay
  TFT_DISPON ,    DELAY,	//  4: Main screen turn on, no args w/delay
  100				//     100 ms delay
};

// Init for ILI7341
// ----------------
static const uint8_t ILI7341_init[] = {
  23,                           // 23 commands in list
  ILI9341_SWRESET, DELAY,   	//  1: Software reset, no args, w/delay
  200,				//     50 ms delay
  ILI9341_POWERA, 5, 0x39, 0x2C, 0x00, 0x34, 0x02, 
  ILI9341_POWERB, 3, 0x00, 0XC1, 0X30, 
  0xEF, 3, 0x03, 0x80, 0x02,
  ILI9341_DTCA, 3, 0x85, 0x00, 0x78, 
  ILI9341_DTCB, 2, 0x00, 0x00, 
  ILI9341_POWER_SEQ, 4, 0x64, 0x03, 0X12, 0X81, 
  ILI9341_PRC, 1, 0x20, 
  ILI9341_PWCTR1, 1,    //Power control 
  0x23,                 //VRH[5:0] 
  ILI9341_PWCTR2, 1,    //Power control 
  0x10,                 //SAP[2:0];BT[3:0] 
  ILI9341_VMCTR1, 2,    //VCM control 
  0x3e,                 //Contrast
  0x28, 
  ILI9341_VMCTR2, 1,    //VCM control2 
  0x86,
  TFT_MADCTL, 1,    // Memory Access Control
  0x48,
  ILI9341_PIXFMT, 1,    
  0x55, 
  ILI9341_FRMCTR1, 2,
  0x00,  
  0x18, 
  ILI9341_DFUNCTR, 3,    // Display Function Control 
  0x08,
  0x82,
  0x27,  
  TFT_PTLAR, 4, 0x00, 0x00, 0x01, 0x3F,
  ILI9341_3GAMMA_EN, 1,  // 3Gamma Function Disable 
  0x00, // 0x02
  ILI9341_GAMMASET, 1,   //Gamma curve selected 
  0x01, 
  ILI9341_GMCTRP1, 15,   //Positive Gamma Correction
  0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 
  0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00, 
  ILI9341_GMCTRN1, 15,   //Negative Gamma Correction
  0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 
  0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT, DELAY, //  Sleep out
  120,			 //  120 ms delay
  TFT_DISPON, 0,
};

//------------------------------------------------------
// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array
//--------------------------------------------
static void commandList(const uint8_t *addr) {
  uint8_t  numCommands, numArgs, cmd;
  uint16_t ms;

  numCommands = *addr++;         // Number of commands to follow
  initccbuf();
  while(numCommands--) {         // For each command...
    cmd = *addr++;               // save command
    numArgs  = *addr++;          //   Number of args to follow
    ms       = numArgs & DELAY;  //   If hibit set, delay follows args
    numArgs &= ~DELAY;           //   Mask out delay bit
    TFT_sendCmd(cmd, numArgs);
    while(numArgs--) {           //   For each argument...
      ccbufPushByte(*addr++);    //     Read, issue argument
    }

    if(ms) {
      ccbufSend();
      ms = *addr++;              // Read post-command delay time (ms)
      if(ms == 255) ms = 500;    // If 255, delay for 500 ms
      vm_thread_sleep(ms);
    }
  }
  ccbufSend();
}

// Initialization code common to both 'B' and 'R' type displays
//-----------------------------------------------------
static void ST7735_commonInit(const uint8_t *cmdList) {
	// toggle RST low to reset; CS low so it'll listen to us
#ifdef LCD_SOFT_RESET
  initccbuf();
  TFT_sendCmd(ST7735_SWRESET,0);
  ccbufSend();
  vm_thread_sleep(125);
#else
  LCD_RST1;
  vm_thread_sleep(10);
  LCD_RST0;
  vm_thread_sleep(50);
  LCD_RST1;
  vm_thread_sleep(125);
#endif    
  if(cmdList) commandList(cmdList);
}

// Initialization for ST7735B screens
//------------------------------
static void ST7735_initB(void) {
  ST7735_commonInit(Bcmd);
}

// Initialization for ST7735R screens (green or red tabs)
//-----------------------------------------
static void ST7735_initR(uint8_t options) {
  vm_thread_sleep(50);
  ST7735_commonInit(Rcmd1);
  if(options == INITR_GREENTAB) {
    commandList(Rcmd2green);
    colstart = 2;
    rowstart = 1;
  } else {
    // colstart, rowstart left at default '0' values
    commandList(Rcmd2red);
  }
  commandList(Rcmd3);

  // if black, change MADCTL color filter
  if (options == INITR_BLACKTAB) {
    initccbuf();
    TFT_sendCmd(TFT_MADCTL,1);
    ccbufPushByte(0xC0);
    ccbufSend();
  }

  //  tabcolor = options;
}


// draw color pixel on screen
//---------------------------------------------------------------
static void TFT_drawPixel(int16_t x, int16_t y, uint16_t color) {

  if ((x < dispWin.x1) || (y < dispWin.y1) || (x > dispWin.x2) || (y >= dispWin.y2)) return;
  
  _TFT_pushColorRep(x,y,x,y, color, 1);
}

//------------------------------------------------------------------------------
static void TFT_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  // clipping
  if ((x < dispWin.x1) || (x > dispWin.x2) || (y > dispWin.y2)) return;
  if (y < dispWin.y1) y = dispWin.y1;
  if ((y + h) > dispWin.y2) h = dispWin.y2 - y;

  _TFT_pushColorRep(x, y, x, y+h-1, color, (uint32_t)h);
}

//------------------------------------------------------------------------------
static void TFT_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  // clipping
  if ((y < dispWin.y1) || (x > dispWin.x2) || (y > dispWin.y2)) return;
  if (x < dispWin.x1) x = dispWin.x1;
  if ((x + w) > dispWin.x2) w = dispWin.x2 - x;

  _TFT_pushColorRep(x, y, x+w-1, y, color, (uint32_t)w);
}

// Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer this uses
// the eficient FastH/V Line draw routine for segments of 2 pixels or more
//--------------------------------------------------------------------------------------
static void TFT_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int steep = 0;
  if (abs(y1 - y0) > abs(x1 - x0)) steep = 1;
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx = x1 - x0, dy = abs(y1 - y0);;
  int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

  if (y0 < y1) ystep = 1;
  
  initccbuf();
  
  // Split into steep and not steep for FastH/V separation
  if (steep) {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;
        if (dlen == 1) TFT_drawPixel(y0, xs, color);
        else TFT_drawFastVLine(y0, xs, dlen, color);
        dlen = 0; y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen) TFT_drawFastVLine(y0, xs, dlen, color);
  }
  else
  {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;
        if (dlen == 1) TFT_drawPixel(xs, y0, color);
        else TFT_drawFastHLine(xs, y0, dlen, color);
        dlen = 0; y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen) TFT_drawFastHLine(xs, y0, dlen, color);
  }

  ccbufSend(); // Flush buffer
}

// fill a rectangle
//------------------------------------------------------------------------------------
static void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // clipping
  if ((x > dispWin.x2) || (y > dispWin.y2)) return;

  if (x < dispWin.x1) x = dispWin.x1;
  if (y < dispWin.y1) y = dispWin.y1;

  if ((x + w) > dispWin.x2) w = dispWin.x2 - x;
  if ((y + h) > dispWin.y2) h = dispWin.y2 - y;
  if (w == 0) w = 1;
  if (h == 0) h = 1;

  initccbuf();
  _TFT_pushColorRep(x, y, x+w-1, y+h-1, color, (uint32_t)(h*w));
  ccbufSend();
}

//---------------------------------------------------------------------------------------
static void TFT_drawRect(uint16_t x1,uint16_t y1,uint16_t w,uint16_t h, uint16_t color) {
  initccbuf();
  TFT_drawFastHLine(x1,y1,w, color);
  TFT_drawFastVLine(x1+w-1,y1,h, color);
  TFT_drawFastHLine(x1,y1+h-1,w, color);
  TFT_drawFastVLine(x1,y1,h, color);
  ccbufSend(); // Flush buffer
}

// Draw a triangle
//------------------------------------------------------------------------------
static void TFT_drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
			     uint16_t x2, uint16_t y2, uint16_t color)
{
  TFT_drawLine(x0, y0, x1, y1, color);
  TFT_drawLine(x1, y1, x2, y2, color);
  TFT_drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
//-----------------------------------------------------------------------
static void TFT_fillTriangle(uint16_t x0, uint16_t y0,
				uint16_t x1, uint16_t y1,
				uint16_t x2, uint16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  initccbuf();

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    TFT_drawFastHLine(a, y0, b-a+1, color);
    ccbufSend(); // Flush buffer
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  int32_t
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    TFT_drawFastHLine(a, y, b-a+1, color);
  }
  
  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    TFT_drawFastHLine(a, y, b-a+1, color);
  }

  ccbufSend(); // Flush buffer
}

//----------------------------------------------------------------------------
static void TFT_drawCircle(int16_t x, int16_t y, int radius, uint16_t color) {
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x1 = 0;
  int y1 = radius;

  initccbuf();

  TFT_drawPixel(x, y + radius, color);
  TFT_drawPixel(x, y - radius, color);
  TFT_drawPixel(x + radius, y, color);
  TFT_drawPixel(x - radius, y, color);
  while(x1 < y1) {
    if (f >= 0) {
      y1--;
      ddF_y += 2;
      f += ddF_y;
    }
    x1++;
    ddF_x += 2;
    f += ddF_x;
    TFT_drawPixel(x + x1, y + y1, color);
    TFT_drawPixel(x - x1, y + y1, color);
    TFT_drawPixel(x + x1, y - y1, color);
    TFT_drawPixel(x - x1, y - y1, color);
    TFT_drawPixel(x + y1, y + x1, color);
    TFT_drawPixel(x - y1, y + x1, color);
    TFT_drawPixel(x + y1, y - x1, color);
    TFT_drawPixel(x - y1, y - x1, color);
  }
  ccbufSend(); // Flush buffer
}

//----------------------------------------------------------------------------
static void TFT_fillCircle(int16_t x, int16_t y, int radius, uint16_t color) {
  int x1,y1;

  initccbuf();
  for (y1=-radius; y1<=0; y1++) 
    for (x1=-radius; x1<=0; x1++)
      if (x1*x1+y1*y1 <= radius*radius) {
        TFT_drawFastHLine(x+x1, y+y1, 2*(-x1), color);
        TFT_drawFastHLine(x+x1, y-y1, 2*(-x1), color);
        break;
      }
  ccbufSend(); // Flush buffer
}

//==============================================================================
/**
 * bit-encoded bar position of all digits' bcd segments
 *
 *                   6
 * 		  +-----+
 * 		3 |  .	| 2
 * 		  +--5--+
 * 		1 |  .	| 0
 * 		  +--.--+
 * 		     4
 */
static const uint16_t font_bcd[] = {
  0x200, // 0010 0000 0000  // -
  0x080, // 0000 1000 0000  // . 
  0x06C, // 0100 0110 1100  // /, degree
  0x05f, // 0000 0101 1111, // 0 
  0x005, // 0000 0000 0101, // 1 
  0x076, // 0000 0111 0110, // 2 
  0x075, // 0000 0111 0101, // 3 
  0x02d, // 0000 0010 1101, // 4 
  0x079, // 0000 0111 1001, // 5 
  0x07b, // 0000 0111 1011, // 6 
  0x045, // 0000 0100 0101, // 7 
  0x07f, // 0000 0111 1111, // 8 
  0x07d, // 0000 0111 1101  // 9 
  0x900  // 1001 0000 0000  // :
};

//-------------------------------------------------------------------------------
static void barVert(int16_t x, int16_t y, int16_t w, int16_t l, uint16_t color) {
  TFT_fillTriangle(x+1, y+2*w, x+w, y+w+1, x+2*w-1, y+2*w, color);
  TFT_fillTriangle(x+1, y+2*w+l+1, x+w, y+3*w+l, x+2*w-1, y+2*w+l+1, color);
  TFT_fillRect(x, y+2*w+1, 2*w+1, l, color);
  if ((cfont.offset) && (color != _bg)) {
    TFT_drawTriangle(x+1, y+2*w, x+w, y+w+1, x+2*w-1, y+2*w, cfont.color);
    TFT_drawTriangle(x+1, y+2*w+l+1, x+w, y+3*w+l, x+2*w-1, y+2*w+l+1, cfont.color);
    TFT_drawRect(x, y+2*w+1, 2*w+1, l, cfont.color);
  }
}

//------------------------------------------------------------------------------
static void barHor(int16_t x, int16_t y, int16_t w, int16_t l, uint16_t color) {
  TFT_fillTriangle(x+2*w, y+2*w-1, x+w+1, y+w, x+2*w, y+1, color);
  TFT_fillTriangle(x+2*w+l+1, y+2*w-1, x+3*w+l, y+w, x+2*w+l+1, y+1, color);
  TFT_fillRect(x+2*w+1, y, l, 2*w+1, color);
  if ((cfont.offset) && (color != _bg)) {
    TFT_drawTriangle(x+2*w, y+2*w-1, x+w+1, y+w, x+2*w, y+1, cfont.color);
    TFT_drawTriangle(x+2*w+l+1, y+2*w-1, x+3*w+l, y+w, x+2*w+l+1, y+1, cfont.color);
    TFT_drawRect(x+2*w+1, y, l, 2*w+1, cfont.color);
  }
}

//------------------------------------------------------------------------------------------------
static void TFT_draw7seg(int16_t x, int16_t y, int8_t num, int16_t w, int16_t l, uint16_t color) {
  // @todo clipping
  if (num < 0x2D || num > 0x3A) return;
  
  int16_t c = font_bcd[num-0x2D];
  int16_t d = 2*w+l+1;
  
  //if (!_transparent) TFT_fillRect(x, y, (2 * (2 * w + 1)) + l, (3 * (2 * w + 1)) + (2 * l), _bg);

  if (!(c & 0x001)) barVert(x+d, y+d, w, l, _bg);
  if (!(c & 0x002)) barVert(x,   y+d, w, l, _bg);
  if (!(c & 0x004)) barVert(x+d, y, w, l, _bg);
  if (!(c & 0x008)) barVert(x,   y, w, l, _bg);
  if (!(c & 0x010)) barHor(x, y+2*d, w, l, _bg);
  if (!(c & 0x020)) barHor(x, y+d, w, l, _bg);
  if (!(c & 0x040)) barHor(x, y, w, l, _bg);

  //if (!(c & 0x080)) TFT_fillRect(x+(d/2), y+2*d, 2*w+1, 2*w+1, _bg);
  if (!(c & 0x100)) TFT_fillRect(x+(d/2), y+d+2*w+1, 2*w+1, l/2, _bg);
  if (!(c & 0x800)) TFT_fillRect(x+(d/2), y+(2*w)+1+(l/2), 2*w+1, l/2, _bg);
  //if (!(c & 0x200)) TFT_fillRect(x+2*w+1, y+d, l, 2*w+1, _bg);

  if (c & 0x001) barVert(x+d, y+d, w, l, color);               // down right
  if (c & 0x002) barVert(x,   y+d, w, l, color);               // down left
  if (c & 0x004) barVert(x+d, y, w, l, color);                 // up right
  if (c & 0x008) barVert(x,   y, w, l, color);                 // up left
  if (c & 0x010) barHor(x, y+2*d, w, l, color);                // down
  if (c & 0x020) barHor(x, y+d, w, l, color);                  // middle
  if (c & 0x040) barHor(x, y, w, l, color);                    // up
 
  if (c & 0x080) {
    TFT_fillRect(x+(d/2), y+2*d, 2*w+1, 2*w+1, color);         // low point
    if (cfont.offset) TFT_drawRect(x+(d/2), y+2*d, 2*w+1, 2*w+1, cfont.color);
  }
  if (c & 0x100) {
    TFT_fillRect(x+(d/2), y+d+2*w+1, 2*w+1, l/2, color);       // down middle point
    if (cfont.offset) TFT_drawRect(x+(d/2), y+d+2*w+1, 2*w+1, l/2, cfont.color);
  }
  if (c & 0x800) {
    TFT_fillRect(x+(d/2), y+(2*w)+1+(l/2), 2*w+1, l/2, color); // up middle point
    if (cfont.offset) TFT_drawRect(x+(d/2), y+(2*w)+1+(l/2), 2*w+1, l/2, cfont.color);
  }
  if (c & 0x200) {
    TFT_fillRect(x+2*w+1, y+d, l, 2*w+1, color);               // middle, minus
    if (cfont.offset) TFT_drawRect(x+2*w+1, y+d, l, 2*w+1, cfont.color);
  }
}
//==============================================================================


// return max width of the proportional font
//--------------------------------
static uint8_t getMaxWidth(void) {
  uint16_t tempPtr = 4; // point at first char data
  uint8_t cc,cw,ch,w = 0;
  do
  {
    cc = cfont.font[tempPtr++];
    tempPtr++;
    cw = cfont.font[tempPtr++];
    ch = cfont.font[tempPtr++];
    tempPtr += 2;
    if (cc != 0xFF) {
      if (cw != 0) {
        if (cw > w) w = cw;
        // packed bits
        tempPtr += (((cw * ch)-1) / 8) + 1;
      }
    }
  } while (cc != 0xFF);
  
  return w;
}

//--------------------------------------------------------
static int load_file_font(const char * fontfile, int info)
{
	if (userfont != NULL) {
		free(userfont);
		userfont = NULL;
	}

	// Open the file
	int ffd = file_open(fontfile, 0);
	if (ffd < 0) {
		vm_log_error("Error opening font file '%s'", fontfile);
		return 0;
	}

	// Get file size
	int fsize = file_size(ffd);
	if (fsize < 30) {
		vm_log_error("Error getting font file size");
		return 0;
	}

	userfont = malloc(fsize+4);
	if (userfont == NULL) {
		vm_log_error("Font memory allocation error");
		file_close(ffd);
		return 0;
	}

	int read = file_read(ffd, userfont, fsize);

	file_close(ffd);

	if (read != fsize) {
		vm_log_error("Font read error");
		free(userfont);
		userfont = NULL;
		return 0;
	}

	userfont[read] = 0;
	if (strstr(userfont+read-8, "RPH_font") == NULL) {
		vm_log_error("Font ID not found");
		free(userfont);
		userfont = NULL;
		return 0;
	}

	// Check size
	int size = 0;
	int numchar = 0;
	int width = userfont[0];
	int height = userfont[1];
	uint8_t first = 255;
	uint8_t last = 0;
	int offst = 0;
	int pminwidth = 255;
	int pmaxwidth = 0;

	if (width != 0) {
		// Fixed font
		numchar = userfont[3];
		first = userfont[2];
		last = first + numchar - 1;
		size = ((width * height * numchar) / 8) + 4;
	}
	else {
		// Proportional font
		size = 4; // point at first char data
		uint8_t charCode;
		int charwidth;

		do {
		    charCode = userfont[size];
		    charwidth = userfont[size+2];

		    if (charCode != 0xFF) {
		    	numchar++;
		    	if (charwidth != 0) size += ((((charwidth * userfont[size+3])-1) / 8) + 7);
		    	else size += 6;

		    	if (info) {
	    			if (charwidth > pmaxwidth) pmaxwidth = charwidth;
	    			if (charwidth < pminwidth) pminwidth = charwidth;
	    			if (charCode < first) first = charCode;
	    			if (charCode > last) last = charCode;
	    		}
		    }
		    else size++;
		  } while ((size < (read-8)) && (charCode != 0xFF));
	}

	if (size != (read-8)) {
		vm_log_error("Font size error: found %d expected %d)", size, (read-8));
		free(userfont);
		userfont = NULL;
		return 0;
	}

	if (info) {
		if (width != 0) {
			printf("Fixed width font:\n");
			printf("size: %d  width: %d  height: %d  characters: %d (%d~%d)\n", size, width, height, numchar, first, last);
		}
		else {
			printf("Proportional font:\n");
			printf("size: %d  width: %d~%d  height: %d  characters: %d (%d~%d)\n", size, pminwidth, pmaxwidth, height, numchar, first, last);
		}
	}
	return 1;
}

//----------------------------------------------------------
static void TFT_setFont(uint8_t font, const char *font_file)
{
  cfont.font = NULL;

  if (font == FONT_7SEG) {
    cfont.bitmap = 2;
    cfont.x_size = 24;
    cfont.y_size = 6;
    cfont.offset = 0;
    cfont.color  = _fg;
  }
  else {
	  if (font == USER_FONT) {
		  if (load_file_font(font_file, 0) == 0) cfont.font = lcd_DefaultFont;
		  else cfont.font = userfont;
	  }
	  else cfont.font = lcd_DefaultFont;

	  cfont.bitmap = 1;
	  cfont.x_size = cfont.font[0];
	  cfont.y_size = cfont.font[1];
	  cfont.offset = cfont.font[2];
	  if (cfont.x_size != 0) cfont.numchars = cfont.font[3];
	  else cfont.numchars = getMaxWidth();
  }
}

// private method to return the Glyph data for an individual character in the proportional font
//--------------------------------
static int getCharPtr(uint8_t c) {
  uint16_t tempPtr = 4; // point at first char data
  
  do {
    fontChar.charCode = cfont.font[tempPtr++];
    fontChar.adjYOffset = cfont.font[tempPtr++];
    fontChar.width = cfont.font[tempPtr++];
    fontChar.height = cfont.font[tempPtr++];
    fontChar.xOffset = cfont.font[tempPtr++];
    fontChar.xOffset = fontChar.xOffset < 0x80 ? fontChar.xOffset : (0x100 - fontChar.xOffset);
    fontChar.xDelta = cfont.font[tempPtr++];
    
    if (c != fontChar.charCode && fontChar.charCode != 0xFF) {
      if (fontChar.width != 0) {
        // packed bits
        tempPtr += (((fontChar.width * fontChar.height)-1) / 8) + 1;
      }
    }
  } while (c != fontChar.charCode && fontChar.charCode != 0xFF);
  
  fontChar.dataPtr = tempPtr;
  if (c == fontChar.charCode) {
    if (_forceFixed > 0) {
      // fix width & offset for forced fixed width
      fontChar.xDelta = cfont.numchars;
      fontChar.xOffset = (fontChar.xDelta - fontChar.width) / 2;
    }
  }
  
  if (fontChar.charCode != 0xFF) return 1;
  else return 0;
}

// print rotated proportional character
// character is already in fontChar
//--------------------------------------------------------------
static int rotatePropChar(int x, int y, int offset) {
  uint8_t ch;
  double radian = rotation * 0.0175;  
  float cos_radian = cos(radian);
  float sin_radian = sin(radian);
   
  initccbuf();
  uint8_t mask = 0x80;
  for (int j=0; j < fontChar.height; j++) {
    for (int i=0; i < fontChar.width; i++) {
      if (((i + (j*fontChar.width)) % 8) == 0) {
        mask = 0x80;
        ch = cfont.font[fontChar.dataPtr++];
      }
      
      int newX = (int)(x + (((offset + i) * cos_radian) - ((j+fontChar.adjYOffset)*sin_radian)));
      int newY = (int)(y + (((j+fontChar.adjYOffset) * cos_radian) + ((offset + i) * sin_radian)));
      
      if ((ch & mask) != 0) TFT_drawPixel(newX,newY,_fg);
      else if (!_transparent) TFT_drawPixel(newX,newY,_bg);
      
      mask >>= 1;
    }
  }
  ccbufSend(); // Flush buffer

  return fontChar.xDelta+1;
}

// print non-rotated proportional character
// character is already in fontChar
//---------------------------------------------------------
static int printProportionalChar(int x, int y) {
  uint8_t i,j,ch;
  uint16_t cx,cy; 
  
  // fill background if not transparent background
  if (!_transparent) {
    TFT_fillRect(x, y, fontChar.xDelta+1, cfont.y_size, _bg);
  }
  
  // draw Glyph
  initccbuf();

  uint8_t mask = 0x80;
  for (j=0; j < fontChar.height; j++) {
    for (i=0; i < fontChar.width; i++) {
      if (((i + (j*fontChar.width)) % 8) == 0) {
        mask = 0x80;
        ch = cfont.font[fontChar.dataPtr++];
      }
      
      if ((ch & mask) !=0) {
        cx = (uint16_t)(x+fontChar.xOffset+i);
        cy = (uint16_t)(y+j+fontChar.adjYOffset);
        _TFT_pushColorRep(cx, cy, cx, cy, _fg, 1);
      }
      mask >>= 1;
    }
  }
  ccbufSend(); // Flush buffer
    
  return fontChar.xDelta;
}

// non-rotated fixed width character
//----------------------------------------------
static void printChar(uint8_t c, int x, int y) {
  uint8_t i,j,ch,fz,mask;
  uint16_t k,temp,cx,cy; 

  // fz = bytes per char row
  fz = cfont.x_size/8;
  if (cfont.x_size % 8) fz++;
  
  // get char address
  temp = ((c-cfont.offset)*((fz)*cfont.y_size))+4;

  // fill background if not transparent background
  if (!_transparent) {
    TFT_fillRect(x, y, cfont.x_size, cfont.y_size, _bg);
  }

  initccbuf();
  for (j=0; j<cfont.y_size; j++) {
    for (k=0; k < fz; k++) {
      ch = cfont.font[temp+k]; 
      mask=0x80;
      for (i=0; i<8; i++) {   
        if ((ch & mask) !=0) {
          cx = (uint16_t)(x+i+(k*8));
          cy = (uint16_t)(y+j);
          _TFT_pushColorRep(cx,cy,cx,cy,_fg, 1);
        }
        mask >>= 1;
      }
    }
    temp+=(fz);
  }
  ccbufSend(); // Flush buffer
}

// rotated fixed width character
//--------------------------------------------------------
static void rotateChar(uint8_t c, int x, int y, int pos) {
  uint8_t i,j,ch,fz,mask;
  uint16_t temp; 
  int newx,newy;
  double radian = rotation*0.0175;
  float cos_radian = cos(radian);
  float sin_radian = sin(radian);
  int zz;

  if( cfont.x_size < 8 ) fz = cfont.x_size;
  else fz = cfont.x_size/8;	
  temp=((c-cfont.offset)*((fz)*cfont.y_size))+4;

  initccbuf();
  for (j=0; j<cfont.y_size; j++) {
    for (zz=0; zz<(fz); zz++) {
      ch = cfont.font[temp+zz];
      mask = 0x80;
      for (i=0; i<8; i++) {
        newx=(int)(x+(((i+(zz*8)+(pos*cfont.x_size))*cos_radian)-((j)*sin_radian)));
        newy=(int)(y+(((j)*cos_radian)+((i+(zz*8)+(pos*cfont.x_size))*sin_radian)));

        if ((ch & mask) != 0) TFT_drawPixel(newx,newy,_fg);
        else if (!_transparent) TFT_drawPixel(newx,newy,_bg);
        mask >>= 1;
      }
    }
    temp+=(fz);
  }
  ccbufSend(); // Flush buffer
  // calculate x,y for the next char
  TFT_X = (int)(x + ((pos+1) * cfont.x_size * cos_radian));
  TFT_Y = (int)(y + ((pos+1) * cfont.x_size * sin_radian));
}

// returns the string width in pixels. Useful for positions strings on the screen.
//----------------------------------
static int getStringWidth(char* str) {
  
  // is it 7-segment font?
  if (cfont.bitmap == 2) return ((2 * (2 * cfont.y_size + 1)) + cfont.x_size) * strlen(str);

  // is it a fixed width font?
  if (cfont.x_size != 0) return strlen(str) * cfont.x_size;
  else {
    // calculate the string width
    char* tempStrptr = str;
    int strWidth = 0;
    while (*tempStrptr != 0) {
      if (getCharPtr(*tempStrptr++)) strWidth += (fontChar.xDelta + 1);
    }
    return strWidth;
  }
}

//---------------------------------------------
static void TFT_print(char *st, int x, int y) {
  int stl, i, tmpw, tmph;
  uint8_t ch;
  
  if (cfont.bitmap == 0) return; // wrong font selected
  
  // for rotated string x cannot be RIGHT or CENTER
  if ((rotation != 0) && (x < -2)) return;
  
  stl = strlen(st); // number of characters in string to print
  
  // set CENTER or RIGHT possition
  tmpw = getStringWidth(st);
  if (x==RIGHT) x = dispWin.x2 - tmpw - 1;
  if (x==CENTER) x = (dispWin.x2 - tmpw - 1)/2;
  if (x < dispWin.x1) x = dispWin.x1;
  if (y < dispWin.y1) y = dispWin.y1;

  TFT_X = x;
  TFT_Y = y;
  int offset = TFT_OFFSET;
  
  tmph = cfont.y_size;
  // for non-proportional fonts, char width is the same for all chars
  if (cfont.x_size != 0) {
    if (cfont.bitmap == 2) { // 7-segment font
      tmpw = (2 * (2 * cfont.y_size + 1)) + cfont.x_size;        // character width
      tmph = (3 * (2 * cfont.y_size + 1)) + (2 * cfont.x_size);  // character height
    }
    else tmpw = cfont.x_size;
  }
  if ((TFT_Y + tmph - 1) > dispWin.y2) return;
    
  for (i=0; i<stl; i++) {
    ch = *st++; // get char
    
    if (cfont.x_size == 0) {
      // for proportional font get char width
      if (getCharPtr(ch)) tmpw = fontChar.xDelta;
    }
    
    if (ch == 0x0D) { // === '\r', erase to eol ====
      if ((!_transparent) && (rotation==0)) TFT_fillRect(TFT_X, TFT_Y,  dispWin.x2+1-TFT_X, tmph, _bg);
    }
    
    else if (ch == 0x0A) { // ==== '\n', new line ====
      if (cfont.bitmap == 1) {
        TFT_Y += tmph;
        if (TFT_Y > (dispWin.y2-tmph)) break;
        TFT_X = dispWin.x1;
      }
    }
    
    else { // ==== other characters ====
      // check if character can be displayed in the current line
      if ((TFT_X+tmpw) > (dispWin.x2+1)) {
        if (_wrap == 0) break;
        TFT_Y += tmph;
        if (TFT_Y > (dispWin.y2-tmph)) break;
        TFT_X = dispWin.x1;
      }
      
      // Let's print the character
      if (cfont.x_size == 0) {
        // == proportional font
        if (rotation==0) {
          TFT_X += printProportionalChar(TFT_X, TFT_Y)+1;
        }
        else {
          offset += rotatePropChar(x, y, offset);
          TFT_OFFSET = offset;
        }
      }
      // == fixed font
      else {
        if (cfont.bitmap == 1) {
          if ((ch < cfont.offset) || ((ch-cfont.offset) > cfont.numchars)) ch = cfont.offset;
          if (rotation==0) {
            printChar(ch, TFT_X, TFT_Y);
            TFT_X += tmpw;
          }
          else rotateChar(ch, x, y, i);
        }
        else if (cfont.bitmap == 2) { // 7-seg font
          TFT_draw7seg(TFT_X, TFT_Y, ch, cfont.y_size, cfont.x_size, _fg);
          TFT_X += (tmpw + 2);
        }
      }
    }
  }
}

/********************************************************************
*********************** Service functions ***************************
*********************************************************************/

//------------------------------------------
static void TFT_fillScreen(uint16_t color) {
  //TFT_fillRect(0, 0,  _width, _height, color);
  initccbuf();
  _TFT_pushColorRep(0, 0, _width-1, _height-1, color, (uint32_t)(_height*_width));
  ccbufSend();
}

// Change the screen rotation.
// Input: m new rotation value (0 to 3)
//--------------------------------------
static void TFT_setRotation(uint8_t m) {
  uint8_t rotation = m & 3; // can't be higher than 3
  uint8_t send = 1;
  uint8_t madctl = 0;

  if (m > 3) madctl = (m & 0xF8); // for testing, manually set MADCTL register
  else {
	  orientation = m;
	  if (TFT_type == 0) {
		if ((rotation & 1)) {
			_width  = ST7735_HEIGHT;
			_height = ST7735_WIDTH;
		}
		else {
			_width  = ST7735_WIDTH;
			_height = ST7735_HEIGHT;
		}
		switch (rotation) {
		  case PORTRAIT:
			madctl = (MADCTL_MX | MADCTL_MY | MADCTL_RGB);
			break;
		  case LANDSCAPE:
			madctl = (MADCTL_MY | MADCTL_MV | MADCTL_RGB);
			break;
		  case PORTRAIT_FLIP:
			madctl = (MADCTL_RGB);
			break;
		  case LANDSCAPE_FLIP:
			madctl = (MADCTL_MX | MADCTL_MV | MADCTL_RGB);
			break;
		}
	  }
	  else if (TFT_type == 1) {
		if ((rotation & 1)) {
			_width  = ILI9341_HEIGHT;
			_height = ILI9341_WIDTH;
		}
		else {
			_width  = ILI9341_WIDTH;
			_height = ILI9341_HEIGHT;
		}
		switch (rotation) {
		  case PORTRAIT:
			madctl = (MADCTL_MX | MADCTL_BGR);
			break;
		  case LANDSCAPE:
			madctl = (MADCTL_MV | MADCTL_BGR);
			break;
		  case PORTRAIT_FLIP:
			madctl = (MADCTL_MY | MADCTL_BGR);
			break;
		  case LANDSCAPE_FLIP:
			madctl = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			break;
		}
	  }
	  else if (TFT_type == 2) {
		_width  = XADOW_WIDTH;
		_height = XADOW_HEIGHT;
		switch (rotation) {
		  case PORTRAIT:
			madctl = 0;
			break;
		  case LANDSCAPE:
			madctl = (MADCTL_MV | MADCTL_MX);
			break;
		  case PORTRAIT_FLIP:
			madctl = 0;
			rotation = PORTRAIT;
			break;
		  case LANDSCAPE_FLIP:
			madctl = (MADCTL_MV | MADCTL_MX);
			rotation = LANDSCAPE;
			break;
		}
	  }
	  else send = 0;
  }

  if (send) {
	  initccbuf();
	  TFT_sendCmd(TFT_MADCTL, 1);
	  ccbufPushByte(madctl);
	  ccbufSend();
  }

  dispWin.x1 = 0;
  dispWin.y1 = 0;
  dispWin.x2 = _width-1;
  dispWin.y2 = _height-1;

}

// Send the command to invert all of the colors.
// Input: i 0 to disable inversion; non-zero to enable inversion
//-------------------------------------------------
static void TFT_invertDisplay(const uint8_t mode) {
  initccbuf();
  if ( mode == INVERT_ON ) TFT_sendCmd(TFT_INVONN,0);
  else TFT_sendCmd(TFT_INVOFF,0);
  ccbufSend();
}

//--------------------------------------------------
static uint8_t checkParam(uint8_t n, lua_State* L) {
  if (lua_gettop(L) < n) {
    l_message( NULL, "not enough parameters" );
    return 1;
  }
  return 0;
}

/**
 * Converts the components of a color, as specified by the HSB
 * model, to an equivalent set of values for the default RGB model.
 * The _sat and _brightnesscomponents
 * should be floating-point values between zero and one (numbers in the range 0.0-1.0)
 * The _hue component can be any floating-point number.  The floor of this number is
 * subtracted from it to create a fraction between 0 and 1.
 * This fractional number is then multiplied by 360 to produce the hue
 * angle in the HSB color model.
 * The integer that is returned by HSBtoRGB encodes the
 * value of a color in bits 0-15 of an integer value
*/
//-------------------------------------------------------------------
static uint16_t HSBtoRGB(float _hue, float _sat, float _brightness) {
 float red = 0.0;
 float green = 0.0;
 float blue = 0.0;
 
 if (_sat == 0.0) {
   red = _brightness;
   green = _brightness;
   blue = _brightness;
 } else {
   if (_hue == 360.0) {
     _hue = 0;
   }

   int slice = (int)(_hue / 60.0);
   float hue_frac = (_hue / 60.0) - slice;

   float aa = _brightness * (1.0 - _sat);
   float bb = _brightness * (1.0 - _sat * hue_frac);
   float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));
   
   switch(slice) {
     case 0:
         red = _brightness;
         green = cc;
         blue = aa;
         break;
     case 1:
         red = bb;
         green = _brightness;
         blue = aa;
         break;
     case 2:
         red = aa;
         green = _brightness;
         blue = cc;
         break;
     case 3:
         red = aa;
         green = bb;
         blue = _brightness;
         break;
     case 4:
         red = cc;
         green = aa;
         blue = _brightness;
         break;
     case 5:
         red = _brightness;
         green = aa;
         blue = bb;
         break;
     default:
         red = 0.0;
         green = 0.0;
         blue = 0.0;
         break;
   }
 }

 uint8_t ired = (uint8_t)(red * 31.0);
 uint8_t igreen = (uint8_t)(green * 63.0);
 uint8_t iblue = (uint8_t)(blue * 31.0);
 
 return (uint16_t)((ired << 11) | (igreen << 5) | (iblue & 0x001F));
}

//-------------------------------------------------
static uint16_t getColor(lua_State* L, uint8_t n) {
  if( lua_istable( L, n ) ) {
    uint8_t i;
    uint8_t cl[3];
    uint8_t datalen = lua_objlen( L, n );
    if (datalen < 3) return _fg;
    
    for( i = 0; i < 3; i++ )
    {
      lua_rawgeti( L, n, i + 1 );
      cl[i] = ( int )luaL_checkinteger( L, -1 );
      lua_pop( L, 1 );
    }
    if (cl[0] > 0x1F) cl[0] = 0x1F;
    if (cl[1] > 0x3F) cl[1] = 0x3F;
    if (cl[2] > 0x1F) cl[2] = 0x1F;
    return (cl[0] << 11) | (cl[1] << 5) | cl[2];
  }
  else {
    return luaL_checkinteger( L, n );
  }
}

//--------------------------
static void _initvar(void) {
  rotation = 0;
  _wrap = 0;
  _transparent = 0;
  _forceFixed = 0;
  dispWin.x2 = _width-1;
  dispWin.y2 = _height-1;
  dispWin.x1 = 0;
  dispWin.y1 = 0;
}

//====================================
static int _init_xadow( lua_State* L )
{
    lcd_st7789s_init();
    lcd_init_st7789s(TFT_BLACK, NULL);

	if (tp_initialized == 0) {
		if (tp_gt9xx_init() < 0) {
			vm_log_error("Touch panel init error");
			tp_initialized = 0;
			g_shell_result = -1;
		}
		else {
			vm_touch_register_event_callback(handle_touch_event);
			tp_initialized = 1;
			g_shell_result = 0;
		}
	}

	vm_signal_post(g_shell_signal);
    return 0;
}

//==========================
int lcd_init( lua_State* L )
{
  uint8_t typ = luaL_checkinteger( L, 1);

  if (typ < 8) {
	  if ((g_spi_handle == VM_DCL_HANDLE_INVALID) || (g_spi.g_spi_dc_handle == VM_DCL_HANDLE_INVALID) || (g_spi.g_spi_cs_handle == VM_DCL_HANDLE_INVALID)) {
		vm_log_error("SPI not yet initialized");
		return 0;
	  }
  }
  
  TFT_setFont(DEFAULT_FONT, NULL);
  _fg = TFT_GREEN;
  _bg = TFT_BLACK;
  
  TFT_type = 0;  // ST7735 type display
  if (typ == 0) ST7735_initB();
  else if (typ == 1) ST7735_initR(INITR_BLACKTAB);
  else if (typ == 2) ST7735_initR(INITR_GREENTAB);
  else if (typ == 3) {
	// ILI7341 type display
    TFT_type = 1;
    commandList(ILI7341_init);
  }
  else if ((typ == 8) || (typ == 9)) {
	int tmo = 0;
	while (g_graphics_ready == 0) {
		vm_thread_sleep(20);
		tmo++;
		if (tmo > 25) break;
	}
	if (tmo <= 25) {
		// Xadow sitronix ST7789S type display
		TFT_type = 2;
		_TS_VER_ = (typ & 1);
		_VM_LCD_DRIVER_ = 0;

		remote_CCall(L, &_init_xadow);
		if (g_shell_result < 0) return 0;
		_set_backlight(50);
	}
	else {
		vm_log_error("graphics system not ready");
		TFT_type = -1;
	}
  }
  else {
	vm_log_error("unknown lcd type");
	return 0;
  }
  
  typ = PORTRAIT_FLIP;  
  if (lua_gettop(L) > 1) {
    typ = luaL_checkinteger( L, 2 ) % 4;
  }
  TFT_setRotation(typ);
  TFT_fillScreen(TFT_BLACK);
  _initvar();
  
  return 0;
}

//==================================
static int lcd_gettype(lua_State *L)
{
    lua_pushinteger( L, TFT_type);
    return 1;
}

//=========================================
static int lcd_set_brightness(lua_State *L)
{
    int brightness = luaL_checkinteger(L, 1);
    if (TFT_type == 2) {
		if (brightness < 0) brightness = 0;
		if (brightness > 100) brightness = 100;

		_set_backlight(brightness);
    }

    return 0;
}

//======================================
static int lcd_setorient( lua_State* L )
{
  orientation = luaL_checkinteger( L, 1 );
  TFT_setRotation(orientation);
  TFT_fillScreen(_bg);
  
  return 0;
}

//==================================
static int lcd_clear( lua_State* L )
{
  uint16_t color = TFT_BLACK;
  
  if (lua_gettop(L) > 0) color = getColor( L, 1 );

  TFT_fillScreen(color);
  _bg = color;
  _initvar();

  return 0;
}

//===================================
static int lcd_invert( lua_State* L )
{
  uint16_t inv = luaL_checkinteger( L, 1 );
  TFT_invertDisplay(inv);
  return 0;
}

//====================================
static int lcd_setwrap( lua_State* L )
{
  _wrap = luaL_checkinteger( L, 1 );
  return 0;
}

//======================================
static int lcd_settransp( lua_State* L )
{
  _transparent = luaL_checkinteger( L, 1 );
  return 0;
}

//===================================
static int lcd_setrot( lua_State* L )
{
  rotation = luaL_checkinteger( L, 1 );
  return 0;
}

//===================================
static int lcd_setfixed( lua_State* L )
{
  _forceFixed = luaL_checkinteger( L, 1 );
  return 0;
}

//====================================
static int lcd_setfont( lua_State* L )
{
  if (checkParam(1, L)) return 0;
  
  uint8_t fnt = DEFAULT_FONT;
  size_t fnlen = 0;
  const char* fname = NULL;

  if (lua_type(L, 1) == LUA_TNUMBER) {
	  fnt = luaL_checkinteger( L, 1 );
  }
  else if (lua_type(L, 1) == LUA_TSTRING) {
	  fnt = USER_FONT;
	  fname = lua_tolstring(L, -1, &fnlen);
  }

  TFT_setFont(fnt, fname);

  if (fnt == FONT_7SEG) {
    if (lua_gettop(L) > 2) {
      uint8_t l = luaL_checkinteger( L, 2 );
      uint8_t w = luaL_checkinteger( L, 3 );
      if (l < 6) l = 6;
      if (l > 40) l = 40;
      if (w < 1) w = 1;
      if (w > (l/2)) w = l/2;
      if (w > 12) w = 12;
      cfont.x_size = l;
      cfont.y_size = w;
      cfont.offset = 0;
      cfont.color  = _fg;
      if (lua_gettop(L) > 3) {
        if (w > 1) {
          cfont.offset = 1;
          cfont.color  = getColor( L, 4 );
        }
      }
    }
    else {  // default size
      cfont.x_size = 12;
      cfont.y_size = 2;
    }
  }
  return 0;
}


//========================================
static int compile_font_file(lua_State* L)
{
	char outfile[128] = {'\0'};
	size_t len;

	const char *fontfile = luaL_checklstring( L, 1, &len );

	// check here that filename end with ".c".
	if ((len < 3) || (len > 125) || (strcmp(fontfile + len - 2, ".c") != 0)) return luaL_error(L, "not a .c file");

	sprintf(outfile, "%s", fontfile);
	sprintf(outfile+strlen(outfile)-1, "fon");

	// Open the source file
	int ffd = file_open(fontfile, 0);
	if (ffd < 0) return luaL_error(L, "error opening source file");

	// Open the font file
	int ffd_out = file_open(outfile, O_CREAT);
	if (ffd_out < 0) {
		file_close(ffd);
		return luaL_error(L, "error opening destination file");
	}

	// Get file size
	int fsize = file_size(ffd);
	if (fsize <= 0) {
		file_close(ffd);
		file_close(ffd_out);
		return luaL_error(L, "error getting source file size");
	}

	char *sourcebuf = malloc(fsize+4);
	if (sourcebuf == NULL) {
		file_close(ffd);
		file_close(ffd_out);
		return luaL_error(L, "memory allocation error");
	}
	char *fbuf = sourcebuf;

	int fread = file_read(ffd, fbuf, fsize);
	file_close(ffd);

	if (fread != fsize) {
		free(fbuf);
		file_close(ffd_out);
		return luaL_error(L, "error reading from source file");
	}

	*(fbuf+fread) = '\0';

	fbuf = strchr(fbuf, '{');			// beginning of font data
	char *fend = strstr(fbuf, "};");	// end of font data

	if ((fbuf == NULL) || (fend == NULL) || ((fend-fbuf) < 22)) {
		free(fbuf);
		file_close(ffd_out);
		return luaL_error(L, "wrong source file format");
	}

	fbuf++;
	*fend = '\0';
	char hexstr[5] = {'\0'};
	int lastline = 0;

	fbuf = strstr(fbuf, "0x");
	int size = 0;
	uint8_t xsize, ysize;
	char *nextline;
	char *numptr;

	int bptr = 0;

	while ((fbuf != NULL) && (fbuf < fend) && (lastline == 0)) {
		nextline = strchr(fbuf, '\n'); // beginning of the next line
		if (nextline == NULL) {
			nextline = fend-1;
			lastline++;
		}
		else nextline++;

		while (fbuf < nextline) {
			numptr = strstr(fbuf, "0x");
			if ((numptr == NULL) || ((fbuf+4) > nextline)) numptr = strstr(fbuf, "0X");
			if ((numptr != NULL) && ((numptr+4) <= nextline)) {
				fbuf = numptr;
				if (bptr >= 128) {
					// buffer full, write to file
					if (file_write(ffd_out, outfile, 128) != 128) goto error;
					bptr = 0;
					if (size == 0) {
						xsize = outfile[0];
						ysize = outfile[1];
					}
					size += 128;
				}
				memcpy(hexstr, fbuf, 4);
				hexstr[4] = 0;
				outfile[bptr++] = (uint8_t)strtol(hexstr, NULL, 0);
				fbuf += 4;
			}
			else fbuf = nextline;
		}
		fbuf = nextline;
	}

	if (bptr > 0) {
		if (size == 0) {
			xsize = outfile[0];
			ysize = outfile[1];
		}
		size += bptr;
		if (file_write(ffd_out, outfile, bptr) != bptr)  goto error;
	}

	// write font ID
	sprintf(outfile, "RPH_font");
	if (file_write(ffd_out, outfile, 8) != 8)  goto error;

	file_flush(ffd_out);
	file_close(ffd_out);

	free(sourcebuf);

	// === Test compiled font ===
	sprintf(outfile, "%s", fontfile);
	sprintf(outfile+strlen(outfile)-1, "fon");

	uint8_t *uf = userfont; // save userfont pointer
	userfont = NULL;
	if (load_file_font(outfile, 1) == 0) printf("Error compiling file!\n");
	else {
		free(userfont);
		printf("File compiled successfully.\n");
	}
	userfont = uf; // restore userfont

	return 0;

error:
	file_close(ffd_out);
	free(sourcebuf);
	return luaL_error(L, "error writing to destination file");

}

//========================================
static int lcd_getfontsize( lua_State* L )
{
  if (cfont.bitmap == 1) {
    if (cfont.x_size != 0) lua_pushinteger( L, cfont.x_size );
    else lua_pushinteger( L, getMaxWidth() );
    lua_pushinteger( L, cfont.y_size );
  }
  else if (cfont.bitmap == 2) {
    lua_pushinteger( L, (2 * (2 * cfont.y_size + 1)) + cfont.x_size );
    lua_pushinteger( L, (3 * (2 * cfont.y_size + 1)) + (2 * cfont.x_size) );
  }
  else {
    lua_pushinteger( L, 0);
    lua_pushinteger( L, 0);
  }
  return 2;
}

//==========================================
static int lcd_getscreensize( lua_State* L )
{
  lua_pushinteger( L, _width);
  lua_pushinteger( L, _height);
  return 2;
}

//==========================================
static int lcd_getfontheight( lua_State* L )
{
  if (cfont.bitmap == 1) {
	// Bitmap font
    lua_pushinteger( L, cfont.y_size );
  }
  else if (cfont.bitmap == 2) {
	// 7-segment font
    lua_pushinteger( L, (3 * (2 * cfont.y_size + 1)) + (2 * cfont.x_size) );
  }
  else {
    lua_pushinteger( L, 0);
  }
  return 1;
}

//===============================
static int lcd_on( lua_State* L )
{
  initccbuf();
  TFT_sendCmd(TFT_DISPON,0);
  ccbufSend();
  return 0;
}

//================================
static int lcd_off( lua_State* L )
{
  initccbuf();
  TFT_sendCmd(TFT_DISPOFF,0);
  ccbufSend();
  return 0;
}

//=====================================
static int lcd_setcolor( lua_State* L )
{
  if (checkParam(1, L)) return 0;
  
  _fg = getColor( L, 1 );
  if (lua_gettop(L) > 1) _bg = getColor( L, 2 );
  return 0;
}

//=======================================
static int lcd_setclipwin( lua_State* L )
{
  if (checkParam(4, L)) return 0;
  
  dispWin.x1 = luaL_checkinteger( L, 1 );
  dispWin.y1 = luaL_checkinteger( L, 2 );
  dispWin.x2 = luaL_checkinteger( L, 3 );
  dispWin.y2 = luaL_checkinteger( L, 4 );

  if (dispWin.x2 >= _width) dispWin.x2 = _width-1;
  if (dispWin.y2 >= _height) dispWin.y2 = _height-1;
  if (dispWin.x1 > dispWin.x2) dispWin.x1 = dispWin.x2;
  if (dispWin.y1 > dispWin.y2) dispWin.y1 = dispWin.y2;
  
  return 0;
}

//=========================================
static int lcd_resetclipwin( lua_State* L )
{
  dispWin.x2 = _width-1;
  dispWin.y2 = _height-1;
  dispWin.x1 = 0;
  dispWin.y1 = 0;
  
  return 0;
}

//=====================================
static int lcd_HSBtoRGB( lua_State* L )
{
  float hue = luaL_checknumber(L, 1);
  float sat = luaL_checknumber(L, 2);
  float bri = luaL_checknumber(L, 3);
  
  lua_pushinteger(L, HSBtoRGB(hue, sat, bri));
    
  return 1;
}

//=====================================
static int lcd_putpixel( lua_State* L )
{
  if (checkParam(2, L)) return 0;
  
  uint16_t x = luaL_checkinteger( L, 1 );
  uint16_t y = luaL_checkinteger( L, 2 );
  uint16_t color = _fg;
  
  if (lua_gettop(L) > 2) color = getColor( L, 3 );

  initccbuf();
  TFT_drawPixel(x,y,color);
  ccbufSend(); // Flush buffer
  
  return 0;
}

//=================================
static int lcd_line( lua_State* L )
{
  if (checkParam(4, L)) return 0;
  
  uint16_t color = _fg;
  if (lua_gettop(L) > 4) color = getColor( L, 5 );
  uint16_t x0 = luaL_checkinteger( L, 1 );
  uint16_t y0 = luaL_checkinteger( L, 2 );
  uint16_t x1 = luaL_checkinteger( L, 3 );
  uint16_t y1 = luaL_checkinteger( L, 4 );
  TFT_drawLine(x0,y0,x1,y1,color);
  return 0;
}

//=================================
static int lcd_rect( lua_State* L )
{
  if (checkParam(5, L)) return 0;
  
  uint16_t fillcolor = _bg;
  if (lua_gettop(L) > 5) fillcolor = getColor( L, 6 );
  uint16_t x = luaL_checkinteger( L, 1 );
  uint16_t y = luaL_checkinteger( L, 2 );
  uint16_t w = luaL_checkinteger( L, 3 );
  uint16_t h = luaL_checkinteger( L, 4 );
  uint16_t color = getColor( L, 5 );
  if (lua_gettop(L) > 5) TFT_fillRect(x,y,w,h,fillcolor);
  if (fillcolor != color) TFT_drawRect(x,y,w,h,color);
  return 0;
}

//=================================
static int lcd_circle( lua_State* L )
{
  if (checkParam(4, L)) return 0;
  
  uint16_t fillcolor = _bg;
  if (lua_gettop(L) > 4) fillcolor = getColor( L, 5 );
  uint16_t x = luaL_checkinteger( L, 1 );
  uint16_t y = luaL_checkinteger( L, 2 );
  uint16_t r = luaL_checkinteger( L, 3 );
  uint16_t color = getColor( L, 4 );
  if (lua_gettop(L) > 4) TFT_fillCircle(x,y,r,fillcolor);
  if (fillcolor != color) TFT_drawCircle(x,y,r,color);
  return 0;
}

//=====================================
static int lcd_triangle( lua_State* L )
{
  if (checkParam(7, L)) return 0;
  
  uint16_t fillcolor = _bg;
  if (lua_gettop(L) > 7) fillcolor = getColor( L, 8 );
  uint16_t x0 = luaL_checkinteger( L, 1 );
  uint16_t y0 = luaL_checkinteger( L, 2 );
  uint16_t x1 = luaL_checkinteger( L, 3 );
  uint16_t y1 = luaL_checkinteger( L, 4 );
  uint16_t x2 = luaL_checkinteger( L, 5 );
  uint16_t y2 = luaL_checkinteger( L, 6 );
  uint16_t color = getColor( L, 7 );
  if (lua_gettop(L) > 7) TFT_fillTriangle(x0,y0,x1,y1,x2,y2,fillcolor);
  if (fillcolor != color) TFT_drawTriangle(x0,y0,x1,y1,x2,y2,color);
  return 0;
}

//lcd.write(x,y,string|intnum|{floatnum,dec},...)
//==================================
static int lcd_write( lua_State* L )
{
  if (checkParam(3, L)) return 0;

  const char* buf;
  char tmps[16];
  size_t len;
  uint8_t numdec = 0;
  uint8_t argn = 0;
  float fnum;
  
  int x = luaL_checkinteger( L, 1 );
  int y = luaL_checkinteger( L, 2 );
  if ((x != LASTX) || (y != LASTY)) TFT_OFFSET = 0;
  if (x == LASTX) x = TFT_X;
  if (y == LASTY) y = TFT_Y;
  
  for( argn = 3; argn <= lua_gettop( L ); argn++ )
  {
    if ( lua_type( L, argn ) == LUA_TNUMBER )
    { // write integer number
      len = lua_tointeger( L, argn );
      sprintf(tmps,"%d",len);
      TFT_print(&tmps[0], x, y);
      x = TFT_X;
      y = TFT_Y;
    }
    else if ( lua_type( L, argn ) == LUA_TTABLE ) {
      if (lua_objlen( L, argn ) == 2) {
        lua_rawgeti( L, argn, 1 );
        fnum = luaL_checknumber( L, -1 );
        lua_pop( L, 1 );
        lua_rawgeti( L, argn, 2 );
        numdec = ( int )luaL_checkinteger( L, -1 );
        lua_pop( L, 1 );
        sprintf(tmps,"%.*f",numdec, fnum);
        TFT_print(&tmps[0], x, y);
        x = TFT_X;
        y = TFT_Y;
      }
    }
    else if ( lua_type( L, argn ) == LUA_TSTRING )
    { // write string
      luaL_checktype( L, argn, LUA_TSTRING );
      buf = lua_tolstring( L, argn, &len );
      TFT_print((char*)buf, x, y);
      x = TFT_X;
      y = TFT_Y;
    }
  }  
  return 0;
}

/*
//--------------------------------------------------------------------------------------------------------------
static void TFT_drawBitmapRotate(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy) {
	int tx, ty, newx, newy;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		TFT_drawBitmap(x, y, sx, sy, data, 1);
	else
	{
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++) {
				newx=(int)(x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian))));
				newy=(int)(y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian))));

				TFT_setAddrWindow(newx, newy, newx, newy);
				TFT_pushColor(data[(ty*sx)+tx]);
			}
	}
}
*/

//==================================
static int lcd_image( lua_State* L )
{
  if (checkParam(5, L)) return 0;

  const char *fname;
  int fhndl = 0;
  uint8_t buf[640];
  uint32_t xrd = 0;
  size_t len;
  
  int x = luaL_checkinteger( L, 1 );
  int y = luaL_checkinteger( L, 2 );
  int xsize = luaL_checkinteger( L, 3 );
  int ysize = luaL_checkinteger( L, 4 );
  fname = luaL_checklstring( L, 5, &len );
  
  if ((len > 63) || (len < 1)) {
	  vm_log_error("Bad file name");
	  return 0;
  }

  if (x == CENTER) x = (_width - xsize) / 2;
  else if (x == RIGHT) x = (_width - xsize);
  if (x < 0) x = 0;

  if (y == CENTER) y = (_height - ysize) / 2;
  else if (y == BOTTOM) y = (_height - ysize);
  if (y < 0) y = 0;

  // crop to disply width
  int xend;
  if ((x+xsize) > _width) xend = _width-1;
  else xend = x+xsize-1;
  int disp_xsize = xend-x+1;
  if ((disp_xsize <= 1) || (y >= _height)) {
	  vm_log_error("image out of screen.");
	  return 0;
  }

  fhndl = file_open(fname, 0);
  if (fhndl < 0) {
	  vm_log_error("Error opening file");
	  return 0;
  }

  disp_xsize *= 2;
  do { // read 1 image line from file and send to display
	initccbuf();
	_TFT_pushAddrWindow(x, y, xend, y);
	ccbufPushByte(TFT_RAMWR);
	ccbufPushUint16(disp_xsize);
	ccbufPushLong(1);

	xrd = file_read(fhndl, buf, 2*xsize);  // read line from file
	if (xrd != (2*xsize)) {
		vm_log_error("Error reading line: %d", xrd);
		break;
	}

	memcpy(ccbuf+ccbufPtr, buf, disp_xsize);
	ccbufPtr += disp_xsize;
	ccbuf[ccbufPtr] = 0;

	send_to_display();

	y++;
	if (y >= _height) break;
	ysize--;

  } while (ysize > 0);
  
  file_close(fhndl);
  
  return 0;
}

//=====================================
static int lcd_bmpimage( lua_State* L )
{
	if (checkParam(3, L)) return 0;

	const char *fname;
	int fhndl = 0;
	uint8_t buf[960];  // max bytes per line = 3 * 320
	uint32_t xrd = 0;
	size_t len;

	int x = luaL_checkinteger( L, 1 );
	int y = luaL_checkinteger( L, 2 );
	fname = luaL_checklstring( L, 3, &len );

	if ((len > 63) || (len < 1)) {
	  vm_log_error("Bad file name");
	  return 0;
	}

	fhndl = file_open(fname, 0);
	if (fhndl < 0) {
	  vm_log_error("Error opening file");
	  return 0;
	}

	xrd = file_read(fhndl, buf, 54);  // read header
	if (xrd != 54) {
exithd:
		file_close(fhndl);
		vm_log_error("Error reading header");
		return 0;
	}

	uint16_t wtemp;
	uint32_t temp;
	uint32_t offset;
	uint32_t xsize;
	uint32_t ysize;

	// Check image header
	if ((buf[0] != 'B') || (buf[1] != 'M')) goto exithd;

	memcpy(&offset, buf+10, 4);
	memcpy(&temp, buf+14, 4);
	if (temp != 40) goto exithd;
	memcpy(&wtemp, buf+26, 2);
	if (wtemp != 1) goto exithd;
	memcpy(&wtemp, buf+28, 2);
	if (wtemp != 24) goto exithd;
	memcpy(&temp, buf+30, 4);
	if (temp != 0) goto exithd;

	memcpy(&xsize, buf+18, 4);
	memcpy(&ysize, buf+22, 4);

	// Adjust position
	if (x == CENTER) x = (_width - xsize) / 2;
	else if (x == RIGHT) x = (_width - xsize);
	if (x < 0) x = 0;

	if (y == CENTER) y = (_height - ysize) / 2;
	else if (y == BOTTOM) y = (_height - ysize);
	if (y < 0) y = 0;

	// Crop to display width
	int xend;
	if ((x+xsize) > _width) xend = _width-1;
	else xend = x+xsize-1;
	int disp_xsize = xend-x+1;
	if ((disp_xsize <= 1) || (y >= _height)) {
	  vm_log_error("image out of screen.");
	  return 0;
	}

	uint16_t color = 0;
	int i;
	while (ysize > 0) {
		// Position at line start
		// ** BMP images are stored in file from LAST to FIRST line
		//    so we have to read from the end line first

		xrd = file_seek(fhndl, offset+((ysize-1)*(xsize*3)), 0);
		if (xrd < offset)  break;

		// ** read one image line from file and send to display **
		// read only the part of image line which can be shown on screen
		xrd = file_read(fhndl, buf, disp_xsize*3);
		if (xrd != (disp_xsize*3)) {
			vm_log_error("Error reading line: %d", xrd);
			break;
		}
		//printf("read line: %d %d %d %d %d\n", y, offset, xrd, xsize, xend);

		initccbuf();
		_TFT_pushAddrWindow(x, y, xend, y);
		ccbufPushByte(TFT_RAMWR);
		ccbufPushUint16(disp_xsize*2);
		ccbufPushLong(1);

		i = 0;
		while (i < xrd) {
			color = ((buf[i++] & 0xF8) >> 3);
			color |= ((buf[i++] & 0xFC) << 3);
			color |= ((buf[i++] & 0xF8) << 8);	// get RGB888
			ccbufPushUint16(color);				// and convert to RGB565
		}
		//printf("npixel: %d\n", disp_xsize);
		ccbuf[ccbufPtr] = 0;
		send_to_display();

		y++;	// next image line
		if (y >= _height) break;
		ysize--;
	}

exit:
	file_close(fhndl);

	return 0;
}


// User defined device identifier
typedef struct {
    int fhndl;		// File handler for input function
    VMUINT16 x;		// image top left point X position
    VMUINT16 y;		// image top left point Y position
} JPGIODEV;

// User defined call-back function to input JPEG data
//---------------------
static UINT tjd_input (
	JDEC* jd,		// Decompression object
	BYTE* buff,		// Pointer to the read buffer (NULL:skip)
	UINT nd			// Number of bytes to read/skip from input stream
)
{
	int rb = 0;
	// Device identifier for the session (5th argument of jd_prepare function)
	JPGIODEV *dev = (JPGIODEV*)jd->device;

	if (buff) {	// Read nd bytes from the input strem
		vm_fs_read(dev->fhndl, buff, nd, &rb);
		return rb;	// Returns actual number of bytes read
	}
	else {	// Remove nd bytes from the input stream
		if (vm_fs_seek(dev->fhndl, nd, VM_FS_BASE_CURRENT) >= 0) return nd;
		else return 0;
	}
}

// User defined call-back function to output RGB bitmap
//----------------------
static UINT tjd_output (
	JDEC* jd,		// Decompression object of current session
	void* bitmap,	// Bitmap data to be output
	JRECT* rect		// Rectangular region to output
)
{
	// Device identifier for the session (5th argument of jd_prepare function)
	JPGIODEV *dev = (JPGIODEV*)jd->device;

	// ** Put the rectangular into the display device **
	VMUINT16 x;
	VMUINT16 y;
	VMUINT16 c;
	VMUINT16 lenPtr;
	BYTE *src = (BYTE*)bitmap;
	VMUINT16 left = rect->left + dev->x;
	VMUINT16 top = rect->top + dev->y;
	VMUINT16 right = rect->right + dev->x;
	VMUINT16 bottom = rect->bottom + dev->y;

	if ((left >= _width) || (top >= _height)) return 1;	// out of screen area, return

	int len = ((right-left+1) * (bottom-top+1))*2;		// calculate length of data

	if ((len > 0) && (len <= (CCBUF_SIZE-32))) {
		initccbuf();
		x = right;
		y = bottom;
		if (x >= _width) x = _width-1;
		if (y >= _height) y = _height-1;

		_TFT_pushAddrWindow(left, top, x, y);	// display rectangle to fill
		//printf("rect: %d,%d,%d,%d",left, top, x, y);
		ccbufPushByte(TFT_RAMWR);				// write RAM command
		lenPtr = ccbufPtr;						// save pointer to length
		ccbufPushUint16(0);						// we don't know the length yet
		ccbufPushLong(1);						// repeat once
		len = ccbufPtr;							// save buffer pointer for length calculation

	    for (y = top; y <= bottom; y++) {
		    for (x = left; x <= right; x++) {
		    	// Clip to display area
		    	if ((x < _width) && (y < _height)) {
		    		ccbuf[ccbufPtr++] = *src++;
		    		ccbuf[ccbufPtr++] = *src++;
		    	}
		    	else src += 2;
		    }
	    }
	    // set length
	    ccbuf[lenPtr++] = (uint8_t)((ccbufPtr-len) >> 8);
	    ccbuf[lenPtr] = (uint8_t)((ccbufPtr-len) & 0x00FF);
	    //printf(", len: %d\n", ccbufPtr-len);
		ccbuf[ccbufPtr] = 0;
		if (TFT_type > 0) {
			if (TFT_type == 2) {
				_sitronix_LcdTransfer(ccbuf, ccbufPtr);			// send data to display
				len = 0;
			}
			else len = _LcdSpiTransfer(ccbuf, ccbufPtr);		// send data to display
		}
		if (len < 0) {
			vm_log_error("error sending data to display: %d", len);
			return 0;  // stop decompression
		}
	}
	else {
		vm_log_error("max data size exceded: %d (%d,%d,%d,%d)", len, left,top,right,bottom);
		return 0;  // stop decompression
	}

	return 1;	// Continue to decompression
}

//=======================================
static int _lcd_jpg_image( lua_State* L )
{
	const char *fname;
	int fhndl = 0;
	size_t len;
	JPGIODEV dev;

	int x = luaL_checkinteger( L, 1 );
	int y = luaL_checkinteger( L, 2 );
	int maxscale = luaL_checkinteger( L, 3 );
	if ((maxscale < 0) || (maxscale > 3)) maxscale = 3;

	fname = luaL_checklstring( L, 4, &len );

	if ((len > 63) || (len < 1)) {
		vm_log_error("Bad file name or file not found!");
		vm_signal_post(g_shell_signal);
		return 0;
	}

	VMWCHAR ucs_name[128];
    full_fname((char *)fname, ucs_name, 128);
	dev.fhndl = vm_fs_open(ucs_name, VM_FS_MODE_READ, VM_TRUE);
	if (dev.fhndl < 0) {
		vm_log_error("Bad file name or file not found!");
		vm_signal_post(g_shell_signal);
		return 0;
	}

	char *work;				// Pointer to the working buffer (must be 4-byte aligned)
	UINT sz_work = 3100;	// Size of the working buffer (must be power of 2)
	JDEC jd;				// Decompression object (70 bytes)
	JRESULT rc;
	BYTE scale = 0;
	VMUINT8 radj = 0;
	VMUINT8 badj = 0;

	if ((x < 0) && (x != CENTER) && (x != RIGHT)) x = 0;
	if ((y < 0) && (y != CENTER) && (y != BOTTOM)) y = 0;
	if (x > (_width-5)) x = _width - 5;
	if (y > (_height-5)) y = _height - 5;

	work = vm_malloc(sz_work);
	if (work != NULL) {
		rc = jd_prepare(&jd, tjd_input, (void *)work, sz_work, &dev);
		if (rc == JDR_OK) {
			if (x == CENTER) {
				x = _width - (jd.width >> scale);
				if (x < 0) {
					if (maxscale) {
						for (scale = 0; scale <= maxscale; scale++) {
							if (((jd.width >> scale) <= (_width)) && ((jd.height >> scale) <= (_height))) break;
							if (scale == maxscale) break;
						}
						x = _width - (jd.width >> scale);
						if (x < 0) x = 0;
						else x >>= 1;
						maxscale = 0;
					}
					else x = 0;
				}
				else x >>= 1;
			}
			if (y == CENTER) {
				y = _height - (jd.height >> scale);
				if (y < 0) {
					if (maxscale) {
						for (scale = 0; scale <= maxscale; scale++) {
							if (((jd.width >> scale) <= (_width)) && ((jd.height >> scale) <= (_height))) break;
							if (scale == maxscale) break;
						}
						y = _height - (jd.height >> scale);
						if (y < 0) y = 0;
						else y >>= 1;
						maxscale = 0;
					}
					else y = 0;
				}
				else y >>= 1;
			}
			if (x == RIGHT) {
				x = 0;
				radj = 1;
			}
			if (y == BOTTOM) {
				y = 0;
				badj = 1;
			}
			// Determine scale factor
			if (maxscale) {
				for (scale = 0; scale <= maxscale; scale++) {
					if (((jd.width >> scale) <= (_width-x)) && ((jd.height >> scale) <= (_height-y))) break;
					if (scale == maxscale) break;
				}
			}
			vm_log_debug("Image dimensions: %dx%d, scale: %d, bytes used: %d", jd.width, jd.height, scale, jd.sz_pool);

			if (radj) {
				x = _width - (jd.width >> scale);
				if (x < 0) x = 0;
			}
			if (badj) {
				y = _height - (jd.height >> scale);
				if (y < 0) y = 0;
			}
			dev.x = x;
			dev.y = y;
			// Start to decompress the JPEG file
			rc = jd_decomp(&jd, tjd_output, scale);
			if (rc != JDR_OK) {
				vm_log_error("jpg decompression error %d", rc);
			}
		}
		else {
			vm_log_error("jpg prepare error %d", rc);
		}

		vm_free(work);  // free work buffer
	}
	else {
		vm_log_error("work buffer allocation error");
	}

	vm_fs_close(dev.fhndl);  // close input file

	vm_signal_post(g_shell_signal);
	return 0;
}

//======================================
static int lcd_jpg_image( lua_State* L )
{
	size_t len;

	int x = luaL_checkinteger( L, 1 );
	int y = luaL_checkinteger( L, 2 );
	const char *fname = luaL_checklstring( L, 3, &len );

	remote_CCall(L, &_lcd_jpg_image);

    return 0;
}

//======================================
static int lcd_on_touch(lua_State *L)
{
    if (TFT_type == 2) {
		if (g_touch_cb_ref != LUA_NOREF) {
			luaL_unref(L, LUA_REGISTRYINDEX, g_touch_cb_ref);
			g_touch_cb_ref = LUA_NOREF;
		}
		if ((lua_type(L, 1) == LUA_TFUNCTION) || (lua_type(L, 1) == LUA_TLIGHTFUNCTION)) {
			lua_pushvalue(L, 1);
			g_touch_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		}
    }
    return 0;
}


//-----------------------------------
static int _tp_get_data(VMUINT8 type)
{
	VMUINT8 buf[4];
	int result;
	vm_dcl_control(tp_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL); // CS=0
	buf[0] = type;
	buf[1] = 0;
	buf[1] = 0;
	result = _spi_wrrd_data(buf,3);
	vm_dcl_control(tp_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL); // CS=1

	if (result == VM_DCL_STATUS_OK) result = (VMUINT32)( ((buf[1] & 0x7F) << 5) | (buf[2] >> 3) );
	return result;
}

//-----------------------------------------------
static int tp_get_data(VMUINT8 type, int samples)
{
	int n, result, val = 0;
	VMUINT32 i = 0;
	VMUINT32 vbuf[18];
	VMUINT32 minval, maxval, dif;
	//VM_TIME_UST_COUNT tmstart = vm_time_ust_get_count();
	//VM_TIME_UST_COUNT tmstop;

    if (samples < 3) samples = 1;
    if (samples > 18) samples = 18;

    // one dummy read
    result = _tp_get_data(type);

    // read data
	while (i < 10) {
    	minval = 5000;
    	maxval = 0;
		// get values
		for (n=0;n<samples;n++) {
		    result = _tp_get_data(type);
			if (result < 0) break;

			vbuf[n] = result;
			if (result < minval) minval = result;
			if (result > maxval) maxval = result;
		}
		if (result < 0) break;
		dif = maxval - minval;
		if (dif < 40) break;
		i++;
    }
	if (result < 0) return -1;

	if (samples > 2) {
		// remove one min value
		for (n = 0; n < samples; n++) {
			if (vbuf[n] == minval) {
				vbuf[n] = 5000;
				break;
			}
		}
		// remove one max value
		for (n = 0; n < samples; n++) {
			if (vbuf[n] == maxval) {
				vbuf[n] = 5000;
				break;
			}
		}
		for (n = 0; n < samples; n++) {
			if (vbuf[n] < 5000) val += vbuf[n];
		}
		val /= (samples-2);
	}
	else val = vbuf[0];

	/*
	tmstop = vm_time_ust_get_count();
	VMUINT32 dur;
	if (tmstop > tmstart) dur = tmstop - tmstart;
	else dur = tmstop + (0xFFFFFFFF - tmstart);
	//printf("Read %02X: time=%d, val=%d, min=%d, max=%d, dif=%d\n", type, dur, val, minval, maxval, (maxval-minval));
	*/

    return val;
}

//-------------------------------------
static int _lcd_get_touch(lua_State *L)
{
	t_spi_params temp_spi;
	int result = 0;

	g_shell_result = 1;

	// save spi settings
	memcpy(&temp_spi, &g_spi, sizeof(t_spi_params));
	g_spi.conf_data.clock_high_time = 15;
	g_spi.conf_data.clock_low_time = 15;
	g_spi.conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_1;
	g_spi.conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_1;

	result = _setup_spi();
	if (result < 0) goto exit;

    int X=0, Y=0, Z=0;

    result = tp_get_data(0xB0, 3);
	if (result < 0) goto exit;

	if (result > 50)  {
		Z = result;

		result = tp_get_data(0xD0, 10);
		if (result < 0) goto exit;
		X = result;

		result = tp_get_data(0x90, 10);
		if (result < 0) goto exit;
		Y = result;
	}

exit:
	// restore spi settings
	memcpy(&g_spi, &temp_spi, sizeof(t_spi_params));
	int status = _setup_spi();

	if (result >= 0) {
		g_shell_result = 3;
		lua_pushinteger(L, Z);
		lua_pushinteger(L, X);
		lua_pushinteger(L, Y);
	}
	else lua_pushinteger(L, result);
	vm_signal_post(g_shell_signal);
	return 0;
}

//====================================
static int lcd_get_touch(lua_State *L)
{
	if ((g_spi_handle == VM_DCL_HANDLE_INVALID) || (tp_cs_handle == VM_DCL_HANDLE_INVALID)) {
		// spi not setup
		lua_pushinteger(L, -1);
		return 1;
	}

	remote_CCall(L, &_lcd_get_touch);
	return g_shell_result;
}

//=======================================
static int lcd_set_touch_cs(lua_State *L)
{
	if (tp_cs_handle != VM_DCL_HANDLE_INVALID) {
    	vm_dcl_close(tp_cs_handle);
    	tp_cs_handle = VM_DCL_HANDLE_INVALID;
	}

    int cs = luaL_checkinteger(L, 1);
    gpio_get_handle(cs, &tp_cs_handle);
    if (tp_cs_handle != VM_DCL_HANDLE_INVALID) {
        vm_dcl_control(tp_cs_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
        vm_dcl_control(tp_cs_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
        vm_dcl_control(tp_cs_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
        lua_pushinteger(L, 0);
    }
    else {
    	tp_cs_handle = VM_DCL_HANDLE_INVALID;
		vm_log_error("error initializing TP CS on pin #%d", cs);
	    lua_pushinteger(L, -1);
    }
    return 1;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)

const LUA_REG_TYPE lcd_map[] =
{
  { LSTRKEY( "init" ), LFUNCVAL( lcd_init )},
  { LSTRKEY( "clear" ), LFUNCVAL( lcd_clear )},
  { LSTRKEY( "on" ), LFUNCVAL( lcd_on )},
  { LSTRKEY( "off" ), LFUNCVAL( lcd_off )},
  { LSTRKEY( "setfont" ), LFUNCVAL( lcd_setfont )},
  { LSTRKEY( "compilefont" ), LFUNCVAL( compile_font_file )},
  { LSTRKEY( "getscreensize" ), LFUNCVAL( lcd_getscreensize )},
  { LSTRKEY( "getfontsize" ), LFUNCVAL( lcd_getfontsize )},
  { LSTRKEY( "getfontheight" ), LFUNCVAL( lcd_getfontheight )},
  { LSTRKEY( "gettype" ), LFUNCVAL( lcd_gettype )},
  { LSTRKEY( "setrot" ), LFUNCVAL( lcd_setrot )},
  { LSTRKEY( "setorient" ), LFUNCVAL( lcd_setorient )},
  { LSTRKEY( "setcolor" ), LFUNCVAL( lcd_setcolor )},
  { LSTRKEY( "settransp" ), LFUNCVAL( lcd_settransp )},
  { LSTRKEY( "setfixed" ), LFUNCVAL( lcd_setfixed )},
  { LSTRKEY( "setwrap" ), LFUNCVAL( lcd_setwrap )},
  { LSTRKEY( "setclipwin" ), LFUNCVAL( lcd_setclipwin )},
  { LSTRKEY( "resetclipwin" ), LFUNCVAL( lcd_resetclipwin )},
  { LSTRKEY( "invert" ), LFUNCVAL( lcd_invert )},
  { LSTRKEY( "putpixel" ), LFUNCVAL( lcd_putpixel )},
  { LSTRKEY( "line" ), LFUNCVAL( lcd_line )},
  { LSTRKEY( "rect" ), LFUNCVAL( lcd_rect )},
  { LSTRKEY( "circle" ), LFUNCVAL( lcd_circle )},
  { LSTRKEY( "triangle" ), LFUNCVAL( lcd_triangle )},
  { LSTRKEY( "write" ), LFUNCVAL( lcd_write )},
  { LSTRKEY( "image" ), LFUNCVAL( lcd_image )},
  { LSTRKEY( "jpgimage" ), LFUNCVAL( lcd_jpg_image )},
  { LSTRKEY( "bmpimage" ), LFUNCVAL( lcd_bmpimage )},
  { LSTRKEY( "hsb2rgb" ), LFUNCVAL( lcd_HSBtoRGB )},
  { LSTRKEY( "setbrightness" ), LFUNCVAL( lcd_set_brightness )},
  { LSTRKEY( "ontouch" ), LFUNCVAL( lcd_on_touch )},
  { LSTRKEY( "gettouch" ), LFUNCVAL( lcd_get_touch )},
  { LSTRKEY( "set_touch_cs" ), LFUNCVAL( lcd_set_touch_cs )},
  
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "PORTRAIT" ),       LNUMVAL( PORTRAIT ) },
  { LSTRKEY( "PORTRAIT_FLIP" ),  LNUMVAL( PORTRAIT_FLIP ) },
  { LSTRKEY( "LANDSCAPE" ),      LNUMVAL( LANDSCAPE ) },
  { LSTRKEY( "LANDSCAPE_FLIP" ), LNUMVAL( LANDSCAPE_FLIP ) },
  { LSTRKEY( "CENTER" ),         LNUMVAL( CENTER ) },
  { LSTRKEY( "RIGHT" ),          LNUMVAL( RIGHT ) },
  { LSTRKEY( "LASTX" ),          LNUMVAL( LASTX ) },
  { LSTRKEY( "LASTY" ),          LNUMVAL( LASTY ) },
  { LSTRKEY( "BLACK" ),          LNUMVAL( TFT_BLACK ) },
  { LSTRKEY( "NAVY" ),           LNUMVAL( TFT_NAVY ) },
  { LSTRKEY( "DARKGREEN" ),      LNUMVAL( TFT_DARKGREEN ) },
  { LSTRKEY( "DARKCYAN" ),       LNUMVAL( TFT_DARKCYAN ) },
  { LSTRKEY( "MAROON" ),         LNUMVAL( TFT_MAROON ) },
  { LSTRKEY( "PURPLE" ),         LNUMVAL( TFT_PURPLE ) },
  { LSTRKEY( "OLIVE" ),          LNUMVAL( TFT_OLIVE ) },
  { LSTRKEY( "LIGHTGREY" ),      LNUMVAL( TFT_LIGHTGREY ) },
  { LSTRKEY( "DARKGREY" ),       LNUMVAL( TFT_DARKGREY ) },
  { LSTRKEY( "BLUE" ),           LNUMVAL( TFT_BLUE ) },
  { LSTRKEY( "GREEN" ),          LNUMVAL( TFT_GREEN ) },
  { LSTRKEY( "CYAN" ),           LNUMVAL( TFT_CYAN ) },
  { LSTRKEY( "RED" ),            LNUMVAL( TFT_RED ) },
  { LSTRKEY( "MAGENTA" ),        LNUMVAL( TFT_MAGENTA ) },
  { LSTRKEY( "YELLOW" ),         LNUMVAL( TFT_YELLOW ) },
  { LSTRKEY( "WHITE" ),          LNUMVAL( TFT_WHITE ) },
  { LSTRKEY( "ORANGE" ),         LNUMVAL( TFT_ORANGE ) },
  { LSTRKEY( "GREENYELLOW" ),    LNUMVAL( TFT_GREENYELLOW ) },
  { LSTRKEY( "PINK" ),           LNUMVAL( TFT_PINK ) },
  { LSTRKEY( "FONT_DEFAULT" ),   LNUMVAL( DEFAULT_FONT ) },
  { LSTRKEY( "FONT_7SEG" ),      LNUMVAL( FONT_7SEG ) },
  { LSTRKEY( "ST7735" ),         LNUMVAL( 0 ) },
  { LSTRKEY( "ST7735B" ),        LNUMVAL( 1 ) },
  { LSTRKEY( "ST7735G" ),        LNUMVAL( 2 ) },
  { LSTRKEY( "ILI7341" ),        LNUMVAL( 3 ) },
#endif      
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_lcd(lua_State *L)
{
	TFT_type = 2;
	TFT_setFont(DEFAULT_FONT, NULL);
	_fg = TFT_GREEN;
	_bg = TFT_BLACK;

	_initvar();
	touch_cb_params.busy = 0;
	touch_cb_params.cb_ref = LUA_NOREF;

#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else    
  luaL_register( L, "lcd", lcd_map );

  MOD_REG_NUMBER( L, "PORTRAIT",        PORTRAIT);
  MOD_REG_NUMBER( L, "PORTRAIT_FLIP",   PORTRAIT_FLIP);
  MOD_REG_NUMBER( L, "LANDSCAPE",       LANDSCAPE);
  MOD_REG_NUMBER( L, "LANDSCAPE_FLIP",  LANDSCAPE_FLIP);
  MOD_REG_NUMBER( L, "CENTER",			CENTER);
  MOD_REG_NUMBER( L, "RIGHT",	        RIGHT);
  MOD_REG_NUMBER( L, "BOTTOM",	        BOTTOM);
  MOD_REG_NUMBER( L, "LASTX",	        LASTY);
  MOD_REG_NUMBER( L, "LASTY",	        LASTX);
  MOD_REG_NUMBER( L, "BLACK",           TFT_BLACK);
  MOD_REG_NUMBER( L, "NAVY",            TFT_NAVY);
  MOD_REG_NUMBER( L, "DARKGREEN",       TFT_DARKGREEN);
  MOD_REG_NUMBER( L, "DARKCYAN",        TFT_DARKCYAN);
  MOD_REG_NUMBER( L, "MAROON",          TFT_MAROON);
  MOD_REG_NUMBER( L, "PURPLE",          TFT_PURPLE);
  MOD_REG_NUMBER( L, "OLIVE",           TFT_OLIVE);
  MOD_REG_NUMBER( L, "LIGHTGREY",       TFT_LIGHTGREY);
  MOD_REG_NUMBER( L, "DARKGREY",        TFT_DARKGREY);
  MOD_REG_NUMBER( L, "BLUE" ,           TFT_BLUE);
  MOD_REG_NUMBER( L, "GREEN",           TFT_GREEN);
  MOD_REG_NUMBER( L, "CYAN" ,           TFT_CYAN);
  MOD_REG_NUMBER( L, "RED",             TFT_RED);
  MOD_REG_NUMBER( L, "MAGENTA",         TFT_MAGENTA);
  MOD_REG_NUMBER( L, "YELLOW",          TFT_YELLOW);
  MOD_REG_NUMBER( L, "WHITE",           TFT_WHITE);
  MOD_REG_NUMBER( L, "ORANGE",          TFT_ORANGE);
  MOD_REG_NUMBER( L, "GREENYELLOW",     TFT_GREENYELLOW);
  MOD_REG_NUMBER( L, "PINK" ,           TFT_PINK);
  MOD_REG_NUMBER( L, "FONT_DEFAULT" ,   DEFAULT_FONT);
  MOD_REG_NUMBER( L, "FONT_7SEG" ,      FONT_7SEG);
  MOD_REG_NUMBER( L, "ST7735" ,         0);
  MOD_REG_NUMBER( L, "ST7735B" ,        1);
  MOD_REG_NUMBER( L, "ST7735G" ,        2);
  MOD_REG_NUMBER( L, "ILI9341" ,        3);
  MOD_REG_NUMBER( L, "XADOW_V0" ,       8);
  MOD_REG_NUMBER( L, "XADOW_V1" ,       9);

  return 1;
#endif
}
