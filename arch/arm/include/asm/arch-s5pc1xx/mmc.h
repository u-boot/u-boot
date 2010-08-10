/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __ASM_ARCH_MMC_H_
#define __ASM_ARCH_MMC_H_

#ifndef __ASSEMBLY__
struct s5p_mmc {
	unsigned int	sysad;
	unsigned short	blksize;
	unsigned short	blkcnt;
	unsigned int	argument;
	unsigned short	trnmod;
	unsigned short	cmdreg;
	unsigned int	rspreg0;
	unsigned int	rspreg1;
	unsigned int	rspreg2;
	unsigned int	rspreg3;
	unsigned int	bdata;
	unsigned int	prnsts;
	unsigned char	hostctl;
	unsigned char	pwrcon;
	unsigned char	blkgap;
	unsigned char	wakcon;
	unsigned short	clkcon;
	unsigned char	timeoutcon;
	unsigned char	swrst;
	unsigned int	norintsts;	/* errintsts */
	unsigned int	norintstsen;	/* errintstsen */
	unsigned int	norintsigen;	/* errintsigen */
	unsigned short	acmd12errsts;
	unsigned char	res1[2];
	unsigned int	capareg;
	unsigned char	res2[4];
	unsigned int	maxcurr;
	unsigned char	res3[0x34];
	unsigned int	control2;
	unsigned int	control3;
	unsigned int	control4;
	unsigned char	res4[0x6e];
	unsigned short	hcver;
	unsigned char	res5[0xFFF00];
};

struct mmc_host {
	struct s5p_mmc *reg;
	unsigned int version;	/* SDHCI spec. version */
	unsigned int clock;	/* Current clock (MHz) */
};

int s5p_mmc_init(int dev_index);

#endif	/* __ASSEMBLY__ */
#endif
