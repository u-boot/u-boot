#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#ccflags-y += -DDEBUG

obj-y += checksum.o
obj-$(CONFIG_CMD_NET)  += arp.o
obj-$(CONFIG_CMD_NET)  += bootp.o
obj-$(CONFIG_CMD_CDP)  += cdp.o
obj-$(CONFIG_CMD_DNS)  += dns.o
ifdef CONFIG_DM_ETH
obj-$(CONFIG_CMD_NET)  += eth-uclass.o
else
obj-$(CONFIG_CMD_NET)  += eth_legacy.o
endif
obj-$(CONFIG_CMD_NET)  += eth_common.o
obj-$(CONFIG_CMD_LINK_LOCAL) += link_local.o
obj-$(CONFIG_CMD_NET)  += net.o
obj-$(CONFIG_CMD_NFS)  += nfs.o
obj-$(CONFIG_CMD_PING) += ping.o
obj-$(CONFIG_CMD_RARP) += rarp.o
obj-$(CONFIG_CMD_SNTP) += sntp.o
obj-$(CONFIG_CMD_NET)  += tftp.o

# Disable this warning as it is triggered by:
# sprintf(buf, index ? "foo%d" : "foo", index)
# and this is intentional usage.
CFLAGS_eth_common.o += -Wno-format-extra-args
