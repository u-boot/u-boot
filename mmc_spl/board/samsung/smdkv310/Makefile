#
# (C) Copyright 2006-2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
#
# (C) Copyright 2011
# Chander Kashyap, Samsung Electronics, <k.chander@samsung.com>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CONFIG_MMC_SPL	= y

include $(TOPDIR)/config.mk

LDSCRIPT= $(TOPDIR)/mmc_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS	= -Bstatic -T $(mmcobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_MMC_SPL
AFLAGS	+= -DCONFIG_SPL_BUILD
CFLAGS	+= -DCONFIG_MMC_SPL
CFLAGS	+= -DCONFIG_SPL_BUILD

SOBJS	= start.o mem_setup.o lowlevel_init.o
COBJS	= mmc_boot.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/mmc_spl/board/$(BOARDDIR)

mmcobj	:= $(OBJTREE)/mmc_spl/


MKV310_MMC_SPL_EXEC = mkv310_mmc_spl_exec
MMC_SPL_BIN = u-boot-mmc-spl.bin

ALL = $(mmcobj)u-boot-spl $(mmcobj)u-boot-spl.bin $(mmcobj)$(MMC_SPL_BIN)

all:    $(obj).depend $(ALL)

$(mmcobj)$(MMC_SPL_BIN):  $(mmcobj)u-boot-spl.bin tools/$(MKV310_MMC_SPL_EXEC)
	./tools/$(MKV310_MMC_SPL_EXEC) $(mmcobj)u-boot-spl.bin $(mmcobj)$(MMC_SPL_BIN)
	rm -f tools/$(MKV310_MMC_SPL_EXEC)

tools/$(MKV310_MMC_SPL_EXEC): tools/mkv310_image.c
	$(HOSTCC) tools/mkv310_image.c -o tools/$(MKV310_MMC_SPL_EXEC)

$(mmcobj)u-boot-spl.bin:	$(mmcobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(mmcobj)u-boot-spl:	$(OBJS) $(mmcobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) \
		-Map $(mmcobj)u-boot-spl.map \
		-o $(mmcobj)u-boot-spl

$(mmcobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

# from cpu directory
start.S:
	@rm -f $@
	@ln -s $(TOPDIR)/arch/arm/cpu/armv7/start.S $@

# from board directory
mem_setup.S:
	@rm -f $@
	@ln -s $(TOPDIR)/board/samsung/smdkv310/mem_setup.S $@

lowlevel_init.S:
	@rm -f $@
	@ln -s $(TOPDIR)/board/samsung/smdkv310/lowlevel_init.S $@

#########################################################################

$(obj)%.o:	%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
