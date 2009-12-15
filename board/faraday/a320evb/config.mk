#
# (C) Copyright 2009 Faraday Technology
# Po-Yu Chuang <ratbert@faraday-tech.com>
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

# Faraday A320 board with FA526/FA626TE/ARM926EJ-S cpus
#
# see http://www.faraday-tech.com/ for more information

# A320 has 1 bank of 64 MB DRAM
#
# 1000'0000 to 1400'0000
#
# Linux-Kernel is expected to be at 1000'8000, entry 1000'8000
#
# we load ourself to 13f8'0000
#
# download area is 1200'0000

TEXT_BASE = 0x13f80000
