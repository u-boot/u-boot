#
# Copyright (C) Freescale Semiconductor, Inc. 2006.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= mvblm7.o pci.o fpga.o

extra-y := bootscript.img

$(obj)bootscript.img:
	@mkimage -T script -C none -n M7_script -d bootscript $@
