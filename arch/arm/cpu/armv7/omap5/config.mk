#
# Copyright 2011 Linaro Limited
#
# Aneesh V <annesh@ti.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
ALL-y	+= MLO
else
ALL-y	+= u-boot.img
endif
