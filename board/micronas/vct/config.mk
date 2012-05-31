#
# (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
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
# vct_xxx boards with MIPS 4Kc CPU core
#

sinclude $(TOPDIR)/board/$(BOARDDIR)/config.tmp

ifndef CONFIG_SYS_TEXT_BASE
CONFIG_SYS_TEXT_BASE = 0x87000000
endif
