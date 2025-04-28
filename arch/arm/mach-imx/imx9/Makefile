# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2022 NXP

obj-y += lowlevel_init.o

ifeq ($(CONFIG_SCMI_FIRMWARE),y)
obj-y += scmi/
else
obj-y += soc.o clock.o clock_root.o trdc.o
endif

ifneq ($(CONFIG_SPL_BUILD),y)
obj-y += imx_bootaux.o
endif