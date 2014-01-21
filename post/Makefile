#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= post.o
obj-$(CONFIG_POST_STD_LIST)	+= tests.o

obj-y += drivers/
obj-$(CONFIG_PPC) += lib_powerpc/
obj-$(CONFIG_MPC83xx) += cpu/mpc83xx/
obj-$(CONFIG_8xx) += cpu/mpc8xx/
obj-$(CONFIG_4xx) += cpu/ppc4xx/
ifneq ($(filter lwmon lwmon5 netta pdm360ng,$(BOARD)),)
obj-y += board/$(BOARD)/
endif
