#######################################################################
#
# Copyright (C) 2004 by FS Forth-Systeme GmbH.
# Markus Pietrek <mpietrek@fsforth.de>
#
# @TODO
# Linux-Kernel is expected to be at 0000'8000, entry 0000'8000
# optionally with a ramdisk at 0080'0000
#
# we load ourself to 0078'0000
#
# download area is 0060'0000
#


CONFIG_SYS_TEXT_BASE = 0x00780000
