#
# (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
#
# SPDX-License-Identifier:     GPL-2.0+
#

cpuflags-$(CONFIG_M5307) := -mcpu=5307

PLATFORM_CPPFLAGS += $(cpuflags-y)
