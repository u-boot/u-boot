#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= board.o

# Copied from Xilinx SDK 2014.4
hw-platform-$(CONFIG_TARGET_ZYNQ_ZED)		:= zed_hw_platform
hw-platform-$(CONFIG_TARGET_ZYNQ_MICROZED)	:= MicroZed_hw_platform
hw-platform-$(CONFIG_TARGET_ZYNQ_ZC702)		:= ZC702_hw_platform
hw-platform-$(CONFIG_TARGET_ZYNQ_ZC706)		:= ZC706_hw_platform
hw-platform-$(CONFIG_TARGET_ZYNQ_ZYBO)		:= zybo_hw_platform
# If you want to use customized ps7_init_gpl.c/h,
# enable CONFIG_ZYNQ_CUSTOM_INIT and put them into custom_hw_platform/.
# This line must be placed at the bottom of the list because
# it takes precedence over the default ones.
hw-platform-$(CONFIG_ZYNQ_CUSTOM_INIT)		:= custom_hw_platform

init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/ps7_init_gpl.c),\
	$(hw-platform-y)/ps7_init_gpl.o)

ifeq ($(init-objs),)
ifneq ($(wildcard $(srctree)/$(src)/ps7_init_gpl.c),)
init-objs := ps7_init_gpl.o
$(if $(CONFIG_SPL_BUILD),\
$(warning Put custom ps7_init_gpl.c/h to board/xilinx/zynq/custom_hw_platform/))
endif
endif

obj-$(CONFIG_SPL_BUILD) += $(init-objs)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_ps7_init_gpl.o := -I$(srctree)/$(src)
