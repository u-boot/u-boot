#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_DEBUG_LL) += lowlevel_debug.o
obj-y += sg_init.o pll_init.o early_clkrst_init.o \
	pll_spectrum.o umc_init.o ddrphy_init.o
obj-$(CONFIG_PFC_MICRO_SUPPORT_CARD) += sbc_init.o
obj-$(CONFIG_DCC_MICRO_SUPPORT_CARD) += sbc_init_3cs.o
else
obj-$(CONFIG_BOARD_EARLY_INIT_F) += pinctrl.o clkrst_init.o
obj-$(if $(CONFIG_OF_CONTROL),,y) += platdevice.o
endif

obj-y += boot-mode.o
