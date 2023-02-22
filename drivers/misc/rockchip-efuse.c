// SPDX-License-Identifier: GPL-2.0+
/*
 * eFuse driver for Rockchip devices
 *
 * Copyright 2017, Theobroma Systems Design und Consulting GmbH
 * Written by Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <display_options.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <malloc.h>
#include <misc.h>

#define EFUSE_CTRL		0x0000
#define RK3399_A_SHIFT		16
#define RK3399_A_MASK		GENMASK(25, 16)
#define RK3399_ADDR(n)		((n) << RK3399_A_SHIFT)
#define RK3399_STROBSFTSEL	BIT(9)
#define RK3399_RSB		BIT(7)
#define RK3399_PD		BIT(5)
#define RK3399_PGENB		BIT(3)
#define RK3399_LOAD		BIT(2)
#define RK3399_STROBE		BIT(1)
#define RK3399_CSB		BIT(0)
#define EFUSE_DOUT		0x0004

struct rockchip_efuse_plat {
	void __iomem *base;
};

struct rockchip_efuse_data {
	int (*read)(struct udevice *dev, int offset, void *buf, int size);
	int size;
	int block_size;
};

#if defined(DEBUG)
static int dump_efuse(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[])
{
	struct udevice *dev;
	u8 data[4];
	int ret, i;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_efuse), &dev);
	if (ret) {
		printf("%s: no misc-device found\n", __func__);
		return 0;
	}

	for (i = 0; true; i += sizeof(data)) {
		ret = misc_read(dev, i, &data, sizeof(data));
		if (ret < 0)
			return 0;

		print_buffer(i, data, 1, sizeof(data), sizeof(data));
	}

	return 0;
}

U_BOOT_CMD(
	dump_efuse, 1, 1, dump_efuse,
	"Dump the content of the efuse",
	""
);
#endif

static int rockchip_rk3399_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_plat *efuse = dev_get_plat(dev);
	u32 *buffer = buf;

	/* Switch to array read mode */
	writel(RK3399_LOAD | RK3399_PGENB | RK3399_STROBSFTSEL | RK3399_RSB,
	       efuse->base + EFUSE_CTRL);
	udelay(1);

	while (size--) {
		setbits_le32(efuse->base + EFUSE_CTRL,
			     RK3399_STROBE | RK3399_ADDR(offset++));
		udelay(1);
		*buffer++ = readl(efuse->base + EFUSE_DOUT);
		clrbits_le32(efuse->base + EFUSE_CTRL, RK3399_STROBE);
		udelay(1);
	}

	/* Switch to power-down mode */
	writel(RK3399_PD | RK3399_CSB, efuse->base + EFUSE_CTRL);

	return 0;
}

static int rockchip_efuse_read(struct udevice *dev, int offset,
			       void *buf, int size)
{
	const struct rockchip_efuse_data *data =
		(void *)dev_get_driver_data(dev);
	u32 block_start, block_end, block_offset, blocks;
	u8 *buffer;
	int ret;

	if (offset < 0 || !buf || size <= 0 || offset + size > data->size)
		return -EINVAL;

	if (!data->read)
		return -ENOSYS;

	if (data->block_size <= 1)
		return data->read(dev, offset, buf, size);

	block_start = offset / data->block_size;
	block_offset = offset % data->block_size;
	block_end = DIV_ROUND_UP(offset + size, data->block_size);
	blocks = block_end - block_start;

	buffer = calloc(blocks, data->block_size);
	if (!buffer)
		return -ENOMEM;

	ret = data->read(dev, block_start, buffer, blocks);
	if (!ret)
		memcpy(buf, buffer + block_offset, size);

	free(buffer);
	return ret;
}

static const struct misc_ops rockchip_efuse_ops = {
	.read = rockchip_efuse_read,
};

static int rockchip_efuse_of_to_plat(struct udevice *dev)
{
	struct rockchip_efuse_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

	return 0;
}

static const struct rockchip_efuse_data rk3399_data = {
	.read = rockchip_rk3399_efuse_read,
	.size = 0x80,
	.block_size = 4,
};

static const struct udevice_id rockchip_efuse_ids[] = {
	{
		.compatible = "rockchip,rk3399-efuse",
		.data = (ulong)&rk3399_data,
	},
	{}
};

U_BOOT_DRIVER(rockchip_efuse) = {
	.name = "rockchip_efuse",
	.id = UCLASS_MISC,
	.of_match = rockchip_efuse_ids,
	.of_to_plat = rockchip_efuse_of_to_plat,
	.plat_auto = sizeof(struct rockchip_efuse_plat),
	.ops = &rockchip_efuse_ops,
};
