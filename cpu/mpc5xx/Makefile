#
# (C) Copyright 2003
# Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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

#
# File:			cpu/mpc5xx/Makefile
# 
# Discription:		Makefile to build mpc5xx cpu configuration.
#			Will include top config.mk which itselfs
#			uses the definitions made in cpu/mpc5xx/config.mk 
#


include $(TOPDIR)/config.mk

LIB	= lib$(CPU).a

START	= start.S 
OBJS	= serial.o cpu.o cpu_init.o interrupts.o traps.o speed.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S)  $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
