#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# (C) Copyright 2003
# Texas Instruments, <www.ti.com>
# Swaminathan <swami.iyer@ti.com>
#
# Davinci EVM board (ARM925EJS) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# Davinci EVM has 1 bank of 256 MB DDR RAM
# Physical Address:
# 8000'0000 to 9000'0000
#
# Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
#
# Visioneering Corp. Sonata board (ARM926EJS) cpu
#
# Sonata board has 1 bank of 128 MB DDR RAM
# Physical Address:
# 8000'0000 to 8800'0000
#
# Razorstream, LLC. SCHMOOGIE board (ARM926EJS) cpu
#
# Schmoogie board has 1 bank of 128 MB DDR RAM
# Physical Address:
# 8000'0000 to 8800'0000
#
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#
# we load ourself to 8108 '0000
#
#

#Provide at least 16MB spacing between us and the Linux Kernel image
TEXT_BASE = 0x81080000
