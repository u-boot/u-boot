#
# (C) Copyright 2004
# Texas Instruments, <www.ti.com>
#
# TI H4 board with OMAP2420 (ARM1136) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# H4 has 1 bank of 32MB or 64MB mDDR-SDRAM on CS0
# H4 has 1 bank of 32MB or 00MB mDDR-SDRAM on CS1
# Physical Address:
# 8000'0000 (bank0)
# A000/0000 (bank1) ES2 will be configurable
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)

# For use with external or internal boots.
# CONFIG_PARTIAL_SRAM must be defined to use this.
TEXT_BASE = 0x80e80000

# Used with full SRAM boot.
# This is either with a GP system or a signed boot image.
# easiest, and safest way to go if you can.
# Comment out //CONFIG_PARTIAL_SRAM for this one.
#
#TEXT_BASE = 0x40280000

