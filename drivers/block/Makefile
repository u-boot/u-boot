#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_BLK) += blk-uclass.o

ifndef CONFIG_BLK
obj-y += blk_legacy.o
endif

obj-$(CONFIG_DWC_AHCI) += dwc_ahci.o
obj-$(CONFIG_AHCI) += ahci-uclass.o
obj-$(CONFIG_DM_SCSI) += scsi-uclass.o
obj-$(CONFIG_SCSI_AHCI) += ahci.o
obj-$(CONFIG_DWC_AHSATA) += dwc_ahsata.o
obj-$(CONFIG_FSL_SATA) += fsl_sata.o
obj-$(CONFIG_IDE) += ide.o
obj-$(CONFIG_IDE_FTIDE020) += ftide020.o
obj-$(CONFIG_LIBATA) += libata.o
obj-$(CONFIG_MVSATA_IDE) += mvsata_ide.o
obj-$(CONFIG_MX51_PATA) += mxc_ata.o
obj-$(CONFIG_SATA_CEVA) += sata_ceva.o
obj-$(CONFIG_SATA_DWC) += sata_dwc.o
obj-$(CONFIG_SATA_MV) += sata_mv.o
obj-$(CONFIG_SATA_SIL3114) += sata_sil3114.o
obj-$(CONFIG_SATA_SIL) += sata_sil.o
obj-$(CONFIG_SANDBOX) += sandbox.o sandbox_scsi.o sata_sandbox.o
obj-$(CONFIG_SCSI_SYM53C8XX) += sym53c8xx.o
obj-$(CONFIG_SYSTEMACE) += systemace.o
obj-$(CONFIG_BLOCK_CACHE) += blkcache.o
