#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD

obj-y += init/ bcu/ memconf/ pll/ early-clk/ early-pinctrl/
obj-$(CONFIG_MICRO_SUPPORT_CARD) += sbc/

else

obj-$(CONFIG_BOARD_EARLY_INIT_F) += board_early_init_f.o
obj-$(CONFIG_DISPLAY_CPUINFO) += cpu_info.o
obj-$(CONFIG_MISC_INIT_F) += print_misc_info.o
obj-y += dram_init.o
obj-y += board_common.o
obj-$(CONFIG_BOARD_EARLY_INIT_R) += board_early_init_r.o
obj-$(CONFIG_BOARD_LATE_INIT) += board_late_init.o
obj-y += reset.o

obj-y += pinctrl/ clk/

endif

obj-y += boards.o
obj-y += soc_info.o
obj-y += boot-mode/
obj-y += dram/

obj-$(CONFIG_MICRO_SUPPORT_CARD) += micro-support-card.o
obj-$(CONFIG_DEBUG_UART_UNIPHIER) += debug-uart/

obj-$(CONFIG_CPU_V7) += arm32/
obj-$(CONFIG_ARM64) += arm64/
