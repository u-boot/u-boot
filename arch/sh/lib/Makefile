#
# Copyright (c) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#


obj-y	+= board.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
ifeq ($(CONFIG_CPU_SH2),y)
obj-y	+= time_sh2.o
else
obj-y	+= time.o
endif
obj-$(CONFIG_CMD_SH_ZIMAGEBOOT) += zimageboot.o

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += ashiftrt.o ashiftlt.o lshiftrt.o \
				    ashldi3.o ashrsi3.o lshrdi3.o movmem.o
