#
# (C) Copyright 2002, 2003
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# MPL VCMA9 board with S3C2410X (ARM920T) cpu
#
# see http://www.mpl.ch/ for more information about the MPL VCMA9
#

#
# MPL VCMA9 has 1 bank of minimal 16 MB DRAM
# from 0x30000000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3040'0000
#
# we load ourself to 33F8'0000
#
# download area is 3080'0000
#


#TEXT_BASE = 0x30F80000
TEXT_BASE = 0x33F80000
