include $(TOPDIR)/config.mk

LIB := $(obj)libatibiosemu.a

X86DIR  = x86emu

$(shell mkdir -p $(obj)$(X86DIR))

COBJS-$(CONFIG_BIOSEMU)	= atibios.o biosemu.o besys.o bios.o \
	$(X86DIR)/decode.o \
	$(X86DIR)/ops2.o \
	$(X86DIR)/ops.o \
	$(X86DIR)/prim_ops.o \
	$(X86DIR)/sys.o \
	$(X86DIR)/debug.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

EXTRA_CFLAGS += -I. -I./include -I$(TOPDIR)/include \
	-D__PPC__  -D__BIG_ENDIAN__

CFLAGS += $(EXTRA_CFLAGS)
HOSTCFLAGS += $(EXTRA_CFLAGS)

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
