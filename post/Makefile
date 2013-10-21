#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_HAS_POST)	+= post.o
obj-$(CONFIG_POST_STD_LIST)	+= tests.o

obj-$(CONFIG_HAS_POST) += drivers/
ifeq ($(ARCH),powerpc)
obj-$(CONFIG_HAS_POST) += lib_powerpc/
endif
ifneq ($(filter mpc83xx mpc8xx ppc4xx,$(CPU)),)
obj-$(CONFIG_HAS_POST) += cpu/$(CPU)/
endif
ifneq ($(filter lwmon lwmon5 netta pdm360ng,$(BOARD)),)
obj-$(CONFIG_HAS_POST) += board/$(BOARD)/
endif
