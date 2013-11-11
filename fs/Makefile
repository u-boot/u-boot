#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_FAT_SUPPORT) += fat/
else
obj-y				+= fs.o

obj-y += cbfs/
obj-y += cramfs/
obj-y += ext4/
obj-y += fat/
obj-y += fdos/
obj-y += jffs2/
obj-y += reiserfs/
obj-y += sandbox/
obj-y += ubifs/
obj-y += yaffs2/
obj-y += zfs/
endif
