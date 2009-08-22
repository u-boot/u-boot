#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

CROSS_COMPILE ?= bfin-uclinux-

CONFIG_BFIN_CPU := $(strip $(subst ",,$(CONFIG_BFIN_CPU)))
CONFIG_BFIN_BOOT_MODE := $(strip $(subst ",,$(CONFIG_BFIN_BOOT_MODE)))
CONFIG_ENV_OFFSET := $(strip $(subst ",,$(CONFIG_ENV_OFFSET)))
CONFIG_ENV_SIZE := $(strip $(subst ",,$(CONFIG_ENV_SIZE)))

PLATFORM_RELFLAGS += -ffixed-P5 -fomit-frame-pointer -mno-fdpic
PLATFORM_CPPFLAGS += -DCONFIG_BLACKFIN

LDFLAGS += --gc-sections
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections

ifneq (,$(CONFIG_BFIN_CPU))
PLATFORM_RELFLAGS += -mcpu=$(CONFIG_BFIN_CPU)
endif

ifneq ($(CONFIG_BFIN_BOOT_MODE),BFIN_BOOT_BYPASS)
ALL += $(obj)u-boot.ldr
endif

SYM_PREFIX = _

LDR_FLAGS-y :=
LDR_FLAGS-$(CONFIG_BFIN_BOOTROM_USES_EVT1) += -J

LDR_FLAGS += --bmode $(subst BFIN_BOOT_,,$(CONFIG_BFIN_BOOT_MODE))
LDR_FLAGS += --use-vmas
LDR_FLAGS += --initcode $(obj)cpu/$(CPU)/initcode.o
ifneq ($(CONFIG_BFIN_BOOT_MODE),BFIN_BOOT_UART)
ifneq ($(ENV_IS_EMBEDDED_CUSTOM),ENV_IS_EMBEDDED_CUSTOM)
LDR_FLAGS += --punchit $$(($(CONFIG_ENV_OFFSET))):$$(($(CONFIG_ENV_SIZE))):$(obj)env-ldr.o
endif
endif
ifneq (,$(findstring s,$(MAKEFLAGS)))
LDR_FLAGS += --quiet
endif

LDR_FLAGS += $(LDR_FLAGS-y)

ifeq ($(wildcard $(TOPDIR)/board/$(BOARD)/u-boot.lds*),)
LDSCRIPT = $(obj)lib_$(ARCH)/u-boot.lds.S
endif
