#
# Copyright (C) 2012 Marek Vasut <marex@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

all:	$(obj)docproc

$(obj)docproc:	docproc.c
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

clean:
	rm -rf docproc

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
