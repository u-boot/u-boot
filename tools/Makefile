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

TOOLSUBDIRS = kernel-doc

#
# Include this after HOSTOS HOSTARCH check
# so that we can act intelligently.
#
include $(TOPDIR)/config.mk

#
# toolchains targeting win32 generate .exe files
#
ifneq (,$(findstring WIN32 ,$(shell $(HOSTCC) -E -dM -xc /dev/null)))
SFX = .exe
else
SFX =
endif

# Enable all the config-independent tools
ifneq ($(HOST_TOOLS_ALL),)
CONFIG_LCD_LOGO = y
CONFIG_CMD_LOADS = y
CONFIG_CMD_NET = y
CONFIG_XWAY_SWAP_BYTES = y
CONFIG_NETCONSOLE = y
CONFIG_SHA1_CHECK_UB_IMG = y
endif

# Merge all the different vars for envcrc into one
ENVCRC-$(CONFIG_ENV_IS_EMBEDDED) = y
ENVCRC-$(CONFIG_ENV_IS_IN_DATAFLASH) = y
ENVCRC-$(CONFIG_ENV_IS_IN_EEPROM) = y
ENVCRC-$(CONFIG_ENV_IS_IN_FLASH) = y
ENVCRC-$(CONFIG_ENV_IS_IN_ONENAND) = y
ENVCRC-$(CONFIG_ENV_IS_IN_NAND) = y
ENVCRC-$(CONFIG_ENV_IS_IN_NVRAM) = y
ENVCRC-$(CONFIG_ENV_IS_IN_SPI_FLASH) = y
CONFIG_BUILD_ENVCRC ?= $(ENVCRC-y)

# Generated executable files
BIN_FILES-$(CONFIG_LCD_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo$(SFX)
BIN_FILES-$(CONFIG_BUILD_ENVCRC) += envcrc$(SFX)
BIN_FILES-$(CONFIG_CMD_NET) += gen_eth_addr$(SFX)
BIN_FILES-$(CONFIG_CMD_LOADS) += img2srec$(SFX)
BIN_FILES-$(CONFIG_XWAY_SWAP_BYTES) += xway-swap-bytes$(SFX)
BIN_FILES-y += mkenvimage$(SFX)
BIN_FILES-y += mkimage$(SFX)
BIN_FILES-$(CONFIG_SMDK5250) += mksmdk5250spl$(SFX)
BIN_FILES-$(CONFIG_MX23) += mxsboot$(SFX)
BIN_FILES-$(CONFIG_MX28) += mxsboot$(SFX)
BIN_FILES-$(CONFIG_NETCONSOLE) += ncb$(SFX)
BIN_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1$(SFX)
BIN_FILES-$(CONFIG_KIRKWOOD) += kwboot$(SFX)
BIN_FILES-y += proftool(SFX)

# Source files which exist outside the tools directory
EXT_OBJ_FILES-$(CONFIG_BUILD_ENVCRC) += common/env_embedded.o
EXT_OBJ_FILES-y += common/image.o
EXT_OBJ_FILES-$(CONFIG_FIT) += common/image-fit.o
EXT_OBJ_FILES-y += common/image-sig.o
EXT_OBJ_FILES-y += lib/crc32.o
EXT_OBJ_FILES-y += lib/md5.o
EXT_OBJ_FILES-y += lib/sha1.o

# Source files located in the tools directory
OBJ_FILES-$(CONFIG_LCD_LOGO) += bmp_logo.o
OBJ_FILES-$(CONFIG_VIDEO_LOGO) += bmp_logo.o
NOPED_OBJ_FILES-y += default_image.o
NOPED_OBJ_FILES-y += proftool.o
OBJ_FILES-$(CONFIG_BUILD_ENVCRC) += envcrc.o
NOPED_OBJ_FILES-y += fit_image.o
OBJ_FILES-$(CONFIG_CMD_NET) += gen_eth_addr.o
OBJ_FILES-$(CONFIG_CMD_LOADS) += img2srec.o
OBJ_FILES-$(CONFIG_XWAY_SWAP_BYTES) += xway-swap-bytes.o
NOPED_OBJ_FILES-y += aisimage.o
NOPED_OBJ_FILES-y += kwbimage.o
NOPED_OBJ_FILES-y += pblimage.o
NOPED_OBJ_FILES-y += imximage.o
NOPED_OBJ_FILES-y += image-host.o
NOPED_OBJ_FILES-y += omapimage.o
NOPED_OBJ_FILES-y += mkenvimage.o
NOPED_OBJ_FILES-y += mkimage.o
OBJ_FILES-$(CONFIG_SMDK5250) += mkexynosspl.o
OBJ_FILES-$(CONFIG_MX23) += mxsboot.o
OBJ_FILES-$(CONFIG_MX28) += mxsboot.o
OBJ_FILES-$(CONFIG_NETCONSOLE) += ncb.o
NOPED_OBJ_FILES-y += os_support.o
OBJ_FILES-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1.o
NOPED_OBJ_FILES-y += ublimage.o
OBJ_FILES-$(CONFIG_KIRKWOOD) += kwboot.o

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
LOGO_DATA_H = $(OBJTREE)/include/bmp_logo_data.h
LOGO-$(CONFIG_LCD_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_LCD_LOGO) += $(LOGO_DATA_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_DATA_H)

# Generic logo
ifeq ($(LOGO_BMP),)
LOGO_BMP= logos/denx.bmp

# Use board logo and fallback to vendor
ifneq ($(wildcard logos/$(BOARD).bmp),)
LOGO_BMP= logos/$(BOARD).bmp
else
ifneq ($(wildcard logos/$(VENDOR).bmp),)
LOGO_BMP= logos/$(VENDOR).bmp
endif
endif

endif # !LOGO_BMP

# now $(obj) is defined
HOSTSRCS += $(addprefix $(SRCTREE)/,$(EXT_OBJ_FILES-y:.o=.c))
HOSTSRCS += $(addprefix $(SRCTREE)/tools/,$(OBJ_FILES-y:.o=.c))
HOSTSRCS += $(addprefix $(SRCTREE)/lib/libfdt/,$(LIBFDT_OBJ_FILES-y:.o=.c))
BINS	:= $(addprefix $(obj),$(sort $(BIN_FILES-y)))
LIBFDT_OBJS	:= $(addprefix $(obj),$(LIBFDT_OBJ_FILES-y))

# We cannot check CONFIG_FIT_SIGNATURE here since it is not set on the host
FIT_SIG_OBJ_FILES	:= image-sig.o
FIT_SIG_OBJS		:= $(addprefix $(obj),$(FIT_SIG_OBJ_FILES))

HOSTOBJS := $(addprefix $(obj),$(OBJ_FILES-y))
NOPEDOBJS := $(addprefix $(obj),$(NOPED_OBJ_FILES-y))

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
#
HOSTCPPFLAGS =	-include $(SRCTREE)/include/libfdt_env.h \
		-idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
	        -I $(SRCTREE)/lib/libfdt \
		-I $(SRCTREE)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES


all:	$(obj).depend $(BINS) $(LOGO-y) subdirs

$(obj)bin2header$(SFX): $(obj)bin2header.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)bmp_logo$(SFX):	$(obj)bmp_logo.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)proftool(SFX):	$(obj)proftool.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)envcrc$(SFX):	$(obj)crc32.o $(obj)env_embedded.o $(obj)envcrc.o $(obj)sha1.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^

$(obj)gen_eth_addr$(SFX):	$(obj)gen_eth_addr.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)img2srec$(SFX):	$(obj)img2srec.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)xway-swap-bytes$(SFX):	$(obj)xway-swap-bytes.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mkenvimage$(SFX):	$(obj)crc32.o $(obj)mkenvimage.o \
	$(obj)os_support.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mkimage$(SFX):	$(obj)aisimage.o \
			$(obj)crc32.o \
			$(obj)default_image.o \
			$(obj)fit_image.o \
			$(obj)image-fit.o \
			$(obj)image.o \
			$(obj)image-host.o \
			$(FIT_SIG_OBJS) \
			$(obj)imximage.o \
			$(obj)kwbimage.o \
			$(obj)pblimage.o \
			$(obj)md5.o \
			$(obj)mkimage.o \
			$(obj)os_support.o \
			$(obj)omapimage.o \
			$(obj)sha1.o \
			$(obj)ublimage.o \
			$(LIBFDT_OBJS)
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mk$(BOARD)spl$(SFX):	$(obj)mkexynosspl.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mpc86x_clk$(SFX):	$(obj)mpc86x_clk.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mxsboot$(SFX):	$(obj)mxsboot.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)ncb$(SFX):	$(obj)ncb.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)ubsha1$(SFX):	$(obj)os_support.o $(obj)sha1.o $(obj)ubsha1.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^

$(obj)kwboot$(SFX): $(obj)kwboot.o
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

# Some of the tool objects need to be accessed from outside the tools directory
$(obj)%.o: $(SRCTREE)/common/%.c
	$(HOSTCC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib/%.c
	$(HOSTCC) -g $(HOSTCFLAGS) -c -o $@ $<

$(obj)%.o: $(SRCTREE)/lib/libfdt/%.c
	$(HOSTCC) -g $(HOSTCFLAGS_NOPED) -c -o $@ $<

subdirs:
ifeq ($(TOOLSUBDIRS),)
	@:
else
	@for dir in $(TOOLSUBDIRS) ; do \
	    $(MAKE) \
		HOSTOS=$(HOSTOS) \
		HOSTARCH=$(HOSTARCH) \
		-C $$dir || exit 1 ; \
	done
endif

$(LOGO_H):	$(obj)bmp_logo $(LOGO_BMP)
	$(obj)./bmp_logo --gen-info $(LOGO_BMP) > $@

$(LOGO_DATA_H):	$(obj)bmp_logo $(LOGO_BMP)
	$(obj)./bmp_logo --gen-data $(LOGO_BMP) > $@

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
