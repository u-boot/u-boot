#
# Copyright 2008 Freescale Semiconductor.
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
# mpc8536ds board
#
ifndef NAND_SPL
ifeq ($(CONFIG_MK_NAND), y)
TEXT_BASE = $(CONFIG_RAMBOOT_TEXT_BASE)
LDSCRIPT := $(TOPDIR)/cpu/$(CPU)/u-boot-nand.lds
endif
endif

ifeq ($(CONFIG_MK_SDCARD), y)
TEXT_BASE = $(CONFIG_RAMBOOT_TEXT_BASE)
RESET_VECTOR_ADDRESS = 0xf8fffffc
endif

ifeq ($(CONFIG_MK_SPIFLASH), y)
TEXT_BASE = $(CONFIG_RAMBOOT_TEXT_BASE)
RESET_VECTOR_ADDRESS = 0xf8fffffc
endif

ifndef TEXT_BASE
TEXT_BASE = 0xeff80000
endif

ifndef RESET_VECTOR_ADDRESS
RESET_VECTOR_ADDRESS = 0xeffffffc
endif
