// SPDX-License-Identifier: GPL-2.0+
/*
 * ASPEED FMC/SPI Controller driver
 *
 * Copyright (c) 2022 ASPEED Corporation.
 * Copyright (c) 2022 IBM Corporation.
 *
 * Author:
 *     Chin-Ting Kuo <chin-ting_kuo@aspeedtech.com>
 *     Cedric Le Goater <clg@kaod.org>
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/mtd/spi-nor.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>

#define ASPEED_SPI_MAX_CS       3

#define CTRL_IO_SINGLE_DATA     0
#define CTRL_IO_QUAD_DATA       BIT(30)
#define CTRL_IO_DUAL_DATA       BIT(29)

#define CTRL_IO_MODE_USER       GENMASK(1, 0)
#define CTRL_IO_MODE_CMD_READ   BIT(0)
#define CTRL_IO_MODE_CMD_WRITE  BIT(1)
#define CTRL_STOP_ACTIVE        BIT(2)

struct aspeed_spi_regs {
	u32 conf;                       /* 0x00 CE Type Setting */
	u32 ctrl;                       /* 0x04 CE Control */
	u32 intr_ctrl;                  /* 0x08 Interrupt Control and Status */
	u32 cmd_ctrl;                   /* 0x0c Command Control */
	u32 ce_ctrl[ASPEED_SPI_MAX_CS]; /* 0x10 .. 0x18 CEx Control */
	u32 _reserved0[5];              /* .. */
	u32 segment_addr[ASPEED_SPI_MAX_CS]; /* 0x30 .. 0x38 Segment Address */
	u32 _reserved1[5];		/* .. */
	u32 soft_rst_cmd_ctrl;          /* 0x50 Auto Soft-Reset Command Control */
	u32 _reserved2[11];             /* .. */
	u32 dma_ctrl;                   /* 0x80 DMA Control/Status */
	u32 dma_flash_addr;             /* 0x84 DMA Flash Side Address */
	u32 dma_dram_addr;              /* 0x88 DMA DRAM Side Address */
	u32 dma_len;                    /* 0x8c DMA Length Register */
	u32 dma_checksum;               /* 0x90 Checksum Calculation Result */
	u32 timings[ASPEED_SPI_MAX_CS]; /* 0x94 Read Timing Compensation */
};

struct aspeed_spi_plat {
	u8 max_cs;
	void __iomem *ahb_base; /* AHB address base for all flash devices. */
	fdt_size_t ahb_sz; /* Overall AHB window size for all flash device. */
};

struct aspeed_spi_flash {
	void __iomem *ahb_base;
	u32 ahb_decoded_sz;
	u32 ce_ctrl_user;
	u32 ce_ctrl_read;
};

struct aspeed_spi_priv {
	u32 num_cs;
	struct aspeed_spi_regs *regs;
	struct aspeed_spi_info *info;
	struct aspeed_spi_flash flashes[ASPEED_SPI_MAX_CS];
};

struct aspeed_spi_info {
	u32 io_mode_mask;
	u32 max_bus_width;
	u32 min_decoded_sz;
	void (*set_4byte)(struct udevice *bus, u32 cs);
	u32 (*segment_start)(struct udevice *bus, u32 reg);
	u32 (*segment_end)(struct udevice *bus, u32 reg);
	u32 (*segment_reg)(u32 start, u32 end);
};

static u32 aspeed_spi_get_io_mode(u32 bus_width)
{
	switch (bus_width) {
	case 1:
		return CTRL_IO_SINGLE_DATA;
	case 2:
		return CTRL_IO_DUAL_DATA;
	case 4:
		return CTRL_IO_QUAD_DATA;
	default:
		/* keep in default value */
		return CTRL_IO_SINGLE_DATA;
	}
}

static u32 ast2500_spi_segment_start(struct udevice *bus, u32 reg)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 start_offset = ((reg >> 16) & 0xff) << 23;

	if (start_offset == 0)
		return (u32)plat->ahb_base;

	return (u32)plat->ahb_base + start_offset;
}

static u32 ast2500_spi_segment_end(struct udevice *bus, u32 reg)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 end_offset = ((reg >> 24) & 0xff) << 23;

	/* Meaningless end_offset, set to physical ahb base. */
	if (end_offset == 0)
		return (u32)plat->ahb_base;

	return (u32)plat->ahb_base + end_offset;
}

static u32 ast2500_spi_segment_reg(u32 start, u32 end)
{
	if (start == end)
		return 0;

	return ((((start) >> 23) & 0xff) << 16) | ((((end) >> 23) & 0xff) << 24);
}

static void ast2500_spi_chip_set_4byte(struct udevice *bus, u32 cs)
{
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 reg_val;

	reg_val = readl(&priv->regs->ctrl);
	reg_val |= 0x1 << cs;
	writel(reg_val, &priv->regs->ctrl);
}

static u32 ast2600_spi_segment_start(struct udevice *bus, u32 reg)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 start_offset = (reg << 16) & 0x0ff00000;

	if (start_offset == 0)
		return (u32)plat->ahb_base;

	return (u32)plat->ahb_base + start_offset;
}

static u32 ast2600_spi_segment_end(struct udevice *bus, u32 reg)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 end_offset = reg & 0x0ff00000;

	/* Meaningless end_offset, set to physical ahb base. */
	if (end_offset == 0)
		return (u32)plat->ahb_base;

	return (u32)plat->ahb_base + end_offset + 0x100000;
}

static u32 ast2600_spi_segment_reg(u32 start, u32 end)
{
	if (start == end)
		return 0;

	return ((start & 0x0ff00000) >> 16) | ((end - 0x100000) & 0x0ff00000);
}

static void ast2600_spi_chip_set_4byte(struct udevice *bus, u32 cs)
{
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 reg_val;

	reg_val = readl(&priv->regs->ctrl);
	reg_val |= 0x11 << cs;
	writel(reg_val, &priv->regs->ctrl);
}

static int aspeed_spi_read_from_ahb(void __iomem *ahb_base, void *buf,
				    size_t len)
{
	size_t offset = 0;

	if (IS_ALIGNED((uintptr_t)ahb_base, sizeof(uintptr_t)) &&
	    IS_ALIGNED((uintptr_t)buf, sizeof(uintptr_t))) {
		readsl(ahb_base, buf, len >> 2);
		offset = len & ~0x3;
		len -= offset;
	}

	readsb(ahb_base, (u8 *)buf + offset, len);

	return 0;
}

static int aspeed_spi_write_to_ahb(void __iomem *ahb_base, const void *buf,
				   size_t len)
{
	size_t offset = 0;

	if (IS_ALIGNED((uintptr_t)ahb_base, sizeof(uintptr_t)) &&
	    IS_ALIGNED((uintptr_t)buf, sizeof(uintptr_t))) {
		writesl(ahb_base, buf, len >> 2);
		offset = len & ~0x3;
		len -= offset;
	}

	writesb(ahb_base, (u8 *)buf + offset, len);

	return 0;
}

/*
 * Currently, only support 1-1-1, 1-1-2 or 1-1-4
 * SPI NOR flash operation format.
 */
static bool aspeed_spi_supports_op(struct spi_slave *slave,
				   const struct spi_mem_op *op)
{
	struct udevice *bus = slave->dev->parent;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);

	if (op->cmd.buswidth > 1)
		return false;

	if (op->addr.nbytes != 0) {
		if (op->addr.buswidth > 1)
			return false;
		if (op->addr.nbytes < 3 || op->addr.nbytes > 4)
			return false;
	}

	if (op->dummy.nbytes != 0) {
		if (op->dummy.buswidth > 1 || op->dummy.nbytes > 7)
			return false;
	}

	if (op->data.nbytes != 0 &&
	    op->data.buswidth > priv->info->max_bus_width)
		return false;

	if (!spi_mem_default_supports_op(slave, op))
		return false;

	return true;
}

static int aspeed_spi_exec_op_user_mode(struct spi_slave *slave,
					const struct spi_mem_op *op)
{
	struct udevice *dev = slave->dev;
	struct udevice *bus = dev->parent;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	u32 cs = slave_plat->cs;
	u32 ce_ctrl_reg = (u32)&priv->regs->ce_ctrl[cs];
	u32 ce_ctrl_val;
	struct aspeed_spi_flash *flash = &priv->flashes[cs];
	u8 dummy_data[16] = {0};
	u8 addr[4] = {0};
	int i;

	dev_dbg(dev, "cmd:%x(%d),addr:%llx(%d),dummy:%d(%d),data_len:0x%x(%d)\n",
		op->cmd.opcode, op->cmd.buswidth, op->addr.val,
		op->addr.buswidth, op->dummy.nbytes, op->dummy.buswidth,
		op->data.nbytes, op->data.buswidth);

	/*
	 * Set controller to 4-byte address mode
	 * if flash is in 4-byte address mode.
	 */
	if (op->cmd.opcode == SPINOR_OP_EN4B)
		priv->info->set_4byte(bus, cs);

	/* Start user mode */
	ce_ctrl_val = flash->ce_ctrl_user;
	writel(ce_ctrl_val, ce_ctrl_reg);
	ce_ctrl_val &= (~CTRL_STOP_ACTIVE);
	writel(ce_ctrl_val, ce_ctrl_reg);

	/* Send command */
	aspeed_spi_write_to_ahb(flash->ahb_base, &op->cmd.opcode, 1);

	/* Send address */
	for (i = op->addr.nbytes; i > 0; i--) {
		addr[op->addr.nbytes - i] =
			((u32)op->addr.val >> ((i - 1) * 8)) & 0xff;
	}

	/* Change io_mode */
	ce_ctrl_val &= ~priv->info->io_mode_mask;
	ce_ctrl_val |= aspeed_spi_get_io_mode(op->addr.buswidth);
	writel(ce_ctrl_val, ce_ctrl_reg);
	aspeed_spi_write_to_ahb(flash->ahb_base, addr, op->addr.nbytes);

	/* Send dummy cycles */
	aspeed_spi_write_to_ahb(flash->ahb_base, dummy_data, op->dummy.nbytes);

	/* Change io_mode */
	ce_ctrl_val &= ~priv->info->io_mode_mask;
	ce_ctrl_val |= aspeed_spi_get_io_mode(op->data.buswidth);
	writel(ce_ctrl_val, ce_ctrl_reg);

	/* Send data */
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		aspeed_spi_write_to_ahb(flash->ahb_base, op->data.buf.out,
					op->data.nbytes);
	} else {
		aspeed_spi_read_from_ahb(flash->ahb_base, op->data.buf.in,
					 op->data.nbytes);
	}

	ce_ctrl_val |= CTRL_STOP_ACTIVE;
	writel(ce_ctrl_val, ce_ctrl_reg);

	/* Restore controller setting. */
	writel(flash->ce_ctrl_read, ce_ctrl_reg);

	return 0;
}

static struct aspeed_spi_flash *aspeed_spi_get_flash(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 cs = slave_plat->cs;

	if (cs >= plat->max_cs) {
		dev_err(dev, "invalid CS %u\n", cs);
		return NULL;
	}

	return &priv->flashes[cs];
}

static void aspeed_spi_decoded_base_calculate(struct udevice *bus)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 cs;

	priv->flashes[0].ahb_base = plat->ahb_base;

	for (cs = 1; cs < plat->max_cs; cs++) {
		priv->flashes[cs].ahb_base =
				priv->flashes[cs - 1].ahb_base +
				priv->flashes[cs - 1].ahb_decoded_sz;
	}
}

static void aspeed_spi_decoded_range_set(struct udevice *bus)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 decoded_reg_val;
	u32 start_addr, end_addr;
	u32 cs;

	for (cs = 0; cs < plat->max_cs; cs++) {
		start_addr = (u32)priv->flashes[cs].ahb_base;
		end_addr = (u32)priv->flashes[cs].ahb_base +
			   priv->flashes[cs].ahb_decoded_sz;

		decoded_reg_val = priv->info->segment_reg(start_addr, end_addr);

		writel(decoded_reg_val, &priv->regs->segment_addr[cs]);

		dev_dbg(bus, "cs: %d, decoded_reg: 0x%x, start: 0x%x, end: 0x%x\n",
			cs, decoded_reg_val, start_addr, end_addr);
	}
}

static int aspeed_spi_decoded_range_config(struct udevice *bus)
{
	aspeed_spi_decoded_base_calculate(bus);
	aspeed_spi_decoded_range_set(bus);

	return 0;
}

/*
 * Initialize SPI controller for each chip select.
 * Here, only the minimum decode range is configured
 * in order to get device (SPI NOR flash) information
 * at the early stage.
 */
static int aspeed_spi_ctrl_init(struct udevice *bus)
{
	int ret;
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 cs;
	u32 reg_val;
	u32 decoded_sz;

	/* Enable write capability for all CS. */
	reg_val = readl(&priv->regs->conf);
	writel(reg_val | (GENMASK(plat->max_cs - 1, 0) << 16),
	       &priv->regs->conf);

	memset(priv->flashes, 0x0,
	       sizeof(struct aspeed_spi_flash) * ASPEED_SPI_MAX_CS);

	/* Initial user mode. */
	for (cs = 0; cs < priv->num_cs; cs++) {
		priv->flashes[cs].ce_ctrl_user =
				(CTRL_STOP_ACTIVE | CTRL_IO_MODE_USER);
	}

	/* Assign basic AHB decoded size for each CS. */
	for (cs = 0; cs < plat->max_cs; cs++) {
		reg_val = readl(&priv->regs->segment_addr[cs]);
		decoded_sz = priv->info->segment_end(bus, reg_val) -
			     priv->info->segment_start(bus, reg_val);

		if (decoded_sz < priv->info->min_decoded_sz)
			decoded_sz = priv->info->min_decoded_sz;

		priv->flashes[cs].ahb_decoded_sz = decoded_sz;
	}

	ret = aspeed_spi_decoded_range_config(bus);

	return ret;
}

static const struct aspeed_spi_info ast2500_fmc_info = {
	.io_mode_mask = 0x70000000,
	.max_bus_width = 2,
	.min_decoded_sz = 0x800000,
	.set_4byte = ast2500_spi_chip_set_4byte,
	.segment_start = ast2500_spi_segment_start,
	.segment_end = ast2500_spi_segment_end,
	.segment_reg = ast2500_spi_segment_reg,
};

/*
 * There are some different between FMC and SPI controllers.
 * For example, DMA operation, but this isn't implemented currently.
 */
static const struct aspeed_spi_info ast2500_spi_info = {
	.io_mode_mask = 0x70000000,
	.max_bus_width = 2,
	.min_decoded_sz = 0x800000,
	.set_4byte = ast2500_spi_chip_set_4byte,
	.segment_start = ast2500_spi_segment_start,
	.segment_end = ast2500_spi_segment_end,
	.segment_reg = ast2500_spi_segment_reg,
};

static const struct aspeed_spi_info ast2600_fmc_info = {
	.io_mode_mask = 0xf0000000,
	.max_bus_width = 4,
	.min_decoded_sz = 0x200000,
	.set_4byte = ast2600_spi_chip_set_4byte,
	.segment_start = ast2600_spi_segment_start,
	.segment_end = ast2600_spi_segment_end,
	.segment_reg = ast2600_spi_segment_reg,
};

static const struct aspeed_spi_info ast2600_spi_info = {
	.io_mode_mask = 0xf0000000,
	.max_bus_width = 4,
	.min_decoded_sz = 0x200000,
	.set_4byte = ast2600_spi_chip_set_4byte,
	.segment_start = ast2600_spi_segment_start,
	.segment_end = ast2600_spi_segment_end,
	.segment_reg = ast2600_spi_segment_reg,
};

static int aspeed_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	dev_dbg(bus, "%s: claim bus CS%u\n", bus->name, slave_plat->cs);

	return 0;
}

static int aspeed_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	dev_dbg(bus, "%s: release bus CS%u\n", bus->name, slave_plat->cs);

	if (!aspeed_spi_get_flash(dev))
		return -ENODEV;

	return 0;
}

static int aspeed_spi_set_mode(struct udevice *bus, uint mode)
{
	dev_dbg(bus, "%s: setting mode to %x\n", bus->name, mode);

	return 0;
}

static int aspeed_spi_set_speed(struct udevice *bus, uint hz)
{
	dev_dbg(bus, "%s: setting speed to %u\n", bus->name, hz);
	/*
	 * ASPEED SPI controller supports multiple CS with different
	 * clock frequency. We cannot distinguish which CS here.
	 * Thus, the related implementation is postponed to claim_bus.
	 */

	return 0;
}

static int apseed_spi_of_to_plat(struct udevice *bus)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);

	priv->regs = (void __iomem *)devfdt_get_addr_index(bus, 0);
	if ((u32)priv->regs == FDT_ADDR_T_NONE) {
		dev_err(bus, "wrong ctrl base\n");
		return -ENODEV;
	}

	plat->ahb_base =
		(void __iomem *)devfdt_get_addr_size_index(bus, 1, &plat->ahb_sz);
	if ((u32)plat->ahb_base == FDT_ADDR_T_NONE) {
		dev_err(bus, "wrong AHB base\n");
		return -ENODEV;
	}

	plat->max_cs = dev_read_u32_default(bus, "num-cs", ASPEED_SPI_MAX_CS);
	if (plat->max_cs > ASPEED_SPI_MAX_CS)
		return -EINVAL;

	dev_dbg(bus, "ctrl_base = 0x%x, ahb_base = 0x%p, size = 0x%lx\n",
		(u32)priv->regs, plat->ahb_base, plat->ahb_sz);
	dev_dbg(bus, "max_cs = %d\n", plat->max_cs);

	return 0;
}

static int aspeed_spi_probe(struct udevice *bus)
{
	int ret;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct udevice *dev;

	priv->info = (struct aspeed_spi_info *)dev_get_driver_data(bus);

	priv->num_cs = 0;
	for (device_find_first_child(bus, &dev); dev;
	     device_find_next_child(&dev)) {
		priv->num_cs++;
	}

	if (priv->num_cs > ASPEED_SPI_MAX_CS)
		return -EINVAL;

	ret = aspeed_spi_ctrl_init(bus);

	return ret;
}

static const struct spi_controller_mem_ops aspeed_spi_mem_ops = {
	.supports_op = aspeed_spi_supports_op,
	.exec_op = aspeed_spi_exec_op_user_mode,
};

static const struct dm_spi_ops aspeed_spi_ops = {
	.claim_bus = aspeed_spi_claim_bus,
	.release_bus = aspeed_spi_release_bus,
	.set_speed = aspeed_spi_set_speed,
	.set_mode = aspeed_spi_set_mode,
	.mem_ops = &aspeed_spi_mem_ops,
};

static const struct udevice_id aspeed_spi_ids[] = {
	{ .compatible = "aspeed,ast2500-fmc", .data = (ulong)&ast2500_fmc_info, },
	{ .compatible = "aspeed,ast2500-spi", .data = (ulong)&ast2500_spi_info, },
	{ .compatible = "aspeed,ast2600-fmc", .data = (ulong)&ast2600_fmc_info, },
	{ .compatible = "aspeed,ast2600-spi", .data = (ulong)&ast2600_spi_info, },
	{ }
};

U_BOOT_DRIVER(aspeed_spi) = {
	.name = "aspeed_spi_smc",
	.id = UCLASS_SPI,
	.of_match = aspeed_spi_ids,
	.ops = &aspeed_spi_ops,
	.of_to_plat = apseed_spi_of_to_plat,
	.plat_auto = sizeof(struct aspeed_spi_plat),
	.priv_auto = sizeof(struct aspeed_spi_priv),
	.probe = aspeed_spi_probe,
};
