#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_FSL_CADMUS)	+= cadmus.o
obj-$(CONFIG_FSL_VIA)		+= cds_via.o
obj-$(CONFIG_FMAN_ENET)	+= fman.o
obj-$(CONFIG_FSL_PIXIS)	+= pixis.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_FSL_NGPIXIS)	+= ngpixis.o
endif
obj-$(CONFIG_FSL_QIXIS)	+= qixis.o
obj-$(CONFIG_PQ_MDS_PIB)	+= pq-mds-pib.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ID_EEPROM)	+= sys_eeprom.o
endif
obj-$(CONFIG_FSL_SGMII_RISER)	+= sgmii_riser.o
ifndef CONFIG_RAMBOOT_PBL
obj-$(CONFIG_FSL_FIXED_MMC_LOCATION)	+= sdhc_boot.o
endif

obj-$(CONFIG_FSL_DIU_CH7301)	+= diu_ch7301.o

obj-$(CONFIG_FSL_DCU_SII9022A)    += dcu_sii9022a.o

obj-$(CONFIG_MPC8541CDS)	+= cds_pci_ft.o
obj-$(CONFIG_MPC8548CDS)	+= cds_pci_ft.o
obj-$(CONFIG_MPC8555CDS)	+= cds_pci_ft.o

obj-$(CONFIG_MPC8536DS)	+= ics307_clk.o
obj-$(CONFIG_MPC8572DS)	+= ics307_clk.o
obj-$(CONFIG_P1022DS)		+= ics307_clk.o
obj-$(CONFIG_P2020DS)		+= ics307_clk.o
obj-$(CONFIG_P3041DS)		+= ics307_clk.o
obj-$(CONFIG_P4080DS)		+= ics307_clk.o
obj-$(CONFIG_P5020DS)		+= ics307_clk.o
obj-$(CONFIG_P5040DS)		+= ics307_clk.o
obj-$(CONFIG_VSC_CROSSBAR)    += vsc3316_3308.o
obj-$(CONFIG_IDT8T49N222A)	+= idt8t49n222a_serdes_clk.o
obj-$(CONFIG_ZM7300)		+= zm7300.o

# deal with common files for P-series corenet based devices
obj-$(CONFIG_P2041RDB)	+= p_corenet/
obj-$(CONFIG_P3041DS)	+= p_corenet/
obj-$(CONFIG_P4080DS)	+= p_corenet/
obj-$(CONFIG_P5020DS)	+= p_corenet/
obj-$(CONFIG_P5040DS)	+= p_corenet/
endif
