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

include $(TOPDIR)/config.mk

LIB	= $(obj)libgeneric.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_ADDR_MAP) += addr_map.o
COBJS-$(CONFIG_BCH) += bch.o
COBJS-$(CONFIG_AES) += aes.o
COBJS-$(CONFIG_BZIP2) += bzlib.o
COBJS-$(CONFIG_BZIP2) += bzlib_crctable.o
COBJS-$(CONFIG_BZIP2) += bzlib_decompress.o
COBJS-$(CONFIG_BZIP2) += bzlib_randtable.o
COBJS-$(CONFIG_BZIP2) += bzlib_huffman.o
COBJS-$(CONFIG_USB_TTY) += circbuf.o
COBJS-y += crc7.o
COBJS-y += crc16.o
COBJS-y += display_options.o
COBJS-y += errno.o
COBJS-$(CONFIG_OF_CONTROL) += fdtdec.o
COBJS-$(CONFIG_TEST_FDTDEC) += fdtdec_test.o
COBJS-$(CONFIG_GZIP) += gunzip.o
COBJS-$(CONFIG_GZIP_COMPRESSED) += gzip.o
COBJS-y += hashtable.o
COBJS-$(CONFIG_LMB) += lmb.o
COBJS-y += ldiv.o
COBJS-$(CONFIG_MD5) += md5.o
COBJS-y += net_utils.o
COBJS-y += qsort.o
COBJS-$(CONFIG_SHA1) += sha1.o
COBJS-$(CONFIG_SHA256) += sha256.o
COBJS-y	+= strmhz.o
COBJS-$(CONFIG_RBTREE)	+= rbtree.o
endif

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_YMODEM_SUPPORT) += crc16.o
COBJS-$(CONFIG_SPL_NET_SUPPORT) += crc32.o
ifneq ($(CONFIG_SPL_SPI_FLASH_SUPPORT)$(CONFIG_SPL_NET_SUPPORT),)
COBJS-y += display_options.o
endif
COBJS-$(CONFIG_SPL_NET_SUPPORT) += errno.o
COBJS-$(CONFIG_SPL_NET_SUPPORT) += hashtable.o
COBJS-$(CONFIG_SPL_NET_SUPPORT) += net_utils.o
endif
COBJS-y += crc32.o
COBJS-y += ctype.o
COBJS-y += div64.o
COBJS-y += string.o
COBJS-y += time.o
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

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
