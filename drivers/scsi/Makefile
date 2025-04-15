# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_XPL_BUILD
obj-$(CONFIG_SCSI) += scsi.o scsi-uclass.o
ifdef CONFIG_SCSI
obj-$(CONFIG_$(PHASE_)BOOTSTD) += scsi_bootdev.o
obj-$(CONFIG_SANDBOX) += sandbox_scsi.o
obj-$(CONFIG_SANDBOX) += scsi_emul.o
endif
endif

ifdef CONFIG_XPL_BUILD
ifdef CONFIG_SPL_SATA
obj-$(CONFIG_SCSI) += scsi.o scsi-uclass.o
endif
endif
