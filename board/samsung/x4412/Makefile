# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2011 Samsung Electronics

ifdef CONFIG_SPL_BUILD
# necessary to create built-in.o
obj-y += exynos_smc.c

hostprogs-y := tools/mkx4412spl
always := $(hostprogs-y)

# omit -O2 option to suppress
#   warning: dereferencing type-punned pointer will break strict-aliasing rules
#
# TODO:
# Fix the root cause in tools/mkx4412spl.c and delete the following work-around
$(obj)/tools/mkx4412spl: HOSTCFLAGS:=$(filter-out -O2,$(HOSTCFLAGS))
else
obj-y	+= x4412.o
endif
