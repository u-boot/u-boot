# SPDX-License-Identifier: GPL-2.0-or-later
#
# (C) Copyright 2022 - Analog Devices, Inc.
#
# Written and/or maintained by Timesys Corporation
#
# Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
# Contact: Greg Malysa <greg.malysa@timesys.com>
#

obj-y += soc.o init/

obj-$(CONFIG_SC57X) += sc57x.o
obj-$(CONFIG_SC58X) += sc58x.o
obj-$(CONFIG_SC59X) += sc59x.o
obj-$(CONFIG_SC59X) += sc59x-spl.o
obj-$(CONFIG_SC59X_64) += sc59x_64.o
obj-$(CONFIG_SC59X_64) += sc59x_64-spl.o

obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SYSCON) += rcu.o
