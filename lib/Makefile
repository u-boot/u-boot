#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libgeneric.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_AES) += aes.o
COBJS-$(CONFIG_BZIP2) += bzlib.o
COBJS-$(CONFIG_BZIP2) += bzlib_crctable.o
COBJS-$(CONFIG_BZIP2) += bzlib_decompress.o
COBJS-$(CONFIG_BZIP2) += bzlib_randtable.o
COBJS-$(CONFIG_BZIP2) += bzlib_huffman.o
COBJS-$(CONFIG_USB_TTY) += circbuf.o
COBJS-y += crc7.o
COBJS-y += crc16.o
COBJS-$(CONFIG_OF_CONTROL) += fdtdec.o
COBJS-$(CONFIG_TEST_FDTDEC) += fdtdec_test.o
COBJS-$(CONFIG_GZIP) += gunzip.o
COBJS-$(CONFIG_GZIP_COMPRESSED) += gzip.o
COBJS-y += initcall.o
COBJS-$(CONFIG_LMB) += lmb.o
COBJS-y += ldiv.o
COBJS-$(CONFIG_MD5) += md5.o
COBJS-y += net_utils.o
COBJS-$(CONFIG_PHYSMEM) += physmem.o
COBJS-y += qsort.o
COBJS-$(CONFIG_SHA1) += sha1.o
COBJS-$(CONFIG_SHA256) += sha256.o
COBJS-y	+= strmhz.o
COBJS-$(CONFIG_TPM) += tpm.o
COBJS-$(CONFIG_RBTREE)	+= rbtree.o
COBJS-$(CONFIG_BITREVERSE) += bitrev.o
endif

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_YMODEM_SUPPORT) += crc16.o
COBJS-$(CONFIG_SPL_NET_SUPPORT) += net_utils.o
endif
COBJS-$(CONFIG_ADDR_MAP) += addr_map.o
COBJS-y += hashtable.o
COBJS-y += errno.o
COBJS-y += display_options.o
COBJS-$(CONFIG_BCH) += bch.o
COBJS-y += crc32.o
COBJS-y += ctype.o
COBJS-y += div64.o
COBJS-y += hang.o
COBJS-y += linux_string.o
COBJS-$(CONFIG_REGEX) += slre.o
COBJS-y += string.o
COBJS-y += time.o
COBJS-$(CONFIG_TRACE) += trace.o
COBJS-$(CONFIG_BOOTP_PXE) += uuid.o
COBJS-y += vsprintf.o
COBJS-$(CONFIG_RANDOM_MACADDR) += rand.o
COBJS-$(CONFIG_BOOTP_RANDOM_DELAY) += rand.o
COBJS-$(CONFIG_CMD_LINK_LOCAL) += rand.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# SEE README.arm-unaligned-accesses
$(obj)bzlib.o: CFLAGS += $(PLATFORM_NO_UNALIGNED)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
