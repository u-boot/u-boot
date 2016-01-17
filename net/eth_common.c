/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include "eth_internal.h"

void eth_parse_enetaddr(const char *addr, uchar *enetaddr)
{
	char *end;
	int i;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

int eth_getenv_enetaddr(const char *name, uchar *enetaddr)
{
	eth_parse_enetaddr(getenv(name), enetaddr);
	return is_valid_ethaddr(enetaddr);
}

int eth_setenv_enetaddr(const char *name, const uchar *enetaddr)
{
	char buf[20];

	sprintf(buf, "%pM", enetaddr);

	return setenv(name, buf);
}

int eth_getenv_enetaddr_by_index(const char *base_name, int index,
				 uchar *enetaddr)
{
	char enetvar[32];
	sprintf(enetvar, index ? "%s%daddr" : "%saddr", base_name, index);
	return eth_getenv_enetaddr(enetvar, enetaddr);
}

int eth_setenv_enetaddr_by_index(const char *base_name, int index,
				 uchar *enetaddr)
{
	char enetvar[32];
	sprintf(enetvar, index ? "%s%daddr" : "%saddr", base_name, index);
	return eth_setenv_enetaddr(enetvar, enetaddr);
}

void eth_common_init(void)
{
	bootstage_mark(BOOTSTAGE_ID_NET_ETH_START);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	miiphy_init();
#endif

#ifdef CONFIG_PHYLIB
	phy_init();
#endif
}
