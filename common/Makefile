#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libcommon.a

AOBJS	=

COBJS-y += main.o
COBJS-y += ACEX1K.o
COBJS-y += altera.o
COBJS-y += bedbug.o
COBJS-y += circbuf.o
COBJS-y += cmd_autoscript.o
COBJS-$(CONFIG_CMD_BDI) += cmd_bdinfo.o
COBJS-$(CONFIG_CMD_BEDBUG) += cmd_bedbug.o
COBJS-$(CONFIG_CMD_BMP) += cmd_bmp.o
COBJS-y += cmd_boot.o
COBJS-y += cmd_bootm.o
COBJS-$(CONFIG_CMD_CACHE) += cmd_cache.o
COBJS-$(CONFIG_CMD_CONSOLE) += cmd_console.o
COBJS-$(CONFIG_CMD_DATE) += cmd_date.o
ifdef CONFIG_4xx
COBJS-$(CONFIG_CMD_SETGETDCR) += cmd_dcr.o
endif
ifdef CONFIG_POST
COBJS-$(CONFIG_CMD_DIAG) += cmd_diag.o
endif
COBJS-$(CONFIG_CMD_DISPLAY) += cmd_display.o
COBJS-$(CONFIG_CMD_DOC) += cmd_doc.o
COBJS-$(CONFIG_CMD_DTT) += cmd_dtt.o
COBJS-y += cmd_eeprom.o
COBJS-$(CONFIG_CMD_ELF) += cmd_elf.o
COBJS-$(CONFIG_CMD_EXT2) += cmd_ext2.o
COBJS-$(CONFIG_CMD_FAT) += cmd_fat.o
COBJS-y += cmd_fdc.o
COBJS-$(CONFIG_OF_LIBFDT) += cmd_fdt.o fdt_support.o
COBJS-$(CONFIG_CMD_FDOS) += cmd_fdos.o
COBJS-$(CONFIG_CMD_FLASH) += cmd_flash.o
ifdef CONFIG_FPGA
COBJS-$(CONFIG_CMD_FPGA) += cmd_fpga.o
endif
COBJS-$(CONFIG_CMD_I2C) += cmd_i2c.o
COBJS-$(CONFIG_CMD_IDE) += cmd_ide.o
COBJS-$(CONFIG_CMD_IMMAP) += cmd_immap.o
COBJS-$(CONFIG_CMD_ITEST) += cmd_itest.o
COBJS-$(CONFIG_CMD_JFFS2) += cmd_jffs2.o
COBJS-y += cmd_load.o
COBJS-$(CONFIG_LOGBUFFER) += cmd_log.o
COBJS-y += cmd_mem.o
COBJS-$(CONFIG_CMD_MII) += cmd_mii.o
COBJS-$(CONFIG_CMD_MISC) += cmd_misc.o
COBJS-$(CONFIG_CMD_MMC) += cmd_mmc.o
COBJS-y += cmd_nand.o
COBJS-$(CONFIG_CMD_NET) += cmd_net.o
COBJS-y += cmd_nvedit.o
COBJS-y += cmd_onenand.o
COBJS-$(CONFIG_CMD_OTP) += cmd_otp.o
ifdef CONFIG_PCI
COBJS-$(CONFIG_CMD_PCI) += cmd_pci.o
endif
COBJS-y += cmd_pcmcia.o
COBJS-$(CONFIG_CMD_PORTIO) += cmd_portio.o
COBJS-$(CONFIG_CMD_REGINFO) += cmd_reginfo.o
COBJS-$(CONFIG_CMD_REISER) += cmd_reiser.o
COBJS-y += cmd_sata.o
COBJS-$(CONFIG_CMD_SCSI) += cmd_scsi.o
COBJS-$(CONFIG_CMD_SPI) += cmd_spi.o
COBJS-$(CONFIG_CMD_TERMINAL) += cmd_terminal.o
COBJS-$(CONFIG_CMD_UNIVERSE) += cmd_universe.o
COBJS-$(CONFIG_CMD_USB) += cmd_usb.o
COBJS-y += cmd_vfd.o
COBJS-y += command.o
COBJS-y += console.o
COBJS-y += cyclon2.o
COBJS-y += devices.o
COBJS-y += dlmalloc.o
COBJS-y += docecc.o
COBJS-y += environment.o
COBJS-y += env_common.o
COBJS-y += env_nand.o
COBJS-y += env_dataflash.o
COBJS-y += env_flash.o
COBJS-y += env_eeprom.o
COBJS-y += env_onenand.o
COBJS-y += env_nvram.o
COBJS-y += env_nowhere.o
COBJS-y += exports.o
COBJS-y += flash.o
COBJS-y += fpga.o
COBJS-y += ft_build.o
COBJS-y += hush.o
COBJS-y += kgdb.o
COBJS-y += lcd.o
COBJS-y += lists.o
COBJS-y += lynxkdi.o
COBJS-y += memsize.o
COBJS-y += miiphybb.o
COBJS-y += miiphyutil.o
COBJS-y += s_record.o
COBJS-y += serial.o
COBJS-y += soft_i2c.o
COBJS-y += soft_spi.o
COBJS-y += spartan2.o
COBJS-y += spartan3.o
COBJS-y += usb.o
COBJS-y += usb_kbd.o
COBJS-y += usb_storage.o
COBJS-y += virtex2.o
COBJS-y += xilinx.o
COBJS-y += crc16.o
COBJS-y += xyzModem.o
COBJS-y += cmd_mac.o
COBJS-$(CONFIG_CMD_MFSL) += cmd_mfsl.o

COBJS	:= $(COBJS-y)
SRCS	:= $(AOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

CPPFLAGS += -I..

all:	$(LIB) $(AOBJS)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(obj)environment.o: $(src)environment.c $(obj)../tools/envcrc
	$(CC) $(AFLAGS) -Wa,--no-warn \
		-DENV_CRC=$(shell $(obj)../tools/envcrc) \
		-c -o $@ $(src)environment.c

$(obj)../tools/envcrc:
	$(MAKE) -C ../tools

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
