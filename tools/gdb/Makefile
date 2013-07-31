#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2000
# Murray Jensen <Murray.Jensen@csiro.au>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

BINS	= gdbsend gdbcont

COBJS	= gdbsend.o gdbcont.o error.o remote.o serial.o

HOSTOBJS := $(addprefix $(obj),$(COBJS))
HOSTSRCS := $(COBJS:.o=.c)
BINS	:= $(addprefix $(obj),$(BINS))

#
# Use native tools and options
#
HOSTCPPFLAGS = -I$(BFD_ROOT_DIR)/include

HOSTOS := $(shell uname -s | sed -e 's/\([Cc][Yy][Gg][Ww][Ii][Nn]\).*/cygwin/')

ifeq ($(HOSTOS),cygwin)

all:
$(obj).depend:

else	# ! CYGWIN

all:	$(obj).depend $(BINS)

$(obj)gdbsend:	$(obj)gdbsend.o $(obj)error.o $(obj)remote.o $(obj)serial.o
		$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^

$(obj)gdbcont:	$(obj)gdbcont.o $(obj)error.o $(obj)remote.o $(obj)serial.o
		$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^

clean:
	rm -f $(HOSTOBJS)

distclean:	clean
	rm -f $(BINS) $(obj)core $(obj)*.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

endif	# cygwin
