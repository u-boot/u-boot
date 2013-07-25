#
# (C) Copyright 2007-2008
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
ENDIANNESS += -EB

ifdef CONFIG_SH2A
PLATFORM_CPPFLAGS += -m2a -m2a-nofpu -mb -ffreestanding
else # SH2
PLATFORM_CPPFLAGS += -m3e -mb
endif
PLATFORM_CPPFLAGS += $(call cc-option,-mno-fdpic)

PLATFORM_RELFLAGS += -ffixed-r13
PLATFORM_LDFLAGS += $(ENDIANNESS)
