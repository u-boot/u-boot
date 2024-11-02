# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2012 The Chromium Authors

obj-y += test-main.o

obj-$(CONFIG_$(XPL_)CMDLINE) += cmd/
obj-$(CONFIG_$(XPL_)CMDLINE) += cmd_ut.o
obj-y += dm/
obj-$(CONFIG_FUZZ) += fuzz/
ifndef CONFIG_SANDBOX_VPL
obj-$(CONFIG_UNIT_TEST) += lib/
endif
ifneq ($(CONFIG_HUSH_PARSER),)
obj-$(CONFIG_$(XPL_)CMDLINE) += hush/
endif
obj-$(CONFIG_UT_TIME) += time_ut.o
obj-y += ut.o

ifeq ($(CONFIG_XPL_BUILD),)
obj-y += boot/
obj-$(CONFIG_UNIT_TEST) += common/
obj-y += log/
obj-$(CONFIG_$(XPL_)UT_UNICODE) += unicode_ut.o
else
obj-$(CONFIG_SPL_UT_LOAD) += image/
endif
