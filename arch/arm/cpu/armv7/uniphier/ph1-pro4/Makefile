#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_DISPLAY_BOARDINFO) += board_info.o
obj-$(if $(CONFIG_OF_CONTROL),,y) += platdevice.o
obj-y += boot-mode.o
obj-$(CONFIG_DEBUG_LL) += lowlevel_debug.o
obj-$(CONFIG_SOC_INIT) += sbc_init.o sg_init.o pll_init.o clkrst_init.o
obj-$(CONFIG_BOARD_POSTCLK_INIT) += pinctrl.o
obj-$(CONFIG_DRAM_INIT) += pll_spectrum.o umc_init.o ddrphy_init.o
