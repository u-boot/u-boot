#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= board.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o

# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
lib-y	+= ashldi3.o
lib-y	+= ashrdi3.o
lib-y	+= lshrdi3.o
endif
