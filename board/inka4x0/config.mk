#
# (C) Copyright 2004
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
# INKA 4X0 board:
#
#	Valid values for TEXT_BASE are:
#
#	0xFFE00000   boot low
#
#	0x00100000   boot from RAM (for testing only)
#

ifndef TEXT_BASE
## Standard: boot low
TEXT_BASE = 0xFFE00000
## For testing: boot from RAM
#TEXT_BASE = 0x00100000
endif

PLATFORM_CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE) -I$(TOPDIR)/board
LDSCRIPT := $(SRCTREE)/arch/powerpc/cpu/mpc5xxx/u-boot-customlayout.lds
