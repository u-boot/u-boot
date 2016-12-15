/*
 * Copyright (C) 2012  Renesas Solutions Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License.
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

#ifndef __SH_TMU_H
#define __SH_TMU_H

#include <asm/types.h>

#if defined(CONFIG_CPU_SH3)
struct tmu_regs {
	u8	tocr;
	u8	reserved0;
	u8	tstr;
	u8	reserved1;
	u32	tcor0;
	u32	tcnt0;
	u16	tcr0;
	u16	reserved2;
	u32	tcor1;
	u32	tcnt1;
	u16	tcr1;
	u16	reserved3;
	u32	tcor2;
	u32	tcnt2;
	u16	tcr2;
	u16	reserved4;
	u32	tcpr2;
};
#endif /* CONFIG_CPU_SH3 */

#if defined(CONFIG_CPU_SH4) || defined(CONFIG_ARCH_RMOBILE)
struct tmu_regs {
	u32 reserved;
	u8  tstr;
	u8  reserved2[3];
	u32 tcor0;
	u32 tcnt0;
	u16 tcr0;
	u16 reserved3;
	u32 tcor1;
	u32 tcnt1;
	u16 tcr1;
	u16 reserved4;
	u32 tcor2;
	u32 tcnt2;
	u16 tcr2;
	u16 reserved5;
};
#endif /* CONFIG_CPU_SH4 */

static inline unsigned long get_tmu0_clk_rate(void)
{
	return CONFIG_SH_TMU_CLK_FREQ;
}

#endif	/* __SH_TMU_H */
