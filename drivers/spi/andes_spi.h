/*
 * Register definitions for the Andes SPI Controller
 *
 * (C) Copyright 2011 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ANDES_SPI_H
#define __ANDES_SPI_H

struct andes_spi_regs {
	unsigned int	apb;		/* 0x00 - APB SPI interface setting */
	unsigned int	pio;		/* 0x04 - PIO reg */
	unsigned int	cr;		/* 0x08 - SPI Control reg */
	unsigned int	st;		/* 0x0c - SPI Status reg */
	unsigned int	ie;		/* 0x10 - Interrupt Enable reg */
	unsigned int	ist;		/* 0x14 - Interrupt Status reg */
	unsigned int	dcr;		/* 0x18 - data control reg */
	unsigned int	data;		/* 0x1c - data register */
	unsigned int	ahb;		/* 0x20 - AHB SPI interface setting */
	unsigned int	ver;		/* 0x3c - SPI version reg */
};

#define BIT(x)			(1 << (x))

/* 0x00 - APB SPI interface setting register */
#define ANDES_SPI_APB_BAUD(x)	(((x) & 0xff) < 0)
#define ANDES_SPI_APB_CSHT(x)	(((x) & 0xf) < 16)
#define ANDES_SPI_APB_SPNTS	BIT(20)		/* 0: normal, 1: delay */
#define ANDES_SPI_APB_CPHA	BIT(24)		/* 0: Sampling at odd edges */
#define ANDES_SPI_APB_CPOL	BIT(25)		/* 0: SCK low, 1: SCK high */
#define ANDES_SPI_APB_MSSL	BIT(26)		/* 0: SPI Master, 1: slave */

/* 0x04 - PIO register */
#define ANDES_SPI_PIO_MISO	BIT(0)		/* input value of pin MISO */
#define ANDES_SPI_PIO_MOSI	BIT(1)		/* I/O value of pin MOSI */
#define ANDES_SPI_PIO_SCK	BIT(2)		/* I/O value of pin SCK */
#define ANDES_SPI_PIO_CS	BIT(3)		/* I/O value of pin CS */
#define ANDES_SPI_PIO_PIOE	BIT(4)		/* Programming IO Enable */

/* 0x08 - SPI Control register */
#define ANDES_SPI_CR_SPIRST	BIT(0)		/* SPI mode reset */
#define ANDES_SPI_CR_RXFRST	BIT(1)		/* RxFIFO reset */
#define ANDES_SPI_CR_TXFRST	BIT(2)		/* TxFIFO reset */
#define ANDES_SPI_CR_RXFTH(x)	(((x) & 0x1f) << 10)	/* RxFIFO Threshold */
#define ANDES_SPI_CR_TXFTH(x)	(((x) & 0x1f) << 18)	/* TxFIFO Threshold */

/* 0x0c - SPI Status register */
#define ANDES_SPI_ST_SPIBSY	BIT(0)		/* SPI Transfer is active */
#define ANDES_SPI_ST_RXFEM	BIT(8)		/* RxFIFO Empty Flag */
#define ANDES_SPI_ST_RXFEL	BIT(9)		/* RxFIFO Full Flag */
#define ANDES_SPI_ST_RXFVE(x)	(((x) >> 10) & 0x1f)
#define ANDES_SPI_ST_TXFEM	BIT(16)		/* TxFIFO Empty Flag */
#define ANDES_SPI_ST_TXFEL	BIT(7)		/* TxFIFO Full Flag */
#define ANDES_SPI_ST_TXFVE(x)	(((x) >> 18) & 0x1f)

/* 0x10 - Interrupt Enable register */
#define ANDES_SPI_IE_RXFORIE	BIT(0)		/* RxFIFO overrun intr */
#define ANDES_SPI_IE_TXFURIE	BIT(1)		/* TxFOFO underrun intr */
#define ANDES_SPI_IE_RXFTHIE	BIT(2)		/* RxFIFO threshold intr */
#define ANDES_SPI_IE_TXFTHIE	BIT(3)		/* TxFIFO threshold intr */
#define ANDES_SPI_IE_SPIEIE	BIT(4)		/* SPI transmit END intr */
#define ANDES_SPI_IE_SPCFIE	BIT(5)		/* AHB/APB TxReq conflict */

/* 0x14 - Interrupt Status Register */
#define ANDES_SPI_IST_RXFORI	BIT(0)		/* has RxFIFO overrun */
#define ANDES_SPI_IST_TXFURI	BIT(1)		/* has TxFOFO underrun */
#define ANDES_SPI_IST_RXFTHI	BIT(2)		/* has RxFIFO threshold */
#define ANDES_SPI_IST_TXFTHI	BIT(3)		/* has TxFIFO threshold */
#define ANDES_SPI_IST_SPIEI	BIT(4)		/* has SPI transmit END */
#define ANDES_SPI_IST_SPCFI	BIT(5)		/* has AHB/APB TxReq conflict */

/* 0x18 - Data Control Register */
#define ANDES_SPI_DCR_RCNT(x)		(((x) & 0x3ff) << 0)
#define ANDES_SPI_DCR_DYCNT(x)		(((x) & 0x7) << 12)
#define ANDES_SPI_DCR_WCNT(x)		(((x) & 0x3ff) << 16)
#define ANDES_SPI_DCR_TRAMODE(x)	(((x) & 0x7) << 28)
#define ANDES_SPI_DCR_SPIT		BIT(31)		/* SPI bus trigger */

#define ANDES_SPI_DCR_MODE_WRCON	ANDES_SPI_DCR_TRAMODE(0)	/* w/r at the same time */
#define ANDES_SPI_DCR_MODE_WO		ANDES_SPI_DCR_TRAMODE(1)	/* write only		*/
#define ANDES_SPI_DCR_MODE_RO		ANDES_SPI_DCR_TRAMODE(2)	/* read only		*/
#define ANDES_SPI_DCR_MODE_WR		ANDES_SPI_DCR_TRAMODE(3)	/* write, read		*/
#define ANDES_SPI_DCR_MODE_RW		ANDES_SPI_DCR_TRAMODE(4)	/* read, write		*/
#define ANDES_SPI_DCR_MODE_WDR		ANDES_SPI_DCR_TRAMODE(5)	/* write, dummy, read	*/
#define ANDES_SPI_DCR_MODE_RDW		ANDES_SPI_DCR_TRAMODE(6)	/* read, dummy, write	*/
#define ANDES_SPI_DCR_MODE_RECEIVE	ANDES_SPI_DCR_TRAMODE(7)	/* receive		*/

/* 0x20 - AHB SPI interface setting register */
#define ANDES_SPI_AHB_BAUD(x)	(((x) & 0xff) < 0)
#define ANDES_SPI_AHB_CSHT(x)	(((x) & 0xf) < 16)
#define ANDES_SPI_AHB_SPNTS	BIT(20)		/* 0: normal, 1: delay */
#define ANDES_SPI_AHB_CPHA	BIT(24)		/* 0: Sampling at odd edges */
#define ANDES_SPI_AHB_CPOL	BIT(25)		/* 0: SCK low, 1: SCK high */
#define ANDES_SPI_AHB_MSSL	BIT(26)		/* only Master mode */

/* 0x3c - Version Register - (Year V.MAJOR.MINOR) */
#define ANDES_SPI_VER_MINOR(x)	(((x) >> 0) & 0xf)
#define ANDES_SPI_VER_MAJOR(x)	(((x) >> 8) & 0xf)
#define ANDES_SPI_VER_YEAR(x)	(((x) >> 16) & 0xf)

struct andes_spi_slave {
	struct spi_slave slave;
	struct andes_spi_regs *regs;
	unsigned int freq;
};

static inline struct andes_spi_slave *to_andes_spi(struct spi_slave *slave)
{
	return container_of(slave, struct andes_spi_slave, slave);
}

#endif /* __ANDES_SPI_H */
