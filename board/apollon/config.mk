#
# (C) Copyright 2005-2007
# Samsung Electronics
#
# Samsung December board with OMAP2420 (ARM1136) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# December has 1 bank of 128MB mDDR-SDRAM on CS0
# December has 1 bank of  00MB mDDR-SDRAM on CS1
# Physical Address:
# 8000'0000 (bank0)
# A000/0000 (bank1) ES2 will be configurable
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
# For use with external or internal boots.
TEXT_BASE = 0x80e80000

# Used with full SRAM boot.
# This is either with a GP system or a signed boot image.
# easiest, and safest way to go if you can.
#TEXT_BASE = 0x40270000

# Handy to get symbols to debug ROM version.
#TEXT_BASE = 0x0
#TEXT_BASE = 0x08000000
