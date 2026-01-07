# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

PLATFORM_CPPFLAGS += -D__M68K__
ifneq ($(CONFIG_M680x0),y)
PLATFORM_CPPFLAGS += -fPIC
endif
KBUILD_LDFLAGS    += -n -pie
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections
PLATFORM_RELFLAGS += -ffixed-d7
ifneq ($(CONFIG_M680x0),y)
PLATFORM_RELFLAGS += -msep-data
endif
LDFLAGS_FINAL     += --gc-sections -pie
