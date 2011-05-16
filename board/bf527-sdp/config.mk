#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2001
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

CFLAGS_lib += -O2
CFLAGS_lib/lzma += -O2
CFLAGS_lib/zlib += -O2

# Set some default LDR flags based on boot mode.
LDR_FLAGS-BFIN_BOOT_PARA := --bits 16 --dma 6
