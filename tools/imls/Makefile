#
# (C) Copyright 2009 Marco Stornelli <marco.stornelli@gmail.com>
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

HOSTCFLAGS = -Wall -pedantic

# Generated executable files
BIN_FILES-y += imls

# Source files which exist outside the tools/imls directory
EXT_OBJ_FILES-y += lib_generic/crc32.o
EXT_OBJ_FILES-y += lib_generic/md5.o
EXT_OBJ_FILES-y += lib_generic/sha1.o
EXT_OBJ_FILES-y += common/image.o

# Source files located in the tools/imls directory
OBJ_FILES-y += imls.o

# Flattened device tree objects
LIBFDT_OBJ_FILES-y += fdt.o
LIBFDT_OBJ_FILES-y += fdt_ro.o
LIBFDT_OBJ_FILES-y += fdt_rw.o
LIBFDT_OBJ_FILES-y += fdt_strerror.o
LIBFDT_OBJ_FILES-y += fdt_wip.o

# now $(obj) is defined
SRCS	+= $(addprefix $(SRCTREE)/,$(EXT_OBJ_FILES-y:.o=.c))
SRCS	+= $(addprefix $(SRCTREE)/tools/,$(OBJ_FILES-y:.o=.c))
SRCS	+= $(addprefix $(SRCTREE)/libfdt/,$(LIBFDT_OBJ_FILES-y:.o=.c))
BINS	:= $(addprefix $(obj),$(sort $(BIN_FILES-y)))
LIBFDT_OBJS	:= $(addprefix $(obj),$(LIBFDT_OBJ_FILES-y))

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
#
CPPFLAGS   = -idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
	        -I $(SRCTREE)/libfdt \
		-I $(SRCTREE)/tools \
		-DUSE_HOSTCC -D__KERNEL_STRICT_NAMES
CFLAGS     = $(HOSTCFLAGS) $(CPPFLAGS) -O

# No -pedantic switch to avoid libfdt compilation warnings
FIT_CFLAGS = -Wall $(CPPFLAGS) -O

CC	   = $(CROSS_COMPILER)gcc
STRIP	   = $(CROSS_COMPILER)strip

ifeq ($(MTD_VERSION),old)
CPPFLAGS += -DMTD_OLD
endif

all:	$(BINS)

$(obj)imls:	$(obj)imls.o $(obj)crc32.o $(obj)image.o $(obj)md5.o \
		$(obj)sha1.o $(LIBFDT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

# Some files complain if compiled with -pedantic, use FIT_CFLAGS
$(obj)image.o: $(SRCTREE)/common/image.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)imls.o: imls.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

# Some of the tool objects need to be accessed from outside the tools/imls directory
$(obj)%.o: $(SRCTREE)/common/%.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib_generic/%.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/libfdt/%.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o imls

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
