/*
 * Faraday FTIDE020_s ATA Controller (AHB)
 *
 * (C) Copyright 2011 Andes Technology
 * Greentime Hu <greentime@andestech.com>
 * Macpaul Lin <macpaul@andestech.com>
 * Kuo-Wei Chou <kwchou@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FTIDE020_H
#define __FTIDE020_H

/* ftide020.h - ide support functions for the FTIDE020_S controller */

/* ATA controller register offset */
struct ftide020_s {
	unsigned int	rw_fifo;	/* 0x00 - READ/WRITE FIFO	*/
	unsigned int	cmd_fifo;	/* 0x04 - R: Status Reg, W: CMD_FIFO */
	unsigned int	cr;		/* 0x08 - Control Reg		*/
	unsigned int	dmatirr;	/* 0x0c - DMA Threshold/Interrupt Reg */
	unsigned int	ctrd0;		/* 0x10 - Command Timing Reg Device 0 */
	unsigned int	dtrd0;		/* 0x14 - Data Timing Reg Device 0 */
	unsigned int	ctrd1;		/* 0x18 - Command Timing Reg Device 1 */
	unsigned int	dtrd1;		/* 0x1c - Data Timing Reg Device 1 */
	unsigned int	ahbtr;		/* 0x20 - AHB Timeout Reg	*/
	unsigned int	RESVD0;		/* 0x24 */
	unsigned int	RESVD1;		/* 0x28 */
	unsigned int	RESVD2;		/* 0x2c */
	unsigned int	f_cfifo;	/* 0x30 - Feature Info of CMD_FIFO */
	unsigned int	f_wfifo;	/* 0x34 - Feature Info of WRITE_FIFO */
	unsigned int	f_rfifo;	/* 0x3c - Feature Info of READ_FIFO */
	unsigned int	revision;	/* 0x38 - Revision No. of FTIDE020_S */
};

/* reference parameters */
#define CONFIG_IDE_REG_CS	0x2	/* ref: ATA spec chaper 10, table 42 */
#define CONFIG_CTRD1_PROBE_T1	0x2
#define CONFIG_CTRD1_PROBE_T2	0x5

/* status register - 0x04 */
#define STATUS_CSEL		(1 << 0)	/* CSEL			*/
#define STATUS_CS(x)		(((x) >> 1) & 0x3)	/* CS#[1:0]	*/
#define STATUS_DMACK		(1 << 3)	/* DMACK#		*/
#define STATUS_DMARQ		(1 << 4)	/* DMA req		*/
#define STATUS_INTRQ		(1 << 5)	/* INT req		*/
#define STATUS_DIOR		(1 << 6)	/* DIOR			*/
#define STATUS_IORDY		(1 << 7)	/* I/O ready		*/
#define STATUS_DIOW		(1 << 8)	/* DIOW#		*/
#define STATUS_PDIAG		(1 << 9)	/* PDIAG		*/
#define STATUS_DASP		(1 << 10)	/* DASP#		*/
#define STATUS_DEV		(1 << 11)	/* selected device	*/
#define STATUS_PIO		(1 << 12)	/* PIO in progress	*/
#define STATUS_DMA		(1 << 13)	/* DMA in progress	*/
#define STATUS_WFE		(1 << 14)	/* write fifo full	*/
#define STATUS_RFE		(1 << 15)	/* read fifo empty	*/
#define STATUS_COUNTER(x)	(((x) >> 16) & 0x3fff)	/* data tx counter */
#define STATUS_ERR		(1 << 30)	/* trasfer terminated	*/
#define STATUS_AER		(1 << 31)	/* AHB timeout indicate	*/

/* Control register - 0x08 */
#define CONTROL_TYPE_PIO	0x0
#define CONTROL_TYPE_UDMA	0x1

/* Device 0 */
#define CONTROL_TYP0(x)		(((x) & 0x7) << 0)
#define CONTROL_IRE0		(1 << 3) /* enable IORDY for PIO */
#define CONTROL_RESVD_DW0	(1 << 4) /* Reserved - DW0 ?	*/
#define CONTROL_E0		(1 << 5) /* E0: 1: Big Endian	*/
#define CONTROL_RESVD_WP0	(1 << 6) /* Reserved - WP0 ?	*/
#define CONTROL_RESVD_SE0	(1 << 7) /* Reserved - SE0 ?	*/
#define CONTROL_RESVD_ECC0	(1 << 8) /* Reserved - ECC0 ?	*/

#define CONTROL_RAEIE		(1 << 9)  /* IRQ - read fifo almost full */
#define CONTROL_RNEIE		(1 << 10) /* IRQ - read fifo not empty	*/
#define CONTROL_WAFIE		(1 << 11) /* IRQ - write fifo almost empty */
#define CONTROL_WNFIE		(1 << 12) /* IRQ - write fifo not full	*/
#define CONTROL_RESVD_FIRQ	(1 << 13) /* RESERVED - FIRQ ?		*/
#define CONTROL_AERIE		(1 << 14) /* IRQ - AHB timeout error	*/
#define CONTROL_IIE		(1 << 15) /* IDE IRQ enable		*/

/* Device 1 */
#define CONTROL_TYP1(x)		(((x) & 0x7) << 16)
#define CONTROL_IRE1		(1 << 19)	/* enable IORDY for PIO */
#define CONTROL_RESVD_DW1	(1 << 20)	/* Reserved - DW1 ?	*/
#define CONTROL_E1		(1 << 21)	/* E1: 1: Big Endian	*/
#define CONTROL_RESVD_WP1	(1 << 22)	/* Reserved - WP1 ?	*/
#define CONTROL_RESVD_SE1	(1 << 23)	/* Reserved - SE1 ?	*/
#define CONTROL_RESVD_ECC1	(1 << 24)	/* Reserved - ECC1 ?	*/

#define CONTROL_DRE	(1 << 25)	/* DMA receive enable		*/
#define CONTROL_DTE	(1 << 26)	/* DMA transmit enable		*/
#define CONTRIL_RESVD	(1 << 27)
#define CONTROL_TERIE	(1 << 28)	/* transfer terminate error IRQ	*/
#define CONTROL_T	(1 << 29)	/* terminate current operation	*/
#define CONTROL_SRST	(1 << 30)	/* IDE soft reset		*/
#define CONTROL_RST	(1 << 31)	/* IDE hardware reset		*/

/* IRQ register - 0x0c */
#define IRQ_RXTHRESH(x)	(((x) & 0x3ff) << 0)	/* Read FIFO threshold	*/
#define IRQ_RFAEIRQ	(1 << 10)	/* Read FIFO almost full intr req */
#define IRQ_RFNEIRQ	(1 << 11)	/* Read FIFO not empty intr req	*/
#define IRQ_WFAFIRQ	(1 << 12)	/* Write FIFO almost empty int req */
#define IRQ_WFNFIRQ	(1 << 13)	/* Write FIFO not full intr req	*/
#define IRQ_RESVD_FIRQ	(1 << 14)	/* Reserved - FIRQ ?		*/
#define IRQ_IIRQ	(1 << 15)	/* IDE device interrupt request	*/
#define IRQ_TXTHRESH(x)	(((x) & 0x3ff) << 16)	/* Write FIFO thershold	*/
#define IRQ_TERMERR	(1 << 28)	/* Transfer termination indication */
#define IRQ_AHBERR	(1 << 29)	/* AHB Timeout indication	*/

/* Command Timing Register 0-1: ctrd (0x10, 0x18) */
#define CT_REG_T1(x)	(((x) & 0xff) << 0)	/* setup time of addressed  */
#define CT_REG_T2(x)	(((x) & 0xff) << 8)	/* pluse width of DIOR/DIOW */
#define CT_REG_T4(x)	(((x) & 0xff) << 16)	/* data hold time */
#define CT_REG_TEOC(x)	(((x) & 0xff) << 24)	/* time to the end of a cycle */

/* Data Timing Register 0-1: dtrd (0x14, 0x1c) */
/*
 * PIO mode:
 *	b(0:7)		DT_REG_PIO_T1: the setup time of addressed
 *	b(8:15)		DT_REG_PIO_T2: the pluse width of DIOR/DIOW
 *	b(16:23)	DT_REG_PIO_T4: data hold time
 *	b(24:31)	DT_REG_PIO_TEOC: the time to the end of a cycle
 */
#define DT_REG_PIO_T1(x)	(((x) & 0xff) << 0)
#define DT_REG_PIO_T2(x)	(((x) & 0xff) << 8)
#define DT_REG_PIO_T4(x)	(((x) & 0xff) << 16)
#define DT_REG_PIO_TEOC(x)	(((x) & 0xff) << 24)

/*
 * UDMA mode:
 *	b(0:3)		DT_REG_UDMA_TENV: the envelope time
 *	b(4:7)		DT_REG_UDMA_TMLI: interlock time
 *	b(8:15)		DT_REG_UDMA_TCYC: cycle time - data time
 *	b(16:19)	DT_REG_UDMA_TACK: setup and hold time of DMACK
 *	b(23:30)	DT_REG_UDMA_TCVS: setup time of CRC
 *	b(24:31)	DT_REG_UDMA_TRP: time to ready to pause
 */
#define DT_REG_UDMA_TENV(x)	(((x) & 0xf) << 0)
#define DT_REG_UDMA_TMLI(x)	(((x) & 0xf) << 4)
#define DT_REG_UDMA_TCYC(x)	(((x) & 0xff) << 8)
#define DT_REG_UDMA_TACK(x)	(((x) & 0xf) << 16)
#define DT_REG_UDMA_TCVS(x)	(((x) & 0xf) << 20)
#define DT_REG_UDMA_TRP(x)	(((x) & 0xff) << 24)

/* ftide020_s command formats */
/* read: IDE Register (CF1) */
#define IDE_REG_OPCODE_READ	(1 << 13)		/* 0x2000 */
#define IDE_REG_CS_READ(x)	(((x) & 0x3) << 11)
#define IDE_REG_DA_READ(x)	(((x) & 0x7) << 8)
#define IDE_REG_CMD_READ(x)	0x0			/* fixed value */

/* write: IDE Register (CF2) */
#define IDE_REG_OPCODE_WRITE	(0x5 << 13)		/* 0xA000 */
#define IDE_REG_CS_WRITE(x)	(((x) & 0x3) << 11)
#define IDE_REG_DA_WRITE(x)	(((x) & 0x7) << 8)
/* b(0:7) IDE_REG_CMD_WRITE(x):	Actual ATA command or data */
#define IDE_REG_CMD_WRITE(x)	(((x) & 0xff) << 0)

/* read/write data: PIO/UDMA (CF3) */
#define IDE_DATA_WRITE		(1 << 15)		/* read: 0, write: 1 */
#define IDE_DATA_OPCODE		(0x2 << 13)	/* device data access opcode */
/* b(0:12) IDE_DATA_COUNTER(x): Number of transfers minus 1 */
#define IDE_DATA_COUNTER(x)	(((x) & 0x1fff) << 0)

/* set device: (CF4) */
#define IDE_SET_OPCODE	(0x2740 << 2)			/* [15:2], 0x9d00 */
/* CF3 counter value: 0: Tx in bytes, 1: in blocks (each block is 8 bytes) */
#define IDE_SET_CX8(x)	(((x) & 0x1) << 1)
#define IDE_SET_DEV(x)	(((x) & 0x1) << 0)	/* 0: Master, 1: Slave */

/*
 * IDE command bit definition
 * This section is designed for minor hardware revision compatibility.
 */
#define READ_REG_CMD	IDE_REG_OPCODE_READ			/* 0x2000 */
#define WRITE_REG_CMD	IDE_REG_OPCODE_WRITE			/* 0xA000 */
#define READ_DATA_CMD	IDE_DATA_OPCODE				/* 0x4000 */
#define WRITE_DATA_CMD	(IDE_DATA_OPCODE | IDE_DATA_WRITE)	/* 0xC000 */
#define SET_DEV_CMD	IDE_SET_OPCODE				/* 0x9D00 */

#define TATOL_TIMING		3
#define CMD_TIMING		0
#define PIO_TIMING		1
#define DMA_TIMING		2

/* Timing Parameters */
/* Register Access Timing Parameters */
#define REG_PARAMETER		4
#define REG_T0			0
#define REG_T1			1
#define REG_T2			2
#define REG_T4			3

#define REG_MODE		5
#define REG_MODE0		0
#define REG_MODE1		1
#define REG_MODE2		2
#define REG_MODE3		3
#define REG_MODE4		4

/* PIO Access Timing Parameters */
#define PIO_PARAMETER		4
#define PIO_T0			0
#define PIO_T1			1
#define PIO_T2			2
#define PIO_T4			3

#define PIO_MODE		5
#define PIO_MODE0		0
#define PIO_MODE1		1
#define PIO_MODE2		2
#define PIO_MODE3		3
#define PIO_MODE4		4

/* UDMA Access Timing Parameters */
#define UDMA_PARAMETER		6
#define UDMA_TCYC		0
#define UDMA_TCVS		1
#define UDMA_TMLI		2
#define UDMA_TENV		3
#define UDMA_TRP		4
#define UDMA_TACK		5

#define UDMA_MODE		7
#define UDMA_MODE0		0
#define UDMA_MODE1		1
#define UDMA_MODE2		2
#define UDMA_MODE3		3
#define UDMA_MODE4		4
#define UDMA_MODE5		5
#define UDMA_MODE6		6

/*
 * RX_THRESH:
 * hardware limitation: max = 8, should support 1,4,8,16,32,64,128,256
 */
#define RX_THRESH		8
#define WRITE_FIFO		32	/* Hardwired value */

/* Time Table */
unsigned int REG_ACCESS_TIMING[REG_PARAMETER][REG_MODE] = {
	{600,	383,	330,	180,	120},
	{70,	50,	30,	30,	25},
	{290,	290,	290,	80,	70},
	{30,	20,	15,	10,	10},
};

unsigned int PIO_ACCESS_TIMING[PIO_PARAMETER][PIO_MODE] = {
	{600,	383,	240,	180,	120},
	{70,	50,	30,	30,	25},
	{165,	125,	100,	80,	70},
	{30,	20,	15,	10,	10},
};

unsigned int UDMA_ACCESS_TIMING[UDMA_PARAMETER][UDMA_MODE] = {
	{1120,	730,	540,	390,	250,	168,	130}, /* 10X */
	{700,	480,	310,	200,	67,	100,	100}, /* 10X */
	{200,	200,	200,	200,	200,	200,	200}, /* 10X */
	{200,	200,	200,	200,	200,	200,	200}, /* 10X */
	{1600,	1250,	1000,	1000,	1000,	850,	850}, /* 10X */
	{200,	200,	200,	200,	200,	200,	200}, /* 10X */
};

#endif /* __FTIDE020_H */
