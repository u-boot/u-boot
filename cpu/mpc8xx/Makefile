#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

# CFLAGS += -DET_DEBUG

LIB	= lib$(CPU).a

START	= start.o kgdb.o
OBJS	= bedbug_860.o commproc.o cpu.o cpu_init.o	\
	  fec.o i2c.o interrupts.o lcd.o scc.o		\
	  serial.o speed.o spi.o \
	  traps.o upatch.o video.o
SOBJS	= plprcr_write.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS) kgdb.o

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c) $(SOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) $(SOBJS:.o=.S) > $@

sinclude .depend

#########################################################################
