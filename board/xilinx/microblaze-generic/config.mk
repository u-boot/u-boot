#
# (C) Copyright 2007 Michal Simek
#
# Michal  SIMEK <monstr@monstr.eu>
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
# CAUTION: This file is a faked configuration !!!
#          There is no real target for the microblaze-generic
#          configuration. You have to replace this file with
#          the generated file from your Xilinx design flow.
#

CONFIG_SYS_TEXT_BASE = 0x29000000

PLATFORM_CPPFLAGS += -mno-xl-soft-mul
PLATFORM_CPPFLAGS += -mno-xl-soft-div
PLATFORM_CPPFLAGS += -mxl-barrel-shift
