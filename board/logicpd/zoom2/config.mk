#
# (C) Copyright 2009
# Texas Instruments, <www.ti.com>
#
# Zoom II uses OMAP3 (ARM-CortexA8) CPU
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
# Physical Address:
# 0x80000000 (bank0)
# 0xA0000000 (bank1)
# Linux-Kernel is expected to be at 0x80008000, entry 0x80008000
# (mem base + reserved)

# For use with external or internal boots.
CONFIG_SYS_TEXT_BASE = 0x80008000
