#
# (C) Copyright 2007-2008 Michal Simek
# Michal SIMEK <monstr@monstr.eu>
#
# (C) Copyright 2004 Atmark Techno, Inc.
# Yasushi SHOJI <yashi@atmark-techno.com>
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

CROSS_COMPILE ?= mb-

STANDALONE_LOAD_ADDR = 0x80F00000

PLATFORM_CPPFLAGS += -ffixed-r31 -D__microblaze__
