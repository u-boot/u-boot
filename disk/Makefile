# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

#ccflags-y += -DET_DEBUG -DDEBUG

obj-$(CONFIG_$(PHASE_)PARTITIONS)  += part.o
ifdef CONFIG_$(PHASE_)BLK
obj-$(CONFIG_$(PHASE_)PARTITIONS)  += disk-uclass.o
endif

# Must have BLK or SPL_LEGACY_BLOCK to support partitions
ifneq ($(CONFIG_$(PHASE_)BLK),$(CONFIG_SPL_LEGACY_BLOCK),)
obj-$(CONFIG_$(PHASE_)MAC_PARTITION)   += part_mac.o
obj-$(CONFIG_$(PHASE_)DOS_PARTITION)   += part_dos.o
obj-$(CONFIG_$(PHASE_)ISO_PARTITION)   += part_iso.o
obj-$(CONFIG_$(PHASE_)AMIGA_PARTITION) += part_amiga.o
obj-$(CONFIG_$(PHASE_)EFI_PARTITION)   += part_efi.o
endif
