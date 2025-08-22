/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2023 Linaro Ltd. <maxim.uvarov@linaro.org> */

#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <vsprintf.h>
#include <rand.h>

#define LWIP_ERRNO_INCLUDE <errno.h>

#define LWIP_ERRNO_STDINCLUDE	1
#define LWIP_NO_UNISTD_H 1
#define LWIP_TIMEVAL_PRIVATE 1

#ifdef CONFIG_LIB_RAND
#define LWIP_RAND() ((u32_t)rand())
#else
#define LWIP_DNS_SECURE 0
#endif

/* different handling for unit test, normally not needed */
#ifdef LWIP_NOASSERT_ON_ERROR
#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
						handler; }} while (0)
#endif

#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#define LWIP_PLATFORM_ASSERT(x) do { \
	printf("Assertion \"%s\" failed at line %d in %s\n", \
	       x, __LINE__, __FILE__); } while (0)

#define atoi(str) (int)dectoul(str, NULL)
#define lwip_strnstr(a, b, c)  strnstr(a, b, c)

#define LWIP_ERR_T int
#define LWIP_CONST_CAST(target_type, val) ((target_type)((uintptr_t)val))

#if defined(CONFIG_SYS_BIG_ENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#endif

#define SNTP_STARTUP_DELAY 0
void sntp_set_system_time(uint32_t sec);
#define SNTP_SET_SYSTEM_TIME(sec) sntp_set_system_time(sec)

static inline const char *sntp_format_time(time_t t)
{
	static char buf[29]; /* "(time_t)" + 20 digits max + \0 */

	snprintf(buf, sizeof(buf), "(time_t)%llu", t);
	return buf;
}

#define sntp_format_time sntp_format_time

#ifdef CONFIG_LWIP_ICMP_SHOW_UNREACH
struct pbuf;
void net_lwip_icmp_dest_unreach(int code, struct pbuf *p);

#define ICMP_DEST_UNREACH_CB(_c, _p) net_lwip_icmp_dest_unreach(_c, _p)
#endif
#endif /* LWIP_ARCH_CC_H */
