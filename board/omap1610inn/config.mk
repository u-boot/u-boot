#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# (C) Copyright 2003
# Texas Instruments, <www.ti.com>
# Kshitij Gupta <Kshitij@ti.com>
#
# TI Innovator board with OMAP1610 (ARM925EJS) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# Innovator has 1 bank of 256 MB SDRAM
# Physical Address:
# 1000'0000 to 2000'0000
#
#
# Linux-Kernel is expected to be at 1000'8000, entry 1000'8000
# (mem base + reserved)
#
# we load ourself to 1108'0000
#
#


TEXT_BASE = 0x11080000
