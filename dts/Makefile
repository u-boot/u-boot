#
# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := notfound
endif

DTS := $(srctree)/arch/$(ARCH)/dts/$(DEVICE_TREE).dts

DTC_FLAGS += -R 4 -p 0x1000

$(obj)/dt.dtb: $(DTS) FORCE
	$(call if_changed_dep,dtc)

targets += dt.dtb

$(DTS):
	@echo >&2
	@echo >&2 "Device Tree Source is not specified."
	@echo >&2 "Please define 'CONFIG_DEFAULT_DEVICE_TREE'"
	@echo >&2 "or build with 'DEVICE_TREE=<dts-file-name>' argument"
	@/bin/false

.SECONDARY: $(obj)/dt.dtb.S

obj-$(CONFIG_OF_EMBED) := dt.dtb.o

dtbs: $(obj)/dt.dtb
	@:

clean-files := dt.dtb.S
