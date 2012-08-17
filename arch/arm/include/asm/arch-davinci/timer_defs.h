/*
 * Copyright (C) 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
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
#ifndef _TIMER_DEFS_H_
#define _TIMER_DEFS_H_

struct davinci_timer {
	u_int32_t	pid12;
	u_int32_t	emumgt;
	u_int32_t	na1;
	u_int32_t	na2;
	u_int32_t	tim12;
	u_int32_t	tim34;
	u_int32_t	prd12;
	u_int32_t	prd34;
	u_int32_t	tcr;
	u_int32_t	tgcr;
	u_int32_t	wdtcr;
};

#define DV_TIMER_TCR_ENAMODE_MASK		3

#define DV_TIMER_TCR_ENAMODE12_SHIFT		6
#define DV_TIMER_TCR_CLKSRC12_SHIFT		8
#define DV_TIMER_TCR_READRSTMODE12_SHIFT	10
#define DV_TIMER_TCR_CAPMODE12_SHIFT		11
#define DV_TIMER_TCR_CAPVTMODE12_SHIFT		12
#define DV_TIMER_TCR_ENAMODE34_SHIFT		22
#define DV_TIMER_TCR_CLKSRC34_SHIFT		24
#define DV_TIMER_TCR_READRSTMODE34_SHIFT	26
#define DV_TIMER_TCR_CAPMODE34_SHIFT		27
#define DV_TIMER_TCR_CAPEVTMODE12_SHIFT		28

#define DV_WDT_ENABLE_SYS_RESET		0x00020000
#define DV_WDT_TRIGGER_SYS_RESET	0x00020002

#ifdef CONFIG_HW_WATCHDOG
void davinci_hw_watchdog_enable(void);
void davinci_hw_watchdog_reset(void);
#endif
#endif /* _TIMER_DEFS_H_ */
