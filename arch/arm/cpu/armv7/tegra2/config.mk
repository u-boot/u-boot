#
# (C) Copyright 2010,2011
# NVIDIA Corporation <www.nvidia.com>
#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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

# Tegra has an ARMv4T CPU which runs board_init_f(), so we must build these
# files with compatible flags
ifdef CONFIG_TEGRA2
CFLAGS_arch/arm/lib/board.o += -march=armv4t
CFLAGS_arch/arm/lib/memset.o += -march=armv4t
CFLAGS_lib/string.o += -march=armv4t
CFLAGS_common/cmd_nvedit.o += -march=armv4t
endif

USE_PRIVATE_LIBGCC = yes

CONFIG_ARCH_DEVICE_TREE := tegra20
