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
is5208:=$(shell grep CONFIG_M5208 $(TOPDIR)/include/$(cfg))
is5249:=$(shell grep CONFIG_M5249 $(TOPDIR)/include/$(cfg))
is5253:=$(shell grep CONFIG_M5253 $(TOPDIR)/include/$(cfg))
is5271:=$(shell grep CONFIG_M5271 $(TOPDIR)/include/$(cfg))
is5272:=$(shell grep CONFIG_M5272 $(TOPDIR)/include/$(cfg))
is5275:=$(shell grep CONFIG_M5275 $(TOPDIR)/include/$(cfg))
is5282:=$(shell grep CONFIG_M5282 $(TOPDIR)/include/$(cfg))

ifneq (,$(findstring CONFIG_M5208,$(is5208)))
PLATFORM_CPPFLAGS += -mcpu=5208
endif
ifneq (,$(findstring CONFIG_M5249,$(is5249)))
PLATFORM_CPPFLAGS += -mcpu=5249
endif
ifneq (,$(findstring CONFIG_M5253,$(is5253)))
PLATFORM_CPPFLAGS += -mcpu=5253
endif
ifneq (,$(findstring CONFIG_M5271,$(is5271)))
PLATFORM_CPPFLAGS += -mcpu=5271
endif
ifneq (,$(findstring CONFIG_M5272,$(is5272)))
PLATFORM_CPPFLAGS += -mcpu=5272
endif
ifneq (,$(findstring CONFIG_M5275,$(is5275)))
PLATFORM_CPPFLAGS += -mcpu=5275
endif
ifneq (,$(findstring CONFIG_M5282,$(is5282)))
PLATFORM_CPPFLAGS += -mcpu=5282
endif
