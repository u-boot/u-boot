#
# Copyright 2011 Linaro Limited
#
# (C) Copyright 2010
# Texas Instruments, <www.ti.com>
#
# Aneesh V <aneesh@ti.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_SPL_BUILD
ALL-y	+= MLO
else
ALL-y	+= u-boot.img
endif
