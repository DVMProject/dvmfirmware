ADIR=$(HOME)/.arduino15

CXX=$(ADIR)/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/arm-none-eabi-g++
CC=$(ADIR)/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/arm-none-eabi-gcc
OBJCOPY=$(ADIR)/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/arm-none-eabi-objcopy
C=$(CC)
SAM=$(ADIR)/packages/arduino/hardware/sam/1.6.11
OBJDIR=obj_due
AR=$(ADIR)/tools/g++_arm_none_eabi/bin/arm-none-eabi-ar 
AR=$(ADIR)/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/arm-none-eabi-ar

# all these values are hard coded and should maybe be configured somehow else,
# like olikraus does in his makefile.
DEFINES=-Dprintf=iprintf -DF_CPU=84000000  -DARDUINO=10611 -D__SAM3X8E__ -DUSB_PID=0x003e -DUSB_VID=0x2341 -DUSBCON \
		-DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM '-DUSB_MANUFACTURER="Arduino LLC"' '-DUSB_PRODUCT="Arduino Due"' -DMADEBYMAKEFILE

INCLUDES=-I$(SAM)/system/libsam -I$(SAM)/system/CMSIS/CMSIS/Include/ -I$(SAM)/system/CMSIS/Device/ATMEL/ -I$(SAM)/cores/arduino -I$(SAM)/variants/arduino_due_x -I.

# compilation flags common to both c and c++ 
COMMON_FLAGS=-g -Os -w -ffunction-sections -fdata-sections -nostdlib --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -fno-threadsafe-statics 
CFLAGS=$(COMMON_FLAGS) -std=gnu11
CXXFLAGS=$(COMMON_FLAGS) -fno-rtti -fno-exceptions -std=gnu++11 -Wall -Wextra

BINNAME=dvm-firmware_due

CXXSRC=$(wildcard ./*.cpp) $(wildcard ./dmr/*.cpp) $(wildcard ./p25/*.cpp)
CXXOBJ=$(addsuffix .o,$(addprefix $(OBJDIR)/,$(notdir $(CXXSRC))))

CORESRCXX=$(shell ls ${SAM}/cores/arduino/*.cpp ${SAM}/cores/arduino/USB/*.cpp  ${SAM}/variants/arduino_due_x/variant.cpp)
CORESRC=$(shell ls ${SAM}/cores/arduino/*.c)

# hey this one is needed too: $(SAM)/cores/arduino/wiring_pulse_asm.S" add -x assembler-with-cpp
# and this one: /1.6.11/cores/arduino/avr/dtostrf.c but it seems to work
# anyway, probably because I do not use that functionality.
# convert the core source files to object files. assume no clashes.
COREOBJSXX:=$(addprefix $(OBJDIR)/core/,$(notdir $(CORESRCXX)) )
COREOBJSXX:=$(addsuffix .o,$(COREOBJSXX))
COREOBJS:=$(addprefix $(OBJDIR)/core/,$(notdir $(CORESRC)) )
COREOBJS:=$(addsuffix .o,$(COREOBJS))

all: compile

# This rule is good to just make sure stuff compiles, without having to wait
# for bossac.
compile: $(BINNAME).elf $(BINNAME).bin

# This is a make rule template to create object files from the source files.
#  arg 1=src file
#  arg 2=object file
#  arg 3= XX if c++, empty if c
define OBJ_template
$(2): $(1)
	$(C$(3)) -MD -c $(C$(3)FLAGS) $(DEFINES) $(INCLUDES) $(1) -o $(2)
endef
# now invoke the template both for c++ sources
$(foreach src,$(CORESRCXX), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(OBJDIR)/core/,$(notdir $(src)))),XX) ) )
# ...and for c sources:
$(foreach src,$(CORESRC), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(OBJDIR)/core/,$(notdir $(src)))),) ) )

# and our own c++ sources
$(foreach src,$(CXXSRC), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(OBJDIR)/,$(notdir $(src)))),XX) ) )

clean:
	test ! -d $(OBJDIR) || rm -rf $(OBJDIR)
	-rm -f $(BINNAME).map
	-rm -f $(BINNAME).elf
	-rm -f $(BINNAME).bin

.PHONY: all

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/core:
	mkdir -p $(OBJDIR)/core

# include the dependencies for our own files
-include $(CXXOBJ:.o=.d)

# create the core library from the core objects. Do this EXACTLY as the
# arduino IDE does it, seems *really* picky about this.
# Sorry for the hard coding.
$(OBJDIR)/core.a: $(OBJDIR)/core $(COREOBJS) $(COREOBJSXX)
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/wiring_shift.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/wiring_analog.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/itoa.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/cortex_handlers.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/hooks.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/wiring.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/WInterrupts.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/syscalls_sam3.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/iar_calls_sam3.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/wiring_digital.c.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/Print.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/USARTClass.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/WString.cpp.o
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/PluggableUSB.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/USBCore.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/CDC.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/wiring_pulse.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/UARTClass.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/main.cpp.o
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/new.cpp.o
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/watchdog.cpp.o
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/Stream.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/RingBuffer.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/IPAddress.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/Reset.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/WMath.cpp.o 
	$(AR) rcs $(OBJDIR)/core.a $(OBJDIR)/core/variant.cpp.o

$(BINNAME).elf: $(OBJDIR)/core.a $(OBJDIR)/core/syscalls_sam3.c.o $(CXXOBJ) 
	$(CC) -mcpu=cortex-m3 -mthumb -Os -Wl,--gc-sections -T$(SAM)/variants/arduino_due_x/linker_scripts/gcc/flash.ld -Wl,-Map,$(BINNAME).map -o $@ -L$(OBJDIR) -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--start-group -u _sbrk -u link -u _close -u _fstat -u _isatty -u _lseek -u _read -u _write -u _exit -u kill -u _getpid $(CXXOBJ) $(OBJDIR)/core/variant.cpp.o $(SAM)/variants/arduino_due_x/libsam_sam3x8e_gcc_rel.a $(SAM)/system/CMSIS/CMSIS/Lib/GCC/libarm_cortexM3l_math.a $(OBJDIR)/core.a -Wl,--end-group -lm -gcc

$(BINNAME).bin: $(BINNAME).elf 
	$(OBJCOPY) -O binary $< $@
