#
# (C) Copyright 2000-2006
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

PROG		= $(obj)updater
IMAGE		= $(obj)updater.image

COBJS		= update.o flash.o flash_hw.o utils.o cmd_flash.o string.o ctype.o dummy.o
COBJS_LINKS	= stubs.o
AOBJS		= ppcstring.o
AOBJS_LINKS	= memio.o

OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS_LINKS) $(AOBJS) $(AOBJS_LINKS))
SRCS	:= $(COBJS:.o=.c) $(AOBJS:.o=.S) $(addprefix $(obj), $(COBJS_LINKS:.o:.c) $(AOBJS_LINKS:.o:.S))

CPPFLAGS += -I$(TOPDIR) -I$(TOPDIR)/board/MAI/AmigaOneG3SE
CFLAGS   += -I$(TOPDIR)/board/MAI/AmigaOneG3SE
AFLAGS   += -I$(TOPDIR)/board/MAI/AmigaOneG3SE

DEPS = $(OBJTREE)/u-boot.bin $(OBJTREE)/tools/mkimage
ifneq ($(DEPS),$(wildcard $(DEPS)))
$(error "updater: Missing required objects, please run regular build first")
endif

all:	$(obj).depend $(PROG) $(IMAGE)

#########################################################################

$(obj)%.srec:	%.o $(LIB)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(<:.o=) -e $(<:.o=) $< $(LIB)
	$(OBJCOPY) -O srec $(<:.o=) $@

$(obj)%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj)%.o: %.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)memio.o: $(obj)memio.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)memio.S:
	rm -f $(obj)memio.c
	ln -s $(SRCTREE)/board/MAI/AmigaOneG3SE/memio.S $(obj)memio.S

$(obj)stubs.o: $(obj)stubs.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj)stubs.c:
	rm -f $(obj)stubs.c
	ln -s $(SRCTREE)/examples/stubs.c $(obj)stubs.c

#########################################################################

$(obj)updater: $(OBJS)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(obj)updater -e _main $(OBJS)
	$(OBJCOPY) -O binary $(obj)updater $(obj)updater.bin

$(obj)updater.image: $(obj)updater $(OBJTREE)/u-boot.bin
	cat >/tmp/tempimage $(obj)updater.bin junk $(OBJTREE)/u-boot.bin
	$(OBJTREE)/tools/mkimage -A ppc -O u-boot -T standalone -C none -a $(LOAD_ADDR) \
	-e `$(NM) $(obj)updater | grep _main | cut --bytes=0-8` \
	-n "Firmware Updater" -d /tmp/tempimage $(obj)updater.image
	rm /tmp/tempimage
	cp $(obj)updater.image /tftpboot

(obj)updater.image2: $(obj)updater $(OBJTREE)/u-boot.bin
	cat >/tmp/tempimage $(obj)updater.bin junk ../../create_image/image
	$(OBJTREE)/tools/mkimage -A ppc -O u-boot -T standalone -C none -a $(LOAD_ADDR) \
	-e `$(NM) $(obj)updater | grep _main | cut --bytes=0-8` \
	-n "Firmware Updater" -d /tmp/tempimage $(obj)updater.image
	rm /tmp/tempimage
	cp $(obj)updater.image /tftpboot

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
