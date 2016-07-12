#
# (C) Copyright 2014 - 2016 Xilinx, Inc.
# Michal Simek <michal.simek@xilinx.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= zynqmp.o

hw-platform-y :=$(shell echo $(CONFIG_SYS_CONFIG_NAME))

init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/psu_init_gpl.c),\
	$(hw-platform-y)/psu_init_gpl.o)

ifeq ($(init-objs),)
ifneq ($(wildcard $(srctree)/$(src)/psu_init_gpl.c),)
init-objs := psu_init_gpl.o
$(if $(CONFIG_SPL_BUILD),\
$(warning Put custom psu_init_gpl.c/h to board/xilinx/zynqmp/custom_hw_platform/))
endif
endif

obj-$(CONFIG_SPL_BUILD) += $(init-objs)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_psu_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_psu_init_gpl.o := -I$(srctree)/$(src)
