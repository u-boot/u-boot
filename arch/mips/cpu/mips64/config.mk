#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
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
# Default optimization level for MIPS64
#
# Note: Toolchains with binutils prior to v2.16
# are no longer supported by U-Boot MIPS tree!
#
MIPSFLAGS = -march=mips64

PLATFORM_CPPFLAGS += $(MIPSFLAGS)
PLATFORM_CPPFLAGS += -mabi=64 -DCONFIG_64BIT
ifdef CONFIG_SYS_BIG_ENDIAN
PLATFORM_LDFLAGS  += -m elf64btsmip
else
PLATFORM_LDFLAGS  += -m elf64ltsmip
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0xffffffff80200000 -T mips64.lds
