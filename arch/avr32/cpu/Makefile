#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2005-2006 Atmel Corporation.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y			+= start.o

obj-y			+= cpu.o
obj-$(CONFIG_SYS_HSDRAMC) += hsdramc.o
obj-y			+= exception.o
obj-y			+= cache.o
obj-y			+= interrupts.o
obj-$(CONFIG_PORTMUX_PIO) += portmux-pio.o
obj-$(CONFIG_PORTMUX_GPIO) += portmux-gpio.o

obj-$(if $(filter at32ap700x,$(SOC)),y) += at32ap700x/
