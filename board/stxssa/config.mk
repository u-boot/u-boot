# Modified by Xianghua Xiao, X.Xiao@motorola.com
# (C) Copyright 2002,2003 Motorola Inc.
#
# Copied from ADS85xx for STx GP3 - Dan Malek
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
# default CCARBAR is at 0xff700000
# assume U-Boot is less than 0.5MB
# U-Boot is less than 256K, so push
# it further up into the flash
#
TEXT_BASE = 0xfffC0000

PLATFORM_CPPFLAGS += -DCONFIG_MPC85xx=1
PLATFORM_CPPFLAGS += -DCONFIG_E500=1
