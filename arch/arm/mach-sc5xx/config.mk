# SPDX-License-Identifier: GPL-2.0-or-later
#
# (C) Copyright 2022 - Analog Devices, Inc.
#
# Written by Timesys Corporation
#

ifdef CONFIG_XPL_BUILD
INPUTS-y += $(obj)/u-boot-spl.ldr
endif

INPUTS-y += u-boot.ldr

LDR_FLAGS += --bcode=$(CONFIG_SC_BCODE)
LDR_FLAGS += --use-vmas
