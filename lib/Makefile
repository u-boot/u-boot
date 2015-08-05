#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD

obj-$(CONFIG_RSA) += rsa/
obj-$(CONFIG_LZMA) += lzma/
obj-$(CONFIG_LZO) += lzo/
obj-$(CONFIG_ZLIB) += zlib/
obj-$(CONFIG_BZIP2) += bzip2/
obj-$(CONFIG_TIZEN) += tizen/
obj-$(CONFIG_OF_LIBFDT) += libfdt/
obj-$(CONFIG_FIT) += libfdt/

obj-$(CONFIG_AES) += aes.o
obj-$(CONFIG_USB_TTY) += circbuf.o
obj-y += crc7.o
obj-y += crc8.o
obj-y += crc16.o
obj-$(CONFIG_FIT) += fdtdec_common.o
obj-$(CONFIG_OF_CONTROL) += fdtdec_common.o
obj-$(CONFIG_OF_CONTROL) += fdtdec.o
obj-$(CONFIG_TEST_FDTDEC) += fdtdec_test.o
obj-$(CONFIG_GZIP) += gunzip.o
obj-$(CONFIG_GZIP_COMPRESSED) += gzip.o
obj-y += initcall.o
obj-$(CONFIG_LMB) += lmb.o
obj-y += ldiv.o
obj-$(CONFIG_MD5) += md5.o
obj-y += net_utils.o
obj-$(CONFIG_PHYSMEM) += physmem.o
obj-y += qsort.o
obj-$(CONFIG_SHA1) += sha1.o
obj-$(CONFIG_SUPPORT_EMMC_RPMB) += sha256.o
obj-$(CONFIG_SHA256) += sha256.o
obj-y	+= strmhz.o
obj-$(CONFIG_TPM) += tpm.o
obj-$(CONFIG_RBTREE)	+= rbtree.o
obj-$(CONFIG_BITREVERSE) += bitrev.o
obj-y += list_sort.o
endif

ifndef CONFIG_SPL_DISABLE_OF_CONTROL
obj-$(CONFIG_OF_LIBFDT) += libfdt/
obj-$(CONFIG_OF_CONTROL) += fdtdec_common.o
obj-$(CONFIG_OF_CONTROL) += fdtdec.o
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_YMODEM_SUPPORT) += crc16.o
obj-$(CONFIG_SPL_NET_SUPPORT) += net_utils.o
endif
obj-$(CONFIG_ADDR_MAP) += addr_map.o
obj-y += hashtable.o
obj-y += errno.o
obj-$(CONFIG_ERRNO_STR) += errno_str.o
obj-y += display_options.o
obj-$(CONFIG_BCH) += bch.o
obj-y += crc32.o
obj-y += ctype.o
obj-y += div64.o
obj-y += hang.o
obj-y += linux_compat.o
obj-y += linux_string.o
obj-$(CONFIG_REGEX) += slre.o
obj-y += string.o
obj-y += time.o
obj-$(CONFIG_TRACE) += trace.o
obj-$(CONFIG_LIB_UUID) += uuid.o
obj-y += vsprintf.o
obj-$(CONFIG_LIB_RAND) += rand.o

subdir-ccflags-$(CONFIG_CC_OPTIMIZE_LIBS_FOR_SPEED) += -O2
