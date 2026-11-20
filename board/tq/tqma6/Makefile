# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2014 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
#

obj-y  := tqma6.o
obj-y  += tqma6_emmc.o

ifdef CONFIG_SPL_BUILD
obj-y += spl_tqma6.o
obj-$(CONFIG_TQMA6DL) += spl_tqma6dl_ram.o
obj-$(CONFIG_TQMA6Q) += spl_tqma6q_ram.o
obj-$(CONFIG_TQMA6QP) += spl_tqma6qp_ram.o
obj-$(CONFIG_TQMA6S) += spl_tqma6s_ram.o
obj-$(CONFIG_MBA6) += spl_mba6.o
endif

obj-$(CONFIG_MBA6) += tqma6_mba6.o
obj-$(CONFIG_WRU4) += tqma6_wru4.o
