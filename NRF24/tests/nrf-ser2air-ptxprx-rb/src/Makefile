BINARY	 = firmware
OBJS	+= main.o
INCDIR	+= -I../../../libnrf24l01/src/include
INCDIR  += -I../../../libserial/src/include
INCDIR  += -I../../../libconio/src/include
LIBDIR 	+= -L../../../libnrf24l01/lib
LIBDIR  += -L../../../libserial/lib
LIBDIR  += -L../../../libconio/lib
LIBS	+= -lnrf24l01 -lserial -lconio
#LIBS	+= -lnrf24l01_meta -lserial -lconio

ifeq ($(TARCH),STM32_100)
LDSCRIPT = ../../f4discovery.ld
CFLAGS   = -mcpu=cortex-m4 -mthumb \
	-mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	-DSTM32F4 -I../libopencm3/include 
else
LDSCRIPT = ../../firmware_stm32_103.ld
CFLAGS   = -DSTM32_103
endif

#CFLAGS  += -DNRF_REG_DEF_META

include ../../../common.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC)
	$(DOXYGEN) $(DOXYGENFLAGS)
