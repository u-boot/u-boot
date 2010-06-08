#
# (C) Copyright 2008, Texas Instruments, Inc. http://www.ti.com/
#
# Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

# Texas Instruments DA8xx EVM board (ARM925EJS) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# DA8xx EVM has 1 bank of 64 MB SDRAM (2 16Meg x16 chips).
# Physical Address:
# C000'0000 to C400'0000
#
# Linux-Kernel is expected to be at C000'8000, entry C000'8000
# (mem base + reserved)
#
# we load ourself to C108 '0000


#Provide at least 16MB spacing between us and the Linux Kernel image
TEXT_BASE = 0xC1080000
