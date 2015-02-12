#
# (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
#
# SPDX-License-Identifier:     GPL-2.0+
#

cfg=$(srctree)/include/configs/$(CONFIG_SYS_CONFIG_NAME:"%"=%).h
is5307:=$(shell grep CONFIG_M5307 $(cfg))

ifneq (,$(findstring CONFIG_M5307,$(is5307)))
PLATFORM_CPPFLAGS += -mcpu=5307
endif
