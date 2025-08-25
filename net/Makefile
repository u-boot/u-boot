# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

#ccflags-y += -DDEBUG

ifeq ($(CONFIG_NET),y)

obj-$(CONFIG_NET)      += arp.o
obj-$(CONFIG_CMD_BOOTP) += bootp.o
obj-$(CONFIG_CMD_CDP)  += cdp.o
obj-$(CONFIG_DNS)  += dns.o
obj-$(CONFIG_CMD_LINK_LOCAL) += link_local.o
obj-$(CONFIG_IPV6)     += ndisc.o
obj-$(CONFIG_$(PHASE_)DM_ETH) += net.o
obj-$(CONFIG_IPV6)     += net6.o
obj-$(CONFIG_CMD_NFS)  += nfs.o
obj-$(CONFIG_CMD_PING) += ping.o
obj-$(CONFIG_CMD_PING6) += ping6.o
obj-$(CONFIG_CMD_DHCP6) += dhcpv6.o
obj-$(CONFIG_CMD_PCAP) += pcap.o
obj-$(CONFIG_CMD_RARP) += rarp.o
obj-$(CONFIG_CMD_SNTP) += sntp.o
obj-$(CONFIG_CMD_TFTPBOOT) += tftp.o
obj-$(CONFIG_$(PHASE_)UDP_FUNCTION_FASTBOOT)  += fastboot_udp.o
obj-$(CONFIG_$(PHASE_)TCP_FUNCTION_FASTBOOT)  += fastboot_tcp.o
obj-$(CONFIG_CMD_WOL)  += wol.o
obj-$(CONFIG_PROT_UDP) += udp.o
obj-$(CONFIG_PROT_TCP) += tcp.o
obj-$(CONFIG_WGET) += wget.o

# Disable this warning as it is triggered by:
# sprintf(buf, index ? "foo%d" : "foo", index)
# and this is intentional usage.
CFLAGS_eth_common.o += -Wno-format-extra-args

endif

ifeq ($(filter y,$(CONFIG_NET) $(CONFIG_NET_LWIP)),y)
obj-$(CONFIG_DM_DSA)   += dsa-uclass.o
obj-$(CONFIG_$(PHASE_)DM_ETH) += eth-uclass.o
obj-$(CONFIG_$(PHASE_)BOOTDEV_ETH) += eth_bootdev.o
obj-$(CONFIG_DM_MDIO)  += mdio-uclass.o
obj-$(CONFIG_DM_MDIO_MUX) += mdio-mux-uclass.o
obj-$(CONFIG_$(PHASE_)DM_ETH) += eth_common.o
obj-y += net-common.o
endif

obj-$(CONFIG_NET_LWIP) += lwip/
