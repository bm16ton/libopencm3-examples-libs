ENABLE_DEBUG ?=
STM32F7 = 1
ifneq ($(V), 1)
MAKEFLAGS += --no-print-dir
Q := @
endif

OPT_FLAGS ?= -O2

CFLAGS += -Wall -Wextra -Werror -Wno-char-subscripts -DSTM32F7 \
	$(OPT_FLAGS) -std=gnu99 -g3 -MD \
	-I.
LDFLAGS += $(OPT_FLAGS)

ifeq ($(ENABLE_DEBUG), 1)
CFLAGS += -DENABLE_DEBUG
endif

SRC =			\
	main.c		\
	platform.c	\
	cdcacm.c  \
	usbuart.c

CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

OPT_FLAGS = -Os
CFLAGS += -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard \
	-DAPP_START=0x08040000 \
	-I../libopencm3/include -I../libopencm3/include/libopencm3/stm32 -I../libopencm3/include/libopencm3/cm3 \
	-I .

LDFLAGS_BOOT := $(LDFLAGS) \
	-DSTM32F7 \
	-mfpu=fpv5-sp-d16 -mfloat-abi=hard \
	--specs=nano.specs -lopencm3_stm32f7 \
	-Wl,-T,stlinkv3.ld -nostartfiles -lc \
	-Wl,-Map=mapfile -mthumb -mcpu=cortex-m7 -Wl,-gc-sections \
	-L../libopencm3/lib
	-Wl,-Ttext=0x08040000 \

LDFLAGS = $(LDFLAGS_BOOT)

ifeq ($(ENABLE_DEBUG), 1)
LDFLAGS += --specs=rdimon.specs
else
LDFLAGS += --specs=nosys.specs
endif

all:	pill_serial.bin

host_clean:
	-$(Q)$(RM) pill_serial.bin

OBJ = $(SRC:.c=.o)

pill_serial.elf: version.h $(OBJ)
	@echo "  LD      $@"
	$(Q)$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o:	%.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.bin:	%.elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary $^ $@

%.hex:	%
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O ihex $^ $@

.PHONY:	clean host_clean FORCE

clean:	host_clean
	$(Q)echo "  CLEAN"
	-$(Q)$(RM) -f *.o *.d *~ *.elf *.bin $(HOSTFILES)
	-$(Q)$(RM) -f mapfile version.h

version.h: FORCE
	$(Q)echo "  GIT     version.h"
	$(Q)echo "#define FIRMWARE_VERSION \"`git describe --always --dirty`\"" > $@

-include *.d
