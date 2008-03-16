#
# (C) Copyright 2001-2006
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

#CFLAGS += -DDEBUG

LIB	= $(obj)librtc.a

COBJS-y += bfin_rtc.o
COBJS-y += date.o
COBJS-y += ds12887.o
COBJS-y += ds1302.o
COBJS-y += ds1306.o
COBJS-y += ds1307.o
COBJS-y += ds1337.o
COBJS-y += ds1374.o
COBJS-y += ds1556.o
COBJS-y += ds164x.o
COBJS-y += ds174x.o
COBJS-y += ds3231.o
COBJS-$(CONFIG_RTC_ISL1208) += isl1208.o
COBJS-y += m41t11.o
COBJS-y += m41t60.o
COBJS-$(CONFIG_RTC_M41T62) += m41t62.o
COBJS-y += max6900.o
COBJS-y += m48t35ax.o
COBJS-y += max6900.o
COBJS-y += mc146818.o
COBJS-y += mcfrtc.o
COBJS-y += mk48t59.o
COBJS-y += mpc5xxx.o
COBJS-y += mpc8xx.o
COBJS-y += pcf8563.o
COBJS-y += rs5c372.o
COBJS-y += rx8025.o
COBJS-y += s3c24x0_rtc.o
COBJS-y += x1205.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
