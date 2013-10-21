#
# Copyright 2013 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL

obj-y	+= spl_minimal.o tlb.o law.o

else

obj-y	+= bsc9132qds.o
obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o

endif
