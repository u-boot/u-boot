# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2015 Stefan Roese <sr@denx.de>
# Copyright (C) 2015 Reinhard Pfau <reinhard.pfau@gdsys.cc>
# Copyright (C) 2016 Mario Six <mario.six@gdsys.cc>

obj-$(CONFIG_TARGET_CONTROLCENTERDC) += controlcenterdc.o hre.o spl.o keyprogram.o dt_helpers.o

ifeq ($(CONFIG_SPL_BUILD),)

obj-$(CONFIG_TARGET_CONTROLCENTERDC) += hydra.o ihs_phys.o

extra-$(CONFIG_TARGET_CONTROLCENTERDC) += kwbimage.cfg

KWB_REPLACE += BOOT_FROM
ifneq ($(CONFIG_SPL_BOOT_DEVICE_SPI),)
	KWB_CFG_BOOT_FROM=spi
endif
ifneq ($(CONFIG_SPL_BOOT_DEVICE_MMC),)
	KWB_CFG_BOOT_FROM=sdio
endif

ifneq ($(CONFIG_SECURED_MODE_IMAGE),)
KWB_REPLACE += CSK_INDEX
KWB_CFG_CSK_INDEX = $(CONFIG_SECURED_MODE_CSK_INDEX)

KWB_REPLACE += SEC_BOOT_DEV
KWB_CFG_SEC_BOOT_DEV=$(patsubst "%",%, \
	$(if $(findstring BOOT_SPI_NOR_FLASH,$(CONFIG_SPL_BOOT_DEVICE)),0x34) \
	$(if $(findstring BOOT_SDIO_MMC_CARD,$(CONFIG_SPL_BOOT_DEVICE)),0x31) \
	)

KWB_REPLACE += SEC_FUSE_DUMP
KWB_CFG_SEC_FUSE_DUMP = a38x
endif

$(src)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	$(Q)sed -ne '$(foreach V,$(KWB_REPLACE),s/^#@$(V)/$(V) $(KWB_CFG_$(V))/;)p' \
	<$< >$(dir $<)$(@F)

endif
