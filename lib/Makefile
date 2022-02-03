#####################################################################################
#
# Raspberry Pi bare-metal GPIO static library make
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
PRINTF ?= -DPRINTF_INCLUDE_CONFIG_H

#------------------------------------------------------------------------------------
# Define RPi model for study examples
#------------------------------------------------------------------------------------
CCFLAGS += -D$(PIMODEL) $(PRINTF)

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
all: libgpio libprintf

#------------------------------------------------------------------------------------
# Build static libraries
#------------------------------------------------------------------------------------
libgpio: gpio.o auxuart.o spi0.o timer.o irq.o irq_util.o mailbox.o
	$(AR) rcsv $@.a $?

libprintf: printf.o
	$(AR) rcsv $@.a $?

#------------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------------

.PHONY: clean

clean:
	rm -f *.o
	rm -f *.bak
	rm -f *.a

