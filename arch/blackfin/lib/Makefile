#
# U-boot Makefile
#
# Copyright (c) 2005-2008 Analog Devices Inc.
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Unnecessary.
# Use CONFIG_SYS_BOARD instead of BFIN_BOARD_NAME
# and delete this.
ccflags-y += -DBFIN_BOARD_NAME='"$(BOARD)"'

obj-y	+= ins.o
obj-y	+= memcmp.o
obj-y	+= memcpy.o
obj-y	+= memmove.o
obj-y	+= memset.o
obj-y	+= outs.o
obj-$(CONFIG_CMD_KGDB) += __kgdb.o
obj-y	+= board.o
obj-y	+= boot.o
obj-y	+= cache.o
obj-y	+= clocks.o
obj-$(CONFIG_CMD_CACHE_DUMP) += cmd_cache_dump.o
obj-$(CONFIG_CMD_KGDB) += kgdb.o
obj-y	+= muldi3.o
obj-$(CONFIG_HAS_POST) += post.o
obj-y	+= string.o
