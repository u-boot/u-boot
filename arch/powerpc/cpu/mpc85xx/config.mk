#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao, X.Xiao@motorola.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi

PLATFORM_CPPFLAGS += -ffixed-r2 -Wa,-me500 -msoft-float -mno-string

# -mspe=yes is needed to have -mno-spe accepted by a buggy GCC;
# see "[PATCH,rs6000] make -mno-spe work as expected" on
# http://gcc.gnu.org/ml/gcc-patches/2008-04/msg00311.html
PF_CPPFLAGS_SPE := $(call cc-option,-mspe=yes) \
		   $(call cc-option,-mno-spe)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_SPE)
