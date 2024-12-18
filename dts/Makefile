# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := unset
endif

ifeq ($(CONFIG_OF_UPSTREAM),y)
ifeq ($(CONFIG_ARM64),y)
dt_dir := dts/upstream/src/arm64
else
dt_dir := dts/upstream/src/$(ARCH)
endif
else
dt_dir := arch/$(ARCH)/dts
endif

ifneq ($(CONFIG_OF_UPSTREAM_INCLUDE_LOCAL_FALLBACK_DTBOS),)
local_dtbos := local-dtbos
else
local_dtbos :=
endif

ifneq ($(EXT_DTB),)
DTB := $(EXT_DTB)
else
DTB := $(dt_dir)/$(DEVICE_TREE).dtb
endif

$(obj)/dt-$(SPL_NAME).dtb: dts/dt.dtb $(objtree)/tools/fdtgrep FORCE
	mkdir -p $(dir $@)
	$(call if_changed,fdtgrep)

ifeq ($(CONFIG_OF_DTB_PROPS_REMOVE),y)
$(obj)/dt.dtb: $(DTB) $(objtree)/tools/fdtgrep FORCE
	$(call if_changed,fdt_rm_props)
else
$(obj)/dt.dtb: $(DTB) FORCE
	$(call if_changed,shipped)
endif

targets += dt.dtb

$(DTB): arch-dtbs $(local_dtbos)
	$(Q)test -e $@ || (						\
	echo >&2;							\
	echo >&2 "Device Tree Source ($@) is not correctly specified.";	\
	echo >&2 "Please define 'CONFIG_DEFAULT_DEVICE_TREE'";		\
	echo >&2 "or build with 'DEVICE_TREE=<device_tree>' argument";	\
	echo >&2;							\
	/bin/false)

PHONY += arch-dtbs
arch-dtbs:
	$(Q)$(MAKE) $(build)=$(dt_dir) dtbs

ifneq ($(CONFIG_OF_UPSTREAM_INCLUDE_LOCAL_FALLBACK_DTBOS),)
PHONY += local-dtbos
local-dtbos:
	$(Q)$(MAKE) $(build)=arch/$(ARCH)/dts dtbos
endif

ifeq ($(CONFIG_XPL_BUILD),y)
obj-$(CONFIG_OF_EMBED) := dt-spl.dtb.o
# support "out-of-tree" build for dtb-spl
$(obj)/dt-spl.dtb.o: $(obj)/dt-spl.dtb.S FORCE
	$(call if_changed_dep,as_o_S)
else
obj-$(CONFIG_OF_EMBED) := dt.dtb.o
endif

# Target for U-Boot proper
dtbs: $(obj)/dt.dtb
	@:

# Target for SPL/TPL
spl_dtbs: $(obj)/dt-$(SPL_NAME).dtb
	@:

clean-files := dt.dtb.S

# Let clean descend into dts directories
subdir- += ../arch/arc/dts ../arch/arm/dts ../arch/m68k/dts ../arch/microblaze/dts	\
	   ../arch/mips/dts ../arch/nios2/dts ../arch/powerpc/dts ../arch/riscv/dts	\
	   ../arch/sandbox/dts ../arch/sh/dts ../arch/x86/dts ../arch/xtensa/dts	\
	   ./upstream/src/arm64 ./upstream/src/$(ARCH)
