ifndef PROG
	PROG = src/main
endif

# avr mcu
MCU = atmega328p
# mcu clock frequency
CLK = 16000000
# avr programmer (and port if necessary)
# e.g. PRG = usbtiny -or- PRG = arduino -P /dev/tty.usbmodem411
PRG = arduino -P /dev/ttyACM0
# program source files
INC = ./include
EXT = ./lib/micr-ecc

INC_C = $(foreach dir, $(INC), $(wildcard $(dir)/*.c))
INC_H = $(foreach dir, $(INC), $(wildcard $(dir)/*.h))
INC_O = $(foreach file, $(INC_C), $(patsubst %.c,%.o,$(file)))
EXT_C = $(foreach dir, $(EXT), $(wildcard $(dir)/*.c))
EXT_H = $(foreach dir, $(EXT), $(wildcard $(dir)/*.h))
EXT_O = $(foreach file, $(EXT_C), $(patsubst %.c,%.o,$(file)))

INCLUDE = $(foreach dir, $(INC), -I $(dir)) $(foreach dir, $(EXT), -I $(dir))
CFLAGS   = -Wall -Os -DF_CPU=$(CLK) -mmcu=$(MCU) $(INCLUDE)

# executables
AVRDUDE = avrdude -c $(PRG) -p $(MCU)
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size --format=avr --mcu=$(MCU)
CC      = avr-gcc

OBJ = $(INC_O) $(EXT_O)

# user targets
# compile all files
all: $(PROG).hex

# test programmer connectivity
test:
	$(AVRDUDE) -v

# flash program to mcu
flash: all
	$(AVRDUDE) -U flash:w:$(PROG).hex:i

# generate disassembly files for debugging
disasm: $(PROG).elf
	$(OBJDUMP) -d $(PROG).elf

# remove compiled files
.PHONY: clean
clean:
	rm -f $(PROG).hex $(PROG).elf $(PROG).o
	rm -f $(OBJ)

# elf file
$(PROG).elf: $(PROG).o $(OBJ)
	$(CC) $(CFLAGS) -o $(PROG).elf $(PROG).o $(OBJ)

# hex file
$(PROG).hex: $(PROG).elf
	rm -f $(PROG).hex
	$(OBJCOPY) -j .text -j .data -O ihex $(PROG).elf $(PROG).hex
	$(SIZE) $(PROG).elf

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@