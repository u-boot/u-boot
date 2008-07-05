#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# Lyrtech SFF SDR board (ARM926EJS) cpu
# see http://www.lyrtech.com/ for more information on Lyrtech
#
# SFF SDR board has 1 bank of 128 MB DDR RAM
# Physical Address:
# 8000'0000 to 87FF'FFFF
#
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)
#
# Integrity kernel is expected to be at 8000'0000, entry 8000'00D0,
# up to 81FF'FFFF (uses up to 32 MB of memory for text, heap, etc).
#
# we load ourself to 8400'0000
#
#

# Provide at least 32MB spacing between us and the Integrity kernel image
TEXT_BASE = 0x84000000
