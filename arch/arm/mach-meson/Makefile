# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Beniamino Galvani <b.galvani@gmail.com>

obj-y += board-common.o sm.o board-info.o capsule.o
obj-$(CONFIG_MESON_GX) += board-gx.o
ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_MESON_GXBB) += dram-gxbb.o
obj-$(CONFIG_MESON_GXL) += dram-gxl.o
obj-$(CONFIG_MESON_GX) += dram-gx.o
obj-$(CONFIG_MESON_GXBB) += spl-gxbb.o
obj-$(CONFIG_MESON_GXL) += spl-gxl.o
obj-$(CONFIG_MESON_GX) += spl-gx.o
obj-$(CONFIG_MESON_GX) += spl.o
endif

obj-$(CONFIG_MESON_AXG) += board-axg.o
obj-$(CONFIG_MESON_G12A) += board-g12a.o
obj-$(CONFIG_MESON_A1) += board-a1.o
