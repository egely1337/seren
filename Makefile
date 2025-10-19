# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2025 Arda Yetistiren
#
# Usage: make [target] [OPTION=value]
#
# Common Targets:
#   all		Build the final bootable OS image. (default)
#   clean	Remove all generated build artifacts and downloaded dependencies.
#   help	Display this help message.
#   iso		Force a rebuild of the ISO image.
#
# Common Options:
#   V=1		Enable verbose output to see full compiler commands.
#   ARCH=...	Specify the target architecture (e.g., x86_64).
#   CROSS_COMPILE=... Specify the toolchain prefix (e.g., x86_64-elf-).

ifeq ($(V),1)
	Q =
else
	Q = @
endif

export Q

TOPDIR := $(shell pwd)

export TOPDIR

ifeq ("$(origin O)", "command line")
	SBUILD_OUTPUT := $(O)
endif

SBUILD_OUTPUT ?= $(TOPDIR)/build
export SBUILD_OUTPUT

include config.mk

TEST ?= 0

ifeq ($(TEST), 1)
	CONFIG_TEST	:= y
	BUILD_SUFFIX	:= -test
	BUILD_DESC	:= "[TEST]"
	SBUILD_OUTPUT	:= $(SBUILD_OUTPUT)-test
else
	CONFIG_TEST	:= n
	BUILD_SUFFIX	:=
	BUILD_DESC	:= 
endif

MODULE_O	:= $(SBUILD_OUTPUT)/module.o
OS_ISO		:= $(SBUILD_OUTPUT)/seren-$(ARCH).iso
LIMINE_DIR	:= limine
KERNEL_ELF	:= $(SBUILD_OUTPUT)/$(KERNEL_NAME).elf

export CONFIG_TEST

ISO_ROOT	:= $(SBUILD_OUTPUT)/iso_root
LIMINE_STAMP := $(LIMINE_DIR)/.fetched

ARCH_MAKEFILE	:= $(TOPDIR)/arch/$(SRCARCH)/Makefile

-include $(ARCH_MAKEFILE)

export ELF_TARGET_FORMAT

# ---[ Core Build Targets ]--- 

.PHONY: all clean

all: $(OS_ISO) ## Build the final bootable OS image. (default)

$(MODULE_O):
	@echo "DESCEND into build..."
	$(Q)$(MAKE) -f $(TOPDIR)/scripts/build.mk -C .

$(KERNEL_ELF): $(MODULE_O) $(TOPDIR)/arch/$(SRCARCH)/linker.ld
	@echo "  LD		$@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $< 

clean: ## Remove all generated build artifacts and downloaded dependencies.
	@echo "  CLEAN		$(SBUILD_OUTPUT) and $(LIMINE_DIR)"
	$(Q)rm -rf $(SBUILD_OUTPUT) $(LIMINE_DIR)

.PHONY: run iso fetch-limine help

iso: $(OS_ISO) ## Force a rebuild of the ISO image.

$(OS_ISO): $(KERNEL_ELF) $(LIMINE_STAMP)
	@echo "  ISO $(BUILD_DESC)	$@"
	$(Q)rm -rf $(ISO_ROOT)
	$(Q)mkdir -p $(ISO_ROOT)
	$(Q)cp $(KERNEL_ELF) $(ISO_ROOT)/$(KERNEL_NAME).elf
	$(Q)cp $(ISO_FILES) $(ISO_ROOT)/
	$(Q)xorriso -as mkisofs \
		-b limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		-quiet \
		$(ISO_ROOT) -o $@

run: $(OS_ISO) ## Build and run the OS in QEMU.
	@echo "  RUN $(BUILD_DESC)	Booting with QEMU..."
	$(Q)$(QEMU) $(QEMU_FLAGS) -cdrom $<

$(LIMINE_STAMP):
	$(Q)$(MAKE) fetch-limine

fetch-limine:
	@echo "  GIT $(BUILD_DESC)	Fetching Limine bootloader"
	$(Q)git clone https://github.com/limine-bootloader/limine.git --branch=v10.x-binary --depth=1 $(LIMINE_DIR)
	$(Q)touch $(LIMINE_STAMP)

# ---[ Test Targets ]---
.PHONY: test

test: ## Build and run the OS with tests enabled.
	@$(MAKE) run TEST=1

test-build: ## Build the test-enabled OS image without running it.
	@$(MAKE) iso TEST=1

help: ## Display this help message.
	@echo "SerenOS Kernel Build System"
	@echo ""
	@echo "Usage: make [target] [OPTION=value]"
	@echo ""
	@echo "Common Targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(firstword $(MAKEFILE_LIST)) | \
		sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-12s %s\n", $$1, $$2}'
	@echo ""
	@echo "Common Options:"
	@echo "  V=1           Enable verbose output to see full compiler commands."
	@echo "  ARCH=...      Specify the target architecture (e.g., x86_64)."
	@echo "  CROSS_COMPILE=... Specify the toolchain prefix (e.g., x86_64-elf-)."
	@echo ""