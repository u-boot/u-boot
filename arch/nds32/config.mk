#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
# Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

CROSS_COMPILE ?= nds32le-linux-

CONFIG_STANDALONE_LOAD_ADDR = 0x300000 -T nds32.lds

PLATFORM_RELFLAGS	+= -fno-strict-aliasing -fno-common -mrelax
PLATFORM_RELFLAGS	+= -gdwarf-2
PLATFORM_CPPFLAGS	+= -DCONFIG_NDS32 -D__nds32__ -G0 -ffixed-10 -fpie

LDFLAGS_u-boot		= --gc-sections --relax
