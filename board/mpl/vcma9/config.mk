#
# (C) Copyright 2002
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# MPL VCMA9 board with S3C2410X (ARM920T) cpu
#
# see http://www.mpl.ch/ for more information about the MPL VCMA9
#

#
# MPL VCMA9 has 1 bank of 64 MB DRAM
#
# 3000'0000 to 3400'0000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# we load ourself to 33F0'0000
#
# download area is 3300'0000
#


TEXT_BASE = 0x33F00000
