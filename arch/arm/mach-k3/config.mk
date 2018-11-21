# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

ifdef CONFIG_SPL_BUILD

ifdef CONFIG_ARM64
SPL_ITS := u-boot-spl-k3.its
$(SPL_ITS): FORCE
	$(srctree)/tools/k3_fit_atf.sh \
	$(patsubst %,$(obj)/dts/%.dtb,$(subst ",,$(CONFIG_SPL_OF_LIST))) > $@

ALL-y	+= tispl.bin
endif

else
ALL-y	+= u-boot.img
endif
