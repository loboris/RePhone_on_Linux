
WORKSPACE_PATH = ../

PROJECT = lua

OBJECTS += main.o retarget.o shell.o audio.o bt.o gsm.o timer.o gpio.o i2c.o spi.o net.o https.o sntp.o struct.o sensor.o bme280.o
OBJECTS += screen.o lua_cjson.o fpconv.o strbuf.o md5.o sha1.o sha2.o bit.o
OBJECTS += mqtt.o mqtt_client.o mqtt_packet.o mqtt_socket.o mqttnet.o email.o ftp.o
OBJECTS += lcd.o DefaultFont.o tjpgd.o term.o term_mdl.o

OBJECTS += lua/lapi.o lua/lcode.o lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o lua/llex.o \
lua/lmem.o lua/lobject.o lua/lopcodes.o lua/lparser.o lua/lstate.o lua/lstring.o lua/ltable.o lua/ltm.o \
lua/lundump.o lua/lvm.o lua/lzio.o lua/lrotable.o lua/lauxlib.o lua/lbaselib.o lua/ldblib.o lua/liolib.o \
lua/lmathlib.o lua/loslib.o lua/ltablib.o lua/lstrlib.o lua/loadlib.o lua/linit.o lua/print.o \
lua/linenoise.o lua/legc.o \
$(WORKSPACE_PATH)common/ymodem.o $(WORKSPACE_PATH)common/CheckSumUtils.o

INCLUDE_PATHS += -I./lua

# ===============================================================================

#PROJECT_PATH := $(patsubst %/Makefile,%,$(realpath $(lastword $(MAKEFILE_LIST))))
#PROJECT_PATH := $(patsubst %/makefile,%,$(PROJECT_PATH))

GCC_BIN ?= /usr/bin/
OBJECTS += $(WORKSPACE_PATH)common/lcd_sitronix_st7789s.o $(WORKSPACE_PATH)common/tp_goodix_gt9xx.o $(WORKSPACE_PATH)common/tp_i2c.o
SYS_OBJECTS += $(WORKSPACE_PATH)common/gccmain.o
INCLUDE_PATHS += -I. -I$(WORKSPACE_PATH)include -I$(WORKSPACE_PATH)common
LIBRARY_PATHS += -L$(WORKSPACE_PATH)lib
LIBRARIES += $(WORKSPACE_PATH)lib/LINKIT10/armgcc/libmtk.a -lm
#LIBRARIES += $(WORKSPACE_PATH)lib/LINKIT10/armgcc/percommon.a -lm
LINKER_SCRIPT = $(WORKSPACE_PATH)lib/LINKIT10/armgcc/scat.ld

###############################################################################
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE    = $(GCC_BIN)arm-none-eabi-size

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	PACK    = python $(WORKSPACE_PATH)tools/packtag.py
	PUSH    = @echo "set RePhone into storage mode, put $(PROJECT).vxp into MRE directory and replace vxp file name with $(PROJECT).vxp"
endif
ifeq ($(UNAME_S),Darwin)
	PACK    = python $(WORKSPACE_PATH)tools/packtag.py
	PUSH    = @echo "set RePhone into storage mode, put $(PROJECT).vxp into MRE directory and replace vxp file name with $(PROJECT).vxp"
endif

CPU = -mcpu=arm7tdmi-s -mthumb -mlittle-endian
#CC_FLAGS = $(CPU) -c -fvisibility=hidden -fpic -Os -Wa,-adhlns=$(PROJECT).lst -save-temps
CC_FLAGS = $(CPU) -c -fvisibility=hidden -fpic -Os
#CC_SYMBOLS += -D__HDK_LINKIT_ASSIST_2502__ -D__COMPILER_GCC__
CC_SYMBOLS += -D__HDK_LINKIT_REPHONE__ -D__COMPILER_GCC__

LD_FLAGS = $(CPU) -Os -Wl,--gc-sections --specs=nosys.specs -fpic -pie -Wl,-Map=$(PROJECT).map  -Wl,--entry=gcc_entry -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-unresolved-symbols
LD_SYS_LIBS =


all: $(PROJECT).vxp size

clean:
	rm -f $(PROJECT).vxp $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst *.o *.i *.s $(OBJECTS) $(SYS_OBJECTS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ -Wl,--start-group $^ $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).vxp: $(PROJECT).elf
#	@$(OBJCOPY) --strip-debug $<
	@$(OBJCOPY) --strip-debug --strip-unneeded $<
	@$(PACK) $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

flash: $(PROJECT).vxp
	$(PUSH)
