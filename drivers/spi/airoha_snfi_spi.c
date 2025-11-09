// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 *
 * Based on spi-airoha-snfi.c on Linux
 *
 * Author: Lorenzo Bianconi <lorenzo@kernel.org>
 * Author: Ray Liu <ray.liu@airoha.com>
 */

#include <asm/unaligned.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitfield.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/spinand.h>
#include <linux/time.h>
#include <regmap.h>
#include <spi.h>
#include <spi-mem.h>

/* SPI */
#define REG_SPI_CTRL_READ_MODE			0x0000
#define REG_SPI_CTRL_READ_IDLE_EN		0x0004
#define REG_SPI_CTRL_SIDLY			0x0008
#define REG_SPI_CTRL_CSHEXT			0x000c
#define REG_SPI_CTRL_CSLEXT			0x0010

#define REG_SPI_CTRL_MTX_MODE_TOG		0x0014
#define SPI_CTRL_MTX_MODE_TOG			GENMASK(3, 0)

#define REG_SPI_CTRL_RDCTL_FSM			0x0018
#define SPI_CTRL_RDCTL_FSM			GENMASK(3, 0)

#define REG_SPI_CTRL_MACMUX_SEL			0x001c

#define REG_SPI_CTRL_MANUAL_EN			0x0020
#define SPI_CTRL_MANUAL_EN			BIT(0)

#define REG_SPI_CTRL_OPFIFO_EMPTY		0x0024
#define SPI_CTRL_OPFIFO_EMPTY			BIT(0)

#define REG_SPI_CTRL_OPFIFO_WDATA		0x0028
#define SPI_CTRL_OPFIFO_LEN			GENMASK(8, 0)
#define SPI_CTRL_OPFIFO_OP			GENMASK(13, 9)

#define REG_SPI_CTRL_OPFIFO_FULL		0x002c
#define SPI_CTRL_OPFIFO_FULL			BIT(0)

#define REG_SPI_CTRL_OPFIFO_WR			0x0030
#define SPI_CTRL_OPFIFO_WR			BIT(0)

#define REG_SPI_CTRL_DFIFO_FULL			0x0034
#define SPI_CTRL_DFIFO_FULL			BIT(0)

#define REG_SPI_CTRL_DFIFO_WDATA		0x0038
#define SPI_CTRL_DFIFO_WDATA			GENMASK(7, 0)

#define REG_SPI_CTRL_DFIFO_EMPTY		0x003c
#define SPI_CTRL_DFIFO_EMPTY			BIT(0)

#define REG_SPI_CTRL_DFIFO_RD			0x0040
#define SPI_CTRL_DFIFO_RD			BIT(0)

#define REG_SPI_CTRL_DFIFO_RDATA		0x0044
#define SPI_CTRL_DFIFO_RDATA			GENMASK(7, 0)

#define REG_SPI_CTRL_DUMMY			0x0080
#define SPI_CTRL_CTRL_DUMMY			GENMASK(3, 0)

#define REG_SPI_CTRL_PROBE_SEL			0x0088
#define REG_SPI_CTRL_INTERRUPT			0x0090
#define REG_SPI_CTRL_INTERRUPT_EN		0x0094
#define REG_SPI_CTRL_SI_CK_SEL			0x009c
#define REG_SPI_CTRL_SW_CFGNANDADDR_VAL		0x010c
#define REG_SPI_CTRL_SW_CFGNANDADDR_EN		0x0110
#define REG_SPI_CTRL_SFC_STRAP			0x0114

#define REG_SPI_CTRL_NFI2SPI_EN			0x0130
#define SPI_CTRL_NFI2SPI_EN			BIT(0)

/* NFI2SPI */
#define REG_SPI_NFI_CNFG			0x0000
#define SPI_NFI_DMA_MODE			BIT(0)
#define SPI_NFI_READ_MODE			BIT(1)
#define SPI_NFI_DMA_BURST_EN			BIT(2)
#define SPI_NFI_HW_ECC_EN			BIT(8)
#define SPI_NFI_AUTO_FDM_EN			BIT(9)
#define SPI_NFI_OPMODE				GENMASK(14, 12)

#define REG_SPI_NFI_PAGEFMT			0x0004
#define SPI_NFI_PAGE_SIZE			GENMASK(1, 0)
#define SPI_NFI_SPARE_SIZE			GENMASK(5, 4)

#define REG_SPI_NFI_CON				0x0008
#define SPI_NFI_FIFO_FLUSH			BIT(0)
#define SPI_NFI_RST				BIT(1)
#define SPI_NFI_RD_TRIG				BIT(8)
#define SPI_NFI_WR_TRIG				BIT(9)
#define SPI_NFI_SEC_NUM				GENMASK(15, 12)

#define REG_SPI_NFI_INTR_EN			0x0010
#define SPI_NFI_RD_DONE_EN			BIT(0)
#define SPI_NFI_WR_DONE_EN			BIT(1)
#define SPI_NFI_RST_DONE_EN			BIT(2)
#define SPI_NFI_ERASE_DONE_EN			BIT(3)
#define SPI_NFI_BUSY_RETURN_EN			BIT(4)
#define SPI_NFI_ACCESS_LOCK_EN			BIT(5)
#define SPI_NFI_AHB_DONE_EN			BIT(6)
#define SPI_NFI_ALL_IRQ_EN					\
	(SPI_NFI_RD_DONE_EN | SPI_NFI_WR_DONE_EN |		\
	 SPI_NFI_RST_DONE_EN | SPI_NFI_ERASE_DONE_EN |		\
	 SPI_NFI_BUSY_RETURN_EN | SPI_NFI_ACCESS_LOCK_EN |	\
	 SPI_NFI_AHB_DONE_EN)

#define REG_SPI_NFI_INTR			0x0014
#define SPI_NFI_AHB_DONE			BIT(6)

#define REG_SPI_NFI_CMD				0x0020

#define REG_SPI_NFI_ADDR_NOB			0x0030
#define SPI_NFI_ROW_ADDR_NOB			GENMASK(6, 4)

#define REG_SPI_NFI_STA				0x0060
#define REG_SPI_NFI_FIFOSTA			0x0064
#define REG_SPI_NFI_STRADDR			0x0080
#define REG_SPI_NFI_FDM0L			0x00a0
#define REG_SPI_NFI_FDM0M			0x00a4
#define REG_SPI_NFI_FDM7L			0x00d8
#define REG_SPI_NFI_FDM7M			0x00dc
#define REG_SPI_NFI_FIFODATA0			0x0190
#define REG_SPI_NFI_FIFODATA1			0x0194
#define REG_SPI_NFI_FIFODATA2			0x0198
#define REG_SPI_NFI_FIFODATA3			0x019c
#define REG_SPI_NFI_MASTERSTA			0x0224

#define REG_SPI_NFI_SECCUS_SIZE			0x022c
#define SPI_NFI_CUS_SEC_SIZE			GENMASK(12, 0)
#define SPI_NFI_CUS_SEC_SIZE_EN			BIT(16)

#define REG_SPI_NFI_RD_CTL2			0x0510
#define SPI_NFI_DATA_READ_CMD			GENMASK(7, 0)

#define REG_SPI_NFI_RD_CTL3			0x0514

#define REG_SPI_NFI_PG_CTL1			0x0524
#define SPI_NFI_PG_LOAD_CMD			GENMASK(15, 8)

#define REG_SPI_NFI_PG_CTL2			0x0528

#define REG_SPI_NFI_NOR_PROG_ADDR		0x052c
#define REG_SPI_NFI_NOR_RD_ADDR			0x0534

#define REG_SPI_NFI_SNF_MISC_CTL		0x0538
#define SPI_NFI_DATA_READ_WR_MODE		GENMASK(18, 16)

#define REG_SPI_NFI_SNF_MISC_CTL2		0x053c
#define SPI_NFI_READ_DATA_BYTE_NUM		GENMASK(12, 0)
#define SPI_NFI_PROG_LOAD_BYTE_NUM		GENMASK(28, 16)

#define REG_SPI_NFI_SNF_STA_CTL1		0x0550
#define SPI_NFI_READ_FROM_CACHE_DONE		BIT(25)
#define SPI_NFI_LOAD_TO_CACHE_DONE		BIT(26)

#define REG_SPI_NFI_SNF_STA_CTL2		0x0554

#define REG_SPI_NFI_SNF_NFI_CNFG		0x055c
#define SPI_NFI_SPI_MODE			BIT(0)

/* SPI NAND Protocol OP */
#define SPI_NAND_OP_GET_FEATURE			0x0f
#define SPI_NAND_OP_SET_FEATURE			0x1f
#define SPI_NAND_OP_PAGE_READ			0x13
#define SPI_NAND_OP_READ_FROM_CACHE_SINGLE	0x03
#define SPI_NAND_OP_READ_FROM_CACHE_SINGLE_FAST	0x0b
#define SPI_NAND_OP_READ_FROM_CACHE_DUAL	0x3b
#define SPI_NAND_OP_READ_FROM_CACHE_DUALIO	0xbb
#define SPI_NAND_OP_READ_FROM_CACHE_QUAD	0x6b
#define SPI_NAND_OP_READ_FROM_CACHE_QUADIO	0xeb
#define SPI_NAND_OP_WRITE_ENABLE		0x06
#define SPI_NAND_OP_WRITE_DISABLE		0x04
#define SPI_NAND_OP_PROGRAM_LOAD_SINGLE		0x02
#define SPI_NAND_OP_PROGRAM_LOAD_QUAD		0x32
#define SPI_NAND_OP_PROGRAM_LOAD_RAMDOM_SINGLE	0x84
#define SPI_NAND_OP_PROGRAM_LOAD_RAMDON_QUAD	0x34
#define SPI_NAND_OP_PROGRAM_EXECUTE		0x10
#define SPI_NAND_OP_READ_ID			0x9f
#define SPI_NAND_OP_BLOCK_ERASE			0xd8
#define SPI_NAND_OP_RESET			0xff
#define SPI_NAND_OP_DIE_SELECT			0xc2

/* SNAND FIFO commands */
#define SNAND_FIFO_TX_BUSWIDTH_SINGLE		0x08
#define SNAND_FIFO_TX_BUSWIDTH_DUAL		0x09
#define SNAND_FIFO_TX_BUSWIDTH_QUAD		0x0a
#define SNAND_FIFO_RX_BUSWIDTH_SINGLE		0x0c
#define SNAND_FIFO_RX_BUSWIDTH_DUAL		0x0e
#define SNAND_FIFO_RX_BUSWIDTH_QUAD		0x0f

#define SPI_NAND_CACHE_SIZE			(SZ_4K + SZ_256)
#define SPI_MAX_TRANSFER_SIZE			511

enum airoha_snand_mode {
	SPI_MODE_AUTO,
	SPI_MODE_MANUAL,
	SPI_MODE_DMA,
};

enum airoha_snand_cs {
	SPI_CHIP_SEL_HIGH,
	SPI_CHIP_SEL_LOW,
};

struct airoha_snand_priv {
	struct regmap *regmap_ctrl;
	struct regmap *regmap_nfi;
	struct clk *spi_clk;

	u8 *txrx_buf;
	int dma;
};

static int airoha_snand_set_fifo_op(struct airoha_snand_priv *priv,
				    u8 op_cmd, int op_len)
{
	int err;
	u32 val;

	err = regmap_write(priv->regmap_ctrl, REG_SPI_CTRL_OPFIFO_WDATA,
			   FIELD_PREP(SPI_CTRL_OPFIFO_LEN, op_len) |
			   FIELD_PREP(SPI_CTRL_OPFIFO_OP, op_cmd));
	if (err)
		return err;

	err = regmap_read_poll_timeout(priv->regmap_ctrl,
				       REG_SPI_CTRL_OPFIFO_FULL,
				       val, !(val & SPI_CTRL_OPFIFO_FULL),
				       0, 250 * USEC_PER_MSEC);
	if (err)
		return err;

	err = regmap_write(priv->regmap_ctrl, REG_SPI_CTRL_OPFIFO_WR,
			   SPI_CTRL_OPFIFO_WR);
	if (err)
		return err;

	return regmap_read_poll_timeout(priv->regmap_ctrl,
					REG_SPI_CTRL_OPFIFO_EMPTY,
					val, (val & SPI_CTRL_OPFIFO_EMPTY),
					0, 250 * USEC_PER_MSEC);
}

static int airoha_snand_set_cs(struct airoha_snand_priv *priv, u8 cs)
{
	return airoha_snand_set_fifo_op(priv, cs, sizeof(cs));
}

static int airoha_snand_write_data_to_fifo(struct airoha_snand_priv *priv,
					   const u8 *data, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		int err;
		u32 val;

		/* 1. Wait until dfifo is not full */
		err = regmap_read_poll_timeout(priv->regmap_ctrl,
					       REG_SPI_CTRL_DFIFO_FULL, val,
					       !(val & SPI_CTRL_DFIFO_FULL),
					       0, 250 * USEC_PER_MSEC);
		if (err)
			return err;

		/* 2. Write data to register DFIFO_WDATA */
		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_DFIFO_WDATA,
				   FIELD_PREP(SPI_CTRL_DFIFO_WDATA, data[i]));
		if (err)
			return err;

		/* 3. Wait until dfifo is not full */
		err = regmap_read_poll_timeout(priv->regmap_ctrl,
					       REG_SPI_CTRL_DFIFO_FULL, val,
					       !(val & SPI_CTRL_DFIFO_FULL),
					       0, 250 * USEC_PER_MSEC);
		if (err)
			return err;
	}

	return 0;
}

static int airoha_snand_read_data_from_fifo(struct airoha_snand_priv *priv,
					    u8 *ptr, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		int err;
		u32 val;

		/* 1. wait until dfifo is not empty */
		err = regmap_read_poll_timeout(priv->regmap_ctrl,
					       REG_SPI_CTRL_DFIFO_EMPTY, val,
					       !(val & SPI_CTRL_DFIFO_EMPTY),
					       0, 250 * USEC_PER_MSEC);
		if (err)
			return err;

		/* 2. read from dfifo to register DFIFO_RDATA */
		err = regmap_read(priv->regmap_ctrl,
				  REG_SPI_CTRL_DFIFO_RDATA, &val);
		if (err)
			return err;

		ptr[i] = FIELD_GET(SPI_CTRL_DFIFO_RDATA, val);
		/* 3. enable register DFIFO_RD to read next byte */
		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_DFIFO_RD, SPI_CTRL_DFIFO_RD);
		if (err)
			return err;
	}

	return 0;
}

static int airoha_snand_set_mode(struct airoha_snand_priv *priv,
				 enum airoha_snand_mode mode)
{
	int err;

	switch (mode) {
	case SPI_MODE_MANUAL: {
		u32 val;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_NFI2SPI_EN, 0);
		if (err)
			return err;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_READ_IDLE_EN, 0);
		if (err)
			return err;

		err = regmap_read_poll_timeout(priv->regmap_ctrl,
					       REG_SPI_CTRL_RDCTL_FSM, val,
					       !(val & SPI_CTRL_RDCTL_FSM),
					       0, 250 * USEC_PER_MSEC);
		if (err)
			return err;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_MTX_MODE_TOG, 9);
		if (err)
			return err;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_MANUAL_EN, SPI_CTRL_MANUAL_EN);
		if (err)
			return err;
		break;
	}
	case SPI_MODE_DMA:
		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_NFI2SPI_EN,
				   SPI_CTRL_MANUAL_EN);
		if (err < 0)
			return err;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_MTX_MODE_TOG, 0x0);
		if (err < 0)
			return err;

		err = regmap_write(priv->regmap_ctrl,
				   REG_SPI_CTRL_MANUAL_EN, 0x0);
		if (err < 0)
			return err;
		break;
	case SPI_MODE_AUTO:
	default:
		break;
	}

	return regmap_write(priv->regmap_ctrl, REG_SPI_CTRL_DUMMY, 0);
}

static int airoha_snand_write_data(struct airoha_snand_priv *priv,
				   const u8 *data, int len, int buswidth)
{
	int i, data_len;
	u8 cmd;

	switch (buswidth) {
	case 0:
	case 1:
		cmd = SNAND_FIFO_TX_BUSWIDTH_SINGLE;
		break;
	case 2:
		cmd = SNAND_FIFO_TX_BUSWIDTH_DUAL;
		break;
	case 4:
		cmd = SNAND_FIFO_TX_BUSWIDTH_QUAD;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < len; i += data_len) {
		int err;

		data_len = min(len - i, SPI_MAX_TRANSFER_SIZE);
		err = airoha_snand_set_fifo_op(priv, cmd, data_len);
		if (err)
			return err;

		err = airoha_snand_write_data_to_fifo(priv, &data[i],
						      data_len);
		if (err < 0)
			return err;
	}

	return 0;
}

static int airoha_snand_read_data(struct airoha_snand_priv *priv,
				  u8 *data, int len, int buswidth)
{
	int i, data_len;
	u8 cmd;

	switch (buswidth) {
	case 0:
	case 1:
		cmd = SNAND_FIFO_RX_BUSWIDTH_SINGLE;
		break;
	case 2:
		cmd = SNAND_FIFO_RX_BUSWIDTH_DUAL;
		break;
	case 4:
		cmd = SNAND_FIFO_RX_BUSWIDTH_QUAD;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < len; i += data_len) {
		int err;

		data_len = min(len - i, SPI_MAX_TRANSFER_SIZE);
		err = airoha_snand_set_fifo_op(priv, cmd, data_len);
		if (err)
			return err;

		err = airoha_snand_read_data_from_fifo(priv, &data[i],
						       data_len);
		if (err < 0)
			return err;
	}

	return 0;
}

static int airoha_snand_nfi_init(struct airoha_snand_priv *priv)
{
	int err;

	/* switch to SNFI mode */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_SNF_NFI_CNFG,
			   SPI_NFI_SPI_MODE);
	if (err)
		return err;

	/* Enable DMA */
	return regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_INTR_EN,
				  SPI_NFI_ALL_IRQ_EN, SPI_NFI_AHB_DONE_EN);
}

static bool airoha_snand_is_page_ops(const struct spi_mem_op *op)
{
	if (op->addr.nbytes != 2)
		return false;

	if (op->addr.buswidth != 1 && op->addr.buswidth != 2 &&
	    op->addr.buswidth != 4)
		return false;

	switch (op->data.dir) {
	case SPI_MEM_DATA_IN:
		if (op->dummy.nbytes * BITS_PER_BYTE / op->dummy.buswidth > 0xf)
			return false;

		/* quad in / quad out */
		if (op->addr.buswidth == 4)
			return op->data.buswidth == 4;

		if (op->addr.buswidth == 2)
			return op->data.buswidth == 2;

		/* standard spi */
		return op->data.buswidth == 4 || op->data.buswidth == 2 ||
		       op->data.buswidth == 1;
	case SPI_MEM_DATA_OUT:
		return !op->dummy.nbytes && op->addr.buswidth == 1 &&
		       (op->data.buswidth == 4 || op->data.buswidth == 1);
	default:
		return false;
	}
}

static bool airoha_snand_supports_op(struct spi_slave *slave,
				     const struct spi_mem_op *op)
{
	if (!spi_mem_default_supports_op(slave, op))
		return false;

	if (op->cmd.buswidth != 1)
		return false;

	if (airoha_snand_is_page_ops(op))
		return true;

	return (!op->addr.nbytes || op->addr.buswidth == 1) &&
	       (!op->dummy.nbytes || op->dummy.buswidth == 1) &&
	       (!op->data.nbytes || op->data.buswidth == 1);
}

static int airoha_snand_dirmap_create(struct spi_mem_dirmap_desc *desc)
{
	struct spi_slave *slave = desc->slave;
	struct udevice *bus = slave->dev->parent;
	struct airoha_snand_priv *priv = dev_get_priv(bus);

	if (!priv->txrx_buf)
		return -EINVAL;

	if (desc->info.offset + desc->info.length > U32_MAX)
		return -EINVAL;

	/* continuous reading is not supported */
	if (desc->info.length > SPI_NAND_CACHE_SIZE)
		return -E2BIG;

	if (!airoha_snand_supports_op(desc->slave, &desc->info.op_tmpl))
		return -EOPNOTSUPP;

	return 0;
}

static ssize_t airoha_snand_dirmap_read(struct spi_mem_dirmap_desc *desc,
					u64 offs, size_t len, void *buf)
{
	struct spi_slave *slave = desc->slave;
	struct udevice *bus = slave->dev->parent;
	struct airoha_snand_priv *priv = dev_get_priv(bus);
	u8 *txrx_buf = priv->txrx_buf;
	dma_addr_t dma_addr;
	u32 val, rd_mode, opcode;
	size_t bytes;
	int err;

	if (!priv->dma) {
		/* simplified version of spi_mem_no_dirmap_read() */
		struct spi_mem_op op = desc->info.op_tmpl;

		op.addr.val = desc->info.offset + offs;
		op.data.buf.in = buf;
		op.data.nbytes = len;
		err = spi_mem_exec_op(desc->slave, &op);
		if (err)
			return err;

		return op.data.nbytes;
	}

	/* minimum oob size is 64 */
	bytes = round_up(offs + len, 64);

	/*
	 * DUALIO and QUADIO opcodes are not supported by the spi controller,
	 * replace them with supported opcodes.
	 */
	opcode = desc->info.op_tmpl.cmd.opcode;
	switch (opcode) {
	case SPI_NAND_OP_READ_FROM_CACHE_SINGLE:
	case SPI_NAND_OP_READ_FROM_CACHE_SINGLE_FAST:
		rd_mode = 0;
		break;
	case SPI_NAND_OP_READ_FROM_CACHE_DUAL:
	case SPI_NAND_OP_READ_FROM_CACHE_DUALIO:
		opcode = SPI_NAND_OP_READ_FROM_CACHE_DUAL;
		rd_mode = 1;
		break;
	case SPI_NAND_OP_READ_FROM_CACHE_QUAD:
	case SPI_NAND_OP_READ_FROM_CACHE_QUADIO:
		opcode = SPI_NAND_OP_READ_FROM_CACHE_QUAD;
		rd_mode = 2;
		break;
	default:
		/* unknown opcode */
		return -EOPNOTSUPP;
	}

	err = airoha_snand_set_mode(priv, SPI_MODE_DMA);
	if (err < 0)
		return err;

	/* NFI reset */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_CON,
			   SPI_NFI_FIFO_FLUSH | SPI_NFI_RST);
	if (err)
		goto error_dma_mode_off;

	/* NFI configure:
	 *   - No AutoFDM (custom sector size (SECCUS) register will be used)
	 *   - No SoC's hardware ECC (flash internal ECC will be used)
	 *   - Use burst mode (faster, but requires 16 byte alignment for addresses)
	 *   - Setup for reading (SPI_NFI_READ_MODE)
	 *   - Setup reading command: FIELD_PREP(SPI_NFI_OPMODE, 6)
	 *   - Use DMA instead of PIO for data reading
	 */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_CNFG,
				 SPI_NFI_DMA_MODE |
				 SPI_NFI_READ_MODE |
				 SPI_NFI_DMA_BURST_EN |
				 SPI_NFI_HW_ECC_EN |
				 SPI_NFI_AUTO_FDM_EN |
				 SPI_NFI_OPMODE,
				 SPI_NFI_DMA_MODE |
				 SPI_NFI_READ_MODE |
				 SPI_NFI_DMA_BURST_EN |
				 FIELD_PREP(SPI_NFI_OPMODE, 6));
	if (err)
		goto error_dma_mode_off;

	/* Set number of sector will be read */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
				 SPI_NFI_SEC_NUM,
				 FIELD_PREP(SPI_NFI_SEC_NUM, 1));
	if (err)
		goto error_dma_mode_off;

	/* Set custom sector size */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_SECCUS_SIZE,
				 SPI_NFI_CUS_SEC_SIZE |
				 SPI_NFI_CUS_SEC_SIZE_EN,
				 FIELD_PREP(SPI_NFI_CUS_SEC_SIZE, bytes) |
				 SPI_NFI_CUS_SEC_SIZE_EN);
	if (err)
		goto error_dma_mode_off;

	dma_addr = dma_map_single(txrx_buf, SPI_NAND_CACHE_SIZE,
				  DMA_FROM_DEVICE);

	/* set dma addr */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_STRADDR,
			   dma_addr);
	if (err)
		goto error_dma_unmap;

	/*
	 * Setup transfer length
	 * ---------------------
	 * The following rule MUST be met:
	 *     transfer_length =
	 *        = NFI_SNF_MISC_CTL2.read_data_byte_number =
	 *        = NFI_CON.sector_number * NFI_SECCUS.custom_sector_size
	 */
	err = regmap_update_bits(priv->regmap_nfi,
				 REG_SPI_NFI_SNF_MISC_CTL2,
				 SPI_NFI_READ_DATA_BYTE_NUM,
				 FIELD_PREP(SPI_NFI_READ_DATA_BYTE_NUM, bytes));
	if (err)
		goto error_dma_unmap;

	/* set read command */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_RD_CTL2,
			   FIELD_PREP(SPI_NFI_DATA_READ_CMD, opcode));
	if (err)
		goto error_dma_unmap;

	/* set read mode */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_SNF_MISC_CTL,
			   FIELD_PREP(SPI_NFI_DATA_READ_WR_MODE, rd_mode));
	if (err)
		goto error_dma_unmap;

	/* set read addr: zero page offset + descriptor read offset */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_RD_CTL3,
			   desc->info.offset);
	if (err)
		goto error_dma_unmap;

	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_CMD, 0x0);
	if (err)
		goto error_dma_unmap;

	/* trigger dma reading */
	err = regmap_clear_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
				SPI_NFI_RD_TRIG);
	if (err)
		goto error_dma_unmap;

	err = regmap_set_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
			      SPI_NFI_RD_TRIG);
	if (err)
		goto error_dma_unmap;

	err = regmap_read_poll_timeout(priv->regmap_nfi,
				       REG_SPI_NFI_SNF_STA_CTL1, val,
				       (val & SPI_NFI_READ_FROM_CACHE_DONE),
				       0, 1 * MSEC_PER_SEC);
	if (err)
		goto error_dma_unmap;

	/*
	 * SPI_NFI_READ_FROM_CACHE_DONE bit must be written at the end
	 * of dirmap_read operation even if it is already set.
	 */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_SNF_STA_CTL1,
				 SPI_NFI_READ_FROM_CACHE_DONE,
				 SPI_NFI_READ_FROM_CACHE_DONE);
	if (err)
		goto error_dma_unmap;

	err = regmap_read_poll_timeout(priv->regmap_nfi, REG_SPI_NFI_INTR,
				       val, (val & SPI_NFI_AHB_DONE), 0,
				       1 * MSEC_PER_SEC);
	if (err)
		goto error_dma_unmap;

	/* DMA read need delay for data ready from controller to DRAM */
	udelay(1);

	dma_unmap_single(dma_addr, SPI_NAND_CACHE_SIZE, DMA_FROM_DEVICE);

	err = airoha_snand_set_mode(priv, SPI_MODE_MANUAL);
	if (err < 0)
		return err;

	memcpy(buf, txrx_buf + offs, len);

	return len;

error_dma_unmap:
	dma_unmap_single(dma_addr, SPI_NAND_CACHE_SIZE, DMA_FROM_DEVICE);
error_dma_mode_off:
	airoha_snand_set_mode(priv, SPI_MODE_MANUAL);
	return err;
}

static ssize_t airoha_snand_dirmap_write(struct spi_mem_dirmap_desc *desc,
					 u64 offs, size_t len, const void *buf)
{
	struct spi_slave *slave = desc->slave;
	struct udevice *bus = slave->dev->parent;
	struct airoha_snand_priv *priv = dev_get_priv(bus);
	u8 *txrx_buf = priv->txrx_buf;
	dma_addr_t dma_addr;
	u32 wr_mode, val, opcode;
	size_t bytes;
	int err;

	if (!priv->dma) {
		/* simplified version of spi_mem_no_dirmap_write() */
		struct spi_mem_op op = desc->info.op_tmpl;

		op.addr.val = desc->info.offset + offs;
		op.data.buf.out = buf;
		op.data.nbytes = len;
		err = spi_mem_exec_op(desc->slave, &op);
		if (err)
			return err;

		return op.data.nbytes;
	}

	/* minimum oob size is 64 */
	bytes = round_up(offs + len, 64);

	opcode = desc->info.op_tmpl.cmd.opcode;
	switch (opcode) {
	case SPI_NAND_OP_PROGRAM_LOAD_SINGLE:
	case SPI_NAND_OP_PROGRAM_LOAD_RAMDOM_SINGLE:
		wr_mode = 0;
		break;
	case SPI_NAND_OP_PROGRAM_LOAD_QUAD:
	case SPI_NAND_OP_PROGRAM_LOAD_RAMDON_QUAD:
		wr_mode = 2;
		break;
	default:
		/* unknown opcode */
		return -EOPNOTSUPP;
	}

	if (offs > 0)
		memset(txrx_buf, 0xff, offs);
	memcpy(txrx_buf + offs, buf, len);
	if (bytes > offs + len)
		memset(txrx_buf + offs + len, 0xff, bytes - offs - len);

	err = airoha_snand_set_mode(priv, SPI_MODE_DMA);
	if (err < 0)
		return err;

	/* NFI reset */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_CON,
			   SPI_NFI_FIFO_FLUSH | SPI_NFI_RST);
	if (err)
		goto error_dma_mode_off;

	/*
	 * NFI configure:
	 *   - No AutoFDM (custom sector size (SECCUS) register will be used)
	 *   - No SoC's hardware ECC (flash internal ECC will be used)
	 *   - Use burst mode (faster, but requires 16 byte alignment for addresses)
	 *   - Setup for writing (SPI_NFI_READ_MODE bit is cleared)
	 *   - Setup writing command: FIELD_PREP(SPI_NFI_OPMODE, 3)
	 *   - Use DMA instead of PIO for data writing
	 */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_CNFG,
				 SPI_NFI_DMA_MODE |
				 SPI_NFI_READ_MODE |
				 SPI_NFI_DMA_BURST_EN |
				 SPI_NFI_HW_ECC_EN |
				 SPI_NFI_AUTO_FDM_EN |
				 SPI_NFI_OPMODE,
				 SPI_NFI_DMA_MODE |
				 SPI_NFI_DMA_BURST_EN |
				 FIELD_PREP(SPI_NFI_OPMODE, 3));
	if (err)
		goto error_dma_mode_off;

	/* Set number of sector will be written */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
				 SPI_NFI_SEC_NUM,
				 FIELD_PREP(SPI_NFI_SEC_NUM, 1));
	if (err)
		goto error_dma_mode_off;

	/* Set custom sector size */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_SECCUS_SIZE,
				 SPI_NFI_CUS_SEC_SIZE |
				 SPI_NFI_CUS_SEC_SIZE_EN,
				 FIELD_PREP(SPI_NFI_CUS_SEC_SIZE, bytes) |
				 SPI_NFI_CUS_SEC_SIZE_EN);
	if (err)
		goto error_dma_mode_off;

	dma_addr = dma_map_single(txrx_buf, SPI_NAND_CACHE_SIZE,
				  DMA_TO_DEVICE);

	/* set dma addr */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_STRADDR,
			   dma_addr);
	if (err)
		goto error_dma_unmap;

	/*
	 * Setup transfer length
	 * ---------------------
	 * The following rule MUST be met:
	 *     transfer_length =
	 *        = NFI_SNF_MISC_CTL2.write_data_byte_number =
	 *        = NFI_CON.sector_number * NFI_SECCUS.custom_sector_size
	 */
	err = regmap_update_bits(priv->regmap_nfi,
				 REG_SPI_NFI_SNF_MISC_CTL2,
				 SPI_NFI_PROG_LOAD_BYTE_NUM,
				 FIELD_PREP(SPI_NFI_PROG_LOAD_BYTE_NUM, bytes));
	if (err)
		goto error_dma_unmap;

	/* set write command */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_PG_CTL1,
			   FIELD_PREP(SPI_NFI_PG_LOAD_CMD, opcode));
	if (err)
		goto error_dma_unmap;

	/* set write mode */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_SNF_MISC_CTL,
			   FIELD_PREP(SPI_NFI_DATA_READ_WR_MODE, wr_mode));
	if (err)
		goto error_dma_unmap;

	/* set write addr: zero page offset + descriptor write offset */
	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_PG_CTL2,
			   desc->info.offset);
	if (err)
		goto error_dma_unmap;

	err = regmap_write(priv->regmap_nfi, REG_SPI_NFI_CMD, 0x80);
	if (err)
		goto error_dma_unmap;

	/* trigger dma writing */
	err = regmap_clear_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
				SPI_NFI_WR_TRIG);
	if (err)
		goto error_dma_unmap;

	err = regmap_set_bits(priv->regmap_nfi, REG_SPI_NFI_CON,
			      SPI_NFI_WR_TRIG);
	if (err)
		goto error_dma_unmap;

	err = regmap_read_poll_timeout(priv->regmap_nfi, REG_SPI_NFI_INTR,
				       val, (val & SPI_NFI_AHB_DONE), 0,
				       1 * MSEC_PER_SEC);
	if (err)
		goto error_dma_unmap;

	err = regmap_read_poll_timeout(priv->regmap_nfi,
				       REG_SPI_NFI_SNF_STA_CTL1, val,
				       (val & SPI_NFI_LOAD_TO_CACHE_DONE),
				       0, 1 * MSEC_PER_SEC);
	if (err)
		goto error_dma_unmap;

	/*
	 * SPI_NFI_LOAD_TO_CACHE_DONE bit must be written at the end
	 * of dirmap_write operation even if it is already set.
	 */
	err = regmap_update_bits(priv->regmap_nfi, REG_SPI_NFI_SNF_STA_CTL1,
				 SPI_NFI_LOAD_TO_CACHE_DONE,
				 SPI_NFI_LOAD_TO_CACHE_DONE);
	if (err)
		goto error_dma_unmap;

	dma_unmap_single(dma_addr, SPI_NAND_CACHE_SIZE, DMA_TO_DEVICE);

	err = airoha_snand_set_mode(priv, SPI_MODE_MANUAL);
	if (err < 0)
		return err;

	return len;

error_dma_unmap:
	dma_unmap_single(dma_addr, SPI_NAND_CACHE_SIZE, DMA_TO_DEVICE);
error_dma_mode_off:
	airoha_snand_set_mode(priv, SPI_MODE_MANUAL);
	return err;
}

static int airoha_snand_exec_op(struct spi_slave *slave,
				const struct spi_mem_op *op)
{
	struct udevice *bus = slave->dev->parent;
	struct airoha_snand_priv *priv;
	int op_len, addr_len, dummy_len;
	u8 buf[20], *data;
	int i, err;

	priv = dev_get_priv(bus);

	op_len = op->cmd.nbytes;
	addr_len = op->addr.nbytes;
	dummy_len = op->dummy.nbytes;

	if (op_len + dummy_len + addr_len > sizeof(buf))
		return -EIO;

	data = buf;
	for (i = 0; i < op_len; i++)
		*data++ = op->cmd.opcode >> (8 * (op_len - i - 1));
	for (i = 0; i < addr_len; i++)
		*data++ = op->addr.val >> (8 * (addr_len - i - 1));
	for (i = 0; i < dummy_len; i++)
		*data++ = 0xff;

	/* switch to manual mode */
	err = airoha_snand_set_mode(priv, SPI_MODE_MANUAL);
	if (err < 0)
		return err;

	err = airoha_snand_set_cs(priv, SPI_CHIP_SEL_LOW);
	if (err < 0)
		return err;

	/* opcode */
	data = buf;
	err = airoha_snand_write_data(priv, data, op_len,
				      op->cmd.buswidth);
	if (err)
		return err;

	/* addr part */
	data += op_len;
	if (addr_len) {
		err = airoha_snand_write_data(priv, data, addr_len,
					      op->addr.buswidth);
		if (err)
			return err;
	}

	/* dummy */
	data += addr_len;
	if (dummy_len) {
		err = airoha_snand_write_data(priv, data, dummy_len,
					      op->dummy.buswidth);
		if (err)
			return err;
	}

	/* data */
	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			err = airoha_snand_read_data(priv, op->data.buf.in,
						     op->data.nbytes,
						     op->data.buswidth);
		else
			err = airoha_snand_write_data(priv, op->data.buf.out,
						      op->data.nbytes,
						      op->data.buswidth);
		if (err)
			return err;
	}

	return airoha_snand_set_cs(priv, SPI_CHIP_SEL_HIGH);
}

static int airoha_snand_probe(struct udevice *dev)
{
	struct airoha_snand_priv *priv = dev_get_priv(dev);
	int ret;
	u32 sfc_strap;

	priv->txrx_buf = memalign(ARCH_DMA_MINALIGN, SPI_NAND_CACHE_SIZE);
	if (!priv->txrx_buf) {
		dev_err(dev, "failed to alloacate memory for dirmap\n");
		return -ENOMEM;
	}

	ret = regmap_init_mem_index(dev_ofnode(dev), &priv->regmap_ctrl, 0);
	if (ret) {
		dev_err(dev, "failed to init spi ctrl regmap\n");
		return ret;
	}

	ret = regmap_init_mem_index(dev_ofnode(dev), &priv->regmap_nfi, 1);
	if (ret) {
		dev_err(dev, "failed to init spi nfi regmap\n");
		return ret;
	}

	priv->spi_clk = devm_clk_get(dev, "spi");
	if (IS_ERR(priv->spi_clk)) {
		dev_err(dev, "unable to get spi clk\n");
		return PTR_ERR(priv->regmap_ctrl);
	}
	clk_enable(priv->spi_clk);

	priv->dma = 1;
	if (device_is_compatible(dev, "airoha,en7523-snand")){
		ret = regmap_read(priv->regmap_ctrl, REG_SPI_CTRL_SFC_STRAP, &sfc_strap);
		if (ret)
			return ret;

		if (!(sfc_strap & 0x04)) {
			priv->dma = 0;
			printf("\n"
				"=== WARNING ======================================================\n"
				"Detected booting in RESERVED mode (UART_TXD was short to GND).\n"
				"This mode is known for incorrect DMA reading of some flashes.\n"
				"Usage of DMA for flash operations will be disabled to prevent data\n"
				"damage. Unplug your serial console and power cycle the board\n"
				"to boot with full performance.\n"
				"==================================================================\n\n");
		}
	}

	return airoha_snand_nfi_init(priv);
}

static int airoha_snand_nfi_set_speed(struct udevice *bus, uint speed)
{
	struct airoha_snand_priv *priv = dev_get_priv(bus);
	int ret;

	ret = clk_set_rate(priv->spi_clk, speed);
	if (ret < 0)
		return ret;

	return 0;
}

static int airoha_snand_nfi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static const struct spi_controller_mem_ops airoha_snand_mem_ops = {
	.supports_op = airoha_snand_supports_op,
	.exec_op = airoha_snand_exec_op,
	.dirmap_create = airoha_snand_dirmap_create,
	.dirmap_read = airoha_snand_dirmap_read,
	.dirmap_write = airoha_snand_dirmap_write,
};

static const struct dm_spi_ops airoha_snfi_spi_ops = {
	.mem_ops = &airoha_snand_mem_ops,
	.set_speed = airoha_snand_nfi_set_speed,
	.set_mode = airoha_snand_nfi_set_mode,
};

static const struct udevice_id airoha_snand_ids[] = {
	{ .compatible = "airoha,en7581-snand" },
	{ }
};

U_BOOT_DRIVER(airoha_snfi_spi) = {
	.name = "airoha-snfi-spi",
	.id = UCLASS_SPI,
	.of_match = airoha_snand_ids,
	.ops = &airoha_snfi_spi_ops,
	.priv_auto = sizeof(struct airoha_snand_priv),
	.probe = airoha_snand_probe,
};
