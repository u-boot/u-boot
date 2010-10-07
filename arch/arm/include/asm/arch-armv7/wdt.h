/*
 * (C) Copyright 2010
 * Matt Waddel, <matt.waddel@linaro.org>
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
#ifndef _WDT_H_
#define _WDT_H_

/* Watchdog timer (SP805) register base address */
#define WDT_BASE	0x100E5000

#define WDT_EN		0x2
#define WDT_RESET_LOAD	0x0

struct wdt {
	u32 wdogload;		/* 0x000 */
	u32 wdogvalue;
	u32 wdogcontrol;
	u32 wdogintclr;
	u32 wdogris;
	u32 wdogmis;
	u32 res1[0x2F9];
	u32 wdoglock;		/* 0xC00 */
	u32 res2[0xBE];
	u32 wdogitcr;		/* 0xF00 */
	u32 wdogitop;
	u32 res3[0x35];
	u32 wdogperiphid0;	/* 0xFE0 */
	u32 wdogperiphid1;
	u32 wdogperiphid2;
	u32 wdogperiphid3;
	u32 wdogpcellid0;
	u32 wdogpcellid1;
	u32 wdogpcellid2;
	u32 wdogpcellid3;
};

#endif /* _WDT_H_ */
