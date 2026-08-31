CROSS_COMPILE ?= aarch64-linux-gnu-
CC		:= $(CROSS_COMPILE)gcc
AS		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld

QEMU	:= qemu-system-aarch64
QFLAGS	:= -M virt -cpu cortex-a53 -nographic -serial mon:stdio

CFLAGS	:= -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles -mgeneral-regs-only -Iuart -Ikernel -Itest -Ipmm -Iheap
ASFLAGS	:= -Wall -ffreestanding -nostdlib
LDFLAGS	:= -T linker.ld -nostdlib --no-warn-rwx-segments

ifeq ($(TEST), 1)
	CFLAGS += -DENABLE_TESTS
endif


BUILD_DIR	:= build
TARGET_ELF	:= $(BUILD_DIR)/kernel.elf

SRCS_C	:= kernel/kernel.c kernel/exception.c uart/uart.c pmm/pmm.c heap/heap.c test/test_exception.c test/test_pmm.c test/test_heap.c
SRCS_S	:= boot/boot.s boot/vector.s

OBJS	:= 	$(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS_C)) \
			$(patsubst %.s, $(BUILD_DIR)/%.o, $(SRCS_S))

.PHONY: all run debug test clean

all: $(TARGET_ELF)

$(TARGET_ELF): $(OBJS) linker.ld
		@mkdir -p $(dir $@)
		$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
		@mkdir -p $(dir $@)
		$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
		@mkdir -p $(dir $@)
		$(AS) $(ASFLAGS) -c $< -o $@

run: $(TARGET_ELF)
		$(QEMU) $(QFLAGS) -kernel $(TARGET_ELF)

debug: $(TARGET_ELF)
		$(QEMU) $(QFLAGS) -kernel $(TARGET_ELF) -s -S

test: 
		@$(MAKE) clean
		@$(MAKE) run TEST=1

clean:
		rm -rf $(BUILD_DIR)
