#
# Copyright 2025-2026 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

# Add include path for NXP device tree header files from Linux.
ccflags-y += -I$(srctree)/dts/upstream/src/arm64/freescale/

obj-y += imx952_evk.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
endif
