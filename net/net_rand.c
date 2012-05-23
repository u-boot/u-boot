/*
 *	Based on LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2004 Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <net.h>
#include "net_rand.h"

static ulong seed1, seed2;

void srand_mac(void)
{
	ulong tst1, tst2, m_mask;
	ulong m_value = 0;
	int reg;
	unsigned char bi_enetaddr[6];

	/* get our mac */
	eth_getenv_enetaddr("ethaddr", bi_enetaddr);

	debug("BootpRequest => Our Mac: ");
	for (reg = 0; reg < 6; reg++)
		debug("%x%c", bi_enetaddr[reg], reg == 5 ? '\n' : ':');

	/* Mac-Manipulation 2 get seed1 */
	tst1 = 0;
	tst2 = 0;
	for (reg = 2; reg < 6; reg++) {
		tst1 = tst1 << 8;
		tst1 = tst1 | bi_enetaddr[reg];
	}
	for (reg = 0; reg < 2; reg++) {
		tst2 = tst2 | bi_enetaddr[reg];
		tst2 = tst2 << 8;
	}

	seed1 = tst1^tst2;

	/* Mirror seed1*/
	m_mask = 0x1;
	for (reg = 1; reg <= 32; reg++) {
		m_value |= (m_mask & seed1);
		seed1 = seed1 >> 1;
		m_value = m_value << 1;
	}
	seed1 = m_value;
	seed2 = 0xb78d0945;
}

unsigned long rand(void)
{
	ulong sum;

	/* Random Number Generator */
	sum = seed1 + seed2;
	if (sum < seed1 || sum < seed2)
		sum++;
	seed2 = seed1;
	seed1 = sum;

	return sum;
}
