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

HOSTCFLAGS = -Wall
HOST_LDFLAGS =

ifeq ($(HOSTOS)-$(HOSTARCH),darwin-ppc)
HOSTCFLAGS += -traditional-cpp
HOST_LDFLAGS += -multiply_defined suppress
else
HOSTCFLAGS += -pedantic
endif

ifeq ($(HOSTOS),cygwin)
HOSTCFLAGS += -ansi
endif

#
# toolchains targeting win32 generate .exe files
#
ifneq (,$(findstring WIN32 ,$(shell $(HOSTCC) -E -dM -xc /dev/null)))
SFX = .exe
else
SFX =
endif

#
# Include this after HOSTOS HOSTARCH check
# so that we can act intelligently.
#
include $(TOPDIR)/config.mk

# Generated executable files
BIN_FILES-$(CONFIG_LCD_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_EMBEDDED) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_DATAFLASH) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_EEPROM) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_FLASH) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_ONENAND) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_NAND) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_NVRAM) += envcrc$(SFX)
BIN_FILES-$(CONFIG_ENV_IS_IN_SPI_FLASH) += envcrc$(SFX)
BIN_FILES-$(CONFIG_CMD_NET) += gen_eth_addr$(SFX)
BIN_FILES-$(CONFIG_CMD_LOADS) += img2srec$(SFX)
BIN_FILES-$(CONFIG_INCA_IP) += inca-swap-bytes$(SFX)
BIN_FILES-y += mkimage$(SFX)
BIN_FILES-$(CONFIG_NETCONSOLE) += ncb$(SFX)
BIN_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1$(SFX)

# Source files which exist outside the tools directory
EXT_OBJ_FILES-y += common/env_embedded.o
EXT_OBJ_FILES-y += common/image.o
EXT_OBJ_FILES-y += lib_generic/crc32.o
EXT_OBJ_FILES-y += lib_generic/md5.o
EXT_OBJ_FILES-y += lib_generic/sha1.o

# Source files located in the tools directory
OBJ_FILES-$(CONFIG_LCD_LOGO) += bmp_logo.o
OBJ_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo.o
OBJ_FILES-y += default_image.o
OBJ_FILES-$(CONFIG_ENV_IS_EMBEDDED) += envcrc.o
OBJ_FILES-y += fit_image.o
OBJ_FILES-$(CONFIG_CMD_NET) += gen_eth_addr.o
OBJ_FILES-$(CONFIG_CMD_LOADS) += img2srec.o
OBJ_FILES-$(CONFIG_INCA_IP) += inca-swap-bytes.o
OBJ_FILES-y += kwbimage.o
OBJ_FILES-y += mkimage.o
OBJ_FILES-$(CONFIG_NETCONSOLE) += ncb.o
OBJ_FILES-y += os_support.o
OBJ_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1.o

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
ifeq ($(VENDOR),ronetix)
LOGO_BMP= logos/ronetix.bmp
endif

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
		-DTEXT_BASE=$(TEXT_BASE) -DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES
CFLAGS     = $(HOSTCFLAGS) $(CPPFLAGS) -O

# No -pedantic switch to avoid libfdt compilation warnings
FIT_CFLAGS = -Wall $(CPPFLAGS) -O

AFLAGS	   = -D__ASSEMBLY__ $(CPPFLAGS)
CC	   = $(HOSTCC)
STRIP	   = $(HOSTSTRIP)
MAKEDEPEND = makedepend

all:	$(obj).depend $(BINS) $(LOGO-y) subdirs

$(obj)bin2header$(SFX): $(obj)bin2header.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)bmp_logo$(SFX):	$(obj)bmp_logo.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)envcrc$(SFX):	$(obj)crc32.o $(obj)env_embedded.o $(obj)envcrc.o $(obj)sha1.o
	$(CC) $(CFLAGS) -o $@ $^

$(obj)gen_eth_addr$(SFX):	$(obj)gen_eth_addr.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)img2srec$(SFX):	$(obj)img2srec.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)inca-swap-bytes$(SFX):	$(obj)inca-swap-bytes.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)mkimage$(SFX):	$(obj)crc32.o \
			$(obj)default_image.o \
			$(obj)fit_image.o \
			$(obj)image.o \
			$(obj)kwbimage.o \
			$(obj)md5.o \
			$(obj)mkimage.o \
			$(obj)os_support.o \
			$(obj)sha1.o \
			$(LIBFDT_OBJS)
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)mpc86x_clk$(SFX):	$(obj)mpc86x_clk.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)ncb$(SFX):	$(obj)ncb.o
	$(CC) $(CFLAGS) $(HOST_LDFLAGS) -o $@ $^
	$(STRIP) $@

$(obj)ubsha1$(SFX):	$(obj)os_support.o $(obj)sha1.o $(obj)ubsha1.o
	$(CC) $(CFLAGS) -o $@ $^

# Some files complain if compiled with -pedantic, use FIT_CFLAGS
$(obj)default_image.o: $(SRCTREE)/tools/default_image.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)fit_image.o: $(SRCTREE)/tools/fit_image.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)image.o: $(SRCTREE)/common/image.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)kwbimage.o: $(SRCTREE)/tools/kwbimage.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)mkimage.o: $(SRCTREE)/tools/mkimage.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)os_support.o: $(SRCTREE)/tools/os_support.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

# Some of the tool objects need to be accessed from outside the tools directory
$(obj)%.o: $(SRCTREE)/common/%.c
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib_generic/%.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

$(LIBFDT_OBJS):
	$(CC) -g $(FIT_CFLAGS) -c -o $@ $<

subdirs:
ifeq ($(TOOLSUBDIRS),)
	@:
else
	@for dir in $(TOOLSUBDIRS) ; do \
	    $(MAKE) \
		HOSTOS=$(HOSTOS) \
		HOSTARCH=$(HOSTARCH) \
		HOSTCFLAGS="$(HOSTCFLAGS)" \
		HOST_LDFLAGS="$(HOST_LDFLAGS)" \
		-C $$dir || exit 1 ; \
	done
endif

$(LOGO_H):	$(obj)bmp_logo $(LOGO_BMP)
	$(obj)./bmp_logo $(LOGO_BMP) >$@

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
