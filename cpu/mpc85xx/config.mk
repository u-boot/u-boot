#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao, X.Xiao@motorola.com
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

PLATFORM_RELFLAGS += -fPIC -meabi

PLATFORM_CPPFLAGS += -ffixed-r2 -Wa,-me500 -msoft-float -mno-string

# -mspe=yes is needed to have -mno-spe accepted by a buggy GCC;
# see "[PATCH,rs6000] make -mno-spe work as expected" on
# http://gcc.gnu.org/ml/gcc-patches/2008-04/msg00311.html
PLATFORM_CPPFLAGS +=$(call cc-option,-mspe=yes)
PLATFORM_CPPFLAGS +=$(call cc-option,-mno-spe)

# Use default linker script.  Board port can override in board/*/config.mk
LDSCRIPT := $(SRCTREE)/cpu/mpc85xx/u-boot.lds
