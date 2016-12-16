#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD

obj-$(CONFIG_EFI) += efi/
obj-$(CONFIG_EFI_LOADER) += efi_loader/
obj-$(CONFIG_LZMA) += lzma/
obj-$(CONFIG_LZO) += lzo/
obj-$(CONFIG_ZLIB) += zlib/
obj-$(CONFIG_BZIP2) += bzip2/
obj-$(CONFIG_TIZEN) += tizen/
obj-$(CONFIG_FIT) += libfdt/
obj-$(CONFIG_CMD_DHRYSTONE) += dhry/

obj-$(CONFIG_AES) += aes.o
obj-$(CONFIG_USB_TTY) += circbuf.o
obj-y += crc7.o
obj-y += crc8.o
obj-y += crc16.o
obj-$(CONFIG_ERRNO_STR) += errno_str.o
obj-$(CONFIG_FIT) += fdtdec_common.o
obj-$(CONFIG_TEST_FDTDEC) += fdtdec_test.o
obj-$(CONFIG_GZIP) += gunzip.o
obj-$(CONFIG_GZIP_COMPRESSED) += gzip.o
obj-$(CONFIG_GENERATE_SMBIOS_TABLE) += smbios.o
obj-y += initcall.o
obj-$(CONFIG_LMB) += lmb.o
obj-y += ldiv.o
obj-$(CONFIG_LZ4) += lz4_wrapper.o
obj-$(CONFIG_MD5) += md5.o
obj-y += net_utils.o
obj-$(CONFIG_PHYSMEM) += physmem.o
obj-y += qsort.o
obj-y += rc4.o
obj-$(CONFIG_SUPPORT_EMMC_RPMB) += sha256.o
obj-$(CONFIG_TPM) += tpm.o
obj-$(CONFIG_RBTREE)	+= rbtree.o
obj-$(CONFIG_BITREVERSE) += bitrev.o
obj-y += list_sort.o
endif

obj-$(CONFIG_$(SPL_)RSA) += rsa/
obj-$(CONFIG_$(SPL_)SHA1) += sha1.o
obj-$(CONFIG_$(SPL_)SHA256) += sha256.o

obj-$(CONFIG_SPL_SAVEENV) += qsort.o
obj-$(CONFIG_$(SPL_)OF_LIBFDT) += libfdt/
ifneq ($(CONFIG_SPL_BUILD)$(CONFIG_SPL_OF_PLATDATA),yy)
obj-$(CONFIG_$(SPL_)OF_CONTROL) += fdtdec_common.o
obj-$(CONFIG_$(SPL_)OF_CONTROL) += fdtdec.o
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_YMODEM_SUPPORT) += crc16.o
obj-$(CONFIG_SPL_NET_SUPPORT) += net_utils.o
endif
obj-$(CONFIG_ADDR_MAP) += addr_map.o
obj-y += hashtable.o
obj-y += errno.o
obj-y += display_options.o
CFLAGS_display_options.o := $(if $(BUILD_TAG),-DBUILD_TAG='"$(BUILD_TAG)"')
obj-$(CONFIG_BCH) += bch.o
obj-y += crc32.o
obj-y += ctype.o
obj-y += div64.o
obj-y += hang.o
obj-y += linux_compat.o
obj-y += linux_string.o
obj-y += membuff.o
obj-$(CONFIG_REGEX) += slre.o
obj-y += string.o
obj-y += tables_csum.o
obj-y += time.o
obj-$(CONFIG_TRACE) += trace.o
obj-$(CONFIG_LIB_UUID) += uuid.o
obj-$(CONFIG_LIB_RAND) += rand.o

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TPL_BUILD
SERIAL_SUPPORT := $(CONFIG_TPL_SERIAL_SUPPORT)
else
SERIAL_SUPPORT := $(CONFIG_SPL_SERIAL_SUPPORT)
endif
# SPL U-Boot may use full-printf, tiny-printf or none at all
ifdef CONFIG_USE_TINY_PRINTF
obj-$(SERIAL_SUPPORT) += tiny-printf.o panic.o strto.o
else
obj-$(SERIAL_SUPPORT) += vsprintf.o panic.o strto.o strmhz.o
endif
else
# Main U-Boot always uses the full printf support
obj-y += vsprintf.o panic.o strto.o strmhz.o
endif

subdir-ccflags-$(CONFIG_CC_OPTIMIZE_LIBS_FOR_SPEED) += -O2
