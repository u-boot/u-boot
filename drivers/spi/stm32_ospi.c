// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_SPI

#include <clk.h>
#include <dm.h>
#include <log.h>
#include <reset.h>
#include <spi.h>
#include <spi-mem.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>

/* OCTOSPI control register */
#define OSPI_CR			0x00
#define OSPI_CR_EN		BIT(0)
#define OSPI_CR_ABORT		BIT(1)
#define OSPI_CR_TCEN		BIT(3)
#define OSPI_CR_FSEL		BIT(7)
#define OSPI_CR_FTHRES_MASK	GENMASK(13, 8)
#define OSPI_CR_FTHRES_SHIFT	8
#define OSPI_CR_CSSEL		BIT(24)
#define OSPI_CR_FMODE_SHIFT	28
#define OSPI_CR_FMODE_MASK	GENMASK(29, 28)

/* OCTOSPI device configuration register */
#define OSPI_DCR1		0x08
#define OSPI_DCR1_CKMODE	BIT(0)
#define OSPI_DCR1_DLYBYP	BIT(3)
#define OSPI_DCR1_CSHT_SHIFT	8
#define OSPI_DCR1_CSHT_MASK	GENMASK(13, 8)
#define OSPI_DCR1_DEVSIZE_MASK	GENMASK(20, 16)
#define OSPI_DCR1_MTYP_MASK	GENMASK(26, 24)

/* OCTOSPI device configuration register 2 */
#define OSPI_DCR2		0x0c
#define OSPI_DCR2_PRESC_SHIFT	0
#define OSPI_DCR2_PRESC_MASK	GENMASK(7, 0)

/* OCTOSPI status register */
#define OSPI_SR			0x20
#define OSPI_SR_TEF		BIT(0)
#define OSPI_SR_TCF		BIT(1)
#define OSPI_SR_FTF		BIT(2)
#define OSPI_SR_BUSY		BIT(5)

/* OCTOSPI flag clear register */
#define OSPI_FCR		0x24
#define OSPI_FCR_CTEF		BIT(0)
#define OSPI_FCR_CTCF		BIT(1)

/* OCTOSPI data length register */
#define OSPI_DLR		0x40

/* OCTOSPI address register */
#define OSPI_AR			0x48

/* OCTOSPI data configuration register */
#define OSPI_DR			0x50

/* OCTOSPI communication configuration register */
#define OSPI_CCR		0x100
#define OSPI_CCR_IMODE_SHIFT	0
#define OSPI_CCR_IMODE_MASK	GENMASK(2, 0)
#define OSPI_CCR_ADMODE_SHIFT	8
#define OSPI_CCR_ADMODE_MASK	GENMASK(10, 8)
#define OSPI_CCR_ADSIZE_SHIFT	12
#define OSPI_CCR_DMODE_SHIFT	24
#define OSPI_CCR_DMODE_MASK	GENMASK(26, 24)
#define OSPI_CCR_IND_WRITE	0
#define OSPI_CCR_IND_READ	1
#define OSPI_CCR_MEM_MAP	3

/* OCTOSPI timing configuration register */
#define OSPI_TCR		0x108
#define OSPI_TCR_DCYC_SHIFT	0x0
#define OSPI_TCR_DCYC_MASK	GENMASK(4, 0)
#define OSPI_TCR_SSHIFT		BIT(30)

/* OCTOSPI instruction register */
#define OSPI_IR			0x110

#define OSPI_MAX_MMAP_SZ	SZ_256M
#define OSPI_MAX_CHIP		2

#define OSPI_FIFO_TIMEOUT_US	30000
#define OSPI_ABT_TIMEOUT_US	100000
#define OSPI_BUSY_TIMEOUT_US	100000
#define OSPI_CMD_TIMEOUT_US	1000000

struct stm32_ospi_flash {
	u32 cr;
	u32 dcr;
	u32 dcr2;
	bool initialized;
};

struct stm32_ospi_priv {
	struct stm32_ospi_flash flash[OSPI_MAX_CHIP];
	int cs_used;
};

struct stm32_ospi_plat {
	phys_addr_t regs_base;		/* register base address */
	phys_addr_t mm_base;		/* memory map base address */
	resource_size_t mm_size;
	struct clk clk;
	struct reset_ctl_bulk rst_ctl;
	ulong clock_rate;
};

static int stm32_ospi_mm(struct udevice *dev,
			 const struct spi_mem_op *op)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev);

	memcpy_fromio(op->data.buf.in,
		      (void __iomem *)ospi_plat->mm_base + op->addr.val,
		      op->data.nbytes);

	return 0;
}

static void stm32_ospi_read_fifo(void *val, phys_addr_t addr, u8 len)
{
	switch (len) {
	case sizeof(u32):
		*((u32 *)val) = readl_relaxed(addr);
		break;
	case sizeof(u16):
		*((u16 *)val) = readw_relaxed(addr);
		break;
	case sizeof(u8):
		*((u8 *)val) = readb_relaxed(addr);
	};
	schedule();
}

static void stm32_ospi_write_fifo(void *val, phys_addr_t addr, u8 len)
{
	switch (len) {
	case sizeof(u32):
		writel_relaxed(*((u32 *)val), addr);
		break;
	case sizeof(u16):
		writew_relaxed(*((u16 *)val), addr);
		break;
	case sizeof(u8):
		writeb_relaxed(*((u8 *)val), addr);
	};
}

int stm32_ospi_tx_poll(struct udevice *dev, void *buf, u32 len, bool read)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev);
	phys_addr_t regs_base = ospi_plat->regs_base;
	void (*fifo)(void *val, phys_addr_t addr, u8 len);
	u32 sr;
	int ret;
	u8 step = 1;

	if (read)
		fifo = stm32_ospi_read_fifo;
	else
		fifo = stm32_ospi_write_fifo;

	while (len) {
		ret = readl_poll_timeout(regs_base + OSPI_SR, sr,
					 sr & OSPI_SR_FTF,
					 OSPI_FIFO_TIMEOUT_US);
		if (ret) {
			dev_err(dev, "fifo timeout (len:%d stat:%#x)\n",
				len, sr);
			return ret;
		}

		if (!IS_ALIGNED((uintptr_t)buf, sizeof(u32))) {
			if (!IS_ALIGNED((uintptr_t)buf, sizeof(u16)))
				step = sizeof(u8);
			else
				step = min((u32)len, (u32)sizeof(u16));
		}
		/* Buf is aligned */
		else if (len >= sizeof(u32))
			step = sizeof(u32);
		else if (len >= sizeof(u16))
			step = sizeof(u16);
		else if (len)
			step = sizeof(u8);

		fifo(buf, regs_base + OSPI_DR, step);
		len -= step;
		buf += step;
	}

	return 0;
}

static int stm32_ospi_tx(struct udevice *dev,
			 const struct spi_mem_op *op,
			 u8 mode)
{
	void *buf;

	if (!op->data.nbytes)
		return 0;

	if (mode == OSPI_CCR_MEM_MAP)
		return stm32_ospi_mm(dev, op);

	if (op->data.dir == SPI_MEM_DATA_IN)
		buf = op->data.buf.in;
	else
		buf = (void *)op->data.buf.out;

	return stm32_ospi_tx_poll(dev, buf, op->data.nbytes,
				 op->data.dir == SPI_MEM_DATA_IN);
}

static int stm32_ospi_get_mode(u8 buswidth)
{
	if (buswidth == 8)
		return 4;

	if (buswidth == 4)
		return 3;

	return buswidth;
}

int stm32_ospi_wait_for_not_busy(struct udevice *dev)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev);
	phys_addr_t regs_base = ospi_plat->regs_base;
	u32 sr;
	int ret;

	ret = readl_poll_timeout(regs_base + OSPI_SR, sr, !(sr & OSPI_SR_BUSY),
				 OSPI_BUSY_TIMEOUT_US);
	if (ret)
		dev_err(dev, "busy timeout (stat:%#x)\n", sr);

	return ret;
}

int stm32_ospi_wait_cmd(struct udevice *dev)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev);
	phys_addr_t regs_base = ospi_plat->regs_base;
	u32 sr;
	int ret = 0;

	ret = readl_poll_timeout(regs_base + OSPI_SR, sr,
				 sr & OSPI_SR_TCF,
				 OSPI_CMD_TIMEOUT_US);
	if (ret) {
		dev_err(dev, "cmd timeout (stat:%#x)\n", sr);
	} else if (readl(regs_base + OSPI_SR) & OSPI_SR_TEF) {
		dev_err(dev, "transfer error (stat:%#x)\n", sr);
		ret = -EIO;
	}

	/* clear flags */
	writel(OSPI_FCR_CTCF | OSPI_FCR_CTEF, regs_base + OSPI_FCR);

	if (!ret)
		ret = stm32_ospi_wait_for_not_busy(dev);

	return ret;
}

static int stm32_ospi_exec_op(struct spi_slave *slave,
			      const struct spi_mem_op *op)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(slave->dev->parent);
	phys_addr_t regs_base = ospi_plat->regs_base;
	u32 cr, ccr = 0, addr_max;
	int timeout, ret;
	int dmode;
	u8 mode = OSPI_CCR_IND_WRITE;
	u8 dcyc = 0;

	dev_dbg(slave->dev, "%s: cmd:%#x mode:%d.%d.%d.%d addr:%#llx len:%#x\n",
		__func__, op->cmd.opcode, op->cmd.buswidth, op->addr.buswidth,
		op->dummy.buswidth, op->data.buswidth,
		op->addr.val, op->data.nbytes);

	addr_max = op->addr.val + op->data.nbytes + 1;

	if (op->data.dir == SPI_MEM_DATA_IN && op->data.nbytes) {
		if (addr_max < ospi_plat->mm_size && op->addr.buswidth)
			mode = OSPI_CCR_MEM_MAP;
		else
			mode = OSPI_CCR_IND_READ;
	}

	if (op->data.nbytes)
		writel(op->data.nbytes - 1, regs_base + OSPI_DLR);

	clrsetbits_le32(regs_base + OSPI_CR, OSPI_CR_FMODE_MASK,
			mode << OSPI_CR_FMODE_SHIFT);

	ccr |= (stm32_ospi_get_mode(op->cmd.buswidth) << OSPI_CCR_IMODE_SHIFT) &
		OSPI_CCR_IMODE_MASK;

	if (op->addr.nbytes) {
		ccr |= ((op->addr.nbytes - 1) << OSPI_CCR_ADSIZE_SHIFT);
		ccr |= (stm32_ospi_get_mode(op->addr.buswidth)
			<< OSPI_CCR_ADMODE_SHIFT) & OSPI_CCR_ADMODE_MASK;
	}

	if (op->dummy.buswidth && op->dummy.nbytes)
		dcyc = op->dummy.nbytes * 8 / op->dummy.buswidth;

	clrsetbits_le32(regs_base + OSPI_TCR, OSPI_TCR_DCYC_MASK,
			dcyc << OSPI_TCR_DCYC_SHIFT);

	if (op->data.nbytes) {
		dmode = stm32_ospi_get_mode(op->data.buswidth);
		ccr |= (dmode << OSPI_CCR_DMODE_SHIFT) & OSPI_CCR_DMODE_MASK;
	}

	writel(ccr, regs_base + OSPI_CCR);

	/* set instruction, must be set after ccr register update */
	writel(op->cmd.opcode, regs_base + OSPI_IR);

	if (op->addr.nbytes && mode != OSPI_CCR_MEM_MAP)
		writel(op->addr.val, regs_base + OSPI_AR);

	ret = stm32_ospi_tx(slave->dev->parent, op, mode);
	/*
	 * Abort in:
	 * -error case
	 * -read memory map: prefetching must be stopped if we read the last
	 *  byte of device (device size - fifo size). like device size is not
	 *  knows, the prefetching is always stop.
	 */
	if (ret || mode == OSPI_CCR_MEM_MAP)
		goto abort;

	/* Wait end of tx in indirect mode */
	ret = stm32_ospi_wait_cmd(slave->dev->parent);
	if (ret)
		goto abort;

	return 0;

abort:
	setbits_le32(regs_base + OSPI_CR, OSPI_CR_ABORT);

	/* Wait clear of abort bit by hw */
	timeout = readl_poll_timeout(regs_base + OSPI_CR, cr,
				     !(cr & OSPI_CR_ABORT),
				     OSPI_ABT_TIMEOUT_US);

	writel(OSPI_FCR_CTCF, regs_base + OSPI_FCR);

	if (ret || timeout)
		dev_err(slave->dev, "%s ret:%d abort timeout:%d\n", __func__,
			ret, timeout);

	return ret;
}

static int stm32_ospi_probe(struct udevice *bus)
{
	struct stm32_ospi_priv *priv = dev_get_priv(bus);
	struct stm32_ospi_plat *ospi_plat;
	phys_addr_t regs_base;
	int ret;

	ospi_plat = dev_get_plat(bus);
	regs_base = ospi_plat->regs_base;

	ret = clk_enable(&ospi_plat->clk);
	if (ret) {
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}

	/* Reset OSPI controller */
	reset_assert_bulk(&ospi_plat->rst_ctl);
	udelay(2);
	reset_deassert_bulk(&ospi_plat->rst_ctl);

	priv->cs_used = -1;

	setbits_le32(regs_base + OSPI_TCR, OSPI_TCR_SSHIFT);

	clrsetbits_le32(regs_base + OSPI_CR, OSPI_CR_FTHRES_MASK,
			3 << OSPI_CR_FTHRES_SHIFT);

	/* Set dcr devsize to max address */
	setbits_le32(regs_base + OSPI_DCR1,
		     OSPI_DCR1_DEVSIZE_MASK | OSPI_DCR1_DLYBYP);

	return 0;
}

static int stm32_ospi_claim_bus(struct udevice *dev)
{
	struct stm32_ospi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev->parent);
	phys_addr_t regs_base = ospi_plat->regs_base;
	unsigned int slave_cs = slave_plat->cs[0];

	if (slave_cs >= OSPI_MAX_CHIP)
		return -ENODEV;

	if (priv->cs_used != slave_cs) {
		struct stm32_ospi_flash *flash = &priv->flash[slave_cs];

		priv->cs_used = slave_cs;

		if (flash->initialized) {
			/* Set the configuration: speed + cs */
			writel(flash->cr, regs_base + OSPI_CR);
			writel(flash->dcr, regs_base + OSPI_DCR1);
			writel(flash->dcr2, regs_base + OSPI_DCR2);
		} else {
			/* Set chip select */
			clrsetbits_le32(regs_base + OSPI_CR,
					OSPI_CR_CSSEL,
					priv->cs_used ? OSPI_CR_CSSEL : 0);

			/* Save the configuration: speed + cs */
			flash->cr = readl(regs_base + OSPI_CR);
			flash->dcr = readl(regs_base + OSPI_DCR1);
			flash->dcr2 = readl(regs_base + OSPI_DCR2);
			flash->initialized = true;
		}
	}

	setbits_le32(regs_base + OSPI_CR, OSPI_CR_EN);

	return 0;
}

static int stm32_ospi_release_bus(struct udevice *dev)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(dev->parent);
	phys_addr_t regs_base = ospi_plat->regs_base;

	clrbits_le32(regs_base + OSPI_CR, OSPI_CR_EN);

	return 0;
}

static int stm32_ospi_set_speed(struct udevice *bus, uint speed)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(bus);
	phys_addr_t regs_base = ospi_plat->regs_base;
	u32 ospi_clk = ospi_plat->clock_rate;
	u32 prescaler = 255;
	u32 csht;
	int ret;

	if (speed > 0) {
		prescaler = 0;
		if (ospi_clk) {
			prescaler = DIV_ROUND_UP(ospi_clk, speed) - 1;
			if (prescaler > 255)
				prescaler = 255;
		}
	}

	csht = (DIV_ROUND_UP((5 * ospi_clk) / (prescaler + 1), 100000000)) - 1;

	ret = stm32_ospi_wait_for_not_busy(bus);
	if (ret)
		return ret;

	clrsetbits_le32(regs_base + OSPI_DCR2, OSPI_DCR2_PRESC_MASK,
			prescaler << OSPI_DCR2_PRESC_SHIFT);

	clrsetbits_le32(regs_base + OSPI_DCR1, OSPI_DCR1_CSHT_MASK,
			csht << OSPI_DCR1_CSHT_SHIFT);

	return 0;
}

static int stm32_ospi_set_mode(struct udevice *bus, uint mode)
{
	struct stm32_ospi_plat *ospi_plat = dev_get_plat(bus);
	phys_addr_t regs_base = ospi_plat->regs_base;
	const char *str_rx, *str_tx;
	int ret;

	ret = stm32_ospi_wait_for_not_busy(bus);
	if (ret)
		return ret;

	if ((mode & SPI_CPHA) && (mode & SPI_CPOL))
		setbits_le32(regs_base + OSPI_DCR1, OSPI_DCR1_CKMODE);
	else if (!(mode & SPI_CPHA) && !(mode & SPI_CPOL))
		clrbits_le32(regs_base + OSPI_DCR1, OSPI_DCR1_CKMODE);
	else
		return -ENODEV;

	if (mode & SPI_CS_HIGH)
		return -ENODEV;

	if (mode & SPI_RX_OCTAL)
		str_rx = "octal";
	else if (mode & SPI_RX_QUAD)
		str_rx = "quad";
	else if (mode & SPI_RX_DUAL)
		str_rx = "dual";
	else
		str_rx = "single";

	if (mode & SPI_TX_OCTAL)
		str_tx = "octal";
	else if (mode & SPI_TX_QUAD)
		str_tx = "quad";
	else if (mode & SPI_TX_DUAL)
		str_tx = "dual";
	else
		str_tx = "single";

	dev_dbg(bus, "mode=%d rx: %s, tx: %s\n", mode, str_rx, str_tx);

	return 0;
}

static const struct spi_controller_mem_ops stm32_ospi_mem_ops = {
	.exec_op = stm32_ospi_exec_op,
};

static const struct dm_spi_ops stm32_ospi_ops = {
	.claim_bus = stm32_ospi_claim_bus,
	.release_bus = stm32_ospi_release_bus,
	.set_speed = stm32_ospi_set_speed,
	.set_mode = stm32_ospi_set_mode,
	.mem_ops = &stm32_ospi_mem_ops,
};

static int stm32_ospi_of_to_plat(struct udevice *dev)
{
	struct stm32_ospi_plat *plat = dev_get_plat(dev);
	struct resource res;
	struct ofnode_phandle_args args;
	const fdt32_t *reg;
	int ret, len;

	reg = dev_read_prop(dev, "reg", &len);
	if (!reg) {
		dev_err(dev, "Can't get regs base address\n");
		return -ENOENT;
	}

	plat->regs_base = (phys_addr_t)dev_translate_address(dev, reg);

	/* optional */
	ret = dev_read_phandle_with_args(dev, "memory-region", NULL, 0, 0, &args);
	if (!ret) {
		ret = ofnode_read_resource(args.node, 0, &res);
		if (ret) {
			dev_err(dev, "Can't get mmap base address(%d)\n", ret);
			return ret;
		}

		plat->mm_base = res.start;
		plat->mm_size = resource_size(&res);

		if (plat->mm_size > OSPI_MAX_MMAP_SZ) {
			dev_err(dev, "Incorrect memory-map size: %lld Bytes\n", plat->mm_size);
			return -EINVAL;
		}

		dev_dbg(dev, "%s: regs_base=<0x%llx> mm_base=<0x%llx> mm_size=<0x%x>\n",
			__func__, plat->regs_base, plat->mm_base, (u32)plat->mm_size);
	} else {
		plat->mm_base = 0;
		plat->mm_size = 0;
		dev_info(dev, "memory-region property not found (%d)\n", ret);
	}

	ret = clk_get_by_index(dev, 0, &plat->clk);
	if (ret < 0) {
		dev_err(dev, "Failed to get clock\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &plat->rst_ctl);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "Failed to get reset\n");
		return ret;
	}

	plat->clock_rate = clk_get_rate(&plat->clk);
	if (!plat->clock_rate)
		return -EINVAL;

	return ret;
};

static const struct udevice_id stm32_ospi_ids[] = {
	{ .compatible = "st,stm32mp25-ospi" },
	{ }
};

U_BOOT_DRIVER(stm32_ospi) = {
	.name = "stm32_ospi",
	.id = UCLASS_SPI,
	.of_match = stm32_ospi_ids,
	.of_to_plat = stm32_ospi_of_to_plat,
	.ops = &stm32_ospi_ops,
	.plat_auto = sizeof(struct stm32_ospi_plat),
	.priv_auto = sizeof(struct stm32_ospi_priv),
	.probe = stm32_ospi_probe,
};
