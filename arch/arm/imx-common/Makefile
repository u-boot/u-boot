#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB     = $(obj)libimx-common.o

ifeq ($(SOC),$(filter $(SOC),mx25 mx35 mx5 mx6 vf610))
COBJS-y	= iomux-v3.o
endif
ifeq ($(SOC),$(filter $(SOC),mx5 mx6))
COBJS-y	+= timer.o cpu.o speed.o
COBJS-$(CONFIG_I2C_MXC) += i2c-mxv7.o
endif
ifeq ($(SOC),$(filter $(SOC),mx6 mxs))
COBJS-y	+= misc.o
endif
COBJS-$(CONFIG_CMD_BMODE) += cmd_bmode.o
COBJS-$(CONFIG_CMD_HDMIDETECT) += cmd_hdmidet.o
COBJS	:= $(sort $(COBJS-y))

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(OBJTREE)/$(patsubst "%",%,$(CONFIG_IMX_CONFIG)).cfgtmp: $(OBJTREE)/%.cfgtmp : $(SRCTREE)/%
	mkdir -p $(dir $@)
	$(CC) -E -x c $< $(CPPFLAGS) -o $@

$(OBJTREE)/u-boot.imx: $(OBJTREE)/u-boot.bin $(OBJTREE)/$(patsubst "%",%,$(CONFIG_IMX_CONFIG)).cfgtmp
	$(OBJTREE)/tools/mkimage -n $(filter-out %.bin,$^) -T imximage \
	-e $(CONFIG_SYS_TEXT_BASE) -d $< $@

$(OBJTREE)/SPL: $(OBJTREE)/spl/u-boot-spl.bin $(OBJTREE)/$(patsubst "%",%,$(CONFIG_IMX_CONFIG)).cfgtmp
	$(OBJTREE)/tools/mkimage -n $(filter-out %.bin,$^) -T imximage \
	-e $(CONFIG_SPL_TEXT_BASE) -d $< $@

$(OBJTREE)/u-boot-with-spl.imx: $(OBJTREE)/SPL $(OBJTREE)/u-boot.bin
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(CONFIG_SPL_PAD_TO) \
		-I binary -O binary $< $(OBJTREE)/spl/u-boot-spl-pad.imx
	$(OBJTREE)/tools/mkimage -A arm -O U-Boot -a $(CONFIG_SYS_TEXT_BASE) \
		-e $(CONFIG_SYS_TEXT_BASE) -C none -d $(OBJTREE)/u-boot.bin \
		$(OBJTREE)/u-boot.uim
	cat $(OBJTREE)/spl/u-boot-spl-pad.imx $(OBJTREE)/u-boot.uim > $@
	rm $(OBJTREE)/spl/u-boot-spl-pad.imx $(OBJTREE)/u-boot.uim

$(OBJTREE)/u-boot-with-nand-spl.imx: $(OBJTREE)/SPL $(OBJTREE)/u-boot.bin
	(echo -ne '\x00\x00\x00\x00\x46\x43\x42\x20\x01' && \
			dd bs=1015 count=1 if=/dev/zero 2>/dev/null) | \
		cat - $< > $(OBJTREE)/spl/u-boot-nand-spl.imx
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(CONFIG_SPL_PAD_TO) \
		-I binary -O binary $(OBJTREE)/spl/u-boot-nand-spl.imx \
		$(OBJTREE)/spl/u-boot-nand-spl-pad.imx
	rm $(OBJTREE)/spl/u-boot-nand-spl.imx
	$(OBJTREE)/tools/mkimage -A arm -O U-Boot -a $(CONFIG_SYS_TEXT_BASE) \
		-e $(CONFIG_SYS_TEXT_BASE) -C none -d $(OBJTREE)/u-boot.bin \
		$(OBJTREE)/u-boot.uim
	cat $(OBJTREE)/spl/u-boot-nand-spl-pad.imx $(OBJTREE)/u-boot.uim > $@
	rm $(OBJTREE)/spl/u-boot-nand-spl-pad.imx $(OBJTREE)/u-boot.uim


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
