#
# Copyright (C) 2010 Samsung Electronics
# Kyungmin Park <kyungmin.park@samsung.com>
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

# On S5PC100 we use the 128 MiB OneDRAM bank at
#
# 0x30000000 to 0x35000000 (80MiB)
# 0x38000000 to 0x40000000 (128MiB)
#
# On S5PC110 we use the 128 MiB OneDRAM bank at
#
# 0x30000000 to 0x35000000 (80MiB)
# 0x40000000 to 0x50000000 (256MiB)
#
TEXT_BASE = 0x34800000
