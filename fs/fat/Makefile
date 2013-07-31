#
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libfat.o

AOBJS	=
COBJS-$(CONFIG_FS_FAT)	:= fat.o
COBJS-$(CONFIG_FAT_WRITE):= fat_write.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_FS_FAT)	+= file.o
endif

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# SEE README.arm-unaligned-accesses
$(obj)file.o: CFLAGS += $(PLATFORM_NO_UNALIGNED)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
