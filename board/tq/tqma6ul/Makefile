# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2021-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel, Matthias Schiffer, Max Merchel
#

obj-y  := tqma6ul.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += spl_tqma6ul_ram.o
obj-$(CONFIG_MBA6UL) += spl_mba6ul.o
else
obj-$(CONFIG_MBA6UL) += tqma6ul_mba6ul.o
endif
