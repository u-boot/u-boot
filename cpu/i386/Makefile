#
# (C) Copyright 2002
# Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

LIB	= lib$(CPU).a

START	= start.o start16.o reset.o
COBJS	= serial.o interrupts.o cpu.o timer.o sc520.o
AOBJS	= sc520_asm.o

all:	.depend $(START) $(LIB)

$(LIB):	$(COBJS) $(AOBJS)
	$(AR) crv $@ $(COBJS) $(AOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(COBJS:.o=.c) $(AOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(COBJS:.o=.c) $(AOBJS:.o=.S) > $@

sinclude .depend

#########################################################################
