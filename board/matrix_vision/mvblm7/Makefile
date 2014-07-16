#
# Copyright (C) Freescale Semiconductor, Inc. 2006.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= mvblm7.o pci.o fpga.o

extra-y := bootscript.img

MKIMAGEFLAGS_bootscript.image := -T script -C none -n M7_script

$(obj)/bootscript.img: $(src)/bootscript
	$(call cmd,mkimage)
