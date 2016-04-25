#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y += timer.o
else
obj-y += mem_map.o smp.o smp_kick_cpus.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20) += arm-cci500.o
endif
