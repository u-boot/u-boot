#
# (C) Copyright 2004
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
#
# SX1 board with OMAP1510 (ARM925T) cpu
# see http://www.ti.com/ for more information on Texas Insturments
#
# SX1 has 1 bank of 256 MB SDRAM
# Physical Address:
# 1000'0000 to 2000'0000
#
#
# Linux-Kernel is expected to be at 1000'8000, entry 1000'8000  (mem base + reserved)
#
# we load ourself to 1108'0000
#
#

TEXT_BASE = 0x11080000
