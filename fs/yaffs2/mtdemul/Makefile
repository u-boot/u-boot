#Makefile for NANDemul MTD
#
# NB this is not yet suitable for putting into the kernel tree.
# YAFFS: Yet another Flash File System. A NAND-flash specific file system. 
#
# Copyright (C) 2002 Aleph One Ltd.
#   for Toby Churchill Ltd and Brightstar Engineering
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Change or override  KERNELDIR to your kernel
## comment out USE_xxxx if you don't want these features.

KERNELDIR = /usr/src/kernel-headers-2.4.27

CFLAGS = -D__KERNEL__ -DMODULE   -I$(KERNELDIR)/include -O2 -Wall -g



TARGET = nandemul2k.o

default: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): %.o: %.c
	gcc -c $(CFLAGS) $< -o $@

