#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2000
# Murray Jensen <Murray.Jensen@csiro.au>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

BINS	= gdbsend gdbcont

COBJS	= gdbsend.o gdbcont.o error.o remote.o serial.o

OBJS	:= $(addprefix $(obj),$(COBJS))
SRCS	:= $(COBJS:.o=.c)
BINS	:= $(addprefix $(obj),$(BINS))

#
# Use native tools and options
#
CPPFLAGS   = -I$(BFD_ROOT_DIR)/include
CFLAGS     = $(HOST_CFLAGS) -O $(CPPFLAGS)
CC	   = $(HOSTCC)
MAKEDEPEND = makedepend

HOSTOS := $(shell uname -s | sed -e 's/\([Cc][Yy][Gg][Ww][Ii][Nn]\).*/cygwin/')

ifeq ($(HOSTOS),cygwin)

all:
.depend:

else	# ! CYGWIN

all:	$(obj).depend $(BINS)

$(obj)gdbsend:	$(obj)gdbsend.o $(obj)error.o $(obj)remote.o $(obj)serial.o
		$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^

$(obj)gdbcont:	$(obj)gdbcont.o $(obj)error.o $(obj)remote.o $(obj)serial.o
		$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS)

distclean:	clean
	rm -f $(BINS) $(obj)core $(obj)*.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

endif	# cygwin
