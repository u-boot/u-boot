#
# (C) Copyright 2003
# Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
# File:			config.mk
#
# Discription:		compiler flags and make definitions
#


PLATFORM_RELFLAGS +=	-fPIC -ffixed-r14 -meabi

PLATFORM_CPPFLAGS +=	-DCONFIG_5xx -ffixed-r2 -mpowerpc -msoft-float

# Use default linker script.  Board port can override in board/*/config.mk
LDSCRIPT := $(SRCTREE)/cpu/mpc5xx/u-boot.lds
