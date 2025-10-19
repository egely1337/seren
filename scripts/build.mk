# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2025 Arda Yetistiren

ifneq ($(CURDIR),$(TOPDIR))
	CURDIR_REL := $(patsubst $(TOPDIR)/%,%,$(CURDIR))
else
	CURDIR_REL :=
endif

$(shell mkdir -p $(SBUILD_OUTPUT)/$(CURDIR_REL))

-include Sbuild

obj-list := $(filter %.o, $(obj-y))
bin-list := $(filter %.o, $(obj-bin-y))

local-c-s-objs := $(patsubst %,$(SBUILD_OUTPUT)/$(CURDIR_REL)/%,$(obj-list))
local-bin-objs := $(patsubst %,$(SBUILD_OUTPUT)/$(CURDIR_REL)/%,$(bin-list))
local-objs := $(local-c-s-objs) $(local-bin-objs)

CFLAGS-$(CONFIG_TEST) += -DSERENOS_TEST_BUILD
CFLAGS += $(CFLAGS-y)

subdirs		:= $(filter %/, $(obj-y))
subdir-objs	:= $(patsubst %/,$(SBUILD_OUTPUT)/$(CURDIR_REL)/%/module.o,$(subdirs))

.PHONY: all

all: $(SBUILD_OUTPUT)/$(CURDIR_REL)/module.o

$(SBUILD_OUTPUT)/$(CURDIR_REL)/module.o: $(local-objs) $(subdir-objs)
	@echo "  LD		[module] $@"
	@$(LD) -r -o $@ $^

$(subdir-objs):
	@$(MAKE) -f $(TOPDIR)/scripts/build.mk -C $(patsubst $(SBUILD_OUTPUT)/$(CURDIR_REL)/%/module.o,%,$@)

vpath %.c $(CURDIR)
vpath %.S $(CURDIR)
vpath %.psf $(CURDIR)

$(SBUILD_OUTPUT)/$(CURDIR_REL)/%.o: %.c
	@echo "  CC		$(CURDIR_REL)/$<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(SBUILD_OUTPUT)/$(CURDIR_REL)/%.o: %.S
	@echo "  AS		$(CURDIR_REL)/$<"
	$(Q)$(CC) $(ASFLAGS) -c $< -o $@

$(SBUILD_OUTPUT)/$(CURDIR_REL)/%.o: %.psf
	@echo "  OBJCOPY	$(CURDIR_REL)/$<"
	$(Q)$(OBJCOPY) -I binary -O $(ELF_TARGET_FORMAT) $< $@

-include $(local-objs:.o=.d)
