#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# HYMOD boards
#

PLATFORM_CPPFLAGS += -I$(srctree)

OBJCOPYFLAGS = --remove-section=.ppcenv
