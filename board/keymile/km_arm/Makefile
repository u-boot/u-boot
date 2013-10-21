#
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Prafulla Wadaskar <prafulla@marvell.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

obj-y	:= km_arm.o ../common/common.o ../common/ivm.o

ifdef CONFIG_KM_FPGA_CONFIG
obj-y	+= fpga_config.o
endif
