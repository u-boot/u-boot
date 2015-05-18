#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= board.o

# Please copy ps7_init_gpl.c/h from hw project to this directory
obj-$(CONFIG_SPL_BUILD) += \
	$(if $(wildcard $(srctree)/$(src)/ps7_init_gpl.c), ps7_init_gpl.o)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes

# Warn if CONFIG_TARGET_ZYNQ_ZC70X is enabled
ifeq ($(CONFIG_TARGET_ZYNQ_ZC70X),y)
ifeq ($(CONFIG_SPL_BUILD),y)
$(warning CONFIG_TARGET_ZYNQ_ZC70X is deprecated.)
$(warning Enable CONFIG_TARGET_ZYNQ_ZC702 or CONFIG_TARGET_ZYNQ_706 instead.)
$(warning "make zynq_zc70x_defconfig" is also deprecated.)
$(warning Use "make zynq_zc702_defconfig" or "make zynq_zc706_defconfig".)
endif
endif
