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

PROG    = updater
IMAGE   = updater.image
SRC     = update.c flash.c flash_hw.c utils.c cmd_flash.c string.c ctype.c dummy.c
ASRC    = ppcstring.S
OBJS	= $(SRC:.c=.o) $(ASRC:.S=.o)

LIB	= $(TOPDIR)/examples/libstubs.a
LIBAOBJS=
LIBCOBJS= $(TOPDIR)/examples/stubs.o
LIBOBJS	= $(LIBAOBJS) $(LIBCOBJS)

CPPFLAGS += -I$(TOPDIR) -I$(TOPDIR)/board/MAI/AmigaOneG3SE
CFLAGS   += -I$(TOPDIR)/board/MAI/AmigaOneG3SE

all:	.depend $(LIB) $(PROG)

#########################################################################
$(LIB): .depend $(LIBOBJS)
	$(AR) crv $@ $(LIBOBJS)

%.srec:	%.o $(LIB)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(<:.o=) -e $(<:.o=) $< $(LIB)
	$(OBJCOPY) -O srec $(<:.o=) $@

%.o: %.c
	$(CC) $(CPPFLAGS) -c $<

%.o: %.S
	$(CC) $(CPPFLAGS) -c $<

#########################################################################

updater: $(OBJS) $(LIB) $(TOPDIR)/board/MAI/AmigaOneG3SE/memio.o
	$(LD) -g -Ttext $(LOAD_ADDR) -o updater -e _main $(OBJS) $(LIB)  \
	$(TOPDIR)/board/MAI/AmigaOneG3SE/memio.o
	$(OBJCOPY) -O binary updater updater.bin

updater.image: updater $(TOPDIR)/u-boot.bin
	cat >/tmp/tempimage updater.bin junk $(TOPDIR)/u-boot.bin
	$(TOPDIR)/tools/mkimage -A ppc -O u-boot -T standalone -C none -a $(LOAD_ADDR) \
	-e `ppc-elf32-nm updater | grep _main | cut --bytes=0-8` \
	-n "Firmware Updater" -d /tmp/tempimage updater.image
	rm /tmp/tempimage
	cp updater.image /tftpboot

updater.image2: updater $(TOPDIR)/u-boot.bin
	cat >/tmp/tempimage updater.bin junk ../../create_image/image
	$(TOPDIR)/tools/mkimage -A ppc -O u-boot -T standalone -C none -a $(LOAD_ADDR) \
	-e `ppc-elf32-nm updater | grep _main | cut --bytes=0-8` \
	-n "Firmware Updater" -d /tmp/tempimage updater.image
	rm /tmp/tempimage
	cp updater.image /tftpboot

.depend:	Makefile $(SRC) $(ASRC) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(SRC) $(ASRC) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S) > $@

sinclude .depend

#########################################################################
