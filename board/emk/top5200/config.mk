#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2003
# Reinhard Meyer, EMK Elektronik GmbH, r.meyer@emk-elektronik.de
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
# TOP5200 board, on optional MINI5200 and EVAL5200 boards
#
# allowed and functional TEXT_BASE values:
#
#   0xff000000		low boot at 0x00000100 (default board setting)
#   0xfff00000		high boot at 0xfff00100 (board needs modification)
#	0x00100000		RAM load and test
#

TEXT_BASE = 0xff000000
#TEXT_BASE = 0xfff00000
#TEXT_BASE = 0x00100000

PLATFORM_CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE) -I$(TOPDIR)/board
