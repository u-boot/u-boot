#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SAMSUNG board with S3C2400X (ARM920T) CPU
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SAMSUNG has 1 bank of 32 MB DRAM
#
# 0C00'0000 to 0E00'0000
#
# Linux-Kernel is expected to be at 0cf0'0000, entry 0cf0'0000
# optionally with a ramdisk at 0c80'0000
#
# we load ourself to 0CF80000 (must be high enough not to be
# overwritten by the uncompessing Linux kernel)
#
# download area is 0C80'0000
#


TEXT_BASE = 0x0CF80000
