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
ifeq ($(ARCH),arm)
LOAD_ADDR = 0x1000000
endif

include $(TOPDIR)/config.mk

# Resulting ELF and binary exectuables will be named demo and demo.bin
OUTPUT-$(CONFIG_API) = $(obj)demo

#CFLAGS += -v

SOBJS-$(CONFIG_API) += crt0.o
COBJS-$(CONFIG_API) += demo.o
ifeq ($(ARCH),ppc)
SOBJS-$(CONFIG_API) += ppcstring.o
endif

OUTPUT	:= $(OUTPUT-y)
COBJS	:= $(COBJS-y)
SOBJS	:= $(SOBJS-y)

LIB	= $(obj)libglue.a
LIBCOBJS-$(CONFIG_API) += glue.o
LIBCOBJS-$(CONFIG_API) += crc32.o
LIBCOBJS-$(CONFIG_API) += ctype.o
LIBCOBJS-$(CONFIG_API) += string.o
LIBCOBJS-$(CONFIG_API) += vsprintf.o
LIBCOBJS-$(CONFIG_API) += libgenwrap.o
LIBCOBJS := $(LIBCOBJS-y)

LIBOBJS	+= $(addprefix $(obj),$(SOBJS))
LIBOBJS	+= $(addprefix $(obj),$(LIBCOBJS))

SRCS	+= $(COBJS:.o=.c)
SRCS	+= $(LIBCOBJS:.o=.c)
SRCS	+= $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

CPPFLAGS += -I..

all:	$(obj).depend $(OBJS) $(LIB) $(OUTPUT)

#########################################################################
$(LIB):	$(obj).depend $(LIBOBJS)
		$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(OUTPUT):
$(obj)%:	$(obj)%.o $(LIB)
		$(LD) $(obj)crt0.o -Ttext $(LOAD_ADDR) \
			-o $@ $< $(LIB) \
			-L$(gcclibdir) -lgcc
		$(OBJCOPY) -O binary $@ $(OUTPUT).bin 2>/dev/null

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
