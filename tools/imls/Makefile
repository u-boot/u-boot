#
# (C) Copyright 2009 Marco Stornelli <marco.stornelli@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

# Generated executable files
BIN_FILES-y += imls

# Source files which exist outside the tools/imls directory
EXT_OBJ_FILES-y += lib/crc32.o
EXT_OBJ_FILES-y += lib/md5.o
EXT_OBJ_FILES-y += lib/sha1.o
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
SRCS	+= $(addprefix $(SRCTREE)/lib/libfdt/,$(LIBFDT_OBJ_FILES-y:.o=.c))
BINS	:= $(addprefix $(obj),$(sort $(BIN_FILES-y)))
LIBFDT_OBJS	:= $(addprefix $(obj),$(LIBFDT_OBJ_FILES-y))

#
# Compile for a hosted environment on the target
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
#
HOSTCPPFLAGS  = -idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
		-I $(SRCTREE)/lib/libfdt \
		-I $(SRCTREE)/tools \
		-DUSE_HOSTCC -D__KERNEL_STRICT_NAMES

ifeq ($(MTD_VERSION),old)
HOSTCPPFLAGS += -DMTD_OLD
endif

all:	$(BINS)

$(obj)imls:	$(obj)imls.o $(obj)crc32.o $(obj)image.o $(obj)md5.o \
		$(obj)sha1.o $(LIBFDT_OBJS)
	$(CC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(STRIP) $@

# Some files complain if compiled with -pedantic, use HOSTCFLAGS_NOPED
$(obj)image.o: $(SRCTREE)/common/image.c
	$(CC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

$(obj)imls.o: $(SRCTREE)/tools/imls/imls.c
	$(CC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

# Some of the tool objects need to be accessed from outside the tools/imls directory
$(obj)%.o: $(SRCTREE)/common/%.c
	$(CC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib/%.c
	$(CC) -g $(HOSTCFLAGS) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib/libfdt/%.c
	$(CC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

clean:
	rm -rf *.o imls

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
