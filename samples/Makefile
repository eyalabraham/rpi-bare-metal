#####################################################################################
#
# Raspberry Pi bare-metal make
#
#####################################################################################

# Remove existing implicit rules (specifically '%.o: %c')
.SUFFIXES:

#------------------------------------------------------------------------------------
# Defines
#   TOOLDIR     - Tool chain directory
#   BINDIR      - Binary output directory
#   CCFLAGS     - C compiler flags
#   ASFLAGS     - Assembler flags
#   LDFLAGS     - Linker flags
#   CC          - Compiler
#   AS          - Assembler
#   LD          - Linker
#   OBJCOPY     - Object code conversion
#   ARMARCH     - Processor architecture
#------------------------------------------------------------------------------------
include ../environment.mk

PIMODEL ?= RPI1

#------------------------------------------------------------------------------------
# Define RPi model for study examples
#------------------------------------------------------------------------------------
CCFLAGS += -D$(PIMODEL)

#------------------------------------------------------------------------------------
# New make patterns
#------------------------------------------------------------------------------------
%.o: %.c
	$(CC) $(CCFLAGS) -g -I $(INCDIR) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

#------------------------------------------------------------------------------------
# Build all targets
#------------------------------------------------------------------------------------
all: uart1

#------------------------------------------------------------------------------------
# Build samples
#------------------------------------------------------------------------------------
led0: start.o led0.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(MYLIBDIR) -o $@.elf $? -lg_nano
	$(OBJCOPY) $@.elf -O binary $@.img
	cp $@.img $(BOOTDIR)/kernel.img

led1: start.o led1.o
	$(LD) $(LDFLAGS) -L $(MYLIBDIR) -o $@.elf $? -lgpio
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

uart1: start.o uart1.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc -lg_nano
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

stopwatch1: start.o stopwatch1.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

uart2: start.o uart2.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

stopwatch2: start.o stopwatch2.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

a2d: start.o a2d.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc -lg_nano
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

fb: start.o fb.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc -lg_nano -lm
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

#------------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------------

.PHONY: clean

clean:
	rm -f *.elf
	rm -f *.o
	rm -f *.map
	rm -f *.lib
	rm -f *.bak
	rm -f *.hex
	rm -f *.out
	rm -f *.img

