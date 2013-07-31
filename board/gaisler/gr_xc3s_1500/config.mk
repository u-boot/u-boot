#
# (C) Copyright 2007
# Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# GR-XC3S-1500 board
#

# U-BOOT IN FLASH
CONFIG_SYS_TEXT_BASE = 0x00000000

# U-BOOT IN RAM
#CONFIG_SYS_TEXT_BASE = 0x40000000

PLATFORM_CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
	-I$(TOPDIR)/board
