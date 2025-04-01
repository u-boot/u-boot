# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_$(PHASE_)BLK) += blk-uclass.o

ifndef CONFIG_$(PHASE_)BLK
obj-$(CONFIG_SPL_LEGACY_BLOCK) += blk_legacy.o
endif

ifndef CONFIG_XPL_BUILD
obj-$(CONFIG_IDE) += ide.o
obj-$(CONFIG_RKMTD) += rkmtd.o
endif
obj-$(CONFIG_SANDBOX) += sandbox.o host-uclass.o host_dev.o
obj-$(CONFIG_$(PHASE_)BLOCK_CACHE) += blkcache.o
obj-$(CONFIG_$(PHASE_)BLKMAP) += blkmap.o
obj-$(CONFIG_$(PHASE_)BLKMAP) += blkmap_helper.o

obj-$(CONFIG_EFI_MEDIA) += efi-media-uclass.o
obj-$(CONFIG_EFI_MEDIA_SANDBOX) += sb_efi_media.o
obj-$(CONFIG_EFI_MEDIA_BLK) += efi_blk.o

