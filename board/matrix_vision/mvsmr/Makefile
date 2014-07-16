#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2004-2008
# Matrix-Vision GmbH, info@matrix-vision.de
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= mvsmr.o fpga.o

extra-y := bootscript.img

MKIMAGEFLAGS_bootscript.image := -T script -C none -n mvSMR_Script

$(obj)/bootscript.img: $(src)/bootscript
	$(call cmd,mkimage)
