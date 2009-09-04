#
# (C) Copyright 2000-2002
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

CROSS_COMPILE ?= sh4-linux-

STANDALONE_LOAD_ADDR = 0x8C000000
ifeq ($(CPU),sh2)
STANDALONE_LOAD_ADDR += -EB
endif

PLATFORM_CPPFLAGS += -DCONFIG_SH -D__SH__
PLATFORM_LDFLAGS += -e $(TEXT_BASE) --defsym reloc_dst=$(TEXT_BASE)

LDSCRIPT := $(SRCTREE)/cpu/$(CPU)/u-boot.lds
