# Optimization options
# -flto  use link-time optimization
#
OPT = -Os -flto
# OPT = -O1    # for debugging

# Object files directory
# Warning: this will be removed by make clean!
#
OBJDIR = obj_boot

# Target file name (without extension)
#
TARGET = $(OBJDIR)/bootloader

# Define all C source files (dependencies are generated automatically)
#
SOURCE_DIR = Bootloader/Src
INCDIRS   += Bootloader/Inc

SOURCES += $(SOURCE_DIR)/main.c
SOURCES += $(SOURCE_DIR)/stm32l0xx_hal_msp.c
SOURCES += $(SOURCE_DIR)/stm32l0xx_it.c
SOURCES += $(SOURCE_DIR)/usb_device.c
SOURCES += $(SOURCE_DIR)/usbd_conf.c
SOURCES += $(SOURCE_DIR)/usbd_desc.c
SOURCES += $(SOURCE_DIR)/usbd_dfu_if.c
SOURCES += $(SOURCE_DIR)/usbd_dfu.c

# USB middleware library
#
USB_DIR = Libraries/STM32_USB_Device_Library-2.2.0
INCDIRS += $(USB_DIR)/Core/Inc

SOURCES += $(USB_DIR)/Core/Src/usbd_core.c
SOURCES += $(USB_DIR)/Core/Src/usbd_ctlreq.c
SOURCES += $(USB_DIR)/Core/Src/usbd_ioreq.c

# HAL driver library
#
CPPFLAGS += -DSTM32L052xx
CPPFLAGS += -DUSE_HAL_DRIVER
CPPFLAGS += -DUSE_FULL_ASSERT

HAL_DIR = Libraries/STM32L0xx_HAL_Driver-1.1.0

INCDIRS += $(HAL_DIR)/Inc
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_iwdg.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_tim.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_tim_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_uart.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_uart_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_usart.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_pcd.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_pcd_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_gpio.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_dma.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_rcc.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_rcc_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_flash.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_flash_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_flash_ramfunc.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_pwr.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_pwr_ex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_cortex.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_i2c.c
SOURCES += $(HAL_DIR)/Src/stm32l0xx_hal_i2c_ex.c

# CMSIS-Library
#
CMSIS_DIR = Libraries/CMSIS-3.2.0

INCDIRS += $(CMSIS_DIR)/Include
INCDIRS += $(CMSIS_DIR)/Device/ST/STM32L0xx/Include

SOURCES += $(CMSIS_DIR)/Device/ST/STM32L0xx/Source/Templates/gcc/startup_stm32l052xx.s
SOURCES += $(CMSIS_DIR)/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c

LDSCRIPT = $(SOURCE_DIR)/STM32L052R8_FLASH.ld


#============================================================================
#
OBJECTS  += $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(SOURCES))))
CPPFLAGS += $(addprefix -I,$(INCDIRS))

#---------------- Preprocessor and common options ----------------

# Optimization options
CPPFLAGS += $(OPT)
CPPFLAGS += -fsingle-precision-constant

# Use a friendly C dialect
CPPFLAGS += -fno-strict-aliasing
CPPFLAGS += -fwrapv

# Generate debugging information
CPPFLAGS += -g

# preserve .s and .i-files
CPPFLAGS += -save-temps=obj

# Use individual sections to enable linker garbage collection
CPPFLAGS += -ffunction-sections
CPPFLAGS += -fdata-sections

# Warning options
CPPFLAGS += -Wall
CPPFLAGS += -Wundef
CPPFLAGS += -Wstrict-prototypes
#CPPFLAGS += -Wextra
#CPPFLAGS += -Wpointer-arith
#CPPFLAGS += -Winline
#CPPFLAGS += -Wunreachable-code

#---------------- C Compiler Options ----------------
#
CFLAGS += -std=gnu11

#---------------- C++ Compiler Options ----------------
#

#---------------- Assembler Options ----------------
#

#---------------- Linker Options ----------------
#
LDFLAGS += $(OPT) -g

LDFLAGS += -specs=nano.specs
# LDFLAGS += -u _printf_float -u _scanf_float

# Link math library
# LDFLAGS += -lm

# Create a map file with cross references
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref

# Garbage collect unused sections (see above)
LDFLAGS += -Wl,--gc-sections

LDFLAGS += -T$(LDSCRIPT)

#============================================================================

# Define programs and commands
#
TOOLCHAIN = arm-none-eabi-
CC       = $(TOOLCHAIN)gcc
OBJCOPY  = $(TOOLCHAIN)objcopy
OBJDUMP  = $(TOOLCHAIN)objdump
SIZE     = $(TOOLCHAIN)size
NM       = $(TOOLCHAIN)nm
MKDIR    = mkdir
DOXYGEN  = doxygen
STLINK   = Tools/st-link/ST-LINK_CLI.exe

# Compiler flags to generate dependency files
#
GENDEPFLAGS = -MMD -MP

# Combine all necessary flags and optional flags
# Add target processor to flags.
#
# CPU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
# CPU = -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16
CPU = -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft

CFLAGS   += $(CPU)
CXXFLAGS += $(CPU)
ASFLAGS  += $(CPU)
LDFLAGS  += $(CPU)

# Default target
#
all:  gccversion build showsize

build: elf hex bin lss sym

elf: $(TARGET).elf
hex: $(TARGET).hex
bin: $(TARGET).bin
lss: $(TARGET).lss
sym: $(TARGET).sym


doxygen:
	@echo
	@echo Creating Doxygen documentation
	@$(DOXYGEN)


#boot:
#	$(MAKE) -f Bootloader/Makefile

#boot_clean:
#	$(MAKE) -f Bootloader/Makefile clean

#boot_flash:
#	$(MAKE) -f Bootloader/Makefile flash


# Display compiler version information
#
gccversion: 
	@$(CC) --version

# Show the final program size
#
showsize: build
	@echo
	@$(SIZE) $(TARGET).elf
#	@echo
#	@$(NM) $(TARGET).elf --size-sort -r | grep " [DdBb] "

# Flash the device  
#
flash: build showsize
	$(STLINK) -c SWD -P $(TARGET).hex -Run

# Target: clean project
#
clean:
	@echo Cleaning project:
	rm -rf $(OBJDIR)
	rm -f  Source/version.c

# Link: create ELF output file from object files
#
$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	@echo
	@echo Linking: $@
	@$(MKDIR) -p $(dir $@)
	$(CC) $(OBJECTS) $(LDFLAGS) --output $@ 

# Create extended listing file from ELF output file
#
%.lss: %.elf
	@echo
	@echo Creating Extended Listing: $@
	@$(MKDIR) -p $(dir $@)
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file
#
%.sym: %.elf
	@echo
	@echo Creating Symbol Table: $@
	@$(MKDIR) -p $(dir $@)
	$(NM) -n $< > $@

# Create final output files from ELF output file.
#
%.hex: %.elf
	@echo
	@echo Creating hex file: $@
	@$(MKDIR) -p $(dir $@)
	$(OBJCOPY) -O ihex $< $@

# Create binary output file from ELF output file.
#
%.bin: %.elf
	@echo
	@echo Creating bin file: $@
	@$(MKDIR) -p $(dir $@)
	$(OBJCOPY) -O binary $< $@

# Compile: create object files from C source files
#
$(OBJDIR)/%.o : %.c
	@echo
	@echo Compiling C: $<
	@$(MKDIR) -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(GENDEPFLAGS) $< -o $@ 

$(OBJDIR)/%.o : %.cpp
	@echo
	@echo Compiling C++: $<
	@$(MKDIR) -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $(CXXFLAGS) $(GENDEPFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files
#
$(OBJDIR)/%.o : %.s
	@echo
	@echo Assembling: $<
	@$(MKDIR) -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $(ASFLAGS) $(GENDEPFLAGS) $< -o $@

# Make everything depend on the makefile
#
# $(OBJECTS): $(MAKEFILE_LIST)

# Include the dependency files
#
-include $(OBJECTS:.o=.d)

# Listing of phony targets
#
.PHONY: all build flash clean \
#        boot boot_clean boot_flash \
        doxygen elf lss sym \
        showsize gccversion
