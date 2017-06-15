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

obj-$(CONFIG_IDE) += ide.o
obj-$(CONFIG_IDE_FTIDE020) += ftide020.o
obj-$(CONFIG_SANDBOX) += sandbox.o sandbox_scsi.o
obj-$(CONFIG_SYSTEMACE) += systemace.o
obj-$(CONFIG_BLOCK_CACHE) += blkcache.o
