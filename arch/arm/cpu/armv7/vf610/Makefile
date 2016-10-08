#
# Copyright 2013 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= generic.o
obj-y	+= timer.o

MKIMAGEFLAGS_u-boot.vyb = -T vybridimage

u-boot.vyb: u-boot.imx
	$(call if_changed,mkimage)
