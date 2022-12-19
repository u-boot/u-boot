# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao, X.Xiao@motorola.com

PLATFORM_CPPFLAGS += -msoft-float -mno-string
PLATFORM_RELFLAGS += -msingle-pic-base -fno-jump-tables

# -mspe=yes is needed to have -mno-spe accepted by a buggy GCC;
# see "[PATCH,rs6000] make -mno-spe work as expected" on
# http://gcc.gnu.org/ml/gcc-patches/2008-04/msg00311.html
PLATFORM_CPPFLAGS += $(call cc-option,-mspe=yes) \
		   $(call cc-option,-mno-spe)

ifdef CONFIG_E6500
PLATFORM_CPPFLAGS += -mcpu=e6500
else ifdef CONFIG_E5500
PLATFORM_CPPFLAGS += -mcpu=e5500
else ifdef CONFIG_E500MC
PLATFORM_CPPFLAGS += -mcpu=e500mc
else
PLATFORM_CPPFLAGS += -mcpu=8540
endif
