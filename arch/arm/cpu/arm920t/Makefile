#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o

obj-y	+= cpu.o
obj-$(CONFIG_USE_IRQ)	+= interrupts.o

obj-$(if $(filter a320,$(SOC)),y) += a320/
obj-$(CONFIG_AT91FAMILY) += at91/
obj-$(CONFIG_EP93XX) += ep93xx/
obj-$(CONFIG_IMX) += imx/
obj-$(CONFIG_KS8695) += ks8695/
obj-$(CONFIG_S3C24X0) += s3c24x0/
