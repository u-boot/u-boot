#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2005
# Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

# TBS: add for debugging purposes
ccflags-y += -DBUILDUSER='"$(shell whoami)"'

obj-y	= metrobox.o ../common/flash.o ../common/sb_common.o
extra-y	+= init.o
