#
# (C) Copyright 2007 Semihalf
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundatio; either version 2 of
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

ifeq ($(ARCH),ppc)
LOAD_ADDR = 0x40000
endif

#ifeq ($(ARCH),arm)
#LOAD_ADDR = 0xc100000
#endif

include $(TOPDIR)/config.mk

ELF-$(CONFIG_API) += demo
BIN-$(CONFIG_API) += demo.bin
ELF	:= $(ELF-y)
BIN	:= $(BIN-y)

#CFLAGS += -v

COBJS-$(CONFIG_API) += $(ELF:=.o)
SOBJS-$(CONFIG_API) += crt0.o
ifeq ($(ARCH),ppc)
SOBJS-$(CONFIG_API) += ppcstring.o
endif
COBJS	:= $(COBJS-y)
SOBJS	:= $(SOBJS-y)

LIB	= $(obj)libglue.a
LIBCOBJS-$(CONFIG_API) += glue.o crc32.o ctype.o string.o vsprintf.o \
				libgenwrap.o
LIBCOBJS := $(LIBCOBJS-y)

LIBOBJS	= $(addprefix $(obj),$(SOBJS) $(LIBCOBJS))

SRCS	:= $(COBJS:.o=.c) $(LIBCOBJS:.o=.c) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS))
ELF	:= $(addprefix $(obj),$(ELF))
BIN	:= $(addprefix $(obj),$(BIN))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

CPPFLAGS += -I..

all:	$(obj).depend $(OBJS) $(LIB) $(ELF) $(BIN)

#########################################################################
$(LIB):	$(obj).depend $(LIBOBJS)
		$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(ELF):
$(obj)%:	$(obj)%.o $(LIB)
		$(LD) $(obj)crt0.o -Ttext $(LOAD_ADDR) \
			-o $@ $< $(LIB) \
			-L$(gcclibdir) -lgcc

$(BIN):
$(obj)%.bin:	$(obj)%
		$(OBJCOPY) -O binary $< $@ 2>/dev/null

$(obj)crc32.c:
	@rm -f $(obj)crc32.c
	ln -s $(src)../lib_generic/crc32.c $(obj)crc32.c

$(obj)ctype.c:
	@rm -f $(obj)ctype.c
	ln -s $(src)../lib_generic/ctype.c $(obj)ctype.c

$(obj)string.c:
	@rm -f $(obj)string.c
	ln -s $(src)../lib_generic/string.c $(obj)string.c

$(obj)vsprintf.c:
	@rm -f $(obj)vsprintf.c
	ln -s $(src)../lib_generic/vsprintf.c $(obj)vsprintf.c

ifeq ($(ARCH),ppc)
$(obj)ppcstring.S:
	@rm -f $(obj)ppcstring.S
	ln -s $(src)../lib_ppc/ppcstring.S $(obj)ppcstring.S
endif

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
