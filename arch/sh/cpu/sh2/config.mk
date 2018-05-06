# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2007-2008
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
ENDIANNESS += -EB

ifdef CONFIG_CPU_SH2A
PLATFORM_CPPFLAGS += -m2a-nofpu -mb
else # SH2
PLATFORM_CPPFLAGS += -m3e -mb
endif
PLATFORM_CPPFLAGS += $(call cc-option,-mno-fdpic)

PLATFORM_LDFLAGS += $(ENDIANNESS)
