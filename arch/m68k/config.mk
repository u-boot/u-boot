# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

PLATFORM_CPPFLAGS += -D__M68K__
KBUILD_LDFLAGS  += -n
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections
PLATFORM_RELFLAGS += -ffixed-d7 -msep-data
LDFLAGS_FINAL                  += --gc-sections
