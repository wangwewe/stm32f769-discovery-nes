# Build from the project root. No CubeMX regeneration required.
.DEFAULT_GOAL := all
TARGET := STM32F769_Disco_NES
BUILD := Debug
-include config.mk
# Set GCC_PATH to the compiler bin directory if it is not already on PATH.
ifneq ($(strip $(GCC_PATH)),)
PREFIX := $(GCC_PATH)/arm-none-eabi-
else
PREFIX := arm-none-eabi-
endif
CC := "$(PREFIX)gcc"
OBJCOPY := "$(PREFIX)objcopy"
SIZE := "$(PREFIX)size"
include sources.mk
CPU := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
DEFINES := -DUSE_HAL_DRIVER -DSTM32F769xx -DUSE_STM32F769I_DISCO -DHSE_VALUE=25000000U -DTS_MULTI_TOUCH_SUPPORTED=1
# ARMCC default char is unsigned; old code has tentative common definitions.
CFLAGS := $(CPU) $(DEFINES) $(INCLUDES) -std=gnu99 -O3 -g3 -funsigned-char -fcommon -fno-strict-aliasing -ffunction-sections -fdata-sections -Wall -MMD -MP
LDFLAGS := $(CPU) -TSTM32F769NIHX_FLASH.ld -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections,-Map=$(BUILD)/$(TARGET).map -Wl,--print-memory-usage
LDFLAGS += -Wl,-z,noexecstack
OBJECTS := $(addprefix $(BUILD)/,$(C_SOURCES:.c=.o)) $(addprefix $(BUILD)/,$(addsuffix .o,$(basename $(ASM_SOURCES))))
MKDIR = mkdir -p "$(@D)"
.PHONY: all clean
all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin
$(BUILD)/%.o: %.c Makefile sources.mk main/nes_config.h
	@$(MKDIR)
	$(CC) $(CFLAGS) -c "$<" -o "$@"
$(BUILD)/%.o: %.s Makefile
	@$(MKDIR)
	$(CC) $(CPU) -x assembler-with-cpp -c "$<" -o "$@"
$(BUILD)/%.o: %.S Makefile
	@$(MKDIR)
	$(CC) $(CPU) -x assembler-with-cpp -c "$<" -o "$@"
$(BUILD)/GCC/game_resources.o: nes/games/SuperMarioBros.nes nes/games/yingzichuanshuo.nes nes/games/SuperDodgeBall.nes
$(BUILD)/$(TARGET).elf: $(OBJECTS) STM32F769NIHX_FLASH.ld Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -Wl,--start-group -lc -lm -lnosys -Wl,--end-group -o "$@"
	$(SIZE) "$@"
$(BUILD)/%.hex: $(BUILD)/%.elf
	$(OBJCOPY) -O ihex "$<" "$@"
$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary "$<" "$@"
clean:
	rm -rf "$(BUILD)"
-include $(OBJECTS:.o=.d)
