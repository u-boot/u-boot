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
MKIMAGE_TARGET-$(CONFIG_MX23) = mx23
MKIMAGE_TARGET-$(CONFIG_MX28) = mx28

$(OBJTREE)/mxsimage.cfg: $(SRCTREE)/$(CPUDIR)/$(SOC)/mxsimage.$(MKIMAGE_TARGET-y).cfg
	sed "s@OBJTREE@$(OBJTREE)@g" $^ > $@

$(OBJTREE)/u-boot.sb: $(OBJTREE)/u-boot.bin $(OBJTREE)/spl/u-boot-spl.bin $(OBJTREE)/mxsimage.cfg
	$(OBJTREE)/tools/mkimage -n $(OBJTREE)/mxsimage.cfg -T mxsimage $@
