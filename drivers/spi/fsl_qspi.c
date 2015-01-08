/*
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 *
 * Freescale Quad Serial Peripheral Interface (QSPI) driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include "fsl_qspi.h"

#define RX_BUFFER_SIZE		0x80
#ifdef CONFIG_MX6SX
#define TX_BUFFER_SIZE		0x200
#else
#define TX_BUFFER_SIZE		0x40
#endif

#define OFFSET_BITS_MASK	0x00ffffff

#define FLASH_STATUS_WEL	0x02

/* SEQID */
#define SEQID_WREN		1
#define SEQID_FAST_READ		2
#define SEQID_RDSR		3
#define SEQID_SE		4
#define SEQID_CHIP_ERASE	5
#define SEQID_PP		6
#define SEQID_RDID		7
#define SEQID_BE_4K		8
#ifdef CONFIG_SPI_FLASH_BAR
#define SEQID_BRRD		9
#define SEQID_BRWR		10
#define SEQID_RDEAR		11
#define SEQID_WREAR		12
#endif

/* QSPI CMD */
#define QSPI_CMD_PP		0x02	/* Page program (up to 256 bytes) */
#define QSPI_CMD_RDSR		0x05	/* Read status register */
#define QSPI_CMD_WREN		0x06	/* Write enable */
#define QSPI_CMD_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define QSPI_CMD_BE_4K		0x20    /* 4K erase */
#define QSPI_CMD_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define QSPI_CMD_SE		0xd8	/* Sector erase (usually 64KiB) */
#define QSPI_CMD_RDID		0x9f	/* Read JEDEC ID */

/* Used for Micron, winbond and Macronix flashes */
#define	QSPI_CMD_WREAR		0xc5	/* EAR register write */
#define	QSPI_CMD_RDEAR		0xc8	/* EAR reigster read */

/* Used for Spansion flashes only. */
#define	QSPI_CMD_BRRD		0x16	/* Bank register read */
#define	QSPI_CMD_BRWR		0x17	/* Bank register write */

/* 4-byte address QSPI CMD - used on Spansion and some Macronix flashes */
#define QSPI_CMD_FAST_READ_4B	0x0c    /* Read data bytes (high frequency) */
#define QSPI_CMD_PP_4B		0x12    /* Page program (up to 256 bytes) */
#define QSPI_CMD_SE_4B		0xdc    /* Sector erase (usually 64KiB) */

#ifdef CONFIG_SYS_FSL_QSPI_LE
#define qspi_read32		in_le32
#define qspi_write32		out_le32
#elif defined(CONFIG_SYS_FSL_QSPI_BE)
#define qspi_read32		in_be32
#define qspi_write32		out_be32
#endif

static unsigned long spi_bases[] = {
	QSPI0_BASE_ADDR,
#ifdef CONFIG_MX6SX
	QSPI1_BASE_ADDR,
#endif
};

static unsigned long amba_bases[] = {
	QSPI0_AMBA_BASE,
#ifdef CONFIG_MX6SX
	QSPI1_AMBA_BASE,
#endif
};

struct fsl_qspi {
	struct spi_slave slave;
	unsigned long reg_base;
	unsigned long amba_base;
	u32 sf_addr;
	u8 cur_seqid;
};

/* QSPI support swapping the flash read/write data
 * in hardware for LS102xA, but not for VF610 */
static inline u32 qspi_endian_xchg(u32 data)
{
#ifdef CONFIG_VF610
	return swab32(data);
#else
	return data;
#endif
}

static inline struct fsl_qspi *to_qspi_spi(struct spi_slave *slave)
{
	return container_of(slave, struct fsl_qspi, slave);
}

static void qspi_set_lut(struct fsl_qspi *qspi)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 lut_base;

	/* Unlock the LUT */
	qspi_write32(&regs->lutkey, LUT_KEY_VALUE);
	qspi_write32(&regs->lckcr, QSPI_LCKCR_UNLOCK);

	/* Write Enable */
	lut_base = SEQID_WREN * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_WREN) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Fast Read */
	lut_base = SEQID_FAST_READ * 4;
#ifdef CONFIG_SPI_FLASH_BAR
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_FAST_READ) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
#else
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_FAST_READ) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base],
			     OPRND0(QSPI_CMD_FAST_READ_4B) |
			     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) |
			     OPRND1(ADDR32BIT) | PAD1(LUT_PAD1) |
			     INSTR1(LUT_ADDR));
#endif
	qspi_write32(&regs->lut[lut_base + 1], OPRND0(8) | PAD0(LUT_PAD1) |
		INSTR0(LUT_DUMMY) | OPRND1(RX_BUFFER_SIZE) | PAD1(LUT_PAD1) |
		INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Read Status */
	lut_base = SEQID_RDSR * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_RDSR) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		PAD1(LUT_PAD1) | INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Erase a sector */
	lut_base = SEQID_SE * 4;
#ifdef CONFIG_SPI_FLASH_BAR
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_SE) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
#else
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_SE) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_SE_4B) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR32BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
#endif
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Erase the whole chip */
	lut_base = SEQID_CHIP_ERASE * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_CHIP_ERASE) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Page Program */
	lut_base = SEQID_PP * 4;
#ifdef CONFIG_SPI_FLASH_BAR
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_PP) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
#else
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_PP) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_PP_4B) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR32BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
#endif
#ifdef CONFIG_MX6SX
	/*
	 * To MX6SX, OPRND0(TX_BUFFER_SIZE) can not work correctly.
	 * So, Use IDATSZ in IPCR to determine the size and here set 0.
	 */
	qspi_write32(&regs->lut[lut_base + 1], OPRND0(0) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_WRITE));
#else
	qspi_write32(&regs->lut[lut_base + 1], OPRND0(TX_BUFFER_SIZE) |
		PAD0(LUT_PAD1) | INSTR0(LUT_WRITE));
#endif
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* READ ID */
	lut_base = SEQID_RDID * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_RDID) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(8) |
		PAD1(LUT_PAD1) | INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* SUB SECTOR 4K ERASE */
	lut_base = SEQID_BE_4K * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_BE_4K) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));

#ifdef CONFIG_SPI_FLASH_BAR
	/*
	 * BRRD BRWR RDEAR WREAR are all supported, because it is hard to
	 * dynamically check whether to set BRRD BRWR or RDEAR WREAR during
	 * initialization.
	 */
	lut_base = SEQID_BRRD * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_BRRD) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_READ));

	lut_base = SEQID_BRWR * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_BRWR) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_WRITE));

	lut_base = SEQID_RDEAR * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_RDEAR) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_READ));

	lut_base = SEQID_WREAR * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(QSPI_CMD_WREAR) |
		     PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		     PAD1(LUT_PAD1) | INSTR1(LUT_WRITE));
#endif
	/* Lock the LUT */
	qspi_write32(&regs->lutkey, LUT_KEY_VALUE);
	qspi_write32(&regs->lckcr, QSPI_LCKCR_LOCK);
}

#if defined(CONFIG_SYS_FSL_QSPI_AHB)
/*
 * If we have changed the content of the flash by writing or erasing,
 * we need to invalidate the AHB buffer. If we do not do so, we may read out
 * the wrong data. The spec tells us reset the AHB domain and Serial Flash
 * domain at the same time.
 */
static inline void qspi_ahb_invalid(struct fsl_qspi *q)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)q->reg_base;
	u32 reg;

	reg = qspi_read32(&regs->mcr);
	reg |= QSPI_MCR_SWRSTHD_MASK | QSPI_MCR_SWRSTSD_MASK;
	qspi_write32(&regs->mcr, reg);

	/*
	 * The minimum delay : 1 AHB + 2 SFCK clocks.
	 * Delay 1 us is enough.
	 */
	udelay(1);

	reg &= ~(QSPI_MCR_SWRSTHD_MASK | QSPI_MCR_SWRSTSD_MASK);
	qspi_write32(&regs->mcr, reg);
}

/* Read out the data from the AHB buffer. */
static inline void qspi_ahb_read(struct fsl_qspi *q, u8 *rxbuf, int len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)q->reg_base;
	u32 mcr_reg;

	mcr_reg = qspi_read32(&regs->mcr);

	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		     QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);

	/* Read out the data directly from the AHB buffer. */
	memcpy(rxbuf, (u8 *)(q->amba_base + q->sf_addr), len);

	qspi_write32(&regs->mcr, mcr_reg);
}

static void qspi_enable_ddr_mode(struct fsl_qspi_regs *regs)
{
	u32 reg, reg2;

	reg = qspi_read32(&regs->mcr);
	/* Disable the module */
	qspi_write32(&regs->mcr, reg | QSPI_MCR_MDIS_MASK);

	/* Set the Sampling Register for DDR */
	reg2 = qspi_read32(&regs->smpr);
	reg2 &= ~QSPI_SMPR_DDRSMP_MASK;
	reg2 |= (2 << QSPI_SMPR_DDRSMP_SHIFT);
	qspi_write32(&regs->smpr, reg2);

	/* Enable the module again (enable the DDR too) */
	reg |= QSPI_MCR_DDR_EN_MASK;
	/* Enable bit 29 for imx6sx */
	reg |= (1 << 29);

	qspi_write32(&regs->mcr, reg);
}

/*
 * There are two different ways to read out the data from the flash:
 *  the "IP Command Read" and the "AHB Command Read".
 *
 * The IC guy suggests we use the "AHB Command Read" which is faster
 * then the "IP Command Read". (What's more is that there is a bug in
 * the "IP Command Read" in the Vybrid.)
 *
 * After we set up the registers for the "AHB Command Read", we can use
 * the memcpy to read the data directly. A "missed" access to the buffer
 * causes the controller to clear the buffer, and use the sequence pointed
 * by the QUADSPI_BFGENCR[SEQID] to initiate a read from the flash.
 */
static void qspi_init_ahb_read(struct fsl_qspi_regs *regs)
{
	/* AHB configuration for access buffer 0/1/2 .*/
	qspi_write32(&regs->buf0cr, QSPI_BUFXCR_INVALID_MSTRID);
	qspi_write32(&regs->buf1cr, QSPI_BUFXCR_INVALID_MSTRID);
	qspi_write32(&regs->buf2cr, QSPI_BUFXCR_INVALID_MSTRID);
	qspi_write32(&regs->buf3cr, QSPI_BUF3CR_ALLMST_MASK |
		     (0x80 << QSPI_BUF3CR_ADATSZ_SHIFT));

	/* We only use the buffer3 */
	qspi_write32(&regs->buf0ind, 0);
	qspi_write32(&regs->buf1ind, 0);
	qspi_write32(&regs->buf2ind, 0);

	/*
	 * Set the default lut sequence for AHB Read.
	 * Parallel mode is disabled.
	 */
	qspi_write32(&regs->bfgencr,
		     SEQID_FAST_READ << QSPI_BFGENCR_SEQID_SHIFT);

	/*Enable DDR Mode*/
	qspi_enable_ddr_mode(regs);
}
#endif

void spi_init()
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct fsl_qspi *qspi;
	struct fsl_qspi_regs *regs;
	u32 smpr_val;
	u32 total_size;

	if (bus >= ARRAY_SIZE(spi_bases))
		return NULL;

	if (cs >= FSL_QSPI_FLASH_NUM)
		return NULL;

	qspi = spi_alloc_slave(struct fsl_qspi, bus, cs);
	if (!qspi)
		return NULL;

	qspi->reg_base = spi_bases[bus];
	/*
	 * According cs, use different amba_base to choose the
	 * corresponding flash devices.
	 *
	 * If not, only one flash device is used even if passing
	 * different cs using `sf probe`
	 */
	qspi->amba_base = amba_bases[bus] + cs * FSL_QSPI_FLASH_SIZE;

	qspi->slave.max_write_size = TX_BUFFER_SIZE;

	regs = (struct fsl_qspi_regs *)qspi->reg_base;
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK | QSPI_MCR_MDIS_MASK);

	smpr_val = qspi_read32(&regs->smpr);
	qspi_write32(&regs->smpr, smpr_val & ~(QSPI_SMPR_FSDLY_MASK |
		QSPI_SMPR_FSPHS_MASK | QSPI_SMPR_HSENA_MASK));
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK);

	total_size = FSL_QSPI_FLASH_SIZE * FSL_QSPI_FLASH_NUM;
	/*
	 * Any read access to non-implemented addresses will provide
	 * undefined results.
	 *
	 * In case single die flash devices, TOP_ADDR_MEMA2 and
	 * TOP_ADDR_MEMB2 should be initialized/programmed to
	 * TOP_ADDR_MEMA1 and TOP_ADDR_MEMB1 respectively - in effect,
	 * setting the size of these devices to 0.  This would ensure
	 * that the complete memory map is assigned to only one flash device.
	 */
	qspi_write32(&regs->sfa1ad, FSL_QSPI_FLASH_SIZE | amba_bases[bus]);
	qspi_write32(&regs->sfa2ad, FSL_QSPI_FLASH_SIZE | amba_bases[bus]);
	qspi_write32(&regs->sfb1ad, total_size | amba_bases[bus]);
	qspi_write32(&regs->sfb2ad, total_size | amba_bases[bus]);

	qspi_set_lut(qspi);

	smpr_val = qspi_read32(&regs->smpr);
	smpr_val &= ~QSPI_SMPR_DDRSMP_MASK;
	qspi_write32(&regs->smpr, smpr_val);
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK);

#ifdef CONFIG_SYS_FSL_QSPI_AHB
	qspi_init_ahb_read(regs);
#endif
	return &qspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct fsl_qspi *qspi = to_qspi_spi(slave);

	free(qspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

#ifdef CONFIG_SPI_FLASH_BAR
/* Bank register read/write, EAR register read/write */
static void qspi_op_rdbank(struct fsl_qspi *qspi, u8 *rxbuf, u32 len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 reg, mcr_reg, data, seqid;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		     QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	qspi_write32(&regs->sfar, qspi->amba_base);

	if (qspi->cur_seqid == QSPI_CMD_BRRD)
		seqid = SEQID_BRRD;
	else
		seqid = SEQID_RDEAR;

	qspi_write32(&regs->ipcr, (seqid << QSPI_IPCR_SEQID_SHIFT) | len);

	/* Wait previous command complete */
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	while (1) {
		reg = qspi_read32(&regs->rbsr);
		if (reg & QSPI_RBSR_RDBFL_MASK) {
			data = qspi_read32(&regs->rbdr[0]);
			data = qspi_endian_xchg(data);
			memcpy(rxbuf, &data, len);
			qspi_write32(&regs->mcr, qspi_read32(&regs->mcr) |
				     QSPI_MCR_CLR_RXF_MASK);
			break;
		}
	}

	qspi_write32(&regs->mcr, mcr_reg);
}
#endif

static void qspi_op_rdid(struct fsl_qspi *qspi, u32 *rxbuf, u32 len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg, rbsr_reg, data;
	int i, size;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	qspi_write32(&regs->sfar, qspi->amba_base);

	qspi_write32(&regs->ipcr, (SEQID_RDID << QSPI_IPCR_SEQID_SHIFT) | 0);
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	i = 0;
	size = len;
	while ((RX_BUFFER_SIZE >= size) && (size > 0)) {
		rbsr_reg = qspi_read32(&regs->rbsr);
		if (rbsr_reg & QSPI_RBSR_RDBFL_MASK) {
			data = qspi_read32(&regs->rbdr[i]);
			data = qspi_endian_xchg(data);
			memcpy(rxbuf, &data, 4);
			rxbuf++;
			size -= 4;
			i++;
		}
	}

	qspi_write32(&regs->mcr, mcr_reg);
}

#ifndef CONFIG_SYS_FSL_QSPI_AHB
/* If not use AHB read, read data from ip interface */
static void qspi_op_read(struct fsl_qspi *qspi, u32 *rxbuf, u32 len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg, data;
	int i, size;
	u32 to_or_from;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	to_or_from = qspi->sf_addr + qspi->amba_base;

	while (len > 0) {
		qspi_write32(&regs->sfar, to_or_from);

		size = (len > RX_BUFFER_SIZE) ?
			RX_BUFFER_SIZE : len;

		qspi_write32(&regs->ipcr,
			(SEQID_FAST_READ << QSPI_IPCR_SEQID_SHIFT) | size);
		while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
			;

		to_or_from += size;
		len -= size;

		i = 0;
		while ((RX_BUFFER_SIZE >= size) && (size > 0)) {
			data = qspi_read32(&regs->rbdr[i]);
			data = qspi_endian_xchg(data);
			memcpy(rxbuf, &data, 4);
			rxbuf++;
			size -= 4;
			i++;
		}
		qspi_write32(&regs->mcr, qspi_read32(&regs->mcr) |
			QSPI_MCR_CLR_RXF_MASK);
	}

	qspi_write32(&regs->mcr, mcr_reg);
}
#endif

static void qspi_op_write(struct fsl_qspi *qspi, u8 *txbuf, u32 len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg, data, reg, status_reg, seqid;
	int i, size, tx_size;
	u32 to_or_from = 0;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	status_reg = 0;
	while ((status_reg & FLASH_STATUS_WEL) != FLASH_STATUS_WEL) {
		qspi_write32(&regs->ipcr,
			(SEQID_WREN << QSPI_IPCR_SEQID_SHIFT) | 0);
		while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
			;

		qspi_write32(&regs->ipcr,
			(SEQID_RDSR << QSPI_IPCR_SEQID_SHIFT) | 1);
		while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
			;

		reg = qspi_read32(&regs->rbsr);
		if (reg & QSPI_RBSR_RDBFL_MASK) {
			status_reg = qspi_read32(&regs->rbdr[0]);
			status_reg = qspi_endian_xchg(status_reg);
		}
		qspi_write32(&regs->mcr,
			qspi_read32(&regs->mcr) | QSPI_MCR_CLR_RXF_MASK);
	}

	/* Default is page programming */
	seqid = SEQID_PP;
#ifdef CONFIG_SPI_FLASH_BAR
	if (qspi->cur_seqid == QSPI_CMD_BRWR)
		seqid = SEQID_BRWR;
	else if (qspi->cur_seqid == QSPI_CMD_WREAR)
		seqid = SEQID_WREAR;
#endif

	to_or_from = qspi->sf_addr + qspi->amba_base;

	qspi_write32(&regs->sfar, to_or_from);

	tx_size = (len > TX_BUFFER_SIZE) ?
		TX_BUFFER_SIZE : len;

	size = tx_size / 4;
	for (i = 0; i < size; i++) {
		memcpy(&data, txbuf, 4);
		data = qspi_endian_xchg(data);
		qspi_write32(&regs->tbdr, data);
		txbuf += 4;
	}

	size = tx_size % 4;
	if (size) {
		data = 0;
		memcpy(&data, txbuf, size);
		data = qspi_endian_xchg(data);
		qspi_write32(&regs->tbdr, data);
	}

	qspi_write32(&regs->ipcr, (seqid << QSPI_IPCR_SEQID_SHIFT) | tx_size);
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	qspi_write32(&regs->mcr, mcr_reg);
}

static void qspi_op_rdsr(struct fsl_qspi *qspi, u32 *rxbuf)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg, reg, data;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	qspi_write32(&regs->sfar, qspi->amba_base);

	qspi_write32(&regs->ipcr,
		(SEQID_RDSR << QSPI_IPCR_SEQID_SHIFT) | 0);
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	while (1) {
		reg = qspi_read32(&regs->rbsr);
		if (reg & QSPI_RBSR_RDBFL_MASK) {
			data = qspi_read32(&regs->rbdr[0]);
			data = qspi_endian_xchg(data);
			memcpy(rxbuf, &data, 4);
			qspi_write32(&regs->mcr, qspi_read32(&regs->mcr) |
				QSPI_MCR_CLR_RXF_MASK);
			break;
		}
	}

	qspi_write32(&regs->mcr, mcr_reg);
}

static void qspi_op_erase(struct fsl_qspi *qspi)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg;
	u32 to_or_from = 0;

	mcr_reg = qspi_read32(&regs->mcr);
	qspi_write32(&regs->mcr, QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	qspi_write32(&regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	to_or_from = qspi->sf_addr + qspi->amba_base;
	qspi_write32(&regs->sfar, to_or_from);

	qspi_write32(&regs->ipcr,
		(SEQID_WREN << QSPI_IPCR_SEQID_SHIFT) | 0);
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	if (qspi->cur_seqid == QSPI_CMD_SE) {
		qspi_write32(&regs->ipcr,
			     (SEQID_SE << QSPI_IPCR_SEQID_SHIFT) | 0);
	} else if (qspi->cur_seqid == QSPI_CMD_BE_4K) {
		qspi_write32(&regs->ipcr,
			     (SEQID_BE_4K << QSPI_IPCR_SEQID_SHIFT) | 0);
	}
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	qspi_write32(&regs->mcr, mcr_reg);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct fsl_qspi *qspi = to_qspi_spi(slave);
	u32 bytes = DIV_ROUND_UP(bitlen, 8);
	static u32 wr_sfaddr;
	u32 txbuf;

	if (dout) {
		if (flags & SPI_XFER_BEGIN) {
			qspi->cur_seqid = *(u8 *)dout;
			memcpy(&txbuf, dout, 4);
		}

		if (flags == SPI_XFER_END) {
			qspi->sf_addr = wr_sfaddr;
			qspi_op_write(qspi, (u8 *)dout, bytes);
			return 0;
		}

		if (qspi->cur_seqid == QSPI_CMD_FAST_READ) {
			qspi->sf_addr = swab32(txbuf) & OFFSET_BITS_MASK;
		} else if ((qspi->cur_seqid == QSPI_CMD_SE) ||
			   (qspi->cur_seqid == QSPI_CMD_BE_4K)) {
			qspi->sf_addr = swab32(txbuf) & OFFSET_BITS_MASK;
			qspi_op_erase(qspi);
		} else if (qspi->cur_seqid == QSPI_CMD_PP)
			wr_sfaddr = swab32(txbuf) & OFFSET_BITS_MASK;
#ifdef CONFIG_SPI_FLASH_BAR
		else if ((qspi->cur_seqid == QSPI_CMD_BRWR) ||
			 (qspi->cur_seqid == QSPI_CMD_WREAR)) {
			wr_sfaddr = 0;
		}
#endif
	}

	if (din) {
		if (qspi->cur_seqid == QSPI_CMD_FAST_READ) {
#ifdef CONFIG_SYS_FSL_QSPI_AHB
			qspi_ahb_read(qspi, din, bytes);
#else
			qspi_op_read(qspi, din, bytes);
#endif
		}
		else if (qspi->cur_seqid == QSPI_CMD_RDID)
			qspi_op_rdid(qspi, din, bytes);
		else if (qspi->cur_seqid == QSPI_CMD_RDSR)
			qspi_op_rdsr(qspi, din);
#ifdef CONFIG_SPI_FLASH_BAR
		else if ((qspi->cur_seqid == QSPI_CMD_BRRD) ||
			 (qspi->cur_seqid == QSPI_CMD_RDEAR)) {
			qspi->sf_addr = 0;
			qspi_op_rdbank(qspi, din, bytes);
		}
#endif
	}

#ifdef CONFIG_SYS_FSL_QSPI_AHB
	if ((qspi->cur_seqid == QSPI_CMD_SE) ||
	    (qspi->cur_seqid == QSPI_CMD_PP) ||
	    (qspi->cur_seqid == QSPI_CMD_BE_4K) ||
	    (qspi->cur_seqid == QSPI_CMD_WREAR) ||
	    (qspi->cur_seqid == QSPI_CMD_BRWR))
		qspi_ahb_invalid(qspi);
#endif

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* Nothing to do */
}
