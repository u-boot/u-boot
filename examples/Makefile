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

LOAD_ADDR = 0x40000

include $(TOPDIR)/config.mk

SREC	= hello_world.srec
BIN	= hello_world.bin

ifeq ($(ARCH),i386)
SREC   += 82559_eeprom.srec
BIN    += 82559_eeprom.bin
endif

ifeq ($(ARCH),ppc)
SREC   += sched.srec
BIN    += sched.bin
endif

ifeq ($(ARCH),mips)
SREC =
BIN =
endif

# The following example is pretty 8xx specific...
ifeq ($(CPU),mpc8xx)
SREC   += timer.srec
BIN    += timer.bin
endif

# The following example is 8260 specific...
ifeq ($(CPU),mpc8260)
SREC   += mem_to_mem_idma2intr.srec
BIN    += mem_to_mem_idma2intr.bin
endif

# Utility for resetting i82559 EEPROM
ifeq ($(BOARD),oxc)
SREC   += eepro100_eeprom.srec
endif

OBJS	= $(SREC:.srec=.o)

LIB	= libsyscall.a
LIBAOBJS= syscall.o
ifeq ($(ARCH),ppc)
LIBAOBJS+= $(ARCH)_longjmp.o $(ARCH)_setjmp.o
endif
LIBCOBJS=
LIBOBJS	= $(LIBAOBJS) $(LIBCOBJS)

CPPFLAGS += -I..

all:	.depend $(LIB) $(SREC) $(BIN)

#########################################################################
$(LIB): .depend $(LIBOBJS)
	$(AR) crv $@ $(LIBOBJS)

%.srec:	%.o $(LIB)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(<:.o=) -e $(<:.o=) $< $(LIB)
	$(OBJCOPY) -O srec $(<:.o=) $@

%.bin:	%.srec
	$(OBJCOPY) -O binary $< $@ 2>/dev/null

#########################################################################

.depend:	Makefile $(OBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S) > $@

sinclude .depend

#########################################################################
