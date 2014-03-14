#
# Copyright (C) Freescale Semiconductor, Inc. 2006.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= mvblm7.o pci.o fpga.o

extra-y := bootscript.img

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
        $(if $(KBUILD_VERBOSE:1=), >/dev/null)

MKIMAGEFLAGS_bootscript.image := -T script -C none -n M7_script

$(obj)/bootscript.img: $(src)/bootscript
	$(call cmd,mkimage)
