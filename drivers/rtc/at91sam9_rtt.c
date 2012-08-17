/*
 * (C) Copyright 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Date & Time support for the internal Real-time Timer
 * of AT91SAM9260 and compatibles.
 * Compatible with the LinuX rtc driver workaround:
 * The RTT cannot be written to, but only reset.
 * The actual time is the sum of RTT and one of
 * the four GPBR registers.
 *
 * The at91sam9260 has 4 GPBR (0-3).
 * For their typical use see at91_gpbr.h !
 *
 * make sure u-boot and kernel use the same GPBR !
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_rtt.h>
#include <asm/arch/at91_gpbr.h>

#if defined(CONFIG_CMD_DATE)

int rtc_get (struct rtc_time *tmp)
{
	at91_rtt_t *rtt = (at91_rtt_t *) ATMEL_BASE_RTT;
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;
	ulong tim;
	ulong tim2;
	ulong off;

	do {
		tim = readl(&rtt->vr);
		tim2 = readl(&rtt->vr);
	} while (tim!=tim2);
	off = readl(&gpbr->reg[AT91_GPBR_INDEX_TIMEOFF]);
	/* off==0 means time is invalid, but we ignore that */
	to_tm (tim+off, tmp);
	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
	at91_rtt_t *rtt = (at91_rtt_t *) ATMEL_BASE_RTT;
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;
	ulong tim;

	tim = mktime (tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
		      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	/* clear alarm, set prescaler to 32768, clear counter */
	writel(32768+AT91_RTT_RTTRST, &rtt->mr);
	writel(~0, &rtt->ar);
	writel(tim, &gpbr->reg[AT91_GPBR_INDEX_TIMEOFF]);
	/* wait for counter clear to happen, takes less than a 1/32768th second */
	while (readl(&rtt->vr) != 0)
		;
	return 0;
}

void rtc_reset (void)
{
	at91_rtt_t *rtt = (at91_rtt_t *) ATMEL_BASE_RTT;
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	/* clear alarm, set prescaler to 32768, clear counter */
	writel(32768+AT91_RTT_RTTRST, &rtt->mr);
	writel(~0, &rtt->ar);
	writel(0, &gpbr->reg[AT91_GPBR_INDEX_TIMEOFF]);
	/* wait for counter clear to happen, takes less than a 1/32768th second */
	while (readl(&rtt->vr) != 0)
		;
}

#endif
