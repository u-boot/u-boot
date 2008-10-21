/*
 *    Copyright (c) 2008 Nuovation System Designs, LLC
 *	Grant Erickson <gerickson@nuovations.com>
 *
 *    Copyright (c) 2007 DENX Software Engineering, GmbH
 *	Stefan Roese <sr@denx.de>
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will abe useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *	This file implements ECC initialization for PowerPC processors
 *	using the SDRAM DDR2 controller, including the 405EX(r),
 *	440SP(E), 460EX and 460GT.
 *
 */

#ifndef _ECC_H_
#define _ECC_H_

#if !defined(CONFIG_SYS_ECC_PATTERN)
#define	CONFIG_SYS_ECC_PATTERN	0x00000000
#endif /* !defined(CONFIG_SYS_ECC_PATTERN) */

/*
 * Since the IBM DDR controller used on 440GP/GX/EP/GR is not register
 * compatible to the IBM DDR/2 controller used on 405EX/440SP/SPe/460EX/GT
 * we need to make some processor dependant defines used later on by the
 * driver.
 */

/* For 440GP/GX/EP/GR */
#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)
#define SDRAM_ECC_CFG		SDRAM_CFG0
#define SDRAM_ECC_CFG_MCHK_MASK	SDRAM_CFG0_MCHK_MASK
#define SDRAM_ECC_CFG_MCHK_GEN	SDRAM_CFG0_MCHK_GEN
#define SDRAM_ECC_CFG_MCHK_CHK	SDRAM_CFG0_MCHK_CHK
#define SDRAM_ECC_CFG_DMWD_MASK	SDRAM_CFG0_DMWD_MASK
#define SDRAM_ECC_CFG_DMWD_32	SDRAM_CFG0_DMWD_32
#endif

/* For 405EX/440SP/SPe/460EX/GT */
#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)
#define SDRAM_ECC_CFG		SDRAM_MCOPT1
#define SDRAM_ECC_CFG_MCHK_MASK	SDRAM_MCOPT1_MCHK_MASK
#define SDRAM_ECC_CFG_MCHK_GEN	SDRAM_MCOPT1_MCHK_GEN
#define SDRAM_ECC_CFG_MCHK_CHK	SDRAM_MCOPT1_MCHK_CHK
#define SDRAM_ECC_CFG_DMWD_MASK	SDRAM_MCOPT1_DMWD_MASK
#define SDRAM_ECC_CFG_DMWD_32	SDRAM_MCOPT1_DMWD_32
#endif

extern void ecc_init(unsigned long * const start, unsigned long size);

#endif /* _ECC_H_ */
