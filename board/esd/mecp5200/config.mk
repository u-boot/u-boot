#
# (C) Copyright 2003
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
# IceCube board:
#
#	Valid values for TEXT_BASE are:
#
#	0xFFF00000   boot high (standard configuration)
#	0xFF000000   boot low for 16 MiB boards
#	0xFF800000   boot low for  8 MiB boards
#	0x00100000   boot from RAM (for testing only)
#

sinclude $(TOPDIR)/board/$(BOARDDIR)/config.tmp

ifndef TEXT_BASE
## Standard: boot high
TEXT_BASE = 0xFFF00000
## For testing: boot from RAM
# TEXT_BASE = 0x00100000
endif

PLATFORM_CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE) -I$(TOPDIR)/board
