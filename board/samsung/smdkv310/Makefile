# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2011 Samsung Electronics

ifdef CONFIG_XPL_BUILD
# necessary to create built-in.a
obj- := __dummy__.a

hostprogs-always-y += tools/mksmdkv310spl
else
obj-y	+= smdkv310.o
endif
