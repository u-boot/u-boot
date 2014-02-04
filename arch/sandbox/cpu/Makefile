#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= cpu.o os.o start.o state.o

# os.c is build in the system environment, so needs standard includes
$(obj)/os.o: CFLAGS := $(filter-out -nostdinc,\
	$(patsubst -I%,-idirafter%,$(CFLAGS)))
$(obj)/.depend.os: CPPFLAGS := $(filter-out -nostdinc,\
	$(patsubst -I%,-idirafter%,$(CPPFLAGS)))
