#
# Copyright 2009-2010 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL

obj-$(CONFIG_FSL_LAW) += law.o

else

ifneq ($(CPU),mpc83xx)
obj-y	+= cpu.o
endif

obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_FSL_IFC) += fsl_ifc.o
obj-$(CONFIG_FSL_LBC) += fsl_lbc.o
obj-$(CONFIG_SYS_SRIO) += srio.o
obj-$(CONFIG_FSL_LAW) += law.o

endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/
else
obj-y += ddr/
endif
