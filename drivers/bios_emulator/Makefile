include $(TOPDIR)/config.mk

LIB := $(obj)libatibiosemu.a

X86DIR  = ./x86emu

OBJS	= atibios.o biosemu.o besys.o bios.o  \
	$(X86DIR)/decode.o \
	$(X86DIR)/ops2.o \
	$(X86DIR)/ops.o \
	$(X86DIR)/prim_ops.o \
	$(X86DIR)/sys.o \
	$(X86DIR)/debug.o

CFLAGS += -I. -I./include  -I$(X86DIR) -I$(TOPDIR)/include \
	-D__PPC__  -D__BIG_ENDIAN__

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
