/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __NET_H__
#define __NET_H__

#include <net-common.h>

#if defined(CONFIG_NET_LWIP)
#include <net-lwip.h>
#else
#include <net-legacy.h>
#endif

#endif /* __NET_H__ */
