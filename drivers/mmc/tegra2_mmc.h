/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Portions Copyright (C) 2011 NVIDIA Corporation
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

#ifndef __TEGRA2_MMC_H_
#define __TEGRA2_MMC_H_

#define TEGRA2_SDMMC1_BASE	0xC8000000
#define TEGRA2_SDMMC2_BASE	0xC8000200
#define TEGRA2_SDMMC3_BASE	0xC8000400
#define TEGRA2_SDMMC4_BASE	0xC8000600

#ifndef __ASSEMBLY__
struct tegra2_mmc {
	unsigned int	sysad;		/* _SYSTEM_ADDRESS_0 */
	unsigned short	blksize;	/* _BLOCK_SIZE_BLOCK_COUNT_0 15:00 */
	unsigned short	blkcnt;		/* _BLOCK_SIZE_BLOCK_COUNT_0 31:16 */
	unsigned int	argument;	/* _ARGUMENT_0 */
	unsigned short	trnmod;		/* _CMD_XFER_MODE_0 15:00 xfer mode */
	unsigned short	cmdreg;		/* _CMD_XFER_MODE_0 31:16 cmd reg */
	unsigned int	rspreg0;	/* _RESPONSE_R0_R1_0 CMD RESP 31:00 */
	unsigned int	rspreg1;	/* _RESPONSE_R2_R3_0 CMD RESP 63:32 */
	unsigned int	rspreg2;	/* _RESPONSE_R4_R5_0 CMD RESP 95:64 */
	unsigned int	rspreg3;	/* _RESPONSE_R6_R7_0 CMD RESP 127:96 */
	unsigned int	bdata;		/* _BUFFER_DATA_PORT_0 */
	unsigned int	prnsts;		/* _PRESENT_STATE_0 */
	unsigned char	hostctl;	/* _POWER_CONTROL_HOST_0 7:00 */
	unsigned char	pwrcon;		/* _POWER_CONTROL_HOST_0 15:8 */
	unsigned char	blkgap;		/* _POWER_CONTROL_HOST_9 23:16 */
	unsigned char	wakcon;		/* _POWER_CONTROL_HOST_0 31:24 */
	unsigned short	clkcon;		/* _CLOCK_CONTROL_0 15:00 */
	unsigned char	timeoutcon;	/* _TIMEOUT_CTRL 23:16 */
	unsigned char	swrst;		/* _SW_RESET_ 31:24 */
	unsigned int	norintsts;	/* _INTERRUPT_STATUS_0 */
	unsigned int	norintstsen;	/* _INTERRUPT_STATUS_ENABLE_0 */
	unsigned int	norintsigen;	/* _INTERRUPT_SIGNAL_ENABLE_0 */
	unsigned short	acmd12errsts;	/* _AUTO_CMD12_ERR_STATUS_0 15:00 */
	unsigned char	res1[2];	/* _RESERVED 31:16 */
	unsigned int	capareg;	/* _CAPABILITIES_0 */
	unsigned char	res2[4];	/* RESERVED, offset 44h-47h */
	unsigned int	maxcurr;	/* _MAXIMUM_CURRENT_0 */
	unsigned char	res3[4];	/* RESERVED, offset 4Ch-4Fh */
	unsigned short	setacmd12err;	/* offset 50h */
	unsigned short	setinterr;	/* offset 52h */
	unsigned char	admaerr;	/* offset 54h */
	unsigned char	res4[3];	/* RESERVED, offset 55h-57h */
	unsigned long	admaaddr;	/* offset 58h-5Fh */
	unsigned char	res5[0x9c];	/* RESERVED, offset 60h-FBh */
	unsigned short	slotintstatus;	/* offset FCh */
	unsigned short	hcver;		/* HOST Version */
	unsigned char	res6[0x100];	/* RESERVED, offset 100h-1FFh */
};

struct mmc_host {
	struct tegra2_mmc *reg;
	unsigned int version;	/* SDHCI spec. version */
	unsigned int clock;	/* Current clock (MHz) */
	unsigned int base;	/* Base address, SDMMC1/2/3/4 */
};

int tegra2_mmc_init(int dev_index, int bus_width);

#endif	/* __ASSEMBLY__ */
#endif	/* __TEGRA2_MMC_H_ */
