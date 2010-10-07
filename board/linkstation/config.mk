#
# (C) Copyright 2001-2003
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

# LinkStation/LinkStation-HG:
#
#       Valid values for CONFIG_SYS_TEXT_BASE are:
#
#	Standard configuration - all models
#       0xFFF00000   boot from flash
#
#	Test configuration (boot from RAM using uloader.o)
#	LinkStation HD-HLAN and KuroBox Standard
#       0x03F00000   boot from RAM
#	LinkStation HD-HGLAN and KuroBox HG
#       0x07F00000   boot from RAM
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef CONFIG_SYS_TEXT_BASE
# For flash image - all models
CONFIG_SYS_TEXT_BASE = 0xFFF00000
# For RAM image
# HLAN and LAN
#CONFIG_SYS_TEXT_BASE = 0x03F00000
# HGLAN and HGTL
#CONFIG_SYS_TEXT_BASE = 0x07F00000
endif

PLATFORM_CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE)
