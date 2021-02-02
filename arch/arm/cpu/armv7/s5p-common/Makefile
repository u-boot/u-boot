# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2009 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>

ifdef CONFIG_ARCH_NEXELL
obj-$(CONFIG_PWM_NX) += pwm.o
obj-$(CONFIG_S5P4418_ONEWIRE) += pwm.o
else
obj-y += cpu_info.o
ifndef CONFIG_SPL_BUILD
obj-y += timer.o
obj-y += sromc.o
obj-$(CONFIG_PWM) += pwm.o
endif
endif
