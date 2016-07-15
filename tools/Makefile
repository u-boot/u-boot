#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

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

hostprogs-$(CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER) += atmel_pmecc_params

hostprogs-$(CONFIG_CMD_LICENSE) += bin2header
hostprogs-$(CONFIG_LCD_LOGO) += bmp_logo
hostprogs-$(CONFIG_VIDEO_LOGO) += bmp_logo
HOSTCFLAGS_bmp_logo.o := -pedantic

hostprogs-$(CONFIG_BUILD_ENVCRC) += envcrc
envcrc-objs := envcrc.o lib/crc32.o common/env_embedded.o lib/sha1.o

hostprogs-$(CONFIG_CMD_NET) += gen_eth_addr
HOSTCFLAGS_gen_eth_addr.o := -pedantic

hostprogs-$(CONFIG_CMD_LOADS) += img2srec
HOSTCFLAGS_img2srec.o := -pedantic

hostprogs-$(CONFIG_XWAY_SWAP_BYTES) += xway-swap-bytes
HOSTCFLAGS_xway-swap-bytes.o := -pedantic

hostprogs-y += mkenvimage
mkenvimage-objs := mkenvimage.o os_support.o lib/crc32.o

hostprogs-y += dumpimage mkimage
hostprogs-$(CONFIG_FIT_SIGNATURE) += fit_info fit_check_sign

FIT_SIG_OBJS-$(CONFIG_FIT_SIGNATURE) := common/image-sig.o
# Flattened device tree objects
LIBFDT_OBJS := $(addprefix lib/libfdt/, \
			fdt.o fdt_ro.o fdt_rw.o fdt_strerror.o fdt_wip.o \
			fdt_region.o fdt_sw.o)
RSA_OBJS-$(CONFIG_FIT_SIGNATURE) := $(addprefix lib/rsa/, \
					rsa-sign.o rsa-verify.o rsa-checksum.o \
					rsa-mod-exp.o)

ROCKCHIP_OBS = lib/rc4.o rkcommon.o rkimage.o rksd.o rkspi.o

# common objs for dumpimage and mkimage
dumpimage-mkimage-objs := aisimage.o \
			atmelimage.o \
			$(FIT_SIG_OBJS-y) \
			common/bootm.o \
			lib/crc32.o \
			default_image.o \
			lib/fdtdec_common.o \
			lib/fdtdec.o \
			fit_common.o \
			fit_image.o \
			common/image-fit.o \
			image-host.o \
			common/image.o \
			imagetool.o \
			imximage.o \
			kwbimage.o \
			lib/md5.o \
			lpc32xximage.o \
			mxsimage.o \
			omapimage.o \
			os_support.o \
			pblimage.o \
			pbl_crc32.o \
			$(ROCKCHIP_OBS) \
			socfpgaimage.o \
			lib/sha1.o \
			lib/sha256.o \
			common/hash.o \
			ublimage.o \
			zynqimage.o \
			zynqmpimage.o \
			$(LIBFDT_OBJS) \
			gpimage.o \
			gpimage-common.o \
			$(RSA_OBJS-y)

dumpimage-objs := $(dumpimage-mkimage-objs) dumpimage.o
mkimage-objs   := $(dumpimage-mkimage-objs) mkimage.o
fit_info-objs   := $(dumpimage-mkimage-objs) fit_info.o
fit_check_sign-objs   := $(dumpimage-mkimage-objs) fit_check_sign.o

# Build a libfdt Python module if swig is available
# Use 'sudo apt-get install swig libpython-dev' to enable this
hostprogs-$(CONFIG_SPL_OF_PLATDATA) += \
	$(if $(shell which swig),_libfdt.so)
_libfdt.so-sharedobjs += $(LIBFDT_OBJS)
libfdt:

tools/_libfdt.so: $(patsubst %.o,%.c,$(LIBFDT_OBJS)) tools/libfdt_wrap.c
	python $(srctree)/lib/libfdt/setup.py "$(_hostc_flags)" $^
	mv _libfdt.so $@

tools/libfdt_wrap.c: $(srctree)/lib/libfdt/libfdt.swig
	swig -python -o $@ $<

# TODO(sjg@chromium.org): Is this correct on Mac OS?

ifneq ($(CONFIG_MX23)$(CONFIG_MX28),)
# Add CONFIG_MXS into host CFLAGS, so we can check whether or not register
# the mxsimage support within tools/mxsimage.c .
HOSTCFLAGS_mxsimage.o += -DCONFIG_MXS
endif

ifdef CONFIG_FIT_SIGNATURE
# This affects include/image.h, but including the board config file
# is tricky, so manually define this options here.
HOST_EXTRACFLAGS	+= -DCONFIG_FIT_SIGNATURE
endif

ifdef CONFIG_SYS_U_BOOT_OFFS
HOSTCFLAGS_kwbimage.o += -DCONFIG_SYS_U_BOOT_OFFS=$(CONFIG_SYS_U_BOOT_OFFS)
endif

# MXSImage needs LibSSL
ifneq ($(CONFIG_MX23)$(CONFIG_MX28)$(CONFIG_FIT_SIGNATURE),)
HOSTLOADLIBES_mkimage += \
	$(shell pkg-config --libs libssl libcrypto 2> /dev/null || echo "-lssl -lcrypto")

# OS X deprecate openssl in favour of CommonCrypto, supress deprecation
# warnings on those systems
ifeq ($(HOSTOS),darwin)
HOSTCFLAGS_mxsimage.o += -Wno-deprecated-declarations
HOSTCFLAGS_image-sig.o += -Wno-deprecated-declarations
HOSTCFLAGS_rsa-sign.o += -Wno-deprecated-declarations
endif
endif

HOSTLOADLIBES_dumpimage := $(HOSTLOADLIBES_mkimage)
HOSTLOADLIBES_fit_info := $(HOSTLOADLIBES_mkimage)
HOSTLOADLIBES_fit_check_sign := $(HOSTLOADLIBES_mkimage)

hostprogs-$(CONFIG_EXYNOS5250) += mkexynosspl
hostprogs-$(CONFIG_EXYNOS5420) += mkexynosspl
HOSTCFLAGS_mkexynosspl.o := -pedantic

ifdtool-objs := $(LIBFDT_OBJS) ifdtool.o
hostprogs-$(CONFIG_X86) += ifdtool

hostprogs-$(CONFIG_MX23) += mxsboot
hostprogs-$(CONFIG_MX28) += mxsboot
HOSTCFLAGS_mxsboot.o := -pedantic

hostprogs-$(CONFIG_SUNXI) += mksunxiboot

hostprogs-$(CONFIG_NETCONSOLE) += ncb
hostprogs-$(CONFIG_SHA1_CHECK_UB_IMG) += ubsha1

ubsha1-objs := os_support.o ubsha1.o lib/sha1.o

HOSTCFLAGS_ubsha1.o := -pedantic

hostprogs-$(CONFIG_KIRKWOOD) += kwboot
hostprogs-$(CONFIG_ARCH_MVEBU) += kwboot
hostprogs-y += proftool
hostprogs-$(CONFIG_STATIC_RELA) += relocate-rela

hostprogs-y += fdtgrep
fdtgrep-objs += $(LIBFDT_OBJS) fdtgrep.o

# We build some files with extra pedantic flags to try to minimize things
# that won't build on some weird host compiler -- though there are lots of
# exceptions for files that aren't complaint.
HOSTCFLAGS_crc32.o := -pedantic
HOSTCFLAGS_md5.o := -pedantic
HOSTCFLAGS_sha1.o := -pedantic
HOSTCFLAGS_sha256.o := -pedantic

quiet_cmd_wrap = WRAP    $@
cmd_wrap = echo "\#include <../$(patsubst $(obj)/%,%,$@)>" >$@

$(obj)/lib/%.c $(obj)/common/%.c:
	$(call cmd,wrap)

clean-dirs := lib common

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

# Generated gziped GPL-2.0 license text
LICENSE_H = $(objtree)/include/license.h
LICENSE-$(CONFIG_CMD_LICENSE) += $(LICENSE_H)

#
# Use native tools and options
# Define __KERNEL_STRICT_NAMES to prevent typedef overlaps
# Define _GNU_SOURCE to obtain the getline prototype from stdio.h
#
HOST_EXTRACFLAGS += -include $(srctree)/include/libfdt_env.h \
		$(patsubst -I%,-idirafter%, $(filter -I%, $(UBOOTINCLUDE))) \
		-I$(srctree)/lib/libfdt \
		-I$(srctree)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES \
		-D_GNU_SOURCE

__build:	$(LOGO-y) $(LICENSE-y)

$(LOGO_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-info $(LOGO_BMP) > $@

$(LOGO_DATA_H):	$(obj)/bmp_logo $(LOGO_BMP)
	$(obj)/bmp_logo --gen-data $(LOGO_BMP) > $@

$(LICENSE_H): $(obj)/bin2header $(srctree)/Licenses/gpl-2.0.txt
	cat $(srctree)/Licenses/gpl-2.0.txt | gzip -9 -c | \
		$(obj)/bin2header license_gzip > $(LICENSE_H)

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
