# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 Siemens AG
#

obj-y += board.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += ../common/factoryset.o
endif
