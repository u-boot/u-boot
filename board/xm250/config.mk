#
# (C) Copyright 2003-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#
# MicroSys XM250 board:
#


# This is the address where U-Boot lives in flash:
#TEXT_BASE = 0

# FIXME: armboot does only work correctly when being compiled
# for the addresses _after_ relocation to RAM!! Otherwhise the
# .bss segment is assumed in flash...
TEXT_BASE = 0xA3F80000
