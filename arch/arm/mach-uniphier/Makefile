#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD

obj-y += lowlevel_init.o
obj-y += init_page_table.o
obj-y += boards.o

obj-y += init/ bcu/ memconf/ pll/ early-clk/ early-pinctrl/ umc/ ddrphy/
obj-$(CONFIG_MICRO_SUPPORT_CARD) += sbc/

obj-$(CONFIG_DEBUG_LL) += debug_ll.o

else

obj-y += late_lowlevel_init.o
obj-$(CONFIG_BOARD_EARLY_INIT_F) += board_early_init_f.o
obj-$(CONFIG_DISPLAY_CPUINFO) += cpu_info.o
obj-$(CONFIG_MISC_INIT_F) += print_misc_info.o
obj-y += dram_init.o
obj-y += board_common.o
obj-$(CONFIG_BOARD_EARLY_INIT_R) += board_early_init_r.o
obj-$(CONFIG_BOARD_LATE_INIT) += board_late_init.o
obj-y += reset.o
obj-y += cache_uniphier.o
obj-$(CONFIG_CMD_PINMON) += cmd_pinmon.o
obj-$(CONFIG_CMD_DDRPHY_DUMP) += cmd_ddrphy.o

obj-y += pinctrl/ clk/

endif

obj-y += timer.o
obj-y += soc_info.o
obj-y += boot-mode/

obj-$(CONFIG_MICRO_SUPPORT_CARD) += micro-support-card.o
