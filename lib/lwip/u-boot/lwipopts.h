/* SPDX-License-Identifier: GPL-2.0+ */

/* Copyright (C) 2023 Linaro Ltd. <maxim.uvarov@linaro.org> */

#ifndef LWIP_UBOOT_LWIPOPTS_H
#define LWIP_UBOOT_LWIPOPTS_H

#include <linux/kconfig.h>

#if defined(CONFIG_LWIP_DEBUG)
#define LWIP_DEBUG 1
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
#define ETHARP_DEBUG                    LWIP_DBG_ON
#define NETIF_DEBUG                     LWIP_DBG_ON
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define API_LIB_DEBUG                   LWIP_DBG_ON
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#define ICMP_DEBUG                      LWIP_DBG_OFF
#define IGMP_DEBUG                      LWIP_DBG_OFF
#define INET_DEBUG                      LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_ON
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                       LWIP_DBG_OFF
#define MEM_DEBUG                       LWIP_DBG_OFF
#define MEMP_DEBUG                      LWIP_DBG_OFF
#define SYS_DEBUG                       LWIP_DBG_OFF
#define TIMERS_DEBUG                    LWIP_DBG_ON
#define TCP_DEBUG                       LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define SLIP_DEBUG                      LWIP_DBG_OFF
#define DHCP_DEBUG                      LWIP_DBG_ON
#define AUTOIP_DEBUG                    LWIP_DBG_ON
#define DNS_DEBUG                       LWIP_DBG_ON
#define IP6_DEBUG                       LWIP_DBG_OFF
#define DHCP6_DEBUG                     LWIP_DBG_OFF
#endif

#define LWIP_TESTMODE                   0

#if !defined(CONFIG_LWIP_ASSERT)
#define LWIP_NOASSERT 1
#define LWIP_ASSERT(message, assertion)
#endif

#include "lwip/debug.h"

#define SYS_LIGHTWEIGHT_PROT            0
#define NO_SYS                          1

#define LWIP_IPV4			1
#define LWIP_IPV6			0

#define MEM_ALIGNMENT                   8

#define MEMP_NUM_TCP_SEG                16
#define PBUF_POOL_SIZE                  8

#define LWIP_ARP                        1
#define ARP_TABLE_SIZE                  4
#define ARP_QUEUEING                    1

#define IP_FORWARD                      0
#define IP_OPTIONS_ALLOWED              1
#define IP_REASSEMBLY                   1
#define IP_FRAG                         1
#define IP_REASS_MAXAGE                 3
#define IP_REASS_MAX_PBUFS              4
#define IP_FRAG_USES_STATIC_BUF         0

#define IP_DEFAULT_TTL                  255

#if defined(CONFIG_PROT_ICMP_LWIP)
#define LWIP_ICMP                       1
#else
#define LWIP_ICMP                       0
#endif

#if defined(CONFIG_PROT_RAW_LWIP)
#define LWIP_RAW                        1
#else
#define LWIP_RAW			0
#endif

#if defined(CONFIG_PROT_DHCP_LWIP)
#define LWIP_DHCP                       1
#define LWIP_DHCP_BOOTP_FILE		1
#else
#define LWIP_DHCP			0
#endif

#define LWIP_DHCP_DOES_ACD_CHECK	0

#define LWIP_AUTOIP                     0

#define LWIP_SNMP                       0

#define LWIP_IGMP                       0

#if defined(CONFIG_PROT_DNS_LWIP)
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  1
#else
#define LWIP_DNS                        0
#endif

#if defined(CONFIG_PROT_UDP_LWIP)
#define LWIP_UDP                        1
#else
#define LWIP_UDP                        0
#endif

#if defined(CONFIG_PROT_TCP_LWIP)
#define LWIP_TCP                        1
#define TCP_MSS                         1460
#define TCP_WND                         CONFIG_LWIP_TCP_WND
#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   0x7
#define TCP_SND_BUF                     (2 * TCP_MSS)
#ifdef CONFIG_PROT_TCP_SACK_LWIP
#define LWIP_TCP_SACK_OUT               1
#endif
#else
#define LWIP_TCP                        0
#endif

#define LWIP_LISTEN_BACKLOG             0

#define PBUF_LINK_HLEN                  14
#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS + 40 + PBUF_LINK_HLEN)

#define LWIP_HAVE_LOOPIF                0

#define LWIP_NETCONN                    0
#define LWIP_DISABLE_MEMP_SANITY_CHECKS 1

#define LWIP_SOCKET                     0
#define SO_REUSE                        0

#define LWIP_STATS                      0

#define PPP_SUPPORT                     0

#define LWIP_TCPIP_CORE_LOCKING		0

#define LWIP_NETIF_LOOPBACK		0

/* use malloc instead of pool */
#define MEMP_MEM_MALLOC                 1
#define MEMP_MEM_INIT			1
#define MEM_LIBC_MALLOC			1

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_TLS)
#define LWIP_ALTCP                      1
#define LWIP_ALTCP_TLS                  1
#define LWIP_ALTCP_TLS_MBEDTLS          1
#endif

#if defined(CONFIG_CMD_SNTP)
#define LWIP_DHCP_GET_NTP_SRV 1
#endif

#endif /* LWIP_UBOOT_LWIPOPTS_H */
