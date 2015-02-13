#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_DEBUG_LL) += lowlevel_debug.o
obj-y += bcu_init.o sbc_init.o sg_init.o pll_init.o clkrst_init.o \
	pll_spectrum.o umc_init.o ddrphy_init.o
else
obj-$(CONFIG_BOARD_EARLY_INIT_F) += pinctrl.o
obj-$(if $(CONFIG_OF_CONTROL),,y) += platdevice.o
endif

obj-y += boot-mode.o
