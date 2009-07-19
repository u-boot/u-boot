#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Modified by, Yuli Barcohen, Arabella Software Ltd., yuli@arabellasw.com
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
# Motorola old MPC821/860ADS, MPC8xxFADS, new MPC866ADS, and
# MPC885ADS boards
#

TEXT_BASE = 0xFE000000
PLATFORM_CPPFLAGS += -I$(TOPDIR)/board/fads
HOSTCFLAGS += -I$(TOPDIR)/board/fads
HOST_ENVIRO_CFLAGS += -I$(TOPDIR)/board/fads
