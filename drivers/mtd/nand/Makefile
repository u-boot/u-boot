#
# (C) Copyright 2006
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

LIB	:= $(obj)libnand.a

ifdef CONFIG_CMD_NAND
COBJS-y += nand.o
COBJS-y += nand_base.o
COBJS-y += nand_bbt.o
COBJS-y += nand_ecc.o
COBJS-y += nand_ids.o
COBJS-y += nand_util.o

COBJS-$(CONFIG_NAND_ATMEL) += atmel_nand.o
COBJS-$(CONFIG_DRIVER_NAND_BFIN) += bfin_nand.o
COBJS-$(CONFIG_NAND_DAVINCI) += davinci_nand.o
COBJS-$(CONFIG_NAND_FSL_ELBC) += fsl_elbc_nand.o
COBJS-$(CONFIG_NAND_FSL_UPM) += fsl_upm.o
COBJS-$(CONFIG_NAND_KB9202) += kb9202_nand.o
COBJS-$(CONFIG_NAND_KIRKWOOD) += kirkwood_nand.o
COBJS-$(CONFIG_NAND_KMETER1) += kmeter1_nand.o
COBJS-$(CONFIG_NAND_MPC5121_NFC) += mpc5121_nfc.o
COBJS-$(CONFIG_NAND_MXC) += mxc_nand.o
COBJS-$(CONFIG_NAND_NDFC) += ndfc.o
COBJS-$(CONFIG_NAND_NOMADIK) += nomadik.o
COBJS-$(CONFIG_NAND_S3C2410) += s3c2410_nand.o
COBJS-$(CONFIG_NAND_S3C64XX) += s3c64xx.o
COBJS-$(CONFIG_NAND_SPEAR) += spr_nand.o
COBJS-$(CONFIG_NAND_OMAP_GPMC) += omap_gpmc.o
COBJS-$(CONFIG_NAND_PLAT) += nand_plat.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
