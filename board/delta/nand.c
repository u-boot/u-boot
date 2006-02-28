/*
 * (C) Copyright 2006 DENX Software Engineering
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

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#ifdef CONFIG_NEW_NAND_CODE

#include <nand.h>
#include <asm/arch/pxa-regs.h>

/*
 * hardware specific access to control-lines
 * function borrowed from Linux 2.6 (drivers/mtd/nand/ppchameleonevb.c)
 */
static void delta_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
#if 0
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W;

	switch(cmd) {
	case NAND_CTL_SETCLE:
		MACRO_NAND_CTL_SETCLE((unsigned long)base);
		break;
	case NAND_CTL_CLRCLE:
		MACRO_NAND_CTL_CLRCLE((unsigned long)base);
		break;
	case NAND_CTL_SETALE:
		MACRO_NAND_CTL_SETALE((unsigned long)base);
		break;
	case NAND_CTL_CLRALE:
		MACRO_NAND_CTL_CLRALE((unsigned long)base);
		break;
	case NAND_CTL_SETNCE:
		MACRO_NAND_ENABLE_CE((unsigned long)base);
		break;
	case NAND_CTL_CLRNCE:
		MACRO_NAND_DISABLE_CE((unsigned long)base);
		break;
	}
#endif
}


/* read device ready pin */
static int delta_device_ready(struct mtd_info *mtdinfo)
{
	if(NDSR & NDSR_RDY)
		return 1;
	else
		return 0;
#if 0
	struct nand_chip *this = mtdinfo->priv;
	ulong rb_gpio_pin;

	/* use the base addr to find out which chip are we dealing with */
	switch((ulong) this->IO_ADDR_W) {
	case CFG_NAND0_BASE:
		rb_gpio_pin = CFG_NAND0_RDY;
		break;
	case CFG_NAND1_BASE:
		rb_gpio_pin = CFG_NAND1_RDY;
		break;
	default: /* this should never happen */
		return 0;
		break;
	}

        if (in32(GPIO0_IR) & rb_gpio_pin)
		return 1;
#endif
	return 0;
}

static u_char delta_read_byte(struct mtd_info *mtd)
{
/* 	struct nand_chip *this = mtd->priv; */
	unsigned long tmp;

	/* wait for read request */
	while(1) {
		if(NDSR & NDSR_RDDREQ) {
			NDSR |= NDSR_RDDREQ;
			break;
		}
	}

	tmp = NDDB;
	printk("delta_read_byte: 0x%x.\n", tmp); 
	return (u_char) tmp;
}

/* this is really monahans, not board specific ... */
static void delta_cmdfunc(struct mtd_info *mtd, unsigned command, 
			  int column, int page_addr)
{
	/* register struct nand_chip *this = mtd->priv; */
	unsigned long ndcb0=0, ndcb1=0, ndcb2=0;
	uchar command2;

	/* Clear NDSR */
	NDSR = 0xFFF;
	
	/* apparently NDCR[NDRUN] needs to be set before writing to NDCBx */
	NDCR |= NDCR_ND_RUN;

	/* wait for write command request 
	 * hmm, might be nice if this could time-out. mk@tbd
	 */
	while(1) {
		if(NDSR & NDSR_WRCMDREQ) {
			NDSR |= NDSR_WRCMDREQ; /* Ack */
			break;
		}
	}

	/* if command is a double byte cmd, we set bit double cmd bit 19 */
	command2 = (command>>8) & 0xFF;
	ndcb0 = command | ((command2 ? 1 : 0) << 19);

	switch (command) {
	case NAND_CMD_READID:
		printk("delta_cmdfunc: NAND_CMD_READID.\n");
		ndcb0 |= ((3 << 21) | (2 << 16));
		break;
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		return;
	case NAND_CMD_RESET:
		return;
	default:
		printk("delta_cmdfunc: error, unkown command issued.\n");
		return;
	}

	NDCB0 = ndcb0;
	NDCB1 = ndcb1;
	NDCB2 = ndcb2;	
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand_new.h):
 * - IO_ADDR_R?: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W?: address to write the 8 I/O lines of the flash device
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - eccmode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
void board_nand_init(struct nand_chip *nand)
{
	unsigned long tCH, tCS, tWH, tWP, tRH, tRP, tRP_high, tR, tWHR, tAR;

	/* set up GPIO Control Registers */
	
	/* turn on the NAND Controller Clock (104 MHz @ D0) */
	CKENA |= (CKENA_4_NAND | CKENA_9_SMC);
	
	/* NAND Timing Parameters (in ns) */
#define NAND_TIMING_tCH 	10
#define NAND_TIMING_tCS 	0
#define NAND_TIMING_tWH		20
#define NAND_TIMING_tWP 	40
#define NAND_TIMING_tRH 	20
#define NAND_TIMING_tRP 	40
#define NAND_TIMING_tR  	11123
#define NAND_TIMING_tWHR	110
#define NAND_TIMING_tAR		10

/* Maximum values for NAND Interface Timing Registers in DFC clock
 * periods */
#define DFC_MAX_tCH		7
#define DFC_MAX_tCS		7
#define DFC_MAX_tWH		7
#define DFC_MAX_tWP		7
#define DFC_MAX_tRH		7
#define DFC_MAX_tRP		15
#define DFC_MAX_tR		65535
#define DFC_MAX_tWHR		15
#define DFC_MAX_tAR		15

#define DFC_CLOCK		104		/* DFC Clock is 104 MHz */
#define DFC_CLK_PER_US		DFC_CLOCK/1000	/* clock period in ns */
#define MIN(x, y)		((x < y) ? x : y)

	
	tCH = MIN(((unsigned long) (NAND_TIMING_tCH * DFC_CLK_PER_US) + 1), 
		  DFC_MAX_tCH);
	tCS = MIN(((unsigned long) (NAND_TIMING_tCS * DFC_CLK_PER_US) + 1), 
		  DFC_MAX_tCS);
	tWH = MIN(((unsigned long) (NAND_TIMING_tWH * DFC_CLK_PER_US) + 1),
		  DFC_MAX_tWH);
	tWP = MIN(((unsigned long) (NAND_TIMING_tWP * DFC_CLK_PER_US) + 1),
		  DFC_MAX_tWP);
	tRH = MIN(((unsigned long) (NAND_TIMING_tRH * DFC_CLK_PER_US) + 1),
		  DFC_MAX_tRH);
	tRP = MIN(((unsigned long) (NAND_TIMING_tRP * DFC_CLK_PER_US) + 1),
		  DFC_MAX_tRP);
	tR = MIN(((unsigned long) (NAND_TIMING_tR * DFC_CLK_PER_US) + 1),
		 DFC_MAX_tR);
	tWHR = MIN(((unsigned long) (NAND_TIMING_tWHR * DFC_CLK_PER_US) + 1),
		   DFC_MAX_tWHR);
	tAR = MIN(((unsigned long) (NAND_TIMING_tAR * DFC_CLK_PER_US) + 1),
		  DFC_MAX_tAR);
	

	/* tRP value is split in the register */
	if(tRP & (1 << 4)) {
		tRP_high = 1;
		tRP &= ~(1 << 4);
	} else {
		tRP_high = 0;
	}

	NDTR0CS0 = (tCH << 19) |
		(tCS << 16) |
		(tWH << 11) |
		(tWP << 8) |
		(tRP_high << 6) |
		(tRH << 3) |
		(tRP << 0);
	
	NDTR1CS0 = (tR << 16) |
		(tWHR << 4) |
		(tAR << 0);

	

	/* If it doesn't work (unlikely) think about:
	 *  - ecc enable
	 *  - chip select don't care
	 *  - read id byte count
	 *
	 * Intentionally enabled by not setting bits:
	 *  - dma (DMA_EN)
	 *  - page size = 512
	 *  - cs don't care, see if we can enable later!
	 *  - row address start position (after second cycle)
	 *  - pages per block = 32
	 */
	NDCR = (NDCR_ND_ARB_EN |	/* enable bus arbiter */
		NDCR_SPARE_EN |		/* use the spare area */
		NDCR_DWIDTH_C |		/* 16bit DFC data bus width  */
		NDCR_DWIDTH_M |		/* 16 bit Flash device data bus width */
		(2 << 16) |		/* read id count = 7 ???? mk@tbd */
		NDCE_RDYM |		/* flash device ready ir masked */
		NDCE_CS0_PAGEDM |	/* ND_nCSx page done ir masked */
		NDCE_CS1_PAGEDM |
		NDCE_CS0_CMDDM |	/* ND_CSx command done ir masked */
		NDCE_CS1_CMDDM |
		NDCE_CS0_BBDM |		/* ND_CSx bad block detect ir masked */
		NDCE_CS1_BBDM |
		NDCE_DBERRM |		/* double bit error ir masked */ 
		NDCE_SBERRM |		/* single bit error ir masked */
		NDCE_WRDREQM |		/* write data request ir masked */
		NDCE_RDDREQM |		/* read data request ir masked */
		NDCE_WRCMDREQM);	/* write command request ir masked */
	
	
	
	nand->hwcontrol = delta_hwcontrol;
	nand->dev_ready = delta_device_ready;
	nand->eccmode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_DELAY_US;
	nand->options = NAND_BUSWIDTH_16;
	nand->read_byte = delta_read_byte;
	nand->cmdfunc = delta_cmdfunc;
	/* 	nand->options = NAND_SAMSUNG_LP_OPTIONS; */
}

#else
#error "U-Boot legacy NAND support not available for delta board."
#endif
#endif
