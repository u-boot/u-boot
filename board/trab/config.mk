#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
#
# TRAB board with S3C2400X (arm920t) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# TRAB has 1 bank of 16 MB DRAM
#
# 0c00'0000 to 0e00'0000
#
# Linux-Kernel is expected to be at 0c00'8000, entry 0c00'8000
#
# we load ourself to 0cf0'0000
#
# download areas is 0c80'0000
#


TEXT_BASE = 0x0cf00000
