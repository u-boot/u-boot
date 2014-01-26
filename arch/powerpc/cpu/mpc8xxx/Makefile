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
obj-$(CONFIG_MPC85xx) += cpu.o
obj-$(CONFIG_MPC86xx) += cpu.o

obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_FSL_LBC) += fsl_lbc.o
obj-$(CONFIG_SYS_SRIO) += srio.o
obj-$(CONFIG_FSL_LAW) += law.o

endif
