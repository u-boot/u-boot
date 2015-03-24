#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD

obj-y += lowlevel_init.o
obj-y += init_page_table.o
obj-y += spl.o
obj-y += memconf.o
obj-y += ddrphy_training.o

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

endif

obj-y += timer.o

obj-$(CONFIG_PFC_MICRO_SUPPORT_CARD) += support_card.o
obj-$(CONFIG_DCC_MICRO_SUPPORT_CARD) += support_card.o

obj-$(CONFIG_MACH_PH1_LD4) += ph1-ld4/
obj-$(CONFIG_MACH_PH1_PRO4) += ph1-pro4/
obj-$(CONFIG_MACH_PH1_SLD8) += ph1-sld8/
