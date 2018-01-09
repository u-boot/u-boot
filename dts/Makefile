#
# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := unset
endif

ARCH_PATH := arch/$(ARCH)/dts
dtb_depends := arch-dtbs

ifneq ($(EXT_DTB),)
DTB := $(EXT_DTB)
else
DTB := $(ARCH_PATH)/$(DEVICE_TREE).dtb
dtb_depends += $(DTB:.dtb=.dts)
endif

# Pass the original device tree file through fdtgrep twice. The first pass
# removes any unwanted nodes (i.e. those which don't have the
# 'u-boot,dm-pre-reloc' property and thus are not needed by SPL. The second
# pass removes various unused properties from the remaining nodes.
# The output is typically a much smaller device tree file.
ifeq ($(CONFIG_TPL_BUILD),y)
fdtgrep_props := -b u-boot,dm-pre-reloc -b u-boot,dm-tpl
else
fdtgrep_props := -b u-boot,dm-pre-reloc -b u-boot,dm-spl
endif
quiet_cmd_fdtgrep = FDTGREP $@
      cmd_fdtgrep = $(objtree)/tools/fdtgrep $(fdtgrep_props) -RT $< \
		-n /chosen -n /config -O dtb | \
	$(objtree)/tools/fdtgrep -r -O dtb - -o $@ \
		$(addprefix -P ,$(subst $\",,$(CONFIG_OF_SPL_REMOVE_PROPS)))

$(obj)/dt-spl.dtb: $(DTB) $(objtree)/tools/fdtgrep FORCE
	$(call if_changed,fdtgrep)

$(obj)/dt.dtb: $(DTB) FORCE
	$(call if_changed,shipped)

targets += dt.dtb dt-spl.dtb

$(DTB): $(dtb_depends)
ifeq ($(EXT_DTB),)
	$(Q)$(MAKE) $(build)=$(ARCH_PATH) $@
endif
	$(Q)test -e $@ || (						\
	echo >&2;							\
	echo >&2 "Device Tree Source is not correctly specified.";	\
	echo >&2 "Please define 'CONFIG_DEFAULT_DEVICE_TREE'";		\
	echo >&2 "or build with 'DEVICE_TREE=<device_tree>' argument";	\
	echo >&2;							\
	/bin/false)

arch-dtbs:
	$(Q)$(MAKE) $(build)=$(ARCH_PATH) dtbs

.SECONDARY: $(obj)/dt.dtb.S $(obj)/dt-spl.dtb.S


ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_OF_EMBED) := dt-spl.dtb.o
# support "out-of-tree" build for dtb-spl
$(obj)/dt-spl.dtb.o: $(obj)/dt-spl.dtb.S FORCE
	$(call if_changed_dep,as_o_S)
else
obj-$(CONFIG_OF_EMBED) := dt.dtb.o
endif

dtbs: $(obj)/dt.dtb $(obj)/dt-spl.dtb
	@:

clean-files := dt.dtb.S dt-spl.dtb.S

# Let clean descend into dts directories
subdir- += ../arch/arm/dts ../arch/microblaze/dts ../arch/mips/dts ../arch/sandbox/dts ../arch/x86/dts
