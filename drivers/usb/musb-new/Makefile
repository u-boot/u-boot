#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_musb-new.o

COBJS-$(CONFIG_MUSB_GADGET) += musb_gadget.o musb_gadget_ep0.o musb_core.o
COBJS-$(CONFIG_MUSB_GADGET) += musb_uboot.o
COBJS-$(CONFIG_MUSB_HOST) += musb_host.o musb_core.o musb_uboot.o
COBJS-$(CONFIG_USB_MUSB_DSPS) += musb_dsps.o
COBJS-$(CONFIG_USB_MUSB_AM35X) += am35x.o
COBJS-$(CONFIG_USB_MUSB_OMAP2PLUS) += omap2430.o

CFLAGS_NO_WARN := $(call cc-option,-Wno-unused-variable) \
			$(call cc-option,-Wno-unused-but-set-variable) \
			$(call cc-option,-Wno-unused-label)
CFLAGS += $(CFLAGS_NO_WARN)

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

#$(LIB):	$(OBJS)
$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
