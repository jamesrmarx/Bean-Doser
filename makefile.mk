# Since we are compiling in windows, select 'cmd' as the default shell.  This
# is important because make will search the path for a linux/unix like shell
# and if it finds it will use it instead.  This is the case when cygwin is
# installed.  That results in commands like 'del' and echo that don't work.
SHELL=cmd
# Specify the compiler to use
CC = xc32-gcc
# Specify the .elf to .hex converter
OBJCPY = xc32-bin2hex
# Specify the device we are using
ARCH = -mprocessor=32MX130F064B
# List the object files involved in this project
OBJ = hx711_driver.o

# The default 'target' (output) is Blinky.elf and 'depends' on
# the object files listed in the 'OBJS' assignment above.
# These object files are linked together to create Blinky.elf.
# The linked file is converted to hex using program $(OBJCPY).
hx711_driver.elf: $(OBJ)
	$(CC) $(ARCH) -o hx711_driver.elf hx711_driver.o -mips16 -DXPRJ_default=default \
		-legacy-libc -Wl,-Map=hx711_driver.map
	$(OBJCPY) hx711_driver.elf
	@echo Success!
	
# The object file hx711_driver.o depends on hx711_driver.c. hx711_driver.c is compiled
# to create hx711_driver.o. 
hx711_driver.o: hx711_driver.c
	$(CC) -g -x c -mips16 -Os -c $(ARCH) -MMD -o hx711_driver.o hx711_driver.c \
		-DXPRJ_default=default -legacy-libc

# Target 'clean' is used to remove all object files and executables
# associated wit this project
clean:
	@del *.o *.elf *.hex *.d *.map 2>NUL

# Target 'LoadFlash' is used to load the hex file to the microcontroller 
# using the flash loader.
LoadFlash:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	pro32 -p -v hx711_driver.hex

# Phony targets can be added to show useful files in the file list of
# CrossIDE or execute arbitrary programs:
dummy: hx711_driver.hex hx711_driver.map

explorer:
	explorer .
