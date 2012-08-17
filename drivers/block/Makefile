#
# (C) Copyright 2000-2007
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

LIB	:= $(obj)libblock.o

COBJS-$(CONFIG_SCSI_AHCI) += ahci.o
COBJS-$(CONFIG_ATA_PIIX) += ata_piix.o
COBJS-$(CONFIG_FSL_SATA) += fsl_sata.o
COBJS-$(CONFIG_IDE_FTIDE020) += ftide020.o
COBJS-$(CONFIG_LIBATA) += libata.o
COBJS-$(CONFIG_CMD_MG_DISK) += mg_disk.o
COBJS-$(CONFIG_MVSATA_IDE) += mvsata_ide.o
COBJS-$(CONFIG_MX51_PATA) += mxc_ata.o
COBJS-$(CONFIG_PATA_BFIN) += pata_bfin.o
COBJS-$(CONFIG_SATA_DWC) += sata_dwc.o
COBJS-$(CONFIG_SATA_SIL3114) += sata_sil3114.o
COBJS-$(CONFIG_SATA_SIL) += sata_sil.o
COBJS-$(CONFIG_IDE_SIL680) += sil680.o
COBJS-$(CONFIG_SCSI_SYM53C8XX) += sym53c8xx.o
COBJS-$(CONFIG_SYSTEMACE) += systemace.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
