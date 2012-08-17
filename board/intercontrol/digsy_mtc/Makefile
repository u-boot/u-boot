
#
# Author: Grzegorz Bernacki, Semihalf, gjb@semihalf.com
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= $(BOARD).o cmd_mtc.o
COBJS-$(CONFIG_VIDEO) += cmd_disp.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
