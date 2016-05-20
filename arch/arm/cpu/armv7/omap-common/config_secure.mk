#
# Copyright (C) 2016, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#
quiet_cmd_mkomapsecimg = MKIMAGE $@
ifneq ($(TI_SECURE_DEV_PKG),)
ifneq ($(wildcard $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh),)
ifneq ($(CONFIG_SPL_BUILD),)
cmd_mkomapsecimg = $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh \
	$(patsubst u-boot-spl_HS_%,%,$(@F)) $< $@ $(CONFIG_ISW_ENTRY_ADDR) \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)
else
cmd_mkomapsecimg = $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh \
    $(patsubst u-boot_HS_%,%,$(@F)) $< $@ $(CONFIG_ISW_ENTRY_ADDR) \
    $(if $(KBUILD_VERBOSE:1=), >/dev/null)
endif
else
cmd_mkomapsecimg = echo "WARNING:" \
	"$(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh not found." \
	"$@ was NOT created!"
endif
else
cmd_mkomapsecimg = echo "WARNING: TI_SECURE_DEV_PKG environment" \
	"variable must be defined for TI secure devices. $@ was NOT created!"
endif

# Standard X-LOADER target (QPSI, NOR flash)
u-boot-spl_HS_X-LOADER: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)

# For MLO targets (SD card boot) the final file name
# that is copied to the SD card fAT partition must
# be MLO, so we make a copy of the output file to a
# new file with that name
u-boot-spl_HS_MLO: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)
	@if [ -f $@ ]; then \
		cp -f $@ MLO; \
	fi

# Standard 2ND target (certain peripheral boot modes)
u-boot-spl_HS_2ND: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)

# Standard ULO target (certain peripheral boot modes)
u-boot-spl_HS_ULO: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)

# Standard ISSW target (certain devices, various boot modes)
u-boot-spl_HS_ISSW: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)

# For SPI flash on AM335x and AM43xx, these
# require special byte swap handling so we use
# the SPI_X-LOADER target instead of X-LOADER
# and let the create-boot-image.sh script handle
# that
u-boot-spl_HS_SPI_X-LOADER: $(obj)/u-boot-spl.bin
	$(call if_changed,mkomapsecimg)

# For supporting single stage XiP QSPI on AM43xx, the
# image is a full u-boot file, not an SPL. In this case
# the mkomapsecimg command looks for a u-boot-HS_* prefix
u-boot_HS_XIP_X-LOADER: $(obj)/u-boot.bin
	$(call if_changed,mkomapsecimg)
