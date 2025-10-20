# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2025 Arda Yetistiren

KERNEL_NAME	:= seren

SUBARCH		?= $(shell uname -m)
ARCH		:= $(SUBARCH)
SRCARCH		:= $(ARCH)

ifeq ($(ARCH),i386)
	SRCARCH := x86
endif
ifeq ($(ARCH),x86_64)
	SRCARCH := x86
endif

export cross_compiling :=
ifneq ($(SRCARCH),$(SUBARCH))
	cross_compiling = 1
endif

HOSTCC		= gcc
CPP		= $(CC) -E
CC		= $(CROSS_COMPILE)gcc
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
AR		= $(CROSS_COMPILE)ar
OBJCOPY		= $(CROSS_COMPILE)objcopy

export SUBARCH ARCH SRCARCH HOSTCC CPP CC LD AR OBJCOPY

CFLAGS := -std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -O2
CFLAGS += -g
CFLAGS += -ffreestanding
CFLAGS += -fno-stack-protector
CFLAGS += -fno-pie
CFLAGS += -mno-red-zone
CFLAGS += -mcmodel=kernel
CFLAGS += -mgeneral-regs-only
CFLAGS += -MMD -MP -nostdinc
CFLAGS += -D__KERNEL__
CFLAGS += -I$(TOPDIR)/include
CFLAGS += -I$(TOPDIR)/arch/$(SRCARCH)/include

ASFLAGS := -g
ASFLAGS += -I$(TOPDIR)/include
ASFLAGS += -I$(TOPDIR)/arch/$(SRCARCH)/include

LDFLAGS := -nostdlib
LDFLAGS += -static
LDFLAGS += -no-pie
LDFLAGS += --no-dynamic-linker
LDFLAGS += -z max-page-size=0x1000

export CFLAGS ASFLAGS LDFLAGS

LIMINE_DIR	:= $(TOPDIR)/limine
LIMINE_CONF	:= $(TOPDIR)/limine.conf
LIMINE_ISO_BIOS	:= $(LIMINE_DIR)/limine-bios-cd.bin
LIMINE_SYS	:= $(LIMINE_DIR)/limine-bios.sys

ISO_FILES := $(LIMINE_CONF) $(LIMINE_SYS) $(LIMINE_ISO_BIOS) $(LIMINE_SYS)

export LIMINE_DIR LIMINE_CONF LIMINE_ISO_BIOS LIMINE_SYS ISO_FILES

QEMU	:= qemu-system-$(ARCH)

