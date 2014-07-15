#
# Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SYS_BIG_ENDIAN
CONFIG_SYS_LITTLE_ENDIAN = 1
endif

ifdef CONFIG_SYS_LITTLE_ENDIAN
ARC_CROSS_COMPILE := arc-buildroot-linux-uclibc-
endif

ifdef CONFIG_SYS_BIG_ENDIAN
ARC_CROSS_COMPILE := arceb-buildroot-linux-uclibc-
PLATFORM_LDFLAGS += -EB
endif

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := $(ARC_CROSS_COMPILE)
endif

PLATFORM_CPPFLAGS += -ffixed-r25 -D__ARC__ -DCONFIG_ARC -gdwarf-2

# Needed for relocation
LDFLAGS_FINAL += -pie

# Load address for standalone apps
CONFIG_STANDALONE_LOAD_ADDR ?= 0x82000000

# Support generic board on ARC
__HAVE_ARCH_GENERIC_BOARD := y
