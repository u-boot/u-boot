# SPDX-License-Identifier: GPL-2.0-or-later
#
# (C) Copyright 2007 Semihalf

obj-y += api.o api_display.o api_net.o api_storage.o

ifeq (CONFIG_PPC,y)
obj-$(CONFIG_PPC) += api_platform-powerpc.o
else
obj-y += api_platform.o
endif
