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

/* ASPEED FMC/SPI memory control register related */
#define REG_CE_TYPE_SETTING          0x00
#define REG_CE_ADDR_MODE_CTRL        0x04
#define REG_INTR_CTRL_STATUS         0x08
#define REG_CE0_CTRL_REG             0x10
#define REG_CE0_DECODED_ADDR_REG     0x30

#define ASPEED_SPI_MAX_CS       3
#define FLASH_CALIBRATION_LEN   0x400

#define CTRL_IO_SINGLE_DATA     0
#define CTRL_IO_QUAD_DATA       BIT(30)
#define CTRL_IO_DUAL_DATA       BIT(29)

#define CTRL_IO_MODE_USER       GENMASK(1, 0)
#define CTRL_IO_MODE_CMD_READ   BIT(0)
#define CTRL_IO_MODE_CMD_WRITE  BIT(1)
#define CTRL_STOP_ACTIVE        BIT(2)

struct aspeed_spi_plat {
	fdt_addr_t ctrl_base;
	void __iomem *ahb_base; /* AHB address base for all flash devices. */
	fdt_size_t ahb_sz; /* Overall AHB window size for all flash device. */
	u32 hclk_rate; /* AHB clock rate */
	u8 max_cs;
};

struct aspeed_spi_flash {
	u8 cs;
	void __iomem *ahb_base;
	u32 ahb_win_sz;
	u32 ce_ctrl_user;
	u32 ce_ctrl_read;
	u32 max_freq;
	bool trimmed_decoded_sz;
};

struct aspeed_spi_priv {
	u32 num_cs;
	struct aspeed_spi_info *info;
	struct aspeed_spi_flash flashes[ASPEED_SPI_MAX_CS];
	u32 decoded_sz_arr[ASPEED_SPI_MAX_CS];
};

struct aspeed_spi_info {
	u32 cmd_io_ctrl_mask;
	u32 clk_ctrl_mask;
	u32 max_data_bus_width;
	u32 min_decoded_sz;
	void (*set_4byte)(struct udevice *bus, u32 cs);
	u32 (*segment_start)(struct udevice *bus, u32 reg);
	u32 (*segment_end)(struct udevice *bus, u32 reg);
	u32 (*segment_reg)(u32 start, u32 end);
	int (*adjust_decoded_sz)(struct udevice *bus, u32 decoded_sz_arr[]);
	u32 (*get_clk_setting)(struct udevice *dev, uint hz);
};

static int aspeed_spi_trim_decoded_size(struct udevice *bus,
					u32 decoded_sz_arr[]);
static int aspeed_spi_decoded_range_config(struct udevice *bus,
					   u32 decoded_sz_arr[]);

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

	return (u32)plat->ahb_base + end_offset + 0x100000;
}

static u32 ast2500_spi_segment_reg(u32 start, u32 end)
{
	if (start == end)
		return 0;

	return ((((start) >> 23) & 0xff) << 16) | ((((end) >> 23) & 0xff) << 24);
}

static void ast2500_spi_chip_set_4byte(struct udevice *bus, u32 cs)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 reg_val;

	reg_val = readl(plat->ctrl_base + REG_CE_ADDR_MODE_CTRL);
	reg_val |= 0x1 << cs;
	writel(reg_val, plat->ctrl_base + REG_CE_ADDR_MODE_CTRL);
}

/*
 * For AST2500, the minimum address decoded size for each CS
 * is 8MB instead of zero. This address decoded size is
 * mandatory for each CS no matter whether it will be used.
 * This is a HW limitation.
 */
static int ast2500_adjust_decoded_size(struct udevice *bus,
				       u32 decoded_sz_arr[])
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	int ret;
	int cs;

	/* Assign min_decoded_sz to unused CS. */
	for (cs = priv->num_cs; cs < plat->max_cs; cs++) {
		if (decoded_sz_arr[cs] < priv->info->min_decoded_sz)
			decoded_sz_arr[cs] = priv->info->min_decoded_sz;
	}

	ret = aspeed_spi_trim_decoded_size(bus, decoded_sz_arr);
	if (ret != 0)
		return ret;

	return 0;
}

/* Transfer maximum clock frequency to register setting */
static u32 ast2500_get_clk_setting(struct udevice *dev, uint max_hz)
{
	struct aspeed_spi_plat *plat = dev_get_plat(dev->parent);
	struct aspeed_spi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 hclk_clk = plat->hclk_rate;
	u32 hclk_div = 0x000; /* default value */
	u32 i;
	bool found = false;
	/* HCLK/1 ..	HCLK/16 */
	u32 hclk_masks[] = {15, 7, 14, 6, 13, 5, 12, 4,
				 11, 3, 10, 2, 9,  1, 8,  0};

	/* FMC/SPIR10[11:8] */
	for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
		if (hclk_clk / (i + 1) <= max_hz) {
			found = true;
			priv->flashes[slave_plat->cs].max_freq =
							hclk_clk / (i + 1);
			break;
		}
	}

	if (found) {
		hclk_div = hclk_masks[i] << 8;
		goto end;
	}

	for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
		if (hclk_clk / ((i + 1) * 4) <= max_hz) {
			found = true;
			priv->flashes[slave_plat->cs].max_freq =
						hclk_clk / ((i + 1) * 4);
			break;
		}
	}

	if (found)
		hclk_div = BIT(13) | (hclk_masks[i] << 8);

end:
	dev_dbg(dev, "found: %s, hclk: %d, max_clk: %d\n", found ? "yes" : "no",
		hclk_clk, max_hz);

	if (found) {
		dev_dbg(dev, "h_div: %d (mask %x), speed: %d\n",
			i + 1, hclk_masks[i], priv->flashes[slave_plat->cs].max_freq);
	}

	return hclk_div;
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
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	u32 reg_val;

	reg_val = readl(plat->ctrl_base + REG_CE_ADDR_MODE_CTRL);
	reg_val |= 0x11 << cs;
	writel(reg_val, plat->ctrl_base + REG_CE_ADDR_MODE_CTRL);
}

static int ast2600_adjust_decoded_size(struct udevice *bus,
				       u32 decoded_sz_arr[])
{
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	int ret;
	int i;
	int cs;
	u32 pre_sz;
	u32 lack_sz;

	/*
	 * If commnad mode or normal mode is used, the start address of a
	 * decoded range should be multiple of its related flash size.
	 * Namely, the total decoded size from flash 0 to flash N should
	 * be multiple of the size of flash (N + 1).
	 */
	for (cs = priv->num_cs - 1; cs >= 0; cs--) {
		pre_sz = 0;
		for (i = 0; i < cs; i++)
			pre_sz += decoded_sz_arr[i];

		if (decoded_sz_arr[cs] != 0 && (pre_sz % decoded_sz_arr[cs]) != 0) {
			lack_sz = decoded_sz_arr[cs] - (pre_sz % decoded_sz_arr[cs]);
			decoded_sz_arr[0] += lack_sz;
		}
	}

	ret = aspeed_spi_trim_decoded_size(bus, decoded_sz_arr);
	if (ret != 0)
		return ret;

	return 0;
}

/* Transfer maximum clock frequency to register setting */
static u32 ast2600_get_clk_setting(struct udevice *dev, uint max_hz)
{
	struct aspeed_spi_plat *plat = dev_get_plat(dev->parent);
	struct aspeed_spi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 hclk_clk = plat->hclk_rate;
	u32 hclk_div = 0x400; /* default value */
	u32 i, j;
	bool found = false;
	/* HCLK/1 ..	HCLK/16 */
	u32 hclk_masks[] = {15, 7, 14, 6, 13, 5, 12, 4,
				 11, 3, 10, 2, 9,  1, 8,  0};

	/* FMC/SPIR10[27:24] */
	for (j = 0; j < 0xf; j++) {
		/* FMC/SPIR10[11:8] */
		for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
			if (i == 0 && j == 0)
				continue;

			if (hclk_clk / (i + 1 + (j * 16)) <= max_hz) {
				found = true;
				break;
			}
		}

		if (found) {
			hclk_div = ((j << 24) | hclk_masks[i] << 8);
			priv->flashes[slave_plat->cs].max_freq =
						hclk_clk / (i + 1 + j * 16);
			break;
		}
	}

	dev_dbg(dev, "found: %s, hclk: %d, max_clk: %d\n", found ? "yes" : "no",
		hclk_clk, max_hz);

	if (found) {
		dev_dbg(dev, "base_clk: %d, h_div: %d (mask %x), speed: %d\n",
			j, i + 1, hclk_masks[i], priv->flashes[slave_plat->cs].max_freq);
	}

	return hclk_div;
}

/*
 * As the flash size grows up, we need to trim some decoded
 * size if needed for the sake of conforming the maximum
 * decoded size. We trim the decoded size from the largest
 * CS in order to avoid affecting the default boot up sequence
 * from CS0 where command mode or normal mode is used.
 * Notice, if a CS decoded size is trimmed, command mode may
 * not work perfectly on that CS.
 */
static int aspeed_spi_trim_decoded_size(struct udevice *bus,
					u32 decoded_sz_arr[])
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	u32 total_sz;
	int cs = plat->max_cs - 1;
	u32 i;

	do {
		total_sz = 0;
		for (i = 0; i < plat->max_cs; i++)
			total_sz += decoded_sz_arr[i];

		if (decoded_sz_arr[cs] <= priv->info->min_decoded_sz)
			cs--;

		if (cs < 0)
			return -ENOMEM;

		if (total_sz > plat->ahb_sz) {
			decoded_sz_arr[cs] -= priv->info->min_decoded_sz;
			total_sz -= priv->info->min_decoded_sz;
			priv->flashes[cs].trimmed_decoded_sz = true;
		}
	} while (total_sz > plat->ahb_sz);

	return 0;
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

	if (op->addr.buswidth > 1 || op->addr.nbytes > 4)
		return false;

	if (op->dummy.buswidth > 1 || op->dummy.nbytes > 7)
		return false;

	if (op->data.buswidth > priv->info->max_data_bus_width)
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
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	u32 cs = slave_plat->cs;
	fdt_addr_t ctrl_reg = plat->ctrl_base + REG_CE0_CTRL_REG + cs * 4;
	struct aspeed_spi_flash *flash = &priv->flashes[cs];
	u32 ctrl_val;
	u8 dummy_data[16] = {0};
	u8 addr[4] = {0};
	int i;

	dev_dbg(dev, "cmd:%x(%d),addr:%llx(%d),dummy:%d(%d),data_len:0x%x(%d)\n",
		op->cmd.opcode, op->cmd.buswidth, op->addr.val,
		op->addr.buswidth, op->dummy.nbytes, op->dummy.buswidth,
		op->data.nbytes, op->data.buswidth);

	/* Start user mode */
	ctrl_val = flash->ce_ctrl_user;
	writel(ctrl_val, ctrl_reg);
	ctrl_val &= (~CTRL_STOP_ACTIVE);
	writel(ctrl_val, ctrl_reg);

	/* Send command */
	aspeed_spi_write_to_ahb(flash->ahb_base, &op->cmd.opcode, 1);

	/* Send address */
	for (i = op->addr.nbytes; i > 0; i--) {
		addr[op->addr.nbytes - i] =
			((u32)op->addr.val >> ((i - 1) * 8)) & 0xff;
	}
	aspeed_spi_write_to_ahb(flash->ahb_base, addr, op->addr.nbytes);

	/* Send dummy cycle */
	aspeed_spi_write_to_ahb(flash->ahb_base, dummy_data, op->dummy.nbytes);

	/* Change io_mode */
	ctrl_val |= aspeed_spi_get_io_mode(op->data.buswidth);
	writel(ctrl_val, ctrl_reg);

	/* Send data */
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		aspeed_spi_write_to_ahb(flash->ahb_base, op->data.buf.out,
					op->data.nbytes);
	} else {
		aspeed_spi_read_from_ahb(flash->ahb_base, op->data.buf.in,
					 op->data.nbytes);
	}

	ctrl_val |= CTRL_STOP_ACTIVE;
	writel(ctrl_val, ctrl_reg);

	/* Restore controller setting. */
	writel(flash->ce_ctrl_read, ctrl_reg);

	return 0;
}

static int aspeed_spi_dirmap_create(struct spi_mem_dirmap_desc *desc)
{
	int ret = 0;
	struct udevice *dev = desc->slave->dev;
	struct udevice *bus = dev->parent;
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	const struct aspeed_spi_info *info = priv->info;
	struct spi_mem_op op_tmpl = desc->info.op_tmpl;
	u32 i;
	u32 cs = slave_plat->cs;
	u32 decoded_sz_arr[ASPEED_SPI_MAX_CS];
	u32 reg_val;

	if (desc->info.op_tmpl.data.dir == SPI_MEM_DATA_IN) {
		for (i = 0; i < priv->num_cs; i++) {
			reg_val = readl(plat->ctrl_base +
					REG_CE0_DECODED_ADDR_REG + i * 4);
			decoded_sz_arr[i] =
				info->segment_end(bus, reg_val) -
				info->segment_start(bus, reg_val);
		}

		decoded_sz_arr[cs] = desc->info.length;

		if (info->adjust_decoded_sz)
			info->adjust_decoded_sz(bus, decoded_sz_arr);

		for (i = 0; i < priv->num_cs; i++) {
			dev_dbg(dev, "cs: %d, sz: 0x%x\n", i,
				decoded_sz_arr[i]);
		}

		ret = aspeed_spi_decoded_range_config(bus, decoded_sz_arr);
		if (ret)
			return ret;

		reg_val = readl(plat->ctrl_base + REG_CE0_CTRL_REG + cs * 4) &
			  (~info->cmd_io_ctrl_mask);
		reg_val |= aspeed_spi_get_io_mode(op_tmpl.data.buswidth) |
			   op_tmpl.cmd.opcode << 16 |
			   ((op_tmpl.dummy.nbytes) & 0x3) << 6 |
			   ((op_tmpl.dummy.nbytes) & 0x4) << 14 |
			   CTRL_IO_MODE_CMD_READ;

		writel(reg_val,
		       plat->ctrl_base + REG_CE0_CTRL_REG + cs * 4);
		priv->flashes[cs].ce_ctrl_read = reg_val;

		if (op_tmpl.addr.nbytes == 4)
			priv->info->set_4byte(bus, cs);

		dev_dbg(dev, "read bus width: %d [0x%08x]\n",
			op_tmpl.data.buswidth, priv->flashes[cs].ce_ctrl_read);
	} else {
		/*
		 * dirmap_write is not supported currently due to a HW
		 * limitation for command write mode: The written data
		 * length should be multiple of 4-byte.
		 */
		return -EOPNOTSUPP;
	}

	return ret;
}

static ssize_t aspeed_spi_dirmap_read(struct spi_mem_dirmap_desc *desc,
				      u64 offs, size_t len, void *buf)
{
	struct udevice *dev = desc->slave->dev;
	struct aspeed_spi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 cs = slave_plat->cs;
	int ret;

	dev_dbg(dev, "read op:0x%x, addr:0x%llx, len:0x%x\n",
		desc->info.op_tmpl.cmd.opcode, offs, len);

	if (priv->flashes[cs].ahb_win_sz < offs + len ||
	    (offs + len) % 4 != 0) {
		ret = aspeed_spi_exec_op_user_mode(desc->slave,
						   &desc->info.op_tmpl);
		if (ret != 0)
			return 0;
	} else {
		memcpy_fromio(buf, priv->flashes[cs].ahb_base + offs, len);
	}

	return len;
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

static int aspeed_spi_decoded_range_config(struct udevice *bus,
					   u32 decoded_sz_arr[])
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	int ret;
	u32 cs;
	u32 decoded_reg_val;
	u32 start_addr;
	u32 end_addr = 0;

	ret = priv->info->adjust_decoded_sz(bus, decoded_sz_arr);
	if (ret != 0)
		return ret;

	/* Configure each CS decoded range */
	for (cs = 0; cs < plat->max_cs; cs++) {
		if (cs == 0)
			start_addr = (u32)plat->ahb_base;
		else
			start_addr = end_addr;

		priv->flashes[cs].ahb_base = (void __iomem *)start_addr;
		priv->flashes[cs].ahb_win_sz = decoded_sz_arr[cs];

		end_addr = start_addr + decoded_sz_arr[cs];
		decoded_reg_val = priv->info->segment_reg(start_addr, end_addr);

		writel(decoded_reg_val,
		       plat->ctrl_base + REG_CE0_DECODED_ADDR_REG + cs * 4);

		dev_dbg(bus, "cs: %d, decoded_reg: 0x%x, start: 0x%x, end: 0x%x\n",
			cs, decoded_reg_val, start_addr, end_addr);
	}

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
	reg_val = readl(plat->ctrl_base + REG_CE_TYPE_SETTING);
	writel(reg_val | (GENMASK(plat->max_cs - 1, 0) << 16),
	       plat->ctrl_base + REG_CE_TYPE_SETTING);

	memset(priv->flashes, 0x0,
	       sizeof(struct aspeed_spi_flash) * ASPEED_SPI_MAX_CS);

	/* Initial each CS controller register */
	for (cs = 0; cs < priv->num_cs; cs++) {
		priv->flashes[cs].ce_ctrl_user &=
				~(priv->info->cmd_io_ctrl_mask);
		priv->flashes[cs].ce_ctrl_user |=
				(CTRL_STOP_ACTIVE | CTRL_IO_MODE_USER);
		writel(priv->flashes[cs].ce_ctrl_user,
		       plat->ctrl_base + REG_CE0_CTRL_REG + cs * 4);
	}

	memset(priv->decoded_sz_arr, 0x0, sizeof(u32) * ASPEED_SPI_MAX_CS);

	for (cs = 0; cs < priv->num_cs; cs++) {
		reg_val = readl(plat->ctrl_base + REG_CE0_DECODED_ADDR_REG + cs * 4);
		decoded_sz = priv->info->segment_end(bus, reg_val) -
			     priv->info->segment_start(bus, reg_val);

		/*
		 * For CS0, if the default address decoded area exists,
		 * keep its value in order to make sure that the whole boot
		 * image can be accessed with normal read mode.
		 */
		if (cs == 0 && decoded_sz != 0)
			priv->decoded_sz_arr[cs] = decoded_sz;
		else
			priv->decoded_sz_arr[cs] = priv->info->min_decoded_sz;
	}

	ret = aspeed_spi_decoded_range_config(bus, priv->decoded_sz_arr);

	return ret;
}

static const struct aspeed_spi_info ast2500_fmc_info = {
	.max_data_bus_width = 2,
	.cmd_io_ctrl_mask = 0x70ff40c3,
	.clk_ctrl_mask = 0x00002f00,
	.min_decoded_sz = 0x800000,
	.set_4byte = ast2500_spi_chip_set_4byte,
	.segment_start = ast2500_spi_segment_start,
	.segment_end = ast2500_spi_segment_end,
	.segment_reg = ast2500_spi_segment_reg,
	.adjust_decoded_sz = ast2500_adjust_decoded_size,
	.get_clk_setting = ast2500_get_clk_setting,
};

/*
 * There are some different between FMC and SPI controllers.
 * For example, DMA operation, but this isn't implemented currently.
 */
static const struct aspeed_spi_info ast2500_spi_info = {
	.max_data_bus_width = 2,
	.cmd_io_ctrl_mask = 0x70ff40c3,
	.clk_ctrl_mask = 0x00002f00,
	.min_decoded_sz = 0x800000,
	.set_4byte = ast2500_spi_chip_set_4byte,
	.segment_start = ast2500_spi_segment_start,
	.segment_end = ast2500_spi_segment_end,
	.segment_reg = ast2500_spi_segment_reg,
	.adjust_decoded_sz = ast2500_adjust_decoded_size,
	.get_clk_setting = ast2500_get_clk_setting,
};

static const struct aspeed_spi_info ast2600_fmc_info = {
	.max_data_bus_width = 4,
	.cmd_io_ctrl_mask = 0xf0ff40c3,
	.clk_ctrl_mask = 0x0f000f00,
	.min_decoded_sz = 0x200000,
	.set_4byte = ast2600_spi_chip_set_4byte,
	.segment_start = ast2600_spi_segment_start,
	.segment_end = ast2600_spi_segment_end,
	.segment_reg = ast2600_spi_segment_reg,
	.adjust_decoded_sz = ast2600_adjust_decoded_size,
	.get_clk_setting = ast2600_get_clk_setting,
};

static const struct aspeed_spi_info ast2600_spi_info = {
	.max_data_bus_width = 4,
	.cmd_io_ctrl_mask = 0xf0ff40c3,
	.clk_ctrl_mask = 0x0f000f00,
	.min_decoded_sz = 0x200000,
	.set_4byte = ast2600_spi_chip_set_4byte,
	.segment_start = ast2600_spi_segment_start,
	.segment_end = ast2600_spi_segment_end,
	.segment_reg = ast2600_spi_segment_reg,
	.adjust_decoded_sz = ast2600_adjust_decoded_size,
	.get_clk_setting = ast2600_get_clk_setting,
};

static int aspeed_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct aspeed_spi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct aspeed_spi_flash *flash = &priv->flashes[slave_plat->cs];
	u32 clk_setting;

	dev_dbg(bus, "%s: claim bus CS%u\n", bus->name, slave_plat->cs);

	if (flash->max_freq == 0) {
		clk_setting = priv->info->get_clk_setting(dev, slave_plat->max_hz);
		flash->ce_ctrl_user &= ~(priv->info->clk_ctrl_mask);
		flash->ce_ctrl_user |= clk_setting;
		flash->ce_ctrl_read &= ~(priv->info->clk_ctrl_mask);
		flash->ce_ctrl_read |= clk_setting;
	}

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
	/* ASPEED SPI controller supports multiple CS with different
	 * clock frequency. We cannot distinguish which CS here.
	 * Thus, the related implementation is postponed to claim_bus.
	 */

	return 0;
}

static int apseed_spi_of_to_plat(struct udevice *bus)
{
	struct aspeed_spi_plat *plat = dev_get_plat(bus);
	struct clk hclk;
	int ret;

	plat->ctrl_base = devfdt_get_addr_index(bus, 0);
	if ((u32)plat->ctrl_base == FDT_ADDR_T_NONE) {
		dev_err(bus, "wrong AHB base\n");
		return -ENODEV;
	}

	plat->ahb_base =
		(void __iomem *)devfdt_get_addr_size_index(bus, 1, &plat->ahb_sz);
	if ((u32)plat->ahb_base == FDT_ADDR_T_NONE) {
		dev_err(bus, "wrong AHB base\n");
		return -ENODEV;
	}

	ret = clk_get_by_index(bus, 0, &hclk);
	if (ret < 0) {
		dev_err(bus, "%s could not get clock: %d\n", bus->name, ret);
		return ret;
	}

	plat->hclk_rate = clk_get_rate(&hclk);
	clk_free(&hclk);

	plat->max_cs = dev_read_u32_default(bus, "num-cs", ASPEED_SPI_MAX_CS);
	if (plat->max_cs > ASPEED_SPI_MAX_CS)
		return -EINVAL;

	dev_dbg(bus, "ctrl_base = 0x%lx, ahb_base = 0x%p, size = 0x%lx\n",
		plat->ctrl_base, plat->ahb_base, plat->ahb_sz);
	dev_dbg(bus, "hclk = %dMHz, max_cs = %d\n",
		plat->hclk_rate / 1000000, plat->max_cs);

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
	.dirmap_create = aspeed_spi_dirmap_create,
	.dirmap_read = aspeed_spi_dirmap_read,
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
	.name = "aspeed_spi",
	.id = UCLASS_SPI,
	.of_match = aspeed_spi_ids,
	.ops = &aspeed_spi_ops,
	.of_to_plat = apseed_spi_of_to_plat,
	.plat_auto = sizeof(struct aspeed_spi_plat),
	.priv_auto = sizeof(struct aspeed_spi_priv),
	.probe = aspeed_spi_probe,
};
