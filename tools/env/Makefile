#
# (C) Copyright 2002-2006
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

SRCS	:= $(obj)crc32.c  fw_env.c  fw_env_main.c
HEADERS	:= fw_env.h

CPPFLAGS := -Wall -DUSE_HOSTCC -I$(SRCTREE)/include

ifeq ($(MTD_VERSION),old)
CPPFLAGS += -DMTD_OLD
endif

all:	$(obj)fw_printenv

$(obj)fw_printenv:	$(SRCS) $(HEADERS)
	$(CROSS_COMPILE)gcc $(CPPFLAGS) $(SRCS) -o $(obj)fw_printenv

clean:
	rm -f $(obj)fw_printenv $(obj)crc32.c

$(obj)crc32.c:
	ln -s $(src)../../lib/crc32.c $(obj)crc32.c

#########################################################################

include $(TOPDIR)/rules.mk

sinclude $(obj).depend

#########################################################################
