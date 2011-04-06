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

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000

PLATFORM_CPPFLAGS += -fno-strict-aliasing
PLATFORM_CPPFLAGS += -Wstrict-prototypes
PLATFORM_CPPFLAGS += -mregparm=3
PLATFORM_CPPFLAGS += -fomit-frame-pointer
PLATFORM_CPPFLAGS += $(call cc-option, -ffreestanding)
PLATFORM_CPPFLAGS += $(call cc-option, -fno-toplevel-reorder,  $(call cc-option, -fno-unit-at-a-time))
PLATFORM_CPPFLAGS += $(call cc-option, -fno-stack-protector)
PLATFORM_CPPFLAGS += $(call cc-option, -mpreferred-stack-boundary=2)
PLATFORM_CPPFLAGS += -fno-dwarf2-cfi-asm
PLATFORM_CPPFLAGS += -DREALMODE_BASE=0x7c0

PLATFORM_RELFLAGS += -ffunction-sections -fvisibility=hidden

PLATFORM_LDFLAGS += --emit-relocs -Bsymbolic -Bsymbolic-functions

LDFLAGS_FINAL += --gc-sections -pie
