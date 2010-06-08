#
# (C) Copyright 2006-2009
# Texas Instruments Incorporated, <www.ti.com>
#
# OMAP 4430 SDP
# see http://www.ti.com/ for more information on Texas Instruments
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#
# SDRAM Address Space:
# 8000'0000 - 9fff'ffff (512 MB)
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)

# Let's place u-boot 1MB before the end of SDRAM.
TEXT_BASE = 0x9ff00000
