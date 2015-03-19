#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := avr32-linux-
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0x00000000

PLATFORM_RELFLAGS	+= -ffixed-r5 -fPIC -mno-init-got -mrelax
PLATFORM_RELFLAGS	+= -ffunction-sections -fdata-sections

LDFLAGS_u-boot		= --gc-sections --relax
