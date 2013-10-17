#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)board/freescale/common)
endif

LIB	= $(obj)libfreescale.o

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifndef MINIMAL
COBJS-$(CONFIG_FSL_CADMUS)	+= cadmus.o
COBJS-$(CONFIG_FSL_VIA)		+= cds_via.o
COBJS-$(CONFIG_FMAN_ENET)	+= fman.o
COBJS-$(CONFIG_FSL_PIXIS)	+= pixis.o
ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_FSL_NGPIXIS)	+= ngpixis.o
endif
COBJS-$(CONFIG_FSL_QIXIS)	+= qixis.o
COBJS-$(CONFIG_PQ_MDS_PIB)	+= pq-mds-pib.o
ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_ID_EEPROM)	+= sys_eeprom.o
endif
COBJS-$(CONFIG_FSL_SGMII_RISER)	+= sgmii_riser.o
ifndef CONFIG_RAMBOOT_PBL
COBJS-$(CONFIG_FSL_FIXED_MMC_LOCATION)	+= sdhc_boot.o
endif

COBJS-$(CONFIG_MPC8541CDS)	+= cds_pci_ft.o
COBJS-$(CONFIG_MPC8548CDS)	+= cds_pci_ft.o
COBJS-$(CONFIG_MPC8555CDS)	+= cds_pci_ft.o

COBJS-$(CONFIG_MPC8536DS)	+= ics307_clk.o
COBJS-$(CONFIG_MPC8572DS)	+= ics307_clk.o
COBJS-$(CONFIG_P1022DS)		+= ics307_clk.o
COBJS-$(CONFIG_P2020DS)		+= ics307_clk.o
COBJS-$(CONFIG_P3041DS)		+= ics307_clk.o
COBJS-$(CONFIG_P4080DS)		+= ics307_clk.o
COBJS-$(CONFIG_P5020DS)		+= ics307_clk.o
COBJS-$(CONFIG_P5040DS)		+= ics307_clk.o
COBJS-$(CONFIG_VSC_CROSSBAR)    += vsc3316_3308.o
COBJS-$(CONFIG_IDT8T49N222A)	+= idt8t49n222a_serdes_clk.o

# deal with common files for P-series corenet based devices
SUBLIB-$(CONFIG_P2041RDB)	+= p_corenet/libp_corenet.o
SUBLIB-$(CONFIG_P3041DS)	+= p_corenet/libp_corenet.o
SUBLIB-$(CONFIG_P4080DS)	+= p_corenet/libp_corenet.o
SUBLIB-$(CONFIG_P5020DS)	+= p_corenet/libp_corenet.o
SUBLIB-$(CONFIG_P5040DS)	+= p_corenet/libp_corenet.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))
SUBLIB	:= $(addprefix $(obj),$(SUBLIB-y))

$(LIB):	$(obj).depend $(OBJS) $(SUBLIB)
	$(call cmd_link_o_target, $(OBJS) $(SUBLIB))

$(SUBLIB): $(obj).depend
	$(MAKE) -C $(dir $(subst $(obj),,$@))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
