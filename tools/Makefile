#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

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

subdir-$(HOST_TOOLS_ALL) += easylogo
subdir-$(HOST_TOOLS_ALL) += gdb

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

# TODO: CONFIG_CMD_LICENSE does not work
hostprogs-$(CONFIG_CMD_LICENSE) += bin2header$(SFX)

hostprogs-$(CONFIG_LCD_LOGO) += bmp_logo$(SFX)
hostprogs-$(CONFIG_VIDEO_LOGO) += bmp_logo$(SFX)
HOSTCFLAGS_bmp_logo$(SFX).o := -pedantic

hostprogs-$(CONFIG_BUILD_ENVCRC) += envcrc$(SFX)
envcrc$(SFX)-objs := crc32.o env_embedded.o envcrc.o sha1.o

hostprogs-$(CONFIG_CMD_NET) += gen_eth_addr$(SFX)
HOSTCFLAGS_gen_eth_addr$(SFX).o := -pedantic

hostprogs-$(CONFIG_CMD_LOADS) += img2srec$(SFX)
HOSTCFLAGS_img2srec$(SFX).o := -pedantic

hostprogs-$(CONFIG_XWAY_SWAP_BYTES) += xway-swap-bytes$(SFX)
HOSTCFLAGS_xway-swap-bytes$(SFX).o := -pedantic

hostprogs-y += mkenvimage$(SFX)
mkenvimage$(SFX)-objs := crc32.o mkenvimage.o os_support.o

hostprogs-y += dumpimage$(SFX) mkimage$(SFX)

FIT_SIG_OBJS-$(CONFIG_FIT_SIGNATURE) := image-sig.o
# Flattened device tree objects
LIBFDT_OBJS := fdt.o fdt_ro.o fdt_rw.o fdt_strerror.o fdt_wip.o
RSA_OBJS-$(CONFIG_FIT_SIGNATURE) := rsa-sign.o

# common objs for dumpimage and mkimage
dumpimage-mkimage-objs := aisimage.o \
			$(FIT_SIG_OBJS-y) \
			crc32.o \
			default_image.o \
			fit_image.o \
			image-fit.o \
			image-host.o \
			image.o \
			imagetool.o \
			imximage.o \
			kwbimage.o \
			md5.o \
			mxsimage.o \
			omapimage.o \
			os_support.o \
			pblimage.o \
			sha1.o \
			ublimage.o \
			$(LIBFDT_OBJS) \
			$(RSA_OBJS-y)

dumpimage$(SFX)-objs := $(dumpimage-mkimage-objs) dumpimage.o
mkimage$(SFX)-objs   := $(dumpimage-mkimage-objs) mkimage.o

# TODO(sjg@chromium.org): Is this correct on Mac OS?

# MXSImage needs LibSSL
ifneq ($(CONFIG_MX23)$(CONFIG_MX28),)
HOSTLOADLIBES_dumpimage$(SFX) := -lssl -lcrypto
HOSTLOADLIBES_mkimage$(SFX) := -lssl -lcrypto
# Add CONFIG_MXS into host CFLAGS, so we can check whether or not register
# the mxsimage support within tools/mxsimage.c .
HOSTCFLAGS_mxsimage.o += -DCONFIG_MXS
endif

ifdef CONFIG_FIT_SIGNATURE
HOSTLOADLIBES_dumpimage$(SFX) := -lssl -lcrypto
HOSTLOADLIBES_mkimage$(SFX) := -lssl -lcrypto

# This affects include/image.h, but including the board config file
# is tricky, so manually define this options here.
HOST_EXTRACFLAGS	+= -DCONFIG_FIT_SIGNATURE
endif

hostprogs-$(CONFIG_EXYNOS5250) += mkexynosspl$(SFX)
hostprogs-$(CONFIG_EXYNOS5420) += mkexynosspl$(SFX)
HOSTCFLAGS_mkexynosspl$(SFX).o := -pedantic

hostprogs-$(CONFIG_MX23) += mxsboot$(SFX)
hostprogs-$(CONFIG_MX28) += mxsboot$(SFX)
HOSTCFLAGS_mxsboot$(SFX).o := -pedantic

hostprogs-$(CONFIG_NETCONSOLE) += ncb$(SFX)
hostprogs-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1$(SFX)

ubsha1$(SFX)-objs := os_support.o sha1.o ubsha1.o

HOSTCFLAGS_ubsha1.o := -pedantic

hostprogs-$(CONFIG_KIRKWOOD) += kwboot$(SFX)
hostprogs-y += proftool$(SFX)
hostprogs-$(CONFIG_STATIC_RELA) += relocate-rela$(SFX)

# We build some files with extra pedantic flags to try to minimize things
# that won't build on some weird host compiler -- though there are lots of
# exceptions for files that aren't complaint.
HOSTCFLAGS_crc32.o := -pedantic
HOSTCFLAGS_md5.o := -pedantic
HOSTCFLAGS_sha1.o := -pedantic

# Don't build by default
#hostprogs-$(CONFIG_PPC) += mpc86x_clk$(SFX)
#HOSTCFLAGS_mpc86x_clk$(SFX).o := -pedantic

always := $(hostprogs-y)

# Generated LCD/video logo
LOGO_H = $(objtree)/include/bmp_logo.h
LOGO_DATA_H = $(objtree)/include/bmp_logo_data.h
LOGO-$(CONFIG_LCD_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_LCD_LOGO) += $(LOGO_DATA_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_H)
LOGO-$(CONFIG_VIDEO_LOGO) += $(LOGO_DATA_H)

# Generic logo
ifeq ($(LOGO_BMP),)
LOGO_BMP= $(srctree)/$(src)/logos/denx.bmp

# Use board logo and fallback to vendor
ifneq ($(wildcard $(srctree)/$(src)/logos/$(BOARD).bmp),)
LOGO_BMP= $(srctree)/$(src)/logos/$(BOARD).bmp
else
ifneq ($(wildcard $(srctree)/$(src)/logos/$(VENDOR).bmp),)
LOGO_BMP= $(srctree)/$(src)/logos/$(VENDOR).bmp
endif
endif

endif # !LOGO_BMP

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
# Define _GNU_SOURCE to obtain the getline prototype from stdio.h
#
HOST_EXTRACFLAGS += -include $(SRCTREE)/include/libfdt_env.h \
		$(patsubst -I%,-idirafter%, $(UBOOTINCLUDE)) \
		-I$(SRCTREE)/lib/libfdt \
		-I$(SRCTREE)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES \
		-D_GNU_SOURCE

__build:	$(LOGO-y)

$(LOGO_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-info $(LOGO_BMP) > $@

$(LOGO_DATA_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-data $(LOGO_BMP) > $@

# Let clean descend into subdirs
subdir- += env

ifneq ($(CROSS_BUILD_TOOLS),)
HOSTCC = $(CC)

quiet_cmd_crosstools_strip = STRIP   $^
      cmd_crosstools_strip = $(STRIP) $^; touch $@
$(obj)/.strip: $(call objectify,$(filter $(always),$(hostprogs-y)))
	$(call cmd,crosstools_strip)

always += .strip
endif
clean-files += .strip
