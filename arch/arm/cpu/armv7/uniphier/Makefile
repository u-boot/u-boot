#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_SPL_BUILD) += lowlevel_init.o init_page_table.o
obj-$(CONFIG_SPL_BUILD) += spl.o

obj-y += timer.o
obj-y += reset.o
obj-y += cache_uniphier.o
obj-y += dram_init.o
obj-$(CONFIG_DISPLAY_CPUINFO) += cpu_info.o
obj-$(CONFIG_BOARD_LATE_INIT) += board_late_init.o
obj-$(CONFIG_UNIPHIER_SMP) += smp.o
obj-$(if $(CONFIG_SPL_BUILD),,y) += cmd_pinmon.o

obj-y += board_common.o
obj-$(CONFIG_PFC_MICRO_SUPPORT_CARD) += support_card.o
obj-$(CONFIG_DCC_MICRO_SUPPORT_CARD) += support_card.o

obj-$(CONFIG_MACH_PH1_LD4) += ph1-ld4/
obj-$(CONFIG_MACH_PH1_PRO4) += ph1-pro4/
obj-$(CONFIG_MACH_PH1_SLD8) += ph1-sld8/
