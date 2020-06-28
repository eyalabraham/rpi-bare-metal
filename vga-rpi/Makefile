###############################################################################
#
# Raspberry Pi bare-metal Makefile for VGA emulator on RPi
#
###############################################################################

# Remove existing implicit rules (specifically '%.o: %c')
.SUFFIXES:

#------------------------------------------------------------------------------
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
#------------------------------------------------------------------------------
include ../environment.mk

PIMODEL ?= RPI1
APPINCDIR ?= ./include

#------------------------------------------------------------------------------
# Define RPi model for study examples
#------------------------------------------------------------------------------
CCFLAGS += -D$(PIMODEL)

#------------------------------------------------------------------------------
# New make patterns
#------------------------------------------------------------------------------
%.o: %.c
	$(CC) $(CCFLAGS) -g -I $(INCDIR) -I $(APPINCDIR) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

#------------------------------------------------------------------------------
# Build all targets
#------------------------------------------------------------------------------
all: vga

#------------------------------------------------------------------------------
# Build samples
#------------------------------------------------------------------------------

vga: start.o vga.o fb.o uart.o util.o
	$(LD) $(LDFLAGS) -L $(LIBDIR1) -L $(LIBDIR2) -L $(MYLIBDIR) -o $@.elf $? -lgpio -lprintf -lgcc -lg_nano
	$(OBJCOPY) $@.elf -O binary $@.img
#	$(OBJCOPY) $@.elf -O binary $@.$(PIMODEL).img
	cp $@.img $(BOOTDIR)/kernel.img

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------

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

