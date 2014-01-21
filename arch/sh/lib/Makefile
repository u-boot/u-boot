#
# Copyright (c) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#


obj-y	+= board.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
ifeq ($(CONFIG_SH2),y)
obj-y	+= time_sh2.o
else
obj-y	+= time.o
endif
obj-$(CONFIG_CMD_SH_ZIMAGEBOOT) += zimageboot.o



# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
lib-y	+= ashiftrt.o
lib-y	+= ashiftlt.o
lib-y	+= lshiftrt.o
lib-y	+= ashldi3.o
lib-y	+= ashrsi3.o
lib-y	+= lshrdi3.o
lib-y	+= movmem.o
endif
