#
# Copyright 2011-2012 Freescale Semiconductor, Inc.
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

obj-y        += bsc9131rdb.o
obj-y        += ddr.o
obj-y        += law.o
obj-y        += tlb.o
#obj-y		+= bsc9131rdb_mux.o

endif
