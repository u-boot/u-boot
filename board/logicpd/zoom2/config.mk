#
# (C) Copyright 2009
# Texas Instruments, <www.ti.com>
#
# Zoom II uses OMAP3 (ARM-CortexA8) CPU
# see http://www.ti.com/ for more information on Texas Instruments
#
# SPDX-License-Identifier:	GPL-2.0+
#
# Physical Address:
# 0x80000000 (bank0)
# 0xA0000000 (bank1)
# Linux-Kernel is expected to be at 0x80008000, entry 0x80008000
# (mem base + reserved)

# For use with external or internal boots.
CONFIG_SYS_TEXT_BASE = 0x80008000
