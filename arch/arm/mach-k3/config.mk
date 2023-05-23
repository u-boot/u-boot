# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

ifdef CONFIG_SPL_BUILD

# Openssl is required to generate x509 certificate.
# Error out if openssl is not available.
ifeq ($(shell which openssl),)
$(error "No openssl in $(PATH), consider installing openssl")
endif

O ?= .

# Board config binary artifacts necessary for packaging of tiboot3.bin
# and sysfw.itb by binman, currently for general purpose devices and
# devices that require sysfw.itb in ROM boot image.

ifdef CONFIG_BINMAN

ifndef CONFIG_TARGET_J7200_R5_EVM
BOARD_YAML = $(srctree)/board/ti/$(BOARD)/board-cfg.yaml
RM_YAML = $(srctree)/board/ti/$(BOARD)/rm-cfg.yaml
SEC_YAML = $(srctree)/board/ti/$(BOARD)/sec-cfg.yaml
PM_YAML = $(srctree)/board/ti/$(BOARD)/pm-cfg.yaml
else
BOARD_YAML = $(srctree)/board/ti/$(BOARD)/board-cfg_j7200.yaml
RM_YAML = $(srctree)/board/ti/$(BOARD)/rm-cfg_j7200.yaml
SEC_YAML = $(srctree)/board/ti/$(BOARD)/sec-cfg_j7200.yaml
PM_YAML = $(srctree)/board/ti/$(BOARD)/pm-cfg_j7200.yaml
endif

CUSTOMER_KEY = $(srctree)/board/ti/keys/custMpk.pem
TI_DEGENERATE_KEY = $(srctree)/board/ti/keys/ti-degenerate-key.pem

SCHEMA_YAML = $(srctree)/board/ti/common/schema.yaml

board-cfg.yaml: $(BOARD_YAML)
	@cp $< $@
rm-cfg.yaml: $(RM_YAML)
	@cp $< $@
sec-cfg.yaml: $(SEC_YAML)
	@cp $< $@
pm-cfg.yaml: $(PM_YAML)
	@cp $< $@

schema.yaml: $(SCHEMA_YAML)
	@cp $< $@
custMpk.pem: $(CUSTOMER_KEY)
	@cp $< $@
ti-degenerate-key.pem: $(TI_DEGENERATE_KEY)
	@cp $< $@

INPUTS-y	+= board-cfg.yaml
INPUTS-y	+= rm-cfg.yaml
INPUTS-y	+= sec-cfg.yaml
INPUTS-y	+= pm-cfg.yaml
INPUTS-y	+= schema.yaml
INPUTS-y	+= custMpk.pem
INPUTS-y	+= ti-degenerate-key.pem
endif
endif
