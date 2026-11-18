# SPDX-License-Identifier: GPL-2.0-or-later
#
# (C) Copyright 2022 - Analog Devices, Inc.
#
# Written by Timesys Corporation
#

# Note: '2>/dev/null' is here to force Make to invoke a shell instead of
# trying to directly execute any utility named 'command' - following the
# fast path - which will cause it to fail with 'command: Command not found'.
# This workaround should be kept until support for GNU Make 4.2.1 and
# previous versions is dropped.
ifneq ($(shell command -v -- $(LDR) 2>/dev/null),)

ifdef CONFIG_XPL_BUILD
INPUTS-y += $(obj)/u-boot-spl.ldr
endif

INPUTS-y += u-boot.ldr

LDR_FLAGS += --bcode=$(CONFIG_SC_BCODE)
LDR_FLAGS += --use-vmas

endif
