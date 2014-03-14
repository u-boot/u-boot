#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-$(CONFIG_SPL_BUILD) := start.o

obj-y	= clock.o mxs.o iomux.o timer.o

ifdef	CONFIG_SPL_BUILD
obj-y	+= spl_boot.o spl_lradc_init.o spl_mem_init.o spl_power_init.o
endif

# Specify the target for use in elftosb call
MKIMAGE_TARGET-$(CONFIG_MX23) = mxsimage.mx23.cfg
MKIMAGE_TARGET-$(CONFIG_MX28) = mxsimage.mx28.cfg

quiet_cmd_mkimage_mxs = MKIMAGE $@
cmd_mkimage_mxs = $(objtree)/tools/mkimage -n $< -T mxsimage $@ \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)

u-boot.sb: $(src)/$(MKIMAGE_TARGET-y) u-boot.bin spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage_mxs)
