#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_DISPLAY_BOARDINFO) += board_info.o
obj-y += platdevice.o
obj-y += boot-mode.o
obj-$(CONFIG_BOARD_POSTCLK_INIT) += bcu_init.o sbc_init.o sg_init.o \
					pll_init.o clkrst_init.o pinctrl.o
obj-$(CONFIG_DRAM_INIT) += pll_spectrum.o umc_init.o
