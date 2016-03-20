# Title:		Serial Controlled Switch
# Author:		Olexandr Davydenko
# Description:	32 output channels, controlled by commands via serial port.

SHELL = sh

# project name (filename WITHOUT extention)
PROJ = sconswitch

# dir for obj files
OBJDIR = obj

# CHIP type
MCU = atmega8515

# clock CPU for use this symbol in sources
F_CPU = 11059200UL

# date binary
DATE = date

#---------------- Compiler Options C ----------------
# compiler
CC = avr-gcc
CPP = avr-g++
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

# Place -D or -U options here for C sources
CDEFS = -DF_CPU=$(F_CPU) -mmcu=$(MCU)
CFLAGS = $(CDEFS) -gstabs -Wall -Wstrict-prototypes -Os
CPPFLAGS = $(CDEFS) -Wall -gdwarf-2 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fpermissive

LDFLAGS = -mmcu=$(MCU) -Wl,-Map,$(OBJDIR)/$(PROJ).map,--cref

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

#---------------- Fuses ATMega8515 ----------------
# high byte HFUSE:
# 0xc1 = 1 1 0 0   0 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- S8515C (AT90S4414/8515 compatibility mode)
# low byte LFUSE:
# 0x8f = 1 0 0 0   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)


#---------------- Burner Options ----------------
BURNER = avrdude
BOPTS_COMMON = -p $(MCU) -c usbasp -y -B 5
BOPTS_FLASH_WR = -U flash:w:
BFUSES_WR = -U hfuse:w:0xc1:m -U lfuse:w:0x8f:m

program: all
	$(BURNER) $(BOPTS_COMMON) $(BOPTS_FLASH_WR)$(OBJDIR)/$(PROJ).hex $(BFUSES_WR)

all:  objdir $(OBJDIR)/$(PROJ).elf

#------------------- Compile here -------------------
source_dirs := src src/uart src/mcucpp src/mcucpp/AVR src/mcucpp/impl src/mcucpp/loki
search_wildcards := $(addsuffix /*.c,$(source_dirs))
VPATH := $(source_dirs)

$(OBJDIR)/$(PROJ).elf: $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(wildcard $(search_wildcards))))
	$(CPP) $(LDFLAGS) $(CDEFS) $^ -o $@
	$(SIZE) $(OBJDIR)/$(PROJ).elf
	$(OBJDUMP) -h -S $(OBJDIR)/$(PROJ).elf > $(OBJDIR)/$(PROJ).lst
	$(OBJCOPY) -R .eeprom -O $(FORMAT) $(OBJDIR)/$(PROJ).elf $(OBJDIR)/$(PROJ).hex
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $(OBJDIR)/$(PROJ).elf $(OBJDIR)/$(PROJ).eep

$(OBJDIR)/%.o: %.c
	$(CPP) $(CPPFLAGS) $(addprefix -I,$(source_dirs)) -c $< -o $(OBJDIR)/$*.o

#-------------------
objdir:
	mkdir -p $(addprefix $(OBJDIR)/,$(source_dirs))
#-------------------
clean:
	rm -rf $(addprefix $(OBJDIR)/,$(source_dirs)) *.o *.map *.elf *~ *.bak *.lst
#-------------------
distclean:
	rm -rf $(OBJDIR) *.o *.map *.elf *~ *.hex *.eep *.cache *.d
#-------------------
backup: clean
	tar -czf ../$(PROJ)_`$(DATE) "+%Y%m%d_%H%M%S"`.tar.gz ./
#-------------------
eeread:
	$(BURNER) $(BOPTS_COMMON) -U eeprom:r:eeprom.bin:r
	$(BURNER) $(BOPTS_COMMON) -U eeprom:r:eeprom.hex:i
#-------------------

.PHONY : all clean distclean eeread backup objdir
