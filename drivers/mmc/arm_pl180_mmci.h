/*
 * ARM PrimeCell MultiMedia Card Interface - PL180
 *
 * Copyright (C) ST-Ericsson SA 2010
 *
 * Author: Ulf Hansson <ulf.hansson@stericsson.com>
 * Author: Martin Lundholm <martin.xa.lundholm@stericsson.com>
 * Ported to drivers/mmc/ by: Matt Waddel <matt.waddel@linaro.org>
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

#ifndef __ARM_PL180_MMCI_H__
#define __ARM_PL180_MMCI_H__

int arm_pl180_mmci_init(void);

#define COMMAND_REG_DELAY	300
#define DATA_REG_DELAY		1000
#define CLK_CHANGE_DELAY	2000

#define INIT_PWR		0xBF /* Power on, full power, not open drain */
#define ARM_MCLK		(100*1000*1000)

/* SDI Power Control register bits */
#define SDI_PWR_PWRCTRL_MASK	0x00000003
#define SDI_PWR_PWRCTRL_ON	0x00000003
#define SDI_PWR_PWRCTRL_OFF	0x00000000
#define SDI_PWR_DAT2DIREN	0x00000004
#define SDI_PWR_CMDDIREN	0x00000008
#define SDI_PWR_DAT0DIREN	0x00000010
#define SDI_PWR_DAT31DIREN	0x00000020
#define SDI_PWR_OPD		0x00000040
#define SDI_PWR_FBCLKEN		0x00000080
#define SDI_PWR_DAT74DIREN	0x00000100
#define SDI_PWR_RSTEN		0x00000200

#define VOLTAGE_WINDOW_MMC	0x00FF8080
#define VOLTAGE_WINDOW_SD	0x80010000

/* SDI clock control register bits */
#define SDI_CLKCR_CLKDIV_MASK	0x000000FF
#define SDI_CLKCR_CLKEN		0x00000100
#define SDI_CLKCR_PWRSAV	0x00000200
#define SDI_CLKCR_BYPASS	0x00000400
#define SDI_CLKCR_WIDBUS_MASK	0x00001800
#define SDI_CLKCR_WIDBUS_1	0x00000000
#define SDI_CLKCR_WIDBUS_4	0x00000800

#define SDI_CLKCR_CLKDIV_INIT	0x000000C6 /* MCLK/(2*(0xC6+1)) => 505KHz */

/* SDI command register bits */
#define SDI_CMD_CMDINDEX_MASK	0x000000FF
#define SDI_CMD_WAITRESP	0x00000040
#define SDI_CMD_LONGRESP	0x00000080
#define SDI_CMD_WAITINT		0x00000100
#define SDI_CMD_WAITPEND	0x00000200
#define SDI_CMD_CPSMEN		0x00000400
#define SDI_CMD_SDIOSUSPEND	0x00000800
#define SDI_CMD_ENDCMDCOMPL	0x00001000
#define SDI_CMD_NIEN		0x00002000
#define SDI_CMD_CE_ATACMD	0x00004000
#define SDI_CMD_CBOOTMODEEN	0x00008000

#define SDI_DTIMER_DEFAULT	0xFFFF0000

/* SDI Status register bits */
#define SDI_STA_CCRCFAIL	0x00000001
#define SDI_STA_DCRCFAIL	0x00000002
#define SDI_STA_CTIMEOUT	0x00000004
#define SDI_STA_DTIMEOUT	0x00000008
#define SDI_STA_TXUNDERR	0x00000010
#define SDI_STA_RXOVERR		0x00000020
#define SDI_STA_CMDREND		0x00000040
#define SDI_STA_CMDSENT		0x00000080
#define SDI_STA_DATAEND		0x00000100
#define SDI_STA_STBITERR	0x00000200
#define SDI_STA_DBCKEND		0x00000400
#define SDI_STA_CMDACT		0x00000800
#define SDI_STA_TXACT		0x00001000
#define SDI_STA_RXACT		0x00002000
#define SDI_STA_TXFIFOBW	0x00004000
#define SDI_STA_RXFIFOBR	0x00008000
#define SDI_STA_TXFIFOF		0x00010000
#define SDI_STA_RXFIFOF		0x00020000
#define SDI_STA_TXFIFOE		0x00040000
#define SDI_STA_RXFIFOE		0x00080000
#define SDI_STA_TXDAVL		0x00100000
#define SDI_STA_RXDAVL		0x00200000
#define SDI_STA_SDIOIT		0x00400000
#define SDI_STA_CEATAEND	0x00800000
#define SDI_STA_CARDBUSY	0x01000000
#define SDI_STA_BOOTMODE	0x02000000
#define SDI_STA_BOOTACKERR	0x04000000
#define SDI_STA_BOOTACKTIMEOUT	0x08000000
#define SDI_STA_RSTNEND		0x10000000

/* SDI Interrupt Clear register bits */
#define SDI_ICR_MASK		0x1DC007FF
#define SDI_ICR_CCRCFAILC	0x00000001
#define SDI_ICR_DCRCFAILC	0x00000002
#define SDI_ICR_CTIMEOUTC	0x00000004
#define SDI_ICR_DTIMEOUTC	0x00000008
#define SDI_ICR_TXUNDERRC	0x00000010
#define SDI_ICR_RXOVERRC	0x00000020
#define SDI_ICR_CMDRENDC	0x00000040
#define SDI_ICR_CMDSENTC	0x00000080
#define SDI_ICR_DATAENDC	0x00000100
#define SDI_ICR_STBITERRC	0x00000200
#define SDI_ICR_DBCKENDC	0x00000400
#define SDI_ICR_SDIOITC		0x00400000
#define SDI_ICR_CEATAENDC	0x00800000
#define SDI_ICR_BUSYENDC	0x01000000
#define SDI_ICR_BOOTACKERRC	0x04000000
#define SDI_ICR_BOOTACKTIMEOUTC	0x08000000
#define SDI_ICR_RSTNENDC	0x10000000

#define SDI_MASK0_MASK		0x1FFFFFFF

/* SDI Data control register bits */
#define SDI_DCTRL_DTEN		0x00000001
#define SDI_DCTRL_DTDIR_IN	0x00000002
#define SDI_DCTRL_DTMODE_STREAM	0x00000004
#define SDI_DCTRL_DMAEN		0x00000008
#define SDI_DCTRL_DBLKSIZE_MASK	0x000000F0
#define SDI_DCTRL_RWSTART	0x00000100
#define SDI_DCTRL_RWSTOP	0x00000200
#define SDI_DCTRL_RWMOD		0x00000200
#define SDI_DCTRL_SDIOEN	0x00000800
#define SDI_DCTRL_DMAREQCTL	0x00001000
#define SDI_DCTRL_DBOOTMODEEN	0x00002000
#define SDI_DCTRL_BUSYMODE	0x00004000
#define SDI_DCTRL_DDR_MODE	0x00008000

#define SDI_FIFO_BURST_SIZE	8

struct sdi_registers {
	u32 power;		/* 0x00*/
	u32 clock;		/* 0x04*/
	u32 argument;		/* 0x08*/
	u32 command;		/* 0x0c*/
	u32 respcommand;	/* 0x10*/
	u32 response0;		/* 0x14*/
	u32 response1;		/* 0x18*/
	u32 response2;		/* 0x1c*/
	u32 response3;		/* 0x20*/
	u32 datatimer;		/* 0x24*/
	u32 datalength;		/* 0x28*/
	u32 datactrl;		/* 0x2c*/
	u32 datacount;		/* 0x30*/
	u32 status;		/* 0x34*/
	u32 status_clear;	/* 0x38*/
	u32 mask0;		/* 0x3c*/
	u32 mask1;		/* 0x40*/
	u32 card_select;	/* 0x44*/
	u32 fifo_count;		/* 0x48*/
	u32 padding1[(0x80-0x4C)>>2];
	u32 fifo;		/* 0x80*/
	u32 padding2[(0xFE0-0x84)>>2];
	u32 periph_id0;		/* 0xFE0 mmc Peripheral Identifcation Register*/
	u32 periph_id1;		/* 0xFE4*/
	u32 periph_id2;		/* 0xFE8*/
	u32 periph_id3;		/* 0xFEC*/
	u32 pcell_id0;		/* 0xFF0*/
	u32 pcell_id1;		/* 0xFF4*/
	u32 pcell_id2;		/* 0xFF8*/
	u32 pcell_id3;		/* 0xFFC*/
};

#endif
