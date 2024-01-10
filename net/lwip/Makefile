# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>

ccflags-y += -I$(srctree)/net/lwip/port/include
ccflags-y += -I$(srctree)/net/lwip/lwip-external/src/include -I$(srctree)/net/lwip

obj-y += lwip-external/src/core/init.o \
	lwip-external/src/core/def.o \
	lwip-external/src/core/dns.o \
	lwip-external/src/core/inet_chksum.o \
	lwip-external/src/core/ip.o \
	lwip-external/src/core/mem.o \
	lwip-external/src/core/memp.o \
	lwip-external/src/core/netif.o \
	lwip-external/src/core/pbuf.o \
	lwip-external/src/core/raw.o \
	lwip-external/src/core/stats.o \
	lwip-external/src/core/sys.o \
	lwip-external/src/core/altcp.o \
	lwip-external/src/core/altcp_alloc.o \
	lwip-external/src/core/altcp_tcp.o \
	lwip-external/src/core/tcp.o \
	lwip-external/src/core/tcp_in.o \
	lwip-external/src/core/tcp_out.o \
	lwip-external/src/core/timeouts.o \
	lwip-external/src/core/udp.o

# IPv4
obj-y += lwip-external/src/core/ipv4/acd.o \
        lwip-external/src/core/ipv4/autoip.o \
        lwip-external/src/core/ipv4/dhcp.o \
        lwip-external/src/core/ipv4/etharp.o \
        lwip-external/src/core/ipv4/icmp.o \
        lwip-external/src/core/ipv4/igmp.o \
        lwip-external/src/core/ipv4/ip4_frag.o \
        lwip-external/src/core/ipv4/ip4.o \
        lwip-external/src/core/ipv4/ip4_addr.o
# IPv6
obj-y += lwip-external/src/core/ipv6/dhcp6.o \
        lwip-external/src/core/ipv6/ethip6.o \
        lwip-external/src/core/ipv6/icmp6.o \
        lwip-external/src/core/ipv6/inet6.o \
        lwip-external/src/core/ipv6/ip6.o \
        lwip-external/src/core/ipv6/ip6_addr.o \
        lwip-external/src/core/ipv6/ip6_frag.o \
        lwip-external/src/core/ipv6/mld6.o \
        lwip-external/src/core/ipv6/nd6.o
# API
obj-y += lwip-external/src/api/api_lib.o \
	lwip-external/src/api/api_msg.o \
	lwip-external/src/api/err.o \
	lwip-external/src/api/if_api.o \
	lwip-external/src/api/netbuf.o \
	lwip-external/src/api/netdb.o \
	lwip-external/src/api/netifapi.o \
	lwip-external/src/api/sockets.o \
	lwip-external/src/api/tcpip.o

# Netdevs
obj-y += lwip-external/src/netif/ethernet.o

obj-y += port/if.o
obj-y += port/ulwip.o
obj-y += port/sys-arch.o

obj-y += apps/dhcp/lwip-dhcp.o
obj-y += apps/dns/lwip-dns.o
obj-y += apps/tftp/
obj-y += apps/http/
obj-y += apps/ping/
obj-y += apps/list/lwip-list.o
