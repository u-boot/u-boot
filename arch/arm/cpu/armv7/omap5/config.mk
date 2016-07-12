#
# Copyright 2011 Linaro Limited
#
# Aneesh V <annesh@ti.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include  $(srctree)/$(CPUDIR)/omap-common/config_secure.mk

ifdef CONFIG_SPL_BUILD
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ALL-y	+= u-boot-spl_HS_MLO u-boot-spl_HS_X-LOADER
else
ALL-y	+= MLO
endif
else
ALL-y	+= u-boot.img
endif
