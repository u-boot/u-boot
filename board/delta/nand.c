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
 * not required for Monahans DFC
 */
static void delta_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	return;
}

/* read device ready pin */
static int delta_device_ready(struct mtd_info *mtdinfo)
{
	if(NDSR & NDSR_RDY)
		return 1;
	else
		return 0;
	return 0;
}

/*
 * Write buf to the DFC Controller Data Buffer
 */
static void delta_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	unsigned long bytes_multi = len & 0xfffffffc;
	unsigned long rest = len & 0x3;
	unsigned long *long_buf;
	int i;

	if(bytes_multi) {
		for(i=0; i<bytes_multi; i+=4) {
			long_buf = (unsigned long*) &buf[i];
			NDDB = *long_buf;
		}
	}
	if(rest) {
		printf("delta_write_buf: ERROR, writing non 4-byte aligned data.\n");
	}
	return;
}


/* 
 * These functions are quite problematic for the DFC. Luckily they are
 * not used in the current nand code, except for nand_command, which
 * we've defined our own anyway. The problem is, that we always need
 * to write 4 bytes to the DFC Data Buffer, but in these functions we
 * don't know if to buffer the bytes/half words until we've gathered 4
 * bytes or if to send them straight away.
 *
 * Solution: Don't use these with Mona's DFC and complain loudly.
 */
static void delta_write_word(struct mtd_info *mtd, u16 word)
{
	printf("delta_write_word: WARNING, this function does not work with the Monahans DFC!\n");
}
static void delta_write_byte(struct mtd_info *mtd, u_char byte)
{
	printf("delta_write_byte: WARNING, this function does not work with the Monahans DFC!\n");
}

/* The original:
 * static void delta_read_buf(struct mtd_info *mtd, const u_char *buf, int len)
 *
 * Shouldn't this be "u_char * const buf" ?
 */
static void delta_read_buf(struct mtd_info *mtd, u_char* const buf, int len)
{
	int i, j;

	/* we have to be carefull not to overflow the buffer if len is
	 * not a multiple of 4 */
	unsigned long bytes_multi = len & 0xfffffffc;
	unsigned long rest = len & 0x3;
	unsigned long *long_buf;

	/* if there are any, first copy multiple of 4 bytes */
	if(bytes_multi) {
		for(i=0; i<bytes_multi; i+=4) {
			long_buf = (unsigned long*) &buf[i];
			*long_buf = NDDB;
		}
	}
	
	/* ...then the rest */
	if(rest) {
		unsigned long rest_data = NDDB;
		for(j=0;j<rest; j++)
			buf[i+j] = (u_char) ((rest_data>>j) & 0xff);
	}

	return;
}

static void delta_read_word(struct mtd_info *mtd, u_char byte)
{
	printf("delta_write_byte: UNIMPLEMENTED.\n");
}

/* global var, too bad: mk@tbd: move to ->priv pointer */
static unsigned long read_buf = 0;
static unsigned char bytes_read = 0;

static u_char delta_read_byte(struct mtd_info *mtd)
{
/* 	struct nand_chip *this = mtd->priv; */
	unsigned char byte;

	if(bytes_read == 0) {
		read_buf = NDDB;
		printk("delta_read_byte: 0x%x.\n", read_buf); 	
	}
	byte = (unsigned char) (read_buf>>(8 * bytes_read++));
	if(bytes_read >= 4)
		bytes_read = 0;

	printf("delta_read_byte: returning 0x%x.\n", byte);
	return byte;
}

/* delay function */
static void wait(unsigned long us)
{
#define OSCR_CLK_FREQ 3.250 /* kHz */

	unsigned long start = OSCR;
	unsigned long delta = 0, cur;
	us *= OSCR_CLK_FREQ;

	while (delta < us) {
		cur = OSCR;
		if(cur < start) /* OSCR overflowed */
			delta = cur + (start^0xffffffff);
		else
			delta = cur - start;
	}
}

/* poll the NAND Controller Status Register for event */
static void delta_wait_event(unsigned long event)
{
	if(!event)
		return;
	
	while(1) {
		if(NDSR & event) {
			NDSR |= event;
			break;
		}
	}
}

/* this is really monahans, not board specific ... */
static void delta_cmdfunc(struct mtd_info *mtd, unsigned command, 
			  int column, int page_addr)
{
	/* register struct nand_chip *this = mtd->priv; */
	unsigned long ndcb0=0, ndcb1=0, ndcb2=0, event=0;
	unsigned long what_the_hack;

	/* clear the ugly byte read buffer */
	bytes_read = 0;
	read_buf = 0;

	
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

	/* if command is a double byte cmd, we set bit double cmd bit 19  */
	/* 	command2 = (command>>8) & 0xFF;  */
	/* 	ndcb0 = command | ((command2 ? 1 : 0) << 19); *\/ */

	switch (command) {
	case NAND_CMD_READ0:
		ndcb0 = (NAND_CMD_READ0 | (4<<16));
		column >>= 1; /* adjust for 16 bit bus */
		ndcb1 = (((column>>1) & 0xff) |
			 ((page_addr<<8) & 0xff00) |
			 ((page_addr<<8) & 0xff0000) |
			 ((page_addr<<8) & 0xff000000)); /* make this 0x01000000 ? */
		event = NDSR_RDDREQ;
		break;	
	case NAND_CMD_READID:
		printk("delta_cmdfunc: NAND_CMD_READID.\n");
		ndcb0 = (NAND_CMD_READID | (3 << 21) | (1 << 16)); /* addr cycles*/
		event = NDSR_RDDREQ;
		break;
	case NAND_CMD_PAGEPROG:
		printk("delta_cmdfunc: NAND_CMD_PAGEPROG.\n");
		ndcb0 = (NAND_CMD_PAGEPROG | (1 << 21));
		break;
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
		printf("delta_cmdfunc: NAND_CMD_ERASEx unimplemented.\n");
		break;
	case NAND_CMD_SEQIN:
		/* send PAGE_PROG command(0x80) */
		printf("delta_cmdfunc: NAND_CMD_SEQIN/PAGE_PROG.\n");
		ndcb0 = (NAND_CMD_SEQIN | (1<<21) | (3<<16));
		column >>= 1; /* adjust for 16 bit bus */
		ndcb1 = (((column>>1) & 0xff) |
			 ((page_addr<<8) & 0xff00) |
			 ((page_addr<<8) & 0xff0000) |
			 ((page_addr<<8) & 0xff000000)); /* make this 0x01000000 ? */
		event = NDSR_WRDREQ;
		break;
/* 	case NAND_CMD_SEQIN_pointer_operation: */

/* 		/\* This is confusing because the command names are */
/* 		 * different compared to the ones in the K9K12Q0C */
/* 		 * datasheet. Infact this has nothing to do with */
/* 		 * reading, as the but with page programming */
/* 		 * (writing). */
/* 		 * Here we send the multibyte commands */
/* 		 * cmd1=0x00, cmd2=0x80 (for programming main area) or */
/* 		 * cmd1=0x50, cmd2=0x80 (for spare area) */
/* 		 * */
/* 		 * When all data is written to the buffer, the page */
/* 		 * program command (0x10) is sent to actually write */
/* 		 * the data. */
/* 		 *\/ */

/* 		printf("delta_cmdfunc: NAND_CMD_SEQIN pointer op called.\n"); */

/* 		ndcb0 = (NAND_CMD_SEQIN<<8) | (1<<21) | (1<<19) | (4<<16); */
/* 		if(column >= mtd->oobblock) { */
/* 			/\* OOB area *\/ */
/* 			column -= mtd->oobblock; */
/* 			ndcb0 |= NAND_CMD_READOOB; */
/* 		} else if (column < 256) { */
/* 			/\* First 256 bytes --> READ0 *\/ */
/* 			ndcb0 |= NAND_CMD_READ0; */
/* 		} else { */
/* 			/\* Only for 8 bit devices - not delta!!! *\/ */
/* 			column -= 256; */
/* 			ndcb0 |= NAND_CMD_READ1; */
/* 		} */
/* 		event = NDSR_WRDREQ; */
/* 		break; */
	case NAND_CMD_STATUS:
		/* oh, this is not nice. for some reason the real
		 * status byte is in the second read from the data
		 * buffer. The hack is to read the first byte right
		 * here, so the next read access by the nand code
		 * yields the right one.
		 */
		ndcb0 = (NAND_CMD_STATUS | (4<<21));
		event = NDSR_RDDREQ;
		NDCB0 = ndcb0;
		NDCB0 = ndcb1;
		NDCB0 = ndcb2;
		delta_wait_event(event);
		what_the_hack = NDDB;
		goto end;
		break;
	case NAND_CMD_RESET:
		printf("delta_cmdfunc: NAND_CMD_RESET unimplemented.\n");
		break;
	default:
		printk("delta_cmdfunc: error, unsupported command.\n");
		return;
	}

	NDCB0 = ndcb0;
	NDCB0 = ndcb1;
	NDCB0 = ndcb2;

	/* wait for event */
	delta_wait_event(event);
 end:
	return;
}

static void delta_dfc_gpio_init()
{
	printf("Setting up DFC GPIO's.\n");

	/* no idea what is done here, see zylonite.c */
	GPIO4 = 0x1;
	
	DF_ALE_WE1 = 0x00000001;
	DF_ALE_WE2 = 0x00000001;
	DF_nCS0 = 0x00000001;
	DF_nCS1 = 0x00000001;
	DF_nWE = 0x00000001;
	DF_nRE = 0x00000001;
	DF_IO0 = 0x00000001;
	DF_IO8 = 0x00000001;
	DF_IO1 = 0x00000001;
	DF_IO9 = 0x00000001;
	DF_IO2 = 0x00000001;
	DF_IO10 = 0x00000001;
	DF_IO3 = 0x00000001;
	DF_IO11 = 0x00000001;
	DF_IO4 = 0x00000001;
	DF_IO12 = 0x00000001;
	DF_IO5 = 0x00000001;
	DF_IO13 = 0x00000001;
	DF_IO6 = 0x00000001;
	DF_IO14 = 0x00000001;
	DF_IO7 = 0x00000001;
	DF_IO15 = 0x00000001;

	DF_nWE = 0x1901;
	DF_nRE = 0x1901;
	DF_CLE_NOE = 0x1900;
	DF_ALE_WE1 = 0x1901;
	DF_INT_RnB = 0x1900;
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
	delta_dfc_gpio_init();

	/* turn on the NAND Controller Clock (104 MHz @ D0) */
	CKENA |= (CKENA_4_NAND | CKENA_9_SMC);

	/* wait ? */
/* 	printf("stupid loop start...\n"); */
/* 	wait(200); */
/* 	printf("stupid loop end.\n"); */
		

	/* NAND Timing Parameters (in ns) */
#define NAND_TIMING_tCH 	10
#define NAND_TIMING_tCS 	0
#define NAND_TIMING_tWH		20
#define NAND_TIMING_tWP 	40
/* #define NAND_TIMING_tRH 	20 */
/* #define NAND_TIMING_tRP 	40 */

#define NAND_TIMING_tRH 	25
#define NAND_TIMING_tRP 	50

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
	

	printf("tCH=%u, tCS=%u, tWH=%u, tWP=%u, tRH=%u, tRP=%u, tR=%u, tWHR=%u, tAR=%u.\n", tCH, tCS, tWH, tWP, tRH, tRP, tR, tWHR, tAR);

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
	 *  - ND_RDY : clears command buffer
	 */
	NDCR = (NDCR_SPARE_EN |		/* use the spare area */
		NDCR_DWIDTH_C |		/* 16bit DFC data bus width  */
		NDCR_DWIDTH_M |		/* 16 bit Flash device data bus width */
		NDCR_NCSX |		/* Chip select busy don't care */
		(7 << 16) |		/* read id count = 7 ???? mk@tbd */
		NDCR_ND_ARB_EN |	/* enable bus arbiter */
		NDCR_RDYM |		/* flash device ready ir masked */
		NDCR_CS0_PAGEDM |	/* ND_nCSx page done ir masked */
		NDCR_CS1_PAGEDM |
		NDCR_CS0_CMDDM |	/* ND_CSx command done ir masked */
		NDCR_CS1_CMDDM |
		NDCR_CS0_BBDM |		/* ND_CSx bad block detect ir masked */
		NDCR_CS1_BBDM |
		NDCR_DBERRM |		/* double bit error ir masked */ 
		NDCR_SBERRM |		/* single bit error ir masked */
		NDCR_WRDREQM |		/* write data request ir masked */
		NDCR_RDDREQM |		/* read data request ir masked */
		NDCR_WRCMDREQM);	/* write command request ir masked */
	

	/* wait 10 us due to cmd buffer clear reset */
/* 	wait(10); */
	
	
	nand->hwcontrol = delta_hwcontrol;
/* 	nand->dev_ready = delta_device_ready; */
	nand->eccmode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_DELAY_US;
	nand->options = NAND_BUSWIDTH_16;

	nand->read_byte = delta_read_byte;
	nand->write_byte = delta_write_byte;
	nand->read_word = delta_read_word;
	nand->write_word = delta_write_word;
	nand->read_buf = delta_read_buf;
	nand->write_buf = delta_write_buf;
	nand->cmdfunc = delta_cmdfunc;
}

#else
 #error "U-Boot legacy NAND support not available for delta board."
#endif
#endif
