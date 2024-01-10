/* SPDX-License-Identifier: GPL-2.0 */

#include <net/ulwip.h>
#include "lwip/ip_addr.h"

void ping_tmo(void *arg);
void ping_init(const ip_addr_t *ping_addr);
void ping_stop(void);

#define LWIP_DEBUG 1 /* ping_time is under ifdef*/
#define PING_RESULT(cond) { \
	if (cond == 1) { \
		printf("host %s is alive\n", ipaddr_ntoa(addr)); \
		printf(" %"U32_F" ms\n", (sys_now() - ping_time)); \
		sys_untimeout(ping_tmo, NULL); \
                sys_check_timeouts(); \
		ping_stop(); \
		ulwip_exit(0); \
	} else { \
		printf("ping failed; host %s in not alive\n",\
		       ipaddr_ntoa(addr)); \
		sys_untimeout(ping_tmo, NULL); \
                sys_check_timeouts(); \
		ping_stop(); \
		ulwip_exit(-1); \
	} \
	} while (0);


