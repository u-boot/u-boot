#
# (C) Copyright 2011
# Julius Baxter <julius@opencores.org>
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

CROSS_COMPILE ?= or32-elf-

# r10 used for global object pointer, already set in OR32 GCC but just to be
# clear
PLATFORM_CPPFLAGS += -DCONFIG_OPENRISC -D__OR1K__ -ffixed-r10

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000
