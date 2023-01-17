# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_DM_SCSI) += scsi-uclass.o
obj-$(CONFIG_SCSI) += scsi.o

ifdef CONFIG_SCSI
ifdef CONFIG_DM_SCSI
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += scsi_bootdev.o
endif
endif

endif

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_SATA
obj-$(CONFIG_DM_SCSI) += scsi-uclass.o
obj-$(CONFIG_SCSI) += scsi.o
endif
endif

ifdef CONFIG_SCSI
obj-$(CONFIG_SANDBOX) += sandbox_scsi.o
obj-$(CONFIG_SANDBOX) += scsi_emul.o
endif
