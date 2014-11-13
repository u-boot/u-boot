#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
obj-y	= cpu.o cache.o

ifdef	CONFIG_SPL_BUILD
ifdef	CONFIG_SPL_NO_CPU_SUPPORT_CODE
extra-y	:=
endif
endif

obj-$(CONFIG_ARMADA100) += armada100/
obj-$(CONFIG_AT91FAMILY) += at91/
obj-$(CONFIG_ARCH_DAVINCI) += davinci/
obj-$(CONFIG_KIRKWOOD) += kirkwood/
obj-$(if $(filter lpc32xx,$(SOC)),y) += lpc32xx/
obj-$(CONFIG_MB86R0x) += mb86r0x/
obj-$(CONFIG_MX25) += mx25/
obj-$(CONFIG_MX27) += mx27/
obj-$(if $(filter mxs,$(SOC)),y) += mxs/
obj-$(CONFIG_ARCH_NOMADIK) += nomadik/
obj-$(CONFIG_ORION5X) += orion5x/
obj-$(CONFIG_PANTHEON) += pantheon/
obj-$(if $(filter spear,$(SOC)),y) += spear/
obj-$(CONFIG_ARCH_VERSATILE) += versatile/
