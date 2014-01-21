#
# Copyright (C) 2012 Samsung Electronics
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= smdk5250_spl.o

ifndef CONFIG_SPL_BUILD
ifdef CONFIG_OF_CONTROL
obj-y	+= exynos5-dt.o
else
obj-y	+= smdk5250.o
endif
endif
