#
# (C) Copyright 2008
# Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# GR-CPCI-AX2000 board
#

# U-BOOT IN FLASH
CONFIG_SYS_TEXT_BASE = 0x00000000

# U-BOOT IN RAM or SDRAM with -nosram flag set when starting GRMON
#CONFIG_SYS_TEXT_BASE = 0x40000000

# U-BOOT IN SDRAM
#CONFIG_SYS_TEXT_BASE = 0x60000000
