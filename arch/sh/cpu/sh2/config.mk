#
# (C) Copyright 2007-2008
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
ENDIANNESS += -EB

ifdef CONFIG_SH2A
PLATFORM_CPPFLAGS += -m2a -m2a-nofpu -mb -ffreestanding
else # SH2
PLATFORM_CPPFLAGS += -m3e -mb
endif
PLATFORM_CPPFLAGS += $(call cc-option,-mno-fdpic)

PLATFORM_RELFLAGS += -ffixed-r13
PLATFORM_LDFLAGS += $(ENDIANNESS)
