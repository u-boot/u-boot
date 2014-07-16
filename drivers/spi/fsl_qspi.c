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
#define TX_BUFFER_SIZE		0x40

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

/* Flash opcodes */
#define OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define OPCODE_RDSR		0x05	/* Read status register */
#define OPCODE_WREN		0x06	/* Write enable */
#define OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes */
#define OPCODE_FAST_READ_4B	0x0c    /* Read data bytes (high frequency) */
#define OPCODE_PP_4B		0x12    /* Page program (up to 256 bytes) */
#define OPCODE_SE_4B		0xdc    /* Sector erase (usually 64KiB) */

#ifdef CONFIG_SYS_FSL_QSPI_LE
#define qspi_read32		in_le32
#define qspi_write32		out_le32
#elif defined(CONFIG_SYS_FSL_QSPI_BE)
#define qspi_read32		in_be32
#define qspi_write32		out_be32
#endif

static unsigned long spi_bases[] = {
	QSPI0_BASE_ADDR,
};

static unsigned long amba_bases[] = {
	QSPI0_AMBA_BASE,
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
	qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_WREN) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Fast Read */
	lut_base = SEQID_FAST_READ * 4;
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_FAST_READ) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_FAST_READ_4B) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR32BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	qspi_write32(&regs->lut[lut_base + 1], OPRND0(8) | PAD0(LUT_PAD1) |
		INSTR0(LUT_DUMMY) | OPRND1(RX_BUFFER_SIZE) | PAD1(LUT_PAD1) |
		INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Read Status */
	lut_base = SEQID_RDSR * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_RDSR) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(1) |
		PAD1(LUT_PAD1) | INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Erase a sector */
	lut_base = SEQID_SE * 4;
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_SE) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_SE_4B) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR32BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Erase the whole chip */
	lut_base = SEQID_CHIP_ERASE * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_CHIP_ERASE) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Page Program */
	lut_base = SEQID_PP * 4;
	if (FSL_QSPI_FLASH_SIZE  <= SZ_16M)
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_PP) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR24BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	else
		qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_PP_4B) |
			PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(ADDR32BIT) |
			PAD1(LUT_PAD1) | INSTR1(LUT_ADDR));
	qspi_write32(&regs->lut[lut_base + 1], OPRND0(TX_BUFFER_SIZE) |
		PAD0(LUT_PAD1) | INSTR0(LUT_WRITE));
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* READ ID */
	lut_base = SEQID_RDID * 4;
	qspi_write32(&regs->lut[lut_base], OPRND0(OPCODE_RDID) |
		PAD0(LUT_PAD1) | INSTR0(LUT_CMD) | OPRND1(8) |
		PAD1(LUT_PAD1) | INSTR1(LUT_READ));
	qspi_write32(&regs->lut[lut_base + 1], 0);
	qspi_write32(&regs->lut[lut_base + 2], 0);
	qspi_write32(&regs->lut[lut_base + 3], 0);

	/* Lock the LUT */
	qspi_write32(&regs->lutkey, LUT_KEY_VALUE);
	qspi_write32(&regs->lckcr, QSPI_LCKCR_LOCK);
}

void spi_init()
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct fsl_qspi *qspi;
	struct fsl_qspi_regs *regs;
	u32 reg_val, smpr_val;
	u32 total_size, seq_id;

	if (bus >= ARRAY_SIZE(spi_bases))
		return NULL;

	qspi = spi_alloc_slave(struct fsl_qspi, bus, cs);
	if (!qspi)
		return NULL;

	qspi->reg_base = spi_bases[bus];
	qspi->amba_base = amba_bases[bus];

	qspi->slave.max_write_size = TX_BUFFER_SIZE;

	regs = (struct fsl_qspi_regs *)qspi->reg_base;
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK | QSPI_MCR_MDIS_MASK);

	smpr_val = qspi_read32(&regs->smpr);
	qspi_write32(&regs->smpr, smpr_val & ~(QSPI_SMPR_FSDLY_MASK |
		QSPI_SMPR_FSPHS_MASK | QSPI_SMPR_HSENA_MASK));
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK);

	total_size = FSL_QSPI_FLASH_SIZE * FSL_QSPI_FLASH_NUM;
	qspi_write32(&regs->sfa1ad, FSL_QSPI_FLASH_SIZE | qspi->amba_base);
	qspi_write32(&regs->sfa2ad, FSL_QSPI_FLASH_SIZE | qspi->amba_base);
	qspi_write32(&regs->sfb1ad, total_size | qspi->amba_base);
	qspi_write32(&regs->sfb2ad, total_size | qspi->amba_base);

	qspi_set_lut(qspi);

	smpr_val = qspi_read32(&regs->smpr);
	smpr_val &= ~QSPI_SMPR_DDRSMP_MASK;
	qspi_write32(&regs->smpr, smpr_val);
	qspi_write32(&regs->mcr, QSPI_MCR_RESERVED_MASK);

	seq_id = 0;
	reg_val = qspi_read32(&regs->bfgencr);
	reg_val &= ~QSPI_BFGENCR_SEQID_MASK;
	reg_val |= (seq_id << QSPI_BFGENCR_SEQID_SHIFT);
	reg_val &= ~QSPI_BFGENCR_PAR_EN_MASK;
	qspi_write32(&regs->bfgencr, reg_val);

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

static void qspi_op_pp(struct fsl_qspi *qspi, u32 *txbuf, u32 len)
{
	struct fsl_qspi_regs *regs = (struct fsl_qspi_regs *)qspi->reg_base;
	u32 mcr_reg, data, reg, status_reg;
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

	to_or_from = qspi->sf_addr + qspi->amba_base;
	qspi_write32(&regs->sfar, to_or_from);

	tx_size = (len > TX_BUFFER_SIZE) ?
		TX_BUFFER_SIZE : len;

	size = (tx_size + 3) / 4;

	for (i = 0; i < size; i++) {
		data = qspi_endian_xchg(*txbuf);
		qspi_write32(&regs->tbdr, data);
		txbuf++;
	}

	qspi_write32(&regs->ipcr,
		(SEQID_PP << QSPI_IPCR_SEQID_SHIFT) | tx_size);
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

static void qspi_op_se(struct fsl_qspi *qspi)
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

	qspi_write32(&regs->ipcr,
		(SEQID_SE << QSPI_IPCR_SEQID_SHIFT) | 0);
	while (qspi_read32(&regs->sr) & QSPI_SR_BUSY_MASK)
		;

	qspi_write32(&regs->mcr, mcr_reg);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct fsl_qspi *qspi = to_qspi_spi(slave);
	u32 bytes = DIV_ROUND_UP(bitlen, 8);
	static u32 pp_sfaddr;
	u32 txbuf;

	if (dout) {
		memcpy(&txbuf, dout, 4);
		qspi->cur_seqid = *(u8 *)dout;

		if (flags == SPI_XFER_END) {
			qspi->sf_addr = pp_sfaddr;
			qspi_op_pp(qspi, (u32 *)dout, bytes);
			return 0;
		}

		if (qspi->cur_seqid == OPCODE_FAST_READ) {
			qspi->sf_addr = swab32(txbuf) & OFFSET_BITS_MASK;
		} else if (qspi->cur_seqid == OPCODE_SE) {
			qspi->sf_addr = swab32(txbuf) & OFFSET_BITS_MASK;
			qspi_op_se(qspi);
		} else if (qspi->cur_seqid == OPCODE_PP) {
			pp_sfaddr = swab32(txbuf) & OFFSET_BITS_MASK;
		}
	}

	if (din) {
		if (qspi->cur_seqid == OPCODE_FAST_READ)
			qspi_op_read(qspi, din, bytes);
		else if (qspi->cur_seqid == OPCODE_RDID)
			qspi_op_rdid(qspi, din, bytes);
		else if (qspi->cur_seqid == OPCODE_RDSR)
			qspi_op_rdsr(qspi, din);
	}

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* Nothing to do */
}
