/**
 * sensor.c
 * ONEWIRE adapted by LoBo from TM_ONEWIRE (author  Tilen Majerle)
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdatetime.h"
#include "vmthread.h"

/* OneWire commands */
#define ONEWIRE_CMD_RSCRATCHPAD		0xBE
#define ONEWIRE_CMD_WSCRATCHPAD		0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD	0x48
#define ONEWIRE_CMD_RECEEPROM		0xB8
#define ONEWIRE_CMD_RPWRSUPPLY		0xB4
#define ONEWIRE_CMD_SEARCHROM		0xF0
#define ONEWIRE_CMD_READROM			0x33
#define ONEWIRE_CMD_MATCHROM		0x55
#define ONEWIRE_CMD_SKIPROM			0xCC

//#define DS18B20ALARMFUNC

//* TM_DS18B20_Macros
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE			0x28
#define DS18B20_CMD_ALARMSEARCH		0xEC

/* DS18B20 read temperature command */
#define DS18B20_CMD_CONVERTTEMP		0x44 	/* Convert temperature */
#define DS18B20_DECIMAL_STEPS_12BIT	0.0625
#define DS18B20_DECIMAL_STEPS_11BIT	0.125
#define DS18B20_DECIMAL_STEPS_10BIT	0.25
#define DS18B20_DECIMAL_STEPS_9BIT	0.5

/* Bits locations for resolution */
#define DS18B20_RESOLUTION_R1		6
#define DS18B20_RESOLUTION_R0		5

/* CRC enabled */
#ifdef DS18B20_USE_CRC
#define DS18B20_DATA_LEN			9
#else
#define DS18B20_DATA_LEN			2
#endif

/* TM_DS18B20_Typedefs */

/* DS18B0 Resolutions available */
typedef enum {
  TM_DS18B20_Resolution_9bits = 	 9, /*!< DS18B20 9 bits resolution */
  TM_DS18B20_Resolution_10bits = 	10, /*!< DS18B20 10 bits resolution */
  TM_DS18B20_Resolution_11bits = 	11, /*!< DS18B20 11 bits resolution */
  TM_DS18B20_Resolution_12bits = 	12  /*!< DS18B20 12 bits resolution */
} TM_DS18B20_Resolution_t;

/* OneWire errors */
typedef enum {
  owOK =0,
  owError_NoDevice,
  owError_Not18b20,
  owError_NotFinished,
  owError_BadCRC
} owState_t;

typedef struct {
	VM_DCL_HANDLE handle;          /*!< GPIO Pin handle to be used for I/O functions */
	unsigned char LastDiscrepancy;       /*!< Search private */
	unsigned char LastFamilyDiscrepancy; /*!< Search private */
	unsigned char LastDeviceFlag;        /*!< Search private */
	unsigned char ROM_NO[8];             /*!< 8-bytes address of last search device */
} TM_OneWire_t;

static TM_OneWire_t OW_DEVICE;

#define MAX_ONEWIRE_SENSORS 2

unsigned char ow_numdev = 0;
unsigned char ow_roms[MAX_ONEWIRE_SENSORS][8];
#ifdef DS18B20ALARMFUNC
unsigned char ow_alarm_device [MAX_ONEWIRE_SENSORS][8];
#endif

//**************************************************

extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);
extern int sys_wdt_rst_time;
extern void _reset_wdg(void);

VM_DCL_HANDLE handle_DHT11 = -1;
int DHT11_22 = 0;


//******************
// ONEWIRE FUNCTIONS
//******************

//--------------------------------------------------------------------------
static unsigned char OW_OUT_SetWait(unsigned char hilo, unsigned char testbit, unsigned int dly) {
  int i, j;
  int bitval = 1;
  VM_TIME_UST_COUNT tstart;
  vm_dcl_gpio_control_level_status_t data;
  
  /* set line low or high, and wait dly us */
  if (hilo==0) {
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
  }
  else {
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    /* wait 5 usek after releasing the line */
  	tstart = vm_time_ust_get_count();
	while (vm_time_ust_get_duration(tstart, vm_time_ust_get_count()) < 5) { }
  }
  
  tstart = vm_time_ust_get_count();
  while (vm_time_ust_get_duration(tstart, vm_time_ust_get_count()) < dly) {
    if (testbit && bitval) {
      vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_READ, &data);
      if (data.level_status == 0 ) {
        bitval = 0;
      }
    }
  }
  return bitval;
}

//---------------------------------
static unsigned char TM_OneWire_Reset() {
  unsigned char i;

  // Line low, and wait 480us minimum
  OW_OUT_SetWait(0, 0, 480);
  /* Release line and wait for 480us minimum*/
  i = OW_OUT_SetWait(1, 1, 480);
  // Return value of presence pulse, 0 = OK, 1 = ERROR
  return i;
}

//--------------------------------------------
static void TM_OneWire_WriteBit(unsigned char bit) {
  if (bit) {
    /* Set line low */
    OW_OUT_SetWait(0, 0, 8);
    /* Bit high */
    /* release the line and Wait for 60 us */
    OW_OUT_SetWait(1, 0, 60);
  } else {
    /* Set line low */
    /* Bit low */
    OW_OUT_SetWait(0, 0, 65);
    /* release the line and Wait for 5 us */
    OW_OUT_SetWait(1, 0, 5);
  }
}

//-----------------------------------
static unsigned char TM_OneWire_ReadBit() {
  unsigned char bit = 0;

  /* Line low */
  OW_OUT_SetWait(0, 0, 3);
  /* Release line and wait for line value */
  bit = OW_OUT_SetWait(1, 1, 60);
  /* Return bit value */
  return bit;
}

//----------------------------------------------
static void TM_OneWire_WriteByte(unsigned char byte) {
  unsigned char i = 8;
  // Write 8 bits
  while (i--) {
    // LSB bit is first
    TM_OneWire_WriteBit(byte & 0x01);
    byte >>= 1;
  }
}

//------------------------------------
static unsigned char TM_OneWire_ReadByte() {
  unsigned char i = 8, byte = 0;
  while (i--) {
    byte >>= 1;
    byte |= (TM_OneWire_ReadBit() << 7);
  }
  return byte;
}

//------------------------------------
static void TM_OneWire_ResetSearch() {
  // Reset the search state
  OW_DEVICE.LastDiscrepancy = 0;
  OW_DEVICE.LastDeviceFlag = 0;
  OW_DEVICE.LastFamilyDiscrepancy = 0;
}

//-------------------------------------------------
static unsigned char TM_OneWire_Search(unsigned char command) {
  unsigned char id_bit_number;
  unsigned char last_zero, rom_byte_number, search_result;
  unsigned char id_bit, cmp_id_bit;
  unsigned char rom_byte_mask, search_direction;

  /* Initialize for search */
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;
  // if the last call was not the last one
  if (!OW_DEVICE.LastDeviceFlag) {
    // 1-Wire reset
    if (TM_OneWire_Reset()) {
      /* Reset the search */
      OW_DEVICE.LastDiscrepancy = 0;
      OW_DEVICE.LastDeviceFlag = 0;
      OW_DEVICE.LastFamilyDiscrepancy = 0;
      return 0;
    }
    // issue the search command 
    TM_OneWire_WriteByte(command);  
    // loop to do the search
    do {
      // read a bit and its complement
      id_bit = TM_OneWire_ReadBit();
      cmp_id_bit = TM_OneWire_ReadBit();
      // check for no devices on 1-wire
      if ((id_bit == 1) && (cmp_id_bit == 1)) {
        break;
      } else {
        // all devices coupled have 0 or 1
        if (id_bit != cmp_id_bit) {
          search_direction = id_bit;  // bit write value for search
        } else {
          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < OW_DEVICE.LastDiscrepancy) {
            search_direction = ((OW_DEVICE.ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          } else {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == OW_DEVICE.LastDiscrepancy);
          }
          // if 0 was picked then record its position in LastZero
          if (search_direction == 0) {
            last_zero = id_bit_number;
            // check for Last discrepancy in family
            if (last_zero < 9) {
              OW_DEVICE.LastFamilyDiscrepancy = last_zero;
            }
          }
        }
        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1) {
          OW_DEVICE.ROM_NO[rom_byte_number] |= rom_byte_mask;
        } else {
          OW_DEVICE.ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }
        // serial number search direction write bit
        TM_OneWire_WriteBit(search_direction);
        // increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;
        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if (rom_byte_mask == 0) {
          //docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    } while (rom_byte_number < 8);  // loop until through all ROM bytes 0-7

    // if the search was successful then
    if (!(id_bit_number < 65)) {
      // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
      OW_DEVICE.LastDiscrepancy = last_zero;
      // check for last device
      if (OW_DEVICE.LastDiscrepancy == 0) {
        OW_DEVICE.LastDeviceFlag = 1;
      }
      search_result = 1;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !OW_DEVICE.ROM_NO[0]) {
    OW_DEVICE.LastDiscrepancy = 0;
    OW_DEVICE.LastDeviceFlag = 0;
    OW_DEVICE.LastFamilyDiscrepancy = 0;
    search_result = 0;
  }

  return search_result;
}

//----------------------------------
static unsigned char TM_OneWire_First() {
  // Reset search values
  TM_OneWire_ResetSearch();
  // Start with searching
  return TM_OneWire_Search(ONEWIRE_CMD_SEARCHROM);
}

//--------------------------------
static unsigned char TM_OneWire_Next() {
  // Leave the search state alone
  return TM_OneWire_Search(ONEWIRE_CMD_SEARCHROM);
}

/*
//------------------------------
static int TM_OneWire_Verify() {
  unsigned char rom_backup[8];
  int i,rslt,ld_backup,ldf_backup,lfd_backup;
  
  // keep a backup copy of the current state
  for (i = 0; i < 8; i++)
    rom_backup[i] = OW_DEVICE.ROM_NO[i];
  ld_backup = OW_DEVICE.LastDiscrepancy;
  ldf_backup = OW_DEVICE.LastDeviceFlag;
  lfd_backup = OW_DEVICE.LastFamilyDiscrepancy;
  // set search to find the same device
  OW_DEVICE.LastDiscrepancy = 64;
  OW_DEVICE.LastDeviceFlag = 0;
  if (TM_OneWire_Search(ONEWIRE_CMD_SEARCHROM)) {
    // check if same device found
    rslt = 1;
    for (i = 0; i < 8; i++) {
      if (rom_backup[i] != OW_DEVICE.ROM_NO[i]) {
        rslt = 1;
        break;
      }
    }
  } else {
    rslt = 0;
  }

  // restore the search state 
  for (i = 0; i < 8; i++) {
    OW_DEVICE.ROM_NO[i] = rom_backup[i];
  }
  OW_DEVICE.LastDiscrepancy = ld_backup;
  OW_DEVICE.LastDeviceFlag = ldf_backup;
  OW_DEVICE.LastFamilyDiscrepancy = lfd_backup;

  // return the result of the verify
  return rslt;
}

//-------------------------------------------------------
static void TM_OneWire_TargetSetup(unsigned char family_code) {
  unsigned char i;

  // set the search state to find SearchFamily type devices
  OW_DEVICE.ROM_NO[0] = family_code;
  for (i = 1; i < 8; i++) {
    OW_DEVICE.ROM_NO[i] = 0;
  }
  OW_DEVICE.LastDiscrepancy = 64;
  OW_DEVICE.LastFamilyDiscrepancy = 0;
  OW_DEVICE.LastDeviceFlag = 0;
}

//----------------------------------------
static void TM_OneWire_FamilySkipSetup() {
  // set the Last discrepancy to last family discrepancy
  OW_DEVICE.LastDiscrepancy = OW_DEVICE.LastFamilyDiscrepancy;
  OW_DEVICE.LastFamilyDiscrepancy = 0;
  // check for end of list
  if (OW_DEVICE.LastDiscrepancy == 0) {
    OW_DEVICE.LastDeviceFlag = 1;
  }
}

//-----------------------------------------------
static unsigned char TM_OneWire_GetROM(unsigned char index) {
  return OW_DEVICE.ROM_NO[index];
}

//--------------------------------------------
static void TM_OneWire_Select(unsigned char* addr) {
  unsigned char i;
  TM_OneWire_WriteByte(ONEWIRE_CMD_MATCHROM);
  for (i = 0; i < 8; i++) {
    TM_OneWire_WriteByte(*(addr + i));
  }
}
*/

//------------------------------------------------------
static void TM_OneWire_SelectWithPointer(unsigned char *ROM) {
  unsigned char i;
  TM_OneWire_WriteByte(ONEWIRE_CMD_MATCHROM);
  for (i = 0; i < 8; i++) {
    TM_OneWire_WriteByte(*(ROM + i));
  }	
}

//------------------------------------------------------
static void TM_OneWire_GetFullROM(unsigned char *firstIndex) {
  unsigned char i;
  for (i = 0; i < 8; i++) {
    *(firstIndex + i) = OW_DEVICE.ROM_NO[i];
  }
}

//----------------------------------------------------------
static unsigned char TM_OneWire_CRC8(unsigned char *addr, unsigned char len) {
  unsigned char crc = 0, inbyte, i, mix;
	
  while (len--) {
    inbyte = *addr++;
    for (i = 8; i; i--) {
      mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) {
        crc ^= 0x8C;
      }
      inbyte >>= 1;
    }
  }
  /* Return calculated CRC */
  return crc;
}


//*********************
// TM_DS18B20_Functions
//*********************

//------------------------------------------
static unsigned char TM_DS18B20_Is(unsigned char *ROM) {
  /* Checks if first byte is equal to DS18B20's family code */
  if (*ROM == DS18B20_FAMILY_CODE) {
    return 1;
  }
  return 0;
}

/*
//-----------------------------------------------
static owState_t TM_DS18B20_Start(unsigned char *ROM) {
  // Check if device is DS18B20
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }
  // Reset line
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  // Select ROM number
  TM_OneWire_SelectWithPointer(ROM);
  // Start temperature conversion
  TM_OneWire_WriteByte(DS18B20_CMD_CONVERTTEMP);
  return owOK;
}
*/

//---------------------------------
static void TM_DS18B20_StartAll() {
  /* Reset pulse */
  if (TM_OneWire_Reset() != 0) return;
  /* Skip rom */
  TM_OneWire_WriteByte(ONEWIRE_CMD_SKIPROM);
  /* Start conversion on all connected devices */
  TM_OneWire_WriteByte(DS18B20_CMD_CONVERTTEMP);
}

//------------------------------------------------------------------
static owState_t TM_DS18B20_Read(unsigned char *ROM, float *destination) {
  unsigned int temperature;
  unsigned char resolution;
  char digit, minus = 0;
  float decimal;
  unsigned char i = 0;
  unsigned char data[9];
  unsigned char crc;
	
  /* Check if device is DS18B20 */
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }
  /* Check if line is released, if it is, then conversion is complete */
  if (!TM_OneWire_ReadBit()) {
    /* Conversion is not finished yet */
    return owError_NotFinished; 
  }
  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Get data */
  for (i = 0; i < 9; i++) {
    /* Read byte by byte */
    data[i] = TM_OneWire_ReadByte();
  }
  /* Calculate CRC */
  crc = TM_OneWire_CRC8(data, 8);
  /* Check if CRC is ok */
  if (crc != data[8]) {
    /* CRC invalid */
    return owError_BadCRC;
  }

  /* First two bytes of scratchpad are temperature values */
  temperature = data[0] | (data[1] << 8);
  /* Reset line */
  TM_OneWire_Reset();
  /* Check if temperature is negative */
  if (temperature & 0x8000) {
    /* Two's complement, temperature is negative */
    temperature = ~temperature + 1;
    minus = 1;
  }
  /* Get sensor resolution */
  resolution = ((data[4] & 0x60) >> 5) + 9;
  /* Store temperature integer digits and decimal digits */
  digit = temperature >> 4;
  digit |= ((temperature >> 8) & 0x7) << 4;

  /* Store decimal digits */
  switch (resolution) {
    case 9: {
      decimal = (temperature >> 3) & 0x01;
      decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
    } break;
    case 10: {
      decimal = (temperature >> 2) & 0x03;
      decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
    } break;
    case 11: {
      decimal = (temperature >> 1) & 0x07;
      decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
    } break;
    case 12: {
      decimal = temperature & 0x0F;
      decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
    } break;
    default: {
      decimal = 0xFF;
      digit = 0;
    }
  }

  /* Check for negative part */
  decimal = digit + decimal;
  if (minus) {
    decimal = 0 - decimal;
  }
  /* Set to pointer */
  *destination = decimal;

  /* Return 1, temperature valid */
  return owOK;
}

//-----------------------------------------------------
static unsigned char TM_DS18B20_GetResolution(unsigned char *ROM) {
  unsigned char conf;

  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Ignore first 4 bytes */
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();

  /* 5th byte of scratchpad is configuration register */
  conf = TM_OneWire_ReadByte();

  /* Return 9 - 12 value according to number of bits */
  return ((conf & 0x60) >> 5) + 9;
}

//-------------------------------------------------------------------------------------------
static owState_t TM_DS18B20_SetResolution(unsigned char *ROM, TM_DS18B20_Resolution_t resolution) {
  unsigned char th, tl, conf;
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();

  th = TM_OneWire_ReadByte();
  tl = TM_OneWire_ReadByte();
  conf = TM_OneWire_ReadByte();

  if (resolution == TM_DS18B20_Resolution_9bits) {
    conf &= ~(1 << DS18B20_RESOLUTION_R1);
    conf &= ~(1 << DS18B20_RESOLUTION_R0);
  } else if (resolution == TM_DS18B20_Resolution_10bits) {
    conf &= ~(1 << DS18B20_RESOLUTION_R1);
    conf |= 1 << DS18B20_RESOLUTION_R0;
  } else if (resolution == TM_DS18B20_Resolution_11bits) {
    conf |= 1 << DS18B20_RESOLUTION_R1;
    conf &= ~(1 << DS18B20_RESOLUTION_R0);
  } else if (resolution == TM_DS18B20_Resolution_12bits) {
    conf |= 1 << DS18B20_RESOLUTION_R1;
    conf |= 1 << DS18B20_RESOLUTION_R0;
  }

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  TM_OneWire_WriteByte(ONEWIRE_CMD_WSCRATCHPAD);

  /* Write bytes */
  TM_OneWire_WriteByte(th);
  TM_OneWire_WriteByte(tl);
  TM_OneWire_WriteByte(conf);

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  TM_OneWire_WriteByte(ONEWIRE_CMD_CPYSCRATCHPAD);

  return owOK;
}

/***************************/
/* DS18B20 Alarm functions */
/***************************/
#ifdef DS18B20ALARMFUNC
//---------------------------------------------------------------------------
static unsigned char TM_DS18B20_SetAlarmLowTemperature(unsigned char *ROM, char temp) {
  unsigned char tl, th, conf;
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }
  if (temp > 125) {
    temp = 125;
  } 
  if (temp < -55) {
    temp = -55;
  }
  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();

  th = TM_OneWire_ReadByte();
  tl = TM_OneWire_ReadByte();
  conf = TM_OneWire_ReadByte();

  tl = (unsigned char)temp;

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  TM_OneWire_WriteByte(ONEWIRE_CMD_WSCRATCHPAD);

  /* Write bytes */
  TM_OneWire_WriteByte(th);
  TM_OneWire_WriteByte(tl);
  TM_OneWire_WriteByte(conf);

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  TM_OneWire_WriteByte(ONEWIRE_CMD_CPYSCRATCHPAD);

  return owOK;
}

//------------------------------------------------------------------------------
static owState_t TM_DS18B20_SetAlarmHighTemperature(unsigned char *ROM, char temp) {
  unsigned char tl, th, conf;
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }
  if (temp > 125) {
    temp = 125;
  } 
  if (temp < -55) {
    temp = -55;
  }
  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();

  th = TM_OneWire_ReadByte();
  tl = TM_OneWire_ReadByte();
  conf = TM_OneWire_ReadByte();

  th = (unsigned char)temp;

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  TM_OneWire_WriteByte(ONEWIRE_CMD_WSCRATCHPAD);

  /* Write bytes */
  TM_OneWire_WriteByte(th);
  TM_OneWire_WriteByte(tl);
  TM_OneWire_WriteByte(conf);

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  TM_OneWire_WriteByte(ONEWIRE_CMD_CPYSCRATCHPAD);

  return owOK;
}

//-----------------------------------------------------------------
static owState_t TM_DS18B20_DisableAlarmTemperature(unsigned char *ROM) {
  unsigned char tl, th, conf;
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }
  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Read scratchpad command by onewire protocol */
  TM_OneWire_WriteByte(ONEWIRE_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  TM_OneWire_ReadByte();
  TM_OneWire_ReadByte();

  th = TM_OneWire_ReadByte();
  tl = TM_OneWire_ReadByte();
  conf = TM_OneWire_ReadByte();

  th = 125;
  tl = (unsigned char)-55;

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  TM_OneWire_WriteByte(ONEWIRE_CMD_WSCRATCHPAD);

  /* Write bytes */
  TM_OneWire_WriteByte(th);
  TM_OneWire_WriteByte(tl);
  TM_OneWire_WriteByte(conf);

  /* Reset line */
  if (TM_OneWire_Reset() != 0) {
    return owError_NoDevice;
  }
  /* Select ROM number */
  TM_OneWire_SelectWithPointer(ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  TM_OneWire_WriteByte(ONEWIRE_CMD_CPYSCRATCHPAD);

  return owOK;
}

//---------------------------------------
static unsigned char TM_DS18B20_AlarmSearch() {
  /* Start alarm search */
  return TM_OneWire_Search(DS18B20_CMD_ALARMSEARCH);
}
#endif

/*
//-----------------------------------
static unsigned char TM_DS18B20_AllDone() {
  // If read bit is low, then device is not finished yet with calculation temperature
  return TM_OneWire_ReadBit();
}
*/


//****************
// dht11 functions
//****************

//--------------------------------------------------------
static unsigned char DHT_IN_Length(unsigned char hilo, unsigned int dly) {
  unsigned int bitlen = 0;
  vm_dcl_gpio_control_level_status_t data;
  VM_TIME_UST_COUNT tstart, tend;
  
  bitlen = 0;
  tstart = vm_time_ust_get_count();
  tend = tstart;
  if (hilo) { // === measure high state length ===
    while (vm_time_ust_get_duration(tstart, tend) <= dly) {
   	  tend = vm_time_ust_get_count();
      if (bitlen == 0) { // wait for high pulse start
        vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_READ, &data);
        if (data.level_status != 0 ) {
          bitlen = 1;
          tstart = vm_time_ust_get_count();
          tend = tstart;
        }
      }
      else { // wait for low (high pulse end)
        vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_READ, &data);
        if (data.level_status == 0 ) {
          bitlen = tend;
          break;
        }
      }
    }
  }
  else { // === measure low state length ===
    while (vm_time_ust_get_duration(tstart, tend) <= dly) {
	  tend = vm_time_ust_get_count();
      if (bitlen == 0) { // wait for low pulse start
          vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_READ, &data);
          if (data.level_status == 0 ) {
            bitlen = 1;
            tstart = vm_time_ust_get_count();
            tend = tstart;
          }
      }
      else {
        // wait for high (low pulse end)
        vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_READ, &data);
        if (data.level_status != 0 ) {
          bitlen = tend;
          break;
        }
      }
    }
  }
  if (bitlen == 1) return 0;
  
  return bitlen; // return pulse length in usec
}

//-------------------------------------------------------
static void DHT_OUT_SetWait(unsigned char hilo, unsigned int dly) {
  VM_TIME_UST_COUNT tstart;
  
  if (hilo==0) {
    vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
    vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
  }
  else {
    vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  	vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  }

  tstart = vm_time_ust_get_count();
  while (vm_time_ust_get_duration(tstart, vm_time_ust_get_count()) < dly) { }
}

//------------------------------
static unsigned char DHT11_Check(void)
{   
  unsigned char len = 0;

  // send reset pulse  
  DHT_OUT_SetWait(0,10);  // data=0, wait 10 usec
  vm_thread_sleep(20);	  // wait 20 msec
  DHT_OUT_SetWait(1,1);   // data=1, wait 1 usec
  // data line is now input

  // 20~40us HIGH -> ~80us LOW -> ~80 us HIGH)
  // measure DHT11 Pull down pulse
  len = DHT_IN_Length(0, 100);
  if ((len < 40) || (len >= 100)) return 1;
  // measure DHT11 Pull up pulse (~80us)
  len = DHT_IN_Length(1, 100);
  if ((len < 40) || (len >= 100)) return 1;
  return 0;  // OK
}

//===========================================
static int lsensor_dht11_init( lua_State* L )
{
  unsigned pin=0;
  pin = luaL_checkinteger( L, 1 );
  gpio_get_handle(pin, &handle_DHT11);
  DHT11_22 = 0;
  
  if (lua_gettop(L) > 1) {
    DHT11_22 = luaL_checkinteger( L, 2 );
  }
  
  // set data high (pull up input)
  DHT_OUT_SetWait(1,40);  // data=1, wait 40 usec
  vm_thread_sleep(50);
  
  if(DHT11_Check()==0)
    lua_pushinteger(L, 1);
  else
     lua_pushinteger(L, 0);
  return 1;
}

//==========================================
static int lsensor_dht11_get( lua_State* L )
{
  if (handle_DHT11 < 0) {
    l_message( NULL, "init DHT11 first" );
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 4);
    return 3;
  }
  
  unsigned char i,j,dat,len,stat;
  unsigned char buf[5];
  unsigned int csum;
  int t, h;
  
  t = 0;
  h = 0;
  stat = 0;
  if(DHT11_Check()==0)
  {
    // Read bytes
    for (j=0;j<5;j++) {
      dat=0;
      for (i=0;i<8;i++) 
      {
        dat<<=1; // next bit
        // read bit:
        // bit=0:  50 us LOW -> 26~28 us HIGH
        // bit=1:  50 us LOW ->    70 us HIGH
        
        // wait & measure data bit length 
        len = DHT_IN_Length(1, 15000);
        if ((len < 10) || (len >= 90)) {
          stat = 1;
          break;
        }
        if (len > 40 ) dat |= 1;
      }
      if (stat) break;
      buf[j] = dat;  // save byte
    }
    
    if (stat == 0) {
      csum = (buf[0]+buf[1]+buf[2]+buf[3]) & 0xFF;
      if ( csum != buf[4]) stat = 2;
    }
  }
  else {
    stat = 3;
  }

  if (stat == 0) {
    if (DHT11_22) { // DHT22
      h = (buf[0]<<8) | buf[1];
      if (buf[2] & 0x80) t = (((buf[2]&0x7f)<<8) | buf[3]) * -1;
      else t = (buf[2]<<8) | buf[3];
    }
    else { // DHT11
      h = buf[0];
      if (buf[2] & 0x80) t = (buf[2]&0x7f) * -1;
      else t = buf[2];
    }
  }
  lua_pushinteger(L, t);
  lua_pushinteger(L, h);
  lua_pushinteger(L, stat);
  return 3;
}

//----------------------------
unsigned char check_dev(unsigned char n) {
  if (((ow_numdev == 0)) || (n == 0) || (n > ow_numdev)) {
     return 1;
  }
  return 0;  
}

//=============================================
static int lsensor_18b20_getres( lua_State* L )
{
  unsigned char dev = 0;
  unsigned char res = 0;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
     lua_pushnil(L);
     return 1;
  }

  /* Get resolution */
  res = TM_DS18B20_GetResolution(ow_roms[dev-1]);
  lua_pushinteger(L, res);
  
  return 1;
}

//=============================================
static int lsensor_18b20_setres( lua_State* L )
{
  unsigned char dev = 0;
  unsigned char res = 0;
  owState_t stat;
  
  dev = luaL_checkinteger( L, 1 );
  res = luaL_checkinteger( L, 2 );
  if (check_dev(dev)) {
     lua_pushnil(L);
     return 1;
  }

  if ( res!=TM_DS18B20_Resolution_9bits &&
       res!=TM_DS18B20_Resolution_10bits && 
       res!=TM_DS18B20_Resolution_11bits &&
       res!=TM_DS18B20_Resolution_12bits ) {
    res = 12;     
  }
  /* Set resolution */
  stat = TM_DS18B20_SetResolution(ow_roms[dev-1], (TM_DS18B20_Resolution_t)res);
  if (stat != owOK) {
    lua_pushinteger(L, stat);
  }
  else {
    lua_pushinteger(L, TM_DS18B20_GetResolution(ow_roms[dev-1]));
  }
  
  return 1;
}

//==============================================
static int lsensor_18b20_gettemp( lua_State* L )
{
  unsigned char dev = 0;
  unsigned char n = 0;
  owState_t stat;
  unsigned int tmo = 0;
  float temper;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
    lua_pushinteger(L, -9999);
    lua_pushinteger(L, -1);
    return 2;
  }

  /* Start temperature conversion on all devices on one bus */
  TM_DS18B20_StartAll();

  /* Wait until all are done on one onewire port */
  vm_thread_sleep(50);
  tmo = 0;
  n = 0;
  while (tmo < 1000) {
	sys_wdt_rst_time = 0;
	_reset_wdg();

	vm_thread_sleep(5);
    if (TM_OneWire_ReadBit()) {
      n += 1;
      if (n > 2) {
        break;
      }
    }
    else {
      n = 0;
    }
    tmo += 1;
  }
  if (tmo >= 1000) {
    /* Timeout */
    lua_pushinteger(L, -9999);
    lua_pushinteger(L, -2);
    return 2;
  }
  
  /* Read temperature from selected device */
  /* Read temperature from ROM address and store it to temps variable */
  stat = TM_DS18B20_Read(ow_roms[dev-1], &temper);
  if ( stat == owOK) {
    lua_pushnumber(L, temper);
    lua_pushinteger(L, stat);
  }
  else {
    /* Reading error */
    lua_pushinteger(L, -9999);
    lua_pushinteger(L, stat);
  }
  
  return 2;
}

//=============================================
static int lsensor_18b20_startm( lua_State* L )
{
  unsigned char dev = 0;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
    lua_pushinteger(L, 1);
    return 1;
  }

  /* Start temperature conversion on all devices on one bus */
  TM_DS18B20_StartAll();

  lua_pushinteger(L, 0);
  return 1;
}

//==========================================
static int lsensor_18b20_get( lua_State* L )
{
  unsigned char dev = 0;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
    lua_pushinteger(L, -9999);
    return 1;
  }

  owState_t stat;
  float temper;

  /* Wait until all are done on one onewire port */
  if (TM_OneWire_ReadBit() == 0) {
   lua_pushinteger(L, -9999);
    return 1;
  }
  
  /* Read temperature from selected device */
  /* Read temperature from ROM address and store it to temps variable */
  stat = TM_DS18B20_Read(ow_roms[dev-1], &temper);
  if ( stat == owOK) {
    lua_pushnumber(L, temper);
  }
  else {
    /* Reading error */
    lua_pushinteger(L, -9999);
  }
  
  return 1;
}

//=============================================
static int lsensor_18b20_search( lua_State* L )
{
  unsigned char count = 0;
  unsigned char owdev = 0;
    
  ow_numdev = 0;
  
  owdev = TM_OneWire_First();
  while (owdev) {
    /* Increase counter */
    count++;
    
    /* Get full ROM value, 8 bytes, give location of first byte where to save */
    TM_OneWire_GetFullROM(ow_roms[count - 1]);
    if (!TM_DS18B20_Is(ow_roms[count - 1])) {
      count--;
    }
    /* Get next device */
    owdev = TM_OneWire_Next();
    if (count >= MAX_ONEWIRE_SENSORS) break;
  }
  
  ow_numdev = count;
  lua_pushinteger(L,count);
  return 1;
}

//========================================
static int lsensor_ow_init( lua_State* L )
{
  unsigned pin=0;
  
  pin = luaL_checkinteger( L, 1 );
  pin = luaL_checkinteger( L, 1 );
  gpio_get_handle(pin, &OW_DEVICE.handle);

  vm_thread_sleep(2);
  
  ow_numdev = 0;
  if (TM_OneWire_Reset() == 0)
    lua_pushinteger(L, 1);
  else
     lua_pushinteger(L, 0);
  return 1;
}

//==========================================
static int lsensor_ow_getrom( lua_State* L )
{
  unsigned dev = 0;
  int i;

  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
     lua_pushnil(L);
     return 1;
  }
  
  lua_newtable(L);
  for (i = 0; i < 8; i++) {
    lua_pushinteger( L, ow_roms[dev-1][i] );
    lua_rawseti(L,-2,i + 1);
  }
  return 1;
}

//==========================================
static int lsensor_ow_search( lua_State* L )
{
  unsigned char count = 0;
  unsigned char owdev = 0;
    
  owdev = TM_OneWire_First();
  while (owdev) {
    /* Increase counter */
    count++;
    
    /* Get full ROM value, 8 bytes, give location of first byte where to save */
    TM_OneWire_GetFullROM(ow_roms[count - 1]);
    
    /* Get next device */
    owdev = TM_OneWire_Next();
    if (count >= MAX_ONEWIRE_SENSORS) break;
  }
  
  ow_numdev = count;
  lua_pushinteger(L,ow_numdev);
  return 1;
}

#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"
static const LUA_REG_TYPE sensor_map[] =
{
  { LSTRKEY( "dht_init" ), LFUNCVAL ( lsensor_dht11_init ) },
  { LSTRKEY( "dht_get" ),  LFUNCVAL ( lsensor_dht11_get ) },
  { LSTRKEY( "ds_init" ),    LFUNCVAL(lsensor_ow_init ) },
  { LSTRKEY( "ds_gettemp" ), LFUNCVAL(lsensor_18b20_gettemp ) },
  { LSTRKEY( "ds_get" ),  LFUNCVAL(lsensor_18b20_get ) },
  { LSTRKEY( "ds_startm" ),  LFUNCVAL(lsensor_18b20_startm ) },
  { LSTRKEY( "ds_search" ),  LFUNCVAL(lsensor_18b20_search ) },
  { LSTRKEY( "ds_getres" ),  LFUNCVAL(lsensor_18b20_getres ) },
  { LSTRKEY( "ds_setres" ),  LFUNCVAL(lsensor_18b20_setres ) },
  { LSTRKEY( "ds_getrom" ),  LFUNCVAL(lsensor_ow_getrom ) },
  { LSTRKEY( "ow_init" ),   LFUNCVAL(lsensor_ow_init ) },
  { LSTRKEY( "ow_search" ), LFUNCVAL(lsensor_ow_search ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "DS18B20_RES9" ),  LNUMVAL( TM_DS18B20_Resolution_9bits ) },
  { LSTRKEY( "DS18B20_RES10" ), LNUMVAL( TM_DS18B20_Resolution_10bits ) },
  { LSTRKEY( "DS18B20_RES11" ), LNUMVAL( TM_DS18B20_Resolution_11bits ) },
  { LSTRKEY( "DS18B20_RES12" ), LNUMVAL( TM_DS18B20_Resolution_12bits ) },
#endif        
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_sensor(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
  luaL_register(L, "sensor", sensor_map);

  MOD_REG_NUMBER( L, "DS18B20_RES9", TM_DS18B20_Resolution_9bits);
  MOD_REG_NUMBER( L, "DS18B20_RES10", TM_DS18B20_Resolution_10bits);
  MOD_REG_NUMBER( L, "DS18B20_RES11", TM_DS18B20_Resolution_11bits);
  MOD_REG_NUMBER( L, "DS18B20_RES12", TM_DS18B20_Resolution_12bits);
  return 1;
#endif  
}
