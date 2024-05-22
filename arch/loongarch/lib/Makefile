# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2024 Jiaxun yang <jiaxun.yang@flygoat.com>
#

obj-$(CONFIG_CMD_GO) += boot.o
obj-y	+= cache.o
obj-y	+= interrupts.o
ifeq ($(CONFIG_$(SPL_)SYSRESET),)
obj-y	+= reset.o
endif
obj-y	+= setjmp.o
