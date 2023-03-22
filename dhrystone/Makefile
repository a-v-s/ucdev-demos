################################################################################
# Set the MCU
################################################################################
MCU ?= STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = dhrystone

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev

C_SOURCES += main.c
#C_SOURCES += dhrystone.c



C_SOURCES += $(UCDEV_ROOT)/lib/libhalglue/common/bshal_delay.c

OPT= -O2 -g3


LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/riscv_rv32imac  
LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m3  -lrtt


# Putting the regarget code in a static library does not work, 
# But adding the source here does work. 
#LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/retarget_rtt/debug/arm_m3  -lretarget_rtt
C_SOURCES +=  $(UCDEV_ROOT)/ext/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Syscalls/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//RTT/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Config/



ifneq (,$(findstring RV32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
endif

ifneq (,$(findstring STM32F0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f0/clocksetup.c
endif

ifneq (,$(findstring STM32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
endif

ifneq (,$(findstring STM32F3,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f3/clocksetup.c
endif

ifneq (,$(findstring STM32F4,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f4/clocksetup.c
endif

ifneq (,$(findstring STM32L0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l0/clocksetup.c
endif

ifneq (,$(findstring STM32L1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l1/clocksetup.c
endif



################################################################################
# Includes
################################################################################

C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/


# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld



MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk



dhrystone: $(BUILD_DIR)/dhrystone_0.o 
dhrystone: $(BUILD_DIR)/dhrystone_1.o 
dhrystone: $(BUILD_DIR)/dhrystone_2.o 
dhrystone: $(BUILD_DIR)/dhrystone_3.o 
dhrystone: $(BUILD_DIR)/dhrystone_s.o


OBJECTS += $(BUILD_DIR)/dhrystone_0.o
OBJECTS += $(BUILD_DIR)/dhrystone_1.o
OBJECTS += $(BUILD_DIR)/dhrystone_2.o
OBJECTS += $(BUILD_DIR)/dhrystone_3.o
OBJECTS += $(BUILD_DIR)/dhrystone_s.o


$(BUILD_DIR)/dhrystone_0.o: dhrystone.c Makefile | $(BUILD_DIR) 
	$(CC) -c -DBENCHMARK=d0 -O0 $(CPU) $(FPU) $(ABI) $(C_DEFS) -Wall -fdata-sections -ffunction-sections  dhrystone.c -o $@  

$(BUILD_DIR)/dhrystone_1.o: dhrystone.c Makefile | $(BUILD_DIR) 
	$(CC) -c -DBENCHMARK=d1 -O1 $(CPU) $(FPU) $(ABI) $(C_DEFS) -Wall -fdata-sections -ffunction-sections  dhrystone.c -o $@

$(BUILD_DIR)/dhrystone_2.o: dhrystone.c Makefile | $(BUILD_DIR) 
	$(CC) -c -DBENCHMARK=d2 -O2 $(CPU) $(FPU) $(ABI) $(C_DEFS) -Wall -fdata-sections -ffunction-sections  dhrystone.c -o $@	
	
$(BUILD_DIR)/dhrystone_3.o: dhrystone.c Makefile | $(BUILD_DIR) 
	$(CC) -c -DBENCHMARK=d3 -O3 $(CPU) $(FPU) $(ABI) $(C_DEFS) -Wall -fdata-sections -ffunction-sections  dhrystone.c -o $@
	
$(BUILD_DIR)/dhrystone_s.o: dhrystone.c Makefile | $(BUILD_DIR) 
	$(CC) -c -DBENCHMARK=ds -Os $(CPU) $(FPU) $(ABI) $(C_DEFS) -Wall -fdata-sections -ffunction-sections  dhrystone.c -o $@	