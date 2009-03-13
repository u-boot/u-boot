#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
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

TOOLSUBDIRS =

#
# Mac OS X / Darwin's C preprocessor is Apple specific.  It
# generates numerous errors and warnings.  We want to bypass it
# and use GNU C's cpp.  To do this we pass the -traditional-cpp
# option to the compiler.  Note that the -traditional-cpp flag
# DOES NOT have the same semantics as GNU C's flag, all it does
# is invoke the GNU preprocessor in stock ANSI/ISO C fashion.
#
# Apple's linker is similar, thanks to the new 2 stage linking
# multiple symbol definitions are treated as errors, hence the
# -multiply_defined suppress option to turn off this error.
#
ifeq ($(HOSTOS)-$(HOSTARCH),darwin-ppc)
HOST_CFLAGS = -traditional-cpp -Wall
HOST_LDFLAGS =-multiply_defined suppress
HOST_ENVIRO_CFLAGS =

else
ifeq ($(HOSTOS)-$(HOSTARCH),netbsd-ppc)
HOST_CFLAGS = -Wall -pedantic
HOST_LDFLAGS =
HOST_ENVIRO_CFLAGS =

#
# Everyone else
#
else
HOST_CFLAGS = -Wall -pedantic
HOST_LDFLAGS =
HOST_ENVIRO_CFLAGS =
endif
endif

#
# Cygwin needs .exe files :-(
#
ifeq ($(HOSTOS),cygwin)
SFX = .exe
HOST_CFLAGS += -ansi
else
SFX =
endif

#
# Include this after HOSTOS HOSTARCH check
# so that we can act intelligently.
#
include $(TOPDIR)/config.mk

# Generated executable files
BIN_FILES-$(CONFIG_CMD_LOADS) += img2srec$(SFX)
BIN_FILES-y += mkimage$(SFX)
BIN_FILES-y += envcrc$(SFX)
BIN_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1$(SFX)
BIN_FILES-y += gen_eth_addr$(SFX)
BIN_FILES-$(CONFIG_LCD_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_INCA_IP) += inca-swap-bytes$(SFX)

# Source files which are symlinked from elsewhere in U-Boot
OBJ_LINKS-y += env_embedded.o
OBJ_LINKS-y += crc32.o md5.o
OBJ_LINKS-y += sha1.o
OBJ_LINKS-y += image.o

# Source files located in the tools directory
OBJ_FILES-$(CONFIG_CMD_LOADS) += img2srec.o
OBJ_FILES-y += mkimage.o
OBJ_FILES-y += envcrc.o
OBJ_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1.o
OBJ_FILES-y += gen_eth_addr.o
OBJ_FILES-$(CONFIG_LCD_LOGO) += bmp_logo.o
OBJ_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo.o
OBJ_FILES-$(CONFIG_INCA_IP) += inca-swap-bytes.o

# Don't build by default
#ifeq ($(ARCH),ppc)
#BIN_FILES-y += mpc86x_clk$(SFX)
#OBJ_FILES-y += mpc86x_clk.o
#endif

# Flattened device tree objects
LIBFDT_OBJ_FILES-y += fdt.o
LIBFDT_OBJ_FILES-y += fdt_ro.o
LIBFDT_OBJ_FILES-y += fdt_rw.o
LIBFDT_OBJ_FILES-y += fdt_strerror.o
LIBFDT_OBJ_FILES-y += fdt_wip.o

# Generated LCD/video logo
LOGO_H = $(OBJTREE)/include/bmp_logo.h
LOGO-$(CONFIG_LCD_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_H)

ifeq ($(LOGO_BMP),)
LOGO_BMP= logos/denx.bmp
endif
ifeq ($(VENDOR),atmel)
LOGO_BMP= logos/atmel.bmp
endif

# now $(obj) is defined
SRCS	:= $(addprefix $(obj),$(OBJ_LINKS-y:.o=.c)) $(OBJ_FILES-y:.o=.c)
BINS	:= $(addprefix $(obj),$(BIN_FILES-y))
LIBFDT_OBJS	:= $(addprefix $(obj),$(LIBFDT_OBJ_FILES-y))

#
# Use native tools and options
#
CPPFLAGS   = -idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
		-DTEXT_BASE=$(TEXT_BASE) -DUSE_HOSTCC
CFLAGS     = $(HOST_CFLAGS) $(CPPFLAGS) -O

# No -pedantic switch to avoid libfdt compilation warnings
FIT_CFLAGS = -Wall $(CPPFLAGS) -O

AFLAGS	   = -D__ASSEMBLY__ $(CPPFLAGS)
CC	   = $(HOSTCC)
STRIP	   = $(HOSTSTRIP)
MAKEDEPEND = makedepend

all:	$(obj).depend $(BINS) $(LOGO-y) subdirs

$(obj)envcrc$(SFX):	$(obj)envcrc.o $(obj)crc32.o $(obj)env_embedded.o $(obj)sha1.o
	$(CC) $(CFLAGS) -o $@ $^

$(obj)ubsha1$(SFX):	$(obj)ubsha1.o $(obj)sha1.o
	$(CC) $(CFLAGS) -o $@ $^

$(obj)img2srec$(SFX):	$(obj)img2srec.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)mkimage$(SFX):	$(obj)mkimage.o $(obj)crc32.o $(obj)image.o $(obj)md5.o \
			$(obj)sha1.o $(LIBFDT_OBJS)
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)ncb$(SFX):	$(obj)ncb.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)gen_eth_addr$(SFX):	$(obj)gen_eth_addr.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)bmp_logo$(SFX):	$(obj)bmp_logo.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)inca-swap-bytes$(SFX):	$(obj)inca-swap-bytes.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)mpc86x_clk$(SFX):	$(obj)mpc86x_clk.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)bin2header$(SFX): $(obj)bin2header.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)envcrc.o:	$(src)envcrc.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)ubsha1.o:	$(src)ubsha1.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)crc32.o:	$(obj)crc32.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)md5.o:	$(obj)md5.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)sha1.o:	$(obj)sha1.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)image.o:	$(obj)image.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)mkimage.o:	$(src)mkimage.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)ncb.o:		$(src)ncb.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)gen_eth_addr.o:	$(src)gen_eth_addr.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)inca-swap-bytes.o:	$(src)inca-swap-bytes.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)mpc86x_clk.o:	$(src)mpc86x_clk.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(obj)fdt.o:	$(obj)fdt.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)fdt_ro.o:	$(obj)fdt_ro.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)fdt_rw.o:	$(obj)fdt_rw.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)fdt_strerror.o:	$(obj)fdt_strerror.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)fdt_wip.o:	$(obj)fdt_wip.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

subdirs:
ifeq ($(TOOLSUBDIRS),)
	@:
else
	@for dir in $(TOOLSUBDIRS) ; do \
	    $(MAKE) \
		HOSTOS=$(HOSTOS) \
		HOSTARCH=$(HOSTARCH) \
		HOST_CFLAGS="$(HOST_CFLAGS)" \
		HOST_LDFLAGS="$(HOST_LDFLAGS)" \
		-C $$dir || exit 1 ; \
	done
endif

$(obj)env_embedded.c:
	@rm -f $(obj)env_embedded.c
	ln -s $(src)../common/env_embedded.c $(obj)env_embedded.c

$(obj)env_embedded.o:	$(obj)env_embedded.c
	$(CC) -g $(HOST_ENVIRO_CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(obj)zlib.h:
	@rm -f $@
	ln -s $(src)../include/zlib.h $@

$(obj)crc32.c: $(obj)zlib.h
	@rm -f $(obj)crc32.c
	ln -s $(src)../lib_generic/crc32.c $(obj)crc32.c

$(obj)md5.c:
	@rm -f $(obj)md5.c
	ln -s $(src)../lib_generic/md5.c $(obj)md5.c

$(obj)sha1.c:
	@rm -f $(obj)sha1.c
	ln -s $(src)../lib_generic/sha1.c $(obj)sha1.c

$(obj)image.c:
	@rm -f $(obj)image.c
	ln -s $(src)../common/image.c $(obj)image.c
	if [ ! -f $(obj)mkimage.h ] ; then \
		ln -s $(src)../tools/mkimage.h $(obj)mkimage.h; \
	fi
	if [ ! -f $(obj)fdt_host.h ] ; then \
		ln -s $(src)../tools/fdt_host.h $(obj)fdt_host.h; \
	fi

$(obj)fdt.c:	$(obj)libfdt_internal.h
	@rm -f $(obj)fdt.c
	ln -s $(src)../libfdt/fdt.c $(obj)fdt.c

$(obj)fdt_ro.c:	$(obj)libfdt_internal.h
	@rm -f $(obj)fdt_ro.c
	ln -s $(src)../libfdt/fdt_ro.c $(obj)fdt_ro.c

$(obj)fdt_rw.c:	$(obj)libfdt_internal.h
	@rm -f $(obj)fdt_rw.c
	ln -s $(src)../libfdt/fdt_rw.c $(obj)fdt_rw.c

$(obj)fdt_strerror.c:	$(obj)libfdt_internal.h
	@rm -f $(obj)fdt_strerror.c
	ln -s $(src)../libfdt/fdt_strerror.c $(obj)fdt_strerror.c

$(obj)fdt_wip.c:	$(obj)libfdt_internal.h
	@rm -f $(obj)fdt_wip.c
	ln -s $(src)../libfdt/fdt_wip.c $(obj)fdt_wip.c

$(obj)libfdt_internal.h:
	@rm -f $(obj)libfdt_internal.h
	ln -s $(src)../libfdt/libfdt_internal.h $(obj)libfdt_internal.h

$(LOGO_H):	$(obj)bmp_logo $(LOGO_BMP)
	$(obj)./bmp_logo $(LOGO_BMP) >$@

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
