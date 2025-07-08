# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2024 Linaro Ltd.

obj-y += \
	lwip/src/api/api_lib.o \
	lwip/src/api/api_msg.o \
	lwip/src/api/err.o \
	lwip/src/api/if_api.o \
	lwip/src/api/netbuf.o \
	lwip/src/api/netdb.o \
	lwip/src/api/netifapi.o \
	lwip/src/api/sockets.o \
	lwip/src/api/tcpip.o \
	lwip/src/apps/http/http_client.o \
	lwip/src/apps/sntp/sntp.o \
	lwip/src/apps/tftp/tftp.o \
	lwip/src/core/altcp_alloc.o \
	lwip/src/core/altcp.o \
	lwip/src/core/altcp_tcp.o \
	lwip/src/core/def.o \
	lwip/src/core/dns.o \
	lwip/src/core/inet_chksum.o \
	lwip/src/core/init.o \
	lwip/src/core/ip.o \
	lwip/src/core/ipv4/acd.o \
	lwip/src/core/ipv4/autoip.o \
	lwip/src/core/ipv4/dhcp.o \
	lwip/src/core/ipv4/etharp.o \
	lwip/src/core/ipv4/icmp.o \
	lwip/src/core/ipv4/igmp.o \
	lwip/src/core/ipv4/ip4_addr.o \
	lwip/src/core/ipv4/ip4.o \
	lwip/src/core/ipv4/ip4_frag.o \
	lwip/src/core/ipv6/dhcp6.o \
	lwip/src/core/ipv6/ethip6.o \
	lwip/src/core/ipv6/icmp6.o \
	lwip/src/core/ipv6/inet6.o \
	lwip/src/core/ipv6/ip6_addr.o \
	lwip/src/core/ipv6/ip6.o \
	lwip/src/core/ipv6/ip6_frag.o \
	lwip/src/core/ipv6/mld6.o \
	lwip/src/core/ipv6/nd6.o \
	lwip/src/core/mem.o \
	lwip/src/core/memp.o \
	lwip/src/core/netif.o \
	lwip/src/core/pbuf.o \
	lwip/src/core/raw.o \
	lwip/src/core/stats.o \
	lwip/src/core/sys.o \
	lwip/src/core/tcp.o \
	lwip/src/core/tcp_in.o \
	lwip/src/core/tcp_out.o \
	lwip/src/core/timeouts.o \
	lwip/src/core/udp.o \
	lwip/src/netif/ethernet.o

obj-$(CONFIG_$(PHASE_)MBEDTLS_LIB_TLS) += lwip/src/apps/altcp_tls/altcp_tls_mbedtls.o \
	lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.o
