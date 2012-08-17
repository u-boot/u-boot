/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* FTAHBC020S - AHB Controller (Arbiter/Decoder) definitions */
#ifndef __FTAHBC020S_H
#define __FTAHBC202S_H

/* Registers Offsets */

/*
 * AHB Slave BSR, offset: n * 4, n=0~31
 */
#ifndef __ASSEMBLY__
struct ftahbc02s {
	unsigned int	s_bsr[32];	/* 0x00-0x7c - Slave n Base/Size Reg */
	unsigned int	pcr;		/* 0x80	- Priority Ctrl Reg */
	unsigned int	tcrg;		/* 0x84	- Transfer Ctrl Reg */
	unsigned int	cr;		/* 0x88	- Ctrl Reg */
};
#endif /* __ASSEMBLY__ */

/*
 * FTAHBC020S_SLAVE_BSR - Slave n Base / Size Register
 */
#define FTAHBC020S_SLAVE_BSR_BASE(x)	(((x) & 0xfff) << 20)
#define FTAHBC020S_SLAVE_BSR_SIZE(x)	(((x) & 0xf) << 16)
/* The value of b(16:19)SLAVE_BSR_SIZE: 1M-2048M, must be power of 2 */
#define FTAHBC020S_BSR_SIZE(x)		(ffs(x) - 1)	/* size of Addr Space */

/*
 * FTAHBC020S_PCR - Priority Control Register
 */
#define FTAHBC020S_PCR_PLEVEL_(x)	(1 << (x))	/* x: 1-15 */

/*
 * FTAHBC020S_CR - Interrupt Control Register
 */
#define FTAHBC020S_CR_INTSTS	(1 << 24)
#define FTAHBC020S_CR_RESP(x)	(((x) & 0x3) << 20)
#define FTAHBC020S_CR_INTSMASK	(1 << 16)
#define FTAHBC020S_CR_REMAP	(1 << 0)

#endif	/* __FTAHBC020S_H */
