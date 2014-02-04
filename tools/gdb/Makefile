#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2000
# Murray Jensen <Murray.Jensen@csiro.au>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq ($(HOSTOS),cygwin)

#
# Use native tools and options
#
HOST_EXTRACFLAGS := -I$(BFD_ROOT_DIR)/include -pedantic

hostprogs-y := gdbsend gdbcont

gdbsend-objs := gdbsend.o error.o remote.o serial.o
gdbcont-objs := gdbcont.o error.o remote.o serial.o

always := $(hostprogs-y)

endif	# cygwin
