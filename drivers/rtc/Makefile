#
# (C) Copyright 2001-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

#CFLAGS += -DDEBUG

LIB	= $(obj)librtc.o

COBJS-$(CONFIG_RTC_AT91SAM9_RTT) += at91sam9_rtt.o
COBJS-$(CONFIG_RTC_BFIN) += bfin_rtc.o
COBJS-y += date.o
COBJS-$(CONFIG_RTC_DAVINCI) += davinci.o
COBJS-$(CONFIG_RTC_DS12887) += ds12887.o
COBJS-$(CONFIG_RTC_DS1302) += ds1302.o
COBJS-$(CONFIG_RTC_DS1306) += ds1306.o
COBJS-$(CONFIG_RTC_DS1307) += ds1307.o
COBJS-$(CONFIG_RTC_DS1338) += ds1307.o
COBJS-$(CONFIG_RTC_DS1337) += ds1337.o
COBJS-$(CONFIG_RTC_DS1374) += ds1374.o
COBJS-$(CONFIG_RTC_DS1388) += ds1337.o
COBJS-$(CONFIG_RTC_DS1556) += ds1556.o
COBJS-$(CONFIG_RTC_DS164x) += ds164x.o
COBJS-$(CONFIG_RTC_DS174x) += ds174x.o
COBJS-$(CONFIG_RTC_DS3231) += ds3231.o
COBJS-$(CONFIG_RTC_FTRTC010) += ftrtc010.o
COBJS-$(CONFIG_RTC_IMXDI) += imxdi.o
COBJS-$(CONFIG_RTC_ISL1208) += isl1208.o
COBJS-$(CONFIG_RTC_M41T11) += m41t11.o
COBJS-$(CONFIG_RTC_M41T60) += m41t60.o
COBJS-$(CONFIG_RTC_M41T62) += m41t62.o
COBJS-$(CONFIG_RTC_M41T94) += m41t94.o
COBJS-$(CONFIG_RTC_M48T35A) += m48t35ax.o
COBJS-$(CONFIG_RTC_MAX6900) += max6900.o
COBJS-$(CONFIG_RTC_MC13XXX) += mc13xxx-rtc.o
COBJS-$(CONFIG_RTC_MC146818) += mc146818.o
COBJS-$(CONFIG_MCFRTC) += mcfrtc.o
COBJS-$(CONFIG_RTC_MK48T59) += mk48t59.o
COBJS-$(CONFIG_RTC_MPC5200) += mpc5xxx.o
COBJS-$(CONFIG_RTC_MPC8xx) += mpc8xx.o
COBJS-$(CONFIG_RTC_MV) += mvrtc.o
COBJS-$(CONFIG_RTC_MX27) += mx27rtc.o
COBJS-$(CONFIG_RTC_MXS) += mxsrtc.o
COBJS-$(CONFIG_RTC_PCF8563) += pcf8563.o
COBJS-$(CONFIG_RTC_PL031) += pl031.o
COBJS-$(CONFIG_RTC_PT7C4338) += pt7c4338.o
COBJS-$(CONFIG_RTC_RS5C372A) += rs5c372.o
COBJS-$(CONFIG_RTC_RTC4543) += rtc4543.o
COBJS-$(CONFIG_RTC_RV3029) += rv3029.o
COBJS-$(CONFIG_RTC_RX8025) += rx8025.o
COBJS-$(CONFIG_RTC_S3C24X0) += s3c24x0_rtc.o
COBJS-$(CONFIG_RTC_X1205) += x1205.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
