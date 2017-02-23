#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y					+= boot-device.o

obj-$(CONFIG_ARCH_UNIPHIER_SLD3)	+= boot-device-sld3.o
obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= boot-device-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= boot-device-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= boot-device-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= boot-device-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= boot-device-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= boot-device-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= boot-device-ld11.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= boot-device-ld11.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_BOARD_LOAD_IMAGE)	+= spl_board.o
endif
