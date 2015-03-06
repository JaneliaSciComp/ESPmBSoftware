################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
ESP3_0AppProj.elf.size \

ELFCHECK += \
ESP3_0AppProj.elf.elfcheck \


# All Target
all: ESP3_0AppProj.elf secondary-outputs

# Tool invocations
ESP3_0AppProj.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc -Wl,-T -Wl,../src/lscript.ld -L../../EngineTestBSP/microblaze_0/lib -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.50.c -mno-xl-soft-mul -Wl,--no-relax -Wl,--gc-sections -o "ESP3_0AppProj.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

ESP3_0AppProj.elf.size: ESP3_0AppProj.elf
	@echo 'Invoking: MicroBlaze Print Size'
	mb-size ESP3_0AppProj.elf  |tee "ESP3_0AppProj.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

ESP3_0AppProj.elf.elfcheck: ESP3_0AppProj.elf
	@echo 'Invoking: Xilinx ELF Check'
	elfcheck ESP3_0AppProj.elf -hw ../../EngineTest1/system.xml -pe microblaze_0  |tee "ESP3_0AppProj.elf.elfcheck"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES)$(ELFSIZE)$(ELFCHECK)$(S_UPPER_DEPS) ESP3_0AppProj.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE) $(ELFCHECK)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
