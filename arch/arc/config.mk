#
# Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_CPU_BIG_ENDIAN
CONFIG_SYS_LITTLE_ENDIAN = 1
else
CONFIG_SYS_BIG_ENDIAN = 1
endif

ifdef CONFIG_SYS_LITTLE_ENDIAN
ARC_CROSS_COMPILE := arc-linux-
PLATFORM_LDFLAGS += -EL
PLATFORM_CPPFLAGS += -mlittle-endian
endif

ifdef CONFIG_SYS_BIG_ENDIAN
ARC_CROSS_COMPILE := arceb-linux-
PLATFORM_LDFLAGS += -EB
PLATFORM_CPPFLAGS += -mbig-endian
endif

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := $(ARC_CROSS_COMPILE)
endif

ifdef CONFIG_ARC_MMU_VER
CONFIG_MMU = 1
endif

ifdef CONFIG_CPU_ARC750D
PLATFORM_CPPFLAGS += -mcpu=arc700
endif

ifdef CONFIG_CPU_ARC770D
PLATFORM_CPPFLAGS += -mcpu=arc700 -mlock -mswape
endif

ifdef CONFIG_CPU_ARCEM6
PLATFORM_CPPFLAGS += -mcpu=arcem
endif

ifdef CONFIG_CPU_ARCHS34
PLATFORM_CPPFLAGS += -mcpu=archs
endif

ifdef CONFIG_CPU_ARCHS38
PLATFORM_CPPFLAGS += -mcpu=archs
endif

PLATFORM_CPPFLAGS += -ffixed-r25 -D__ARC__ -gdwarf-2

# Needed for relocation
LDFLAGS_FINAL += -pie

# Load address for standalone apps
CONFIG_STANDALONE_LOAD_ADDR ?= 0x82000000
