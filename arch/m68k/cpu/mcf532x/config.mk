#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
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

PLATFORM_RELFLAGS += -ffixed-d7 -msep-data

cfg=$(shell grep configs $(OBJTREE)/include/config.h | sed 's/.*<\(configs.*\)>/\1/')
is5301x:=$(shell grep CONFIG_MCF5301x $(TOPDIR)/include/$(cfg))
is532x:=$(shell grep CONFIG_MCF532x $(TOPDIR)/include/$(cfg))

ifneq ($(findstring 4.1,$(shell $(CC) --version)),4.1)

ifneq (,$(findstring CONFIG_MCF5301x,$(is5301x)))
PLATFORM_CPPFLAGS += -mcpu=53015 -fPIC
endif
ifneq (,$(findstring CONFIG_MCF532x,$(is532x)))
PLATFORM_CPPFLAGS += -mcpu=5329 -fPIC
endif

else
PLATFORM_CPPFLAGS += -m5307 -fPIC
endif
