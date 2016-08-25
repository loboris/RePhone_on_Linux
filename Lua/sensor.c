/**
 * sensor.c
 * ONEWIRE adapted by LoBo from TM_ONEWIRE (author  Tilen Majerle)
 */

#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "vmdatetime.h"
#include "vmthread.h"
#include "vmlog.h"

#include "bme280.h"

//#define DS18B20ALARMFUNC
//#define DS18B20_USE_CRC

#define BME280_I2C_SPEED			400

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


//* TM_DS18B20_Macros
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE			0x28
#define DS18S20_FAMILY_CODE			0x10
#define DS1822_FAMILY_CODE			0x22
#define DS28EA00_FAMILY_CODE		0x42
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
  owError_BadCRC,
  owError_NotReady,
  owError_Convert
} owState_t;

typedef struct {
	VM_DCL_HANDLE handle;          /*!< GPIO Pin handle to be used for I/O functions */
	unsigned char LastDiscrepancy;       /*!< Search private */
	unsigned char LastFamilyDiscrepancy; /*!< Search private */
	unsigned char LastDeviceFlag;        /*!< Search private */
	unsigned char ROM_NO[8];             /*!< 8-bytes address of last search device */
} TM_OneWire_t;


#define MAX_ONEWIRE_SENSORS 2

static TM_OneWire_t OW_DEVICE;
static int ds_parasite_pwr = 0;
static unsigned char ow_numdev = 0;
static unsigned char ow_roms[MAX_ONEWIRE_SENSORS][8];
static int ds_measure_time = 800;
static VM_TIME_UST_COUNT ds_start_measure_time = 0;

static struct bme280_t bme280;

#ifdef DS18B20ALARMFUNC
static unsigned char ow_alarm_device [MAX_ONEWIRE_SENSORS][8];
#endif

//**************************************************

extern VMUINT8 g_i2c_used_address;
extern VM_DCL_HANDLE g_i2c_handle;
extern int _i2c_setup(VMUINT8 address, VMUINT32 speed);
extern int _i2c_writedata(char *data, VMUINT32 len);
extern int _i2c_write_and_read_data(uint8_t *wdata, uint8_t *rdata, VMUINT32 wlen, VMUINT32 rlen);
extern int gpio_get_handle(int pin, VM_DCL_HANDLE* handle);


//******************
// ONEWIRE FUNCTIONS
//******************

//---------------------------------
static unsigned char TM_OneWire_Reset() {
	unsigned char bit = 1;
	int i;
    vm_dcl_gpio_control_level_status_t data;

    VMUINT32 imask = vm_irq_mask();
	i = 330;
	// Set line low and wait ~500 us
	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	while (i > 0) {
		vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		i--;
	}

	// Release the line and wait 500 us for line value
	i = 330;
	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_READ, &data);
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_READ, &data);
	while (i > 0) {
	    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_READ, &data);
	    if (data.level_status == 0) {
	    	bit = 0;
	    	break;
	    }
	    i--;
	}
	if (bit == 0) {
		// wait up to 500 us
		while (i > 0) {
		  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
			i--;
		}
	}
	vm_irq_restore(imask);
    // Return value of presence pulse, 0 = OK, 1 = ERROR
    return bit;
}

// DS1820 WRITE slot
//--------------------------------------------------
static void TM_OneWire_WriteBit(unsigned char bit) {
  int i = 0;
  VMUINT32 imask = vm_irq_mask();
  if (bit) {
	// ** Bit high
	// Set line low and wait 8 us
	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);

	// Release the line and wait ~65 us
	i = 45;
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	while (i > 0) {
	  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
		i--;
	}
  }
  else {
    // ** Bit low
	i = 45;
	// Set line low and wait ~65 us
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	while (i > 0) {
		vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		i--;
	}
    // Release the line and wait 5 us
    vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  }
  vm_irq_restore(imask);
}

// DS1820 READ slot
//-----------------------------------------
static unsigned char TM_OneWire_ReadBit() {
  unsigned char bit = 1;
  vm_dcl_gpio_control_level_status_t data;
  int i = 0;

  VMUINT32 imask = vm_irq_mask();
  // Set line low and wait 3 us
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);

  // Release the line and wait ~65 us for line value
  i = 40;
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  while (i > 0) {
      vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_READ, &data);
      if (data.level_status == 0) {
    	  bit = 0;
    	  break;
      }
	  i--;
  }
  if (bit == 0) {
	// wait up to 65 us
	while (i > 0) {
	  	vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
		i--;
	}
  }
  vm_irq_restore(imask);
  // Return bit value
  return bit;
}

//----------------------------------------------------
static void TM_OneWire_WriteByte(unsigned char byte) {
  unsigned char i = 8;
  // Write 8 bits
  while (i--) {
    // LSB bit is first
    TM_OneWire_WriteBit(byte & 0x01);
    byte >>= 1;
  }
}

//------------------------------------------
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

//-------------------------------------------------------------
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

//---------------------------------------
static unsigned char TM_OneWire_First() {
  // Reset search values
  TM_OneWire_ResetSearch();
  // Start with searching
  return TM_OneWire_Search(ONEWIRE_CMD_SEARCHROM);
}

//--------------------------------------
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

//-------------------------------------------------------------
static void TM_OneWire_SelectWithPointer(unsigned char *ROM) {
  unsigned char i;
  TM_OneWire_WriteByte(ONEWIRE_CMD_MATCHROM);
  for (i = 0; i < 8; i++) {
    TM_OneWire_WriteByte(*(ROM + i));
  }	
}

//------------------------------------------------------------
static void TM_OneWire_GetFullROM(unsigned char *firstIndex) {
  unsigned char i;
  for (i = 0; i < 8; i++) {
    *(firstIndex + i) = OW_DEVICE.ROM_NO[i];
  }
}

//----------------------------------------------------------------------------
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

//------------------------------------------------------
static unsigned char TM_DS18B20_Is(unsigned char *ROM) {
  /* Checks if first byte is equal to DS18B20's family code */
  if ((*ROM == DS18B20_FAMILY_CODE) ||
	  (*ROM == DS18S20_FAMILY_CODE) ||
	  (*ROM == DS1822_FAMILY_CODE)  ||
	  (*ROM == DS28EA00_FAMILY_CODE)) {
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
  // Reset pulse
  if (TM_OneWire_Reset() != 0) return;
  // Skip rom
  TM_OneWire_WriteByte(ONEWIRE_CMD_SKIPROM);
  // Test parasite power
  TM_OneWire_WriteByte(ONEWIRE_CMD_RPWRSUPPLY);
  if (TM_OneWire_ReadBit() == 0) ds_parasite_pwr = 1;
  else ds_parasite_pwr = 0;
  //vm_log_debug("DS18B20 Parasite Pwr = %d", ds_parasite_pwr);

  // Reset pulse
  if (TM_OneWire_Reset() != 0) return;
  // Skip rom
  TM_OneWire_WriteByte(ONEWIRE_CMD_SKIPROM);
  // Start conversion on all connected devices
  TM_OneWire_WriteByte(DS18B20_CMD_CONVERTTEMP);
  if (ds_parasite_pwr) {
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
	  ds_start_measure_time = vm_time_ust_get_count();
  }
}

//------------------------------------------------------------------------
static owState_t TM_DS18B20_Read(unsigned char *ROM, double *destination) {
  unsigned int temperature;
  unsigned char resolution;
  char digit, minus = 0;
  double decimal;
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
  if (*ROM != DS18S20_FAMILY_CODE) {
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
		  decimal *= (double)DS18B20_DECIMAL_STEPS_9BIT;
		} break;
		case 10: {
		  decimal = (temperature >> 2) & 0x03;
		  decimal *= (double)DS18B20_DECIMAL_STEPS_10BIT;
		} break;
		case 11: {
		  decimal = (temperature >> 1) & 0x07;
		  decimal *= (double)DS18B20_DECIMAL_STEPS_11BIT;
		} break;
		case 12: {
		  decimal = temperature & 0x0F;
		  decimal *= (double)DS18B20_DECIMAL_STEPS_12BIT;
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

  }
  else {
	if (!data[7]) {
	    return owError_Convert;
	}
	if (data[1] == 0) {
		temperature = ((int)(data[0] >> 1))*1000;
	}
	else { // negative
		temperature = 1000*(-1*(int)(0x100-data[0]) >> 1);
	}
	temperature -= 250;
	decimal = 1000*((int)(data[7] - data[6]));
	decimal /= (int)data[7];
	temperature += decimal;
    /* Set to pointer */
	*destination = (double)temperature / 1000.0;
  }
  /* Return 1, temperature valid */
  return owOK;
}

//-----------------------------------------------------------------
static unsigned char TM_DS18B20_GetResolution(unsigned char *ROM) {
  unsigned char conf;

  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }

  if (*ROM == DS18S20_FAMILY_CODE) return TM_DS18B20_Resolution_12bits;

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

//-------------------------------------------------------------------------------------------------
static owState_t TM_DS18B20_SetResolution(unsigned char *ROM, TM_DS18B20_Resolution_t resolution) {
  unsigned char th, tl, conf;
  if (!TM_DS18B20_Is(ROM)) {
    return owError_Not18b20;
  }

  if (*ROM == DS18S20_FAMILY_CODE) return owOK;

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
  if (ds_parasite_pwr) {
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
  }
  vm_thread_sleep(12);
  if (ds_parasite_pwr) {
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  }

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


//----------------------------
unsigned char check_dev(unsigned char n) {
  if (((ow_numdev == 0)) || (n == 0) || (n > ow_numdev)) {
     return 1;
  }
  return 0;  
}

//-------------------------------------------
static void _set_measure_time(int resolution)
{
  switch (resolution) {
	case 9:
	  ds_measure_time = 100;
	  break;
	case 10:
	  ds_measure_time = 200;
	  break;
	case 11:
	  ds_measure_time = 400;
	  break;
	case 12:
	  ds_measure_time = 800;
	  break;
	default:
	  ds_measure_time = 800;
  }
}

// Set DS1820 resolution
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

  // Get resolution
  res = TM_DS18B20_GetResolution(ow_roms[dev-1]);
  _set_measure_time(res);

  lua_pushinteger(L, res);
  lua_pushinteger(L, ds_measure_time);
  
  return 2;
}

// Set DS1820 resolution
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
    res = TM_DS18B20_Resolution_12bits;
  }
  // Set resolution
  if (ow_roms[dev-1][0] == DS18S20_FAMILY_CODE) {
    res = TM_DS18B20_Resolution_12bits;
    stat = owOK;
  }
  else {
	stat = TM_DS18B20_SetResolution(ow_roms[dev-1], (TM_DS18B20_Resolution_t)res);
  }
  
  if (stat != owOK) lua_pushinteger(L, stat);
  else {
	  res = TM_DS18B20_GetResolution(ow_roms[dev-1]);
	  lua_pushinteger(L, res);
  }
  _set_measure_time(res);

  return 1;
}

// Start temperature measurement and wait for result
//==============================================
static int lsensor_18b20_gettemp( lua_State* L )
{
  unsigned char dev = 0;
  owState_t stat;
  double temper;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
    lua_pushinteger(L, -9999);
    lua_pushinteger(L, -1);
    return 2;
  }

  int res = TM_DS18B20_GetResolution(ow_roms[dev-1]);
  _set_measure_time(res);

  // Start temperature conversion on all devices on one bus
  TM_DS18B20_StartAll();

  // Wait until all are done on one onewire port (max 1.5 second)
  unsigned int tmo;
  if (ds_parasite_pwr) {
	  tmo = 0;
	  while (tmo < ds_measure_time) {
		vm_thread_sleep(10);
		tmo += 10;
	  }
      vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
	  ds_start_measure_time = 0;
	  vm_thread_sleep(2);
  }
  else {
	  tmo = 0;
	  while (tmo < ds_measure_time) {
		vm_thread_sleep(10);
		if (TM_OneWire_ReadBit()) break;
		tmo += 10;
	  }
	  if (tmo >= ds_measure_time) {
		/* Timeout */
		lua_pushinteger(L, -9999);
		lua_pushinteger(L, owError_NotReady);
		return 2;
	  }
  }
  
  // Read temperature from selected device
  // Read temperature from ROM address and store it to temper variable
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

// Start temperature measurement on all devices
//=============================================
static int lsensor_18b20_startm( lua_State* L )
{
  if (ow_numdev == 0) {
	// no devices
    lua_pushinteger(L, -1);
    return 1;
  }

  // Start temperature conversion on all devices on one bus
  TM_DS18B20_StartAll();

  lua_pushinteger(L, 0);
  return 1;
}

// Get the last measured value from device
//==========================================
static int lsensor_18b20_get( lua_State* L )
{
  unsigned char dev = 0;
  
  dev = luaL_checkinteger( L, 1 );
  if (check_dev(dev)) {
    lua_pushinteger(L, -9999);
	lua_pushinteger(L, owError_Not18b20);
	return 2;
  }

  // Check if measurement finished
  if (ds_parasite_pwr) {
	  if (vm_time_ust_get_duration(ds_start_measure_time, vm_time_ust_get_count()) < (ds_measure_time * 1000)) {
		lua_pushinteger(L, -9999);
		lua_pushinteger(L, owError_NotReady);
		return 2;
	  }
      vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
	  ds_start_measure_time = 0;
	  vm_thread_sleep(2);
  }
  else {
	  if (TM_OneWire_ReadBit() == 0) {
		lua_pushinteger(L, -9999);
		lua_pushinteger(L, owError_NotReady);
		return 2;
	  }
  }

  owState_t stat;
  double temper;

  // Read temperature from selected device
  // Read temperature from ROM address and store it to temper variable
  stat = TM_DS18B20_Read(ow_roms[dev-1], &temper);

  if ( stat == owOK) lua_pushnumber(L, temper);
  else lua_pushinteger(L, -9999); // error
  lua_pushinteger(L, stat);
  
  return 2;
}

// Search for DS1820 devices
//=============================================
static int lsensor_18b20_search( lua_State* L )
{
  unsigned char count = 0;
  unsigned char owdev = 0;
    
  // Get first device
  owdev = TM_OneWire_First();
  while (owdev) {
    count++; // Increase device counter
    // Get full ROM value, 8 bytes, give location of first byte where to save
    TM_OneWire_GetFullROM(ow_roms[count - 1]);
    // Check if it is DS1820 device
    if (!TM_DS18B20_Is(ow_roms[count - 1])) count--; // error
    // Get next device
    owdev = TM_OneWire_Next();
    if (count >= MAX_ONEWIRE_SENSORS) break;
  }
  
  ow_numdev = count;
  lua_pushinteger(L, count);
  return 1;
}

// Search for any 1-wire device
//==========================================
static int lsensor_ow_search( lua_State* L )
{
  unsigned char count = 0;
  unsigned char owdev = 0;

  owdev = TM_OneWire_First();
  while (owdev) {
    count++;  // Increase device counter

    // Get full ROM value, 8 bytes, give location of first byte where to save
    TM_OneWire_GetFullROM(ow_roms[count - 1]);

    // Get next device
    owdev = TM_OneWire_Next();
    if (count >= MAX_ONEWIRE_SENSORS) break;
  }

  ow_numdev = count;
  lua_pushinteger(L, ow_numdev);
  return 1;
}

// Initialize 1-wire bus on given gpio
//========================================
static int lsensor_ow_init( lua_State* L )
{
  unsigned pin=0;
  
  pin = luaL_checkinteger( L, 1 );
  if (gpio_get_handle(pin, &OW_DEVICE.handle) == VM_DCL_HANDLE_INVALID) {
      return luaL_error(L, "invalid pin handle");
  }

  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  vm_dcl_control(OW_DEVICE.handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

  vm_thread_sleep(2);
  
  ow_numdev = 0;
  // Check if any device connected
  if (TM_OneWire_Reset() == 0) lua_pushinteger(L, 1);
  else lua_pushinteger(L, 0);
  return 1;
}

// Get device ROM (8 bytes) to Lua table or string
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


// =====================
// DHT11/DHT22 functions
// =====================

#define MAX_BIT_WAIT_TIME 100

//--------------------------
static int _dht11_read(unsigned char *dhtdata, VM_DCL_HANDLE handle)
{
    vm_dcl_gpio_control_level_status_t data;
	VMUINT8  state = 1;
	int      start_time;
	int      min_time = 9000;
	int      max_time = 0;
	VMUINT8  data_idx = 0;
	VMUINT8  bit_no = 0;

    int err = 0;

    VMUINT32 imask = vm_irq_mask();
    while (err == 0) {
	    switch (state) {
        	case 1:  // wait dht first pulse
				start_time = 0;
				// --- wait for low state, response to start signal
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 0 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -1; // no start pulse low
					continue;
				}
				state = 2;
				data_idx = 0;
				bit_no = 0;
				start_time = 0;
				// --- wait for high state, end of response signal
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 1 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -2; // no start pulse high
					continue;
				}
				start_time = 0;
				// --- wait for low state, start of transmision, first bit of the firs byte
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 0 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -3; // no start of transmision
					continue;
				}
        		break;
        	case 2:  // dht read data
				start_time = 0;
				// --- wait for high state, bit value
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 1 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -4; // no bit pulse high
					continue;
				}
				start_time = 0;
				// wait for low state, measure bit time
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 0 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -5; // no bit pulse low
					continue;
				}
				if (start_time > max_time) max_time = start_time;
				if (start_time < min_time) min_time = start_time;

				dhtdata[data_idx] <<= 1;
				if (start_time > 35 ) {
					dhtdata[data_idx] |= 1;
				}
				bit_no++;          // next bit
				if (bit_no > 7) {  // next byte
					bit_no = 0;
					data_idx++;
					if (data_idx > 4) state = 3;
					else dhtdata[data_idx] = 0;
				}
        		break;
        	case 3:  // dht read last pulse
				start_time = 0;
				// wait for high state
				while (start_time < MAX_BIT_WAIT_TIME) {
					vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
					if (data.level_status == 1 ) break;
					start_time++;
				}
				if (start_time >= MAX_BIT_WAIT_TIME ) {
					err = -6; // no end pulse high
					continue;
				}
				err = 1;
				break;
        	default:
				err = -7;
				continue;
		}
	}
    vm_irq_restore(imask);
    /*
    printf("state=%d idx=%d bit=%d min=%d max=%d\n", state, data_idx, bit_no, min_time, max_time);
    if (data_idx > 0) {
  	  for (int i=0; i<5; i++) {
  		  printf(" %d", dhtdata[i]);
  	  }
  	  printf("\n");
    }
    */
    if (err == 1) err = 0;
    return err;
}

//===========================================
static int lsensor_dht11_get( lua_State* L )
{
  VM_DCL_HANDLE handle_DHT11 = -1;
  int DHT11_22 = 0;
  int pin=0;
  VMUINT8 dht_data[5];

  pin = luaL_checkinteger( L, 1 );
  if (gpio_get_handle(pin, &handle_DHT11) == VM_DCL_HANDLE_INVALID) {
      return luaL_error(L, "invalid pin handle");
  }

  if (lua_gettop(L) > 1) {
    DHT11_22 = luaL_checkinteger( L, 2 );
  }

  memset(dht_data, 0x00, 5);
  // send 25 msec reset pulse
  vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
  vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
  vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
  vm_thread_sleep(25);
  vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  vm_dcl_control(handle_DHT11, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

  // read data
  int t=0, h=0;
  int stat = _dht11_read(dht_data, handle_DHT11);

  if (stat == 0) {
	  unsigned int csum;
	  csum = dht_data[0] + dht_data[1] + dht_data[2] + dht_data[3];

	  if (csum == dht_data[4]) {
		  if (DHT11_22) { // DHT22
			h = (dht_data[0]<<8) | dht_data[1];
			if (dht_data[2] & 0x80) t = (((dht_data[2]&0x7f)<<8) | dht_data[3]) * -1;
			else t = (dht_data[2]<<8) | dht_data[3];
		  }
		  else { // DHT11
			h = dht_data[0];
			if (dht_data[2] & 0x80) t = (dht_data[2]&0x7f) * -1;
			else t = dht_data[2];
		  }
	  }
	  else csum = -9;
  }
  lua_pushinteger(L, t);
  lua_pushinteger(L, h);
  lua_pushinteger(L, stat);
  return 3;
}

/*
//=====================================
static int lsensor_test( lua_State* L )
{
  VM_DCL_HANDLE handle = -1;
  VMUINT32 imask;
  int i, j;
  unsigned char bit;
  vm_dcl_gpio_control_level_status_t data;

  int pin = luaL_checkinteger( L, 1 );
  int tmo = luaL_checkinteger( L, 2 );
  int tmo1 = luaL_checkinteger( L, 3 );

  if (gpio_get_handle(pin, &handle) == VM_DCL_HANDLE_INVALID) {
      return luaL_error(L, "invalid pin handle");
  }

  vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);

  VM_TIME_UST_COUNT startt = vm_time_ust_get_count();
  vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
  lua_pushinteger(L, vm_time_ust_get_duration(startt, vm_time_ust_get_count()));

  i = tmo;
  startt = vm_time_ust_get_count();
  //imask = vm_irq_mask();
  while (i > 0) {
	  vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
	i--;
  }
  //vm_irq_restore(imask);
  lua_pushinteger(L, vm_time_ust_get_duration(startt, vm_time_ust_get_count()));

  startt = vm_time_ust_get_count();
  vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
  vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
  lua_pushinteger(L, vm_time_ust_get_duration(startt, vm_time_ust_get_count()));

  i = tmo1;
  startt = vm_time_ust_get_count();
  //imask = vm_irq_mask();
  while (i > 0) {
		vm_dcl_control(handle, VM_DCL_GPIO_COMMAND_READ, &data);
		if (data.level_status == 0) {
			bit = 0;
		}
		i--;
  }
  //vm_irq_restore(imask);
  lua_pushinteger(L, vm_time_ust_get_duration(startt, vm_time_ust_get_count()));
  return 4;
}
*/

// ================================ BME280 ===========================================

#define	I2C_BUFFER_LEN 8
#define BME280_DATA_INDEX	1
#define BME280_ADDRESS_INDEX	2

/*-------------------------------------------------------------------*
*	This is a sample code for read and write the data by using I2C
*	The device address defined in the bme280.h file
*--------------------------------------------------------------------*/

/*	\Brief: The function is used as I2C bus write
 *	\Return : Status of the I2C write
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */

//---------------------------------------------------------------------
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	s32 iError = BME280_INIT_VALUE;
	u8 array[I2C_BUFFER_LEN];
	u8 stringpos = BME280_INIT_VALUE;
	array[BME280_INIT_VALUE] = reg_addr;

	for (stringpos = BME280_INIT_VALUE; stringpos < cnt; stringpos++) {
		array[stringpos + BME280_DATA_INDEX] = *(reg_data + stringpos);
	}

	if ((g_i2c_used_address != dev_addr) || (g_i2c_handle < 0)) _i2c_setup(dev_addr, BME280_I2C_SPEED);

	iError = _i2c_writedata(array, cnt+1);

	return (s8)iError;
}

 /*	\Brief: The function is used as I2C bus write&read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of data byte of to be read
 */
//--------------------------------------------------------------------
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
    s32 iError = BME280_INIT_VALUE;
	u8 regadr = reg_addr;

    // Send and receive
	if ((g_i2c_used_address != dev_addr) || (g_i2c_handle < 0)) _i2c_setup(dev_addr, BME280_I2C_SPEED);

	iError = _i2c_write_and_read_data(&regadr, reg_data, 1, cnt);

	return (s8)iError;
}

/*	Brief : The delay routine
 *	\param : delay in ms
*/
//------------------------------
void BME280_delay_msek(u32 msek)
{
    vm_thread_sleep(msek);
}

//---------------------------------------------------------
static void bme280_getsby(lua_State* L, int index, u8 *sby)
{
	if (lua_gettop(L) >= index) {
		int sb = luaL_checkinteger(L, index);
		if (sb < 10) *sby = BME280_STANDBY_TIME_1_MS;
		else if (sb < 20) *sby = BME280_STANDBY_TIME_10_MS;
		else if (sb < 62) *sby = BME280_STANDBY_TIME_20_MS;
		else if (sb < 125) *sby = BME280_STANDBY_TIME_63_MS;
		else if (sb < 250) *sby = BME280_STANDBY_TIME_125_MS;
		else if (sb < 500) *sby = BME280_STANDBY_TIME_250_MS;
		else if (sb < 1000) *sby = BME280_STANDBY_TIME_500_MS;
		else *sby = BME280_STANDBY_TIME_1000_MS;
	}
}

//====================================
static int bme280_doinit(lua_State* L)
{
	u8 v_stand_by_time_u8 = BME280_INIT_VALUE;
	s32 com_rslt = ERROR;
	u8 mode = BME280_SLEEP_MODE;
	u8 sby = BME280_STANDBY_TIME_125_MS;

	u8 addr = luaL_checkinteger(L, 1);
	if ((addr != BME280_I2C_ADDRESS1) && (addr != BME280_I2C_ADDRESS2)) addr = BME280_I2C_ADDRESS1;

	if (lua_gettop(L) >= 2) {
		int md = luaL_checkinteger(L, 2);
		if ((md == BME280_NORMAL_MODE) || (md == BME280_FORCED_MODE)) mode = md;

		if (mode == BME280_NORMAL_MODE) {
			bme280_getsby(L, 3, &sby);
		}
	}

	bme280.chip_id = 0;
	bme280.mode = 255;
	bme280.bus_write = BME280_I2C_bus_write;
	bme280.bus_read = BME280_I2C_bus_read;
	bme280.dev_addr = addr;
	bme280.delay_msec = BME280_delay_msek;

	/*--------------------------------------------------------------------------*
	 *  This function used to assign the value/reference of
	 *	the following parameters
	 *	I2C address
	 *	Bus Write
	 *	Bus read
	 *	Chip id
	*-------------------------------------------------------------------------*/
	com_rslt = bme280_init(&bme280);
	if (com_rslt == BME280_CHIP_ID_READ_SUCCESS) {
		bme280.mode = mode;

		com_rslt += bme280_set_soft_rst();
		vm_thread_sleep(5);

		//	For reading the pressure, humidity and temperature data it is required to
		//	set the OSS setting of humidity, pressure and temperature
		// set the humidity oversampling
		com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
		// set the pressure oversampling
		com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_2X);
		// set the temperature oversampling
		com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_4X);
		// set standby time
		com_rslt += bme280_set_standby_durn(sby);

		com_rslt += bme280_set_power_mode(mode);
	}

	// **************** END INITIALIZATION ****************
	lua_pushinteger(L, com_rslt);
	return 1;
}

//=====================================
static int bme280_setmode(lua_State* L)
{
	u8 mode, md;
	s32 com_rslt = ERROR;

	if (bme280.chip_id == BME280_CHIP_ID) {
		md = luaL_checkinteger(L, 1);
		if ((md == BME280_SLEEP_MODE) || (md == BME280_FORCED_MODE) || (md == BME280_NORMAL_MODE)) {
			com_rslt = bme280_get_power_mode(&mode);
			if (com_rslt == 0) {
				if (mode != md) {
					com_rslt = bme280_set_power_mode(md);
					if (com_rslt == 0) {
						bme280.mode = md;
						if (md == BME280_NORMAL_MODE) {
							u8 sby = 255;
							bme280_getsby(L, 2, &sby);
							if (sby < 8) com_rslt = bme280_set_standby_durn(sby);
						}
					}
				}
			}
		}
	}

	lua_pushinteger(L, com_rslt);
	return 1;
}

//=====================================
static int bme280_getmode(lua_State* L)
{
	u8 mode;
	u8 sby = 255;
	s32 com_rslt = ERROR;

	if (bme280.chip_id == BME280_CHIP_ID) {
		com_rslt = bme280_get_power_mode(&mode);
		if (com_rslt == 0) {
			com_rslt += bme280_get_standby_durn(&sby);
			if (com_rslt != 0) sby = 255;
			com_rslt = mode;
		}
	}

	lua_pushinteger(L, com_rslt);
	if (sby < 255) {
		int sb = 1000;
		if (sby == BME280_STANDBY_TIME_1_MS) sb = 1;
		else if (sby == BME280_STANDBY_TIME_10_MS) sb = 10;
		else if (sby == BME280_STANDBY_TIME_20_MS) sb = 20;
		else if (sby == BME280_STANDBY_TIME_63_MS) sb = 63;
		else if (sby == BME280_STANDBY_TIME_125_MS) sb = 125;
		else if (sby == BME280_STANDBY_TIME_250_MS) sb = 250;
		else if (sby == BME280_STANDBY_TIME_500_MS) sb = 500;
		lua_pushinteger(L, sb);
	}
	else lua_pushnil(L);

	return 2;
}

//=================================
static int bme280_get(lua_State* L)
{
	// The variable used to read uncompensated temperature
	s32 v_data_uncomp_temp_s32 = BME280_INIT_VALUE;
	// The variable used to read uncompensated pressure
	s32 v_data_uncomp_pres_s32 = BME280_INIT_VALUE;
	// The variable used to read uncompensated pressure
	s32 v_data_uncomp_hum_s32 = BME280_INIT_VALUE;

	double temp = 0.0;
	double pres = 0.0;
	double hum = 0.0;

	s32 com_rslt = ERROR;
	if (bme280.chip_id == BME280_CHIP_ID) {
		if (bme280.mode != BME280_NORMAL_MODE)
			com_rslt = bme280_get_forced_uncomp_pressure_temperature_humidity(
					&v_data_uncomp_pres_s32, &v_data_uncomp_temp_s32, &v_data_uncomp_hum_s32);
		else com_rslt = bme280_read_uncomp_pressure_temperature_humidity(
				&v_data_uncomp_pres_s32, &v_data_uncomp_temp_s32, &v_data_uncomp_hum_s32);

		//printf("BME280 READ uncomp %d, data: %d  %d  %d\n", com_rslt, v_data_uncomp_pres_s32, v_data_uncomp_temp_s32, v_data_uncomp_hum_s32);
		temp = bme280_compensate_temperature_double(v_data_uncomp_temp_s32);
		pres = bme280_compensate_pressure_double(v_data_uncomp_pres_s32);
		hum = bme280_compensate_humidity_double(v_data_uncomp_hum_s32);
	}

	lua_pushnumber(L, temp);
	lua_pushnumber(L, hum);
	lua_pushnumber(L, pres);
	return 3;
}

//====================================================================================


#define MOD_REG_NUMBER(L, name, value) \
    lua_pushnumber(L, value);          \
    lua_setfield(L, -2, name)

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"
static const LUA_REG_TYPE sensor_map[] =
{
  //{ LSTRKEY( "test" ),			LFUNCVAL(lsensor_test) },
  { LSTRKEY( "dht_get" ),			LFUNCVAL(lsensor_dht11_get ) },
  { LSTRKEY( "ds_init" ),			LFUNCVAL(lsensor_ow_init ) },
  { LSTRKEY( "ds_gettemp" ),		LFUNCVAL(lsensor_18b20_gettemp ) },
  { LSTRKEY( "ds_get" ),			LFUNCVAL(lsensor_18b20_get ) },
  { LSTRKEY( "ds_startm" ),			LFUNCVAL(lsensor_18b20_startm ) },
  { LSTRKEY( "ds_search" ),			LFUNCVAL(lsensor_18b20_search ) },
  { LSTRKEY( "ds_getres" ),			LFUNCVAL(lsensor_18b20_getres ) },
  { LSTRKEY( "ds_setres" ),			LFUNCVAL(lsensor_18b20_setres ) },
  { LSTRKEY( "ds_getrom" ),			LFUNCVAL(lsensor_ow_getrom ) },
  { LSTRKEY( "ow_init" ),			LFUNCVAL(lsensor_ow_init ) },
  { LSTRKEY( "ow_search" ),			LFUNCVAL(lsensor_ow_search ) },
  { LSTRKEY( "bme280_init" ),		LFUNCVAL(bme280_doinit ) },
  { LSTRKEY( "bme280_getmode" ),	LFUNCVAL(bme280_getmode ) },
  { LSTRKEY( "bme280_setmode" ),	LFUNCVAL(bme280_setmode ) },
  { LSTRKEY( "bme280_get" ),		LFUNCVAL(bme280_get ) },
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
	bme280.chip_id = 0;
	bme280.mode = 255;

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
