#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# Coldfire contribution by Bernhard Kuhn <bkuhn@metrowerks.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

PLATFORM_CPPFLAGS += -DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE)
