/*
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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

#ifndef __ASM_ARM_ARCH_PWM_H_
#define __ASM_ARM_ARCH_PWM_H_

/* PWM timer addressing */
#define S5PC100_TIMER_BASE	S5PC100_PWMTIMER_BASE
#define S5PC110_TIMER_BASE	S5PC110_PWMTIMER_BASE

/* Interval mode(Auto Reload) of PWM Timer 4 */
#define S5PC1XX_TCON4_AUTO_RELOAD	(1 << 22)
/* Update TCNTB4 */
#define S5PC1XX_TCON4_UPDATE		(1 << 21)
/* start bit of PWM Timer 4 */
#define S5PC1XX_TCON4_START		(1 << 20)

#ifndef __ASSEMBLY__
struct s5pc1xx_timer {
	unsigned long	tcfg0;
	unsigned long	tcfg1;
	unsigned long	tcon;
	unsigned long	tcntb0;
	unsigned long	tcmpb0;
	unsigned long	tcnto0;
	unsigned long	tcntb1;
	unsigned long	tcmpb1;
	unsigned long	tcnto1;
	unsigned long	tcntb2;
	unsigned long	tcmpb2;
	unsigned long	tcnto2;
	unsigned long	tcntb3;
	unsigned long	res1;
	unsigned long	tcnto3;
	unsigned long	tcntb4;
	unsigned long	tcnto4;
	unsigned long	tintcstat;
};
#endif	/* __ASSEMBLY__ */

#endif
