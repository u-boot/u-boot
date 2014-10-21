#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifndef CONFIG_SPL_BUILD
ALL-y	+= u-boot.img
endif

# Added for handoff support
PLATFORM_RELFLAGS += -Iboard/$(VENDOR)/$(BOARD)
