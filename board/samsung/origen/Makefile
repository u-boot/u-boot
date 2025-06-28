# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2011 Samsung Electronics

ifdef CONFIG_XPL_BUILD
# necessary to create built-in.a
obj- := __dummy__.a

hostprogs-y := tools/mkorigenspl
always := $(hostprogs-y)

# omit -O2 option to suppress
#   warning: dereferencing type-punned pointer will break strict-aliasing rules
#
# TODO:
# Fix the root cause in tools/mkorigenspl.c and delete the following work-around
$(obj)/tools/mkorigenspl: HOSTCFLAGS:=$(filter-out -O2,$(HOSTCFLAGS))
else
obj-y	+= origen.o
endif
