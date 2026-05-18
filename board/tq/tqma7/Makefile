# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.


obj-y += tqma7.o
obj-$(CONFIG_MBA7) += tqma7_mba7.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += spl_tqma7_ram.o
obj-$(CONFIG_MBA7) += spl_mba7.o
endif
