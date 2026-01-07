# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

## Build a couple of necessary functions into a private libgcc
## if the user asked for it
lib-$(CONFIG_USE_PRIVATE_LIBGCC) += lshrdi3.o muldi3.o ashldi3.o ashrdi3.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-$(CONFIG_ARCH_COLDFIRE) += cache.o interrupts.o time.o traps.o bdinfo.o fec.o
