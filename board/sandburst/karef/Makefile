#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2005
# Sandburst Corporation
# Travis B. Sawyer
#
# SPDX-License-Identifier:	GPL-2.0+
#

# TBS: add for debugging purposes
BUILDUSER := $(shell whoami)
FORCEBUILD := $(shell rm -f karef.o)

CFLAGS += -DBUILDUSER='"$(BUILDUSER)"'
# TBS: end debugging

obj-y	= karef.o ../common/flash.o ../common/sb_common.o
extra-y	+= init.o
