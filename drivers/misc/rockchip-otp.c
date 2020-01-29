// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <misc.h>

/* OTP Register Offsets */
#define OTPC_SBPI_CTRL			0x0020
#define OTPC_SBPI_CMD_VALID_PRE		0x0024
#define OTPC_SBPI_CS_VALID_PRE		0x0028
#define OTPC_SBPI_STATUS		0x002C
#define OTPC_USER_CTRL			0x0100
#define OTPC_USER_ADDR			0x0104
#define OTPC_USER_ENABLE		0x0108
#define OTPC_USER_QP			0x0120
#define OTPC_USER_Q			0x0124
#define OTPC_INT_STATUS			0x0304
#define OTPC_SBPI_CMD0_OFFSET		0x1000
#define OTPC_SBPI_CMD1_OFFSET		0x1004

/* OTP Register bits and masks */
#define OTPC_USER_ADDR_MASK		GENMASK(31, 16)
#define OTPC_USE_USER			BIT(0)
#define OTPC_USE_USER_MASK		GENMASK(16, 16)
#define OTPC_USER_FSM_ENABLE		BIT(0)
#define OTPC_USER_FSM_ENABLE_MASK	GENMASK(16, 16)
#define OTPC_SBPI_DONE			BIT(1)
#define OTPC_USER_DONE			BIT(2)

#define SBPI_DAP_ADDR			0x02
#define SBPI_DAP_ADDR_SHIFT		8
#define SBPI_DAP_ADDR_MASK		GENMASK(31, 24)
#define SBPI_CMD_VALID_MASK		GENMASK(31, 16)
#define SBPI_DAP_CMD_WRF		0xC0
#define SBPI_DAP_REG_ECC		0x3A
#define SBPI_ECC_ENABLE			0x00
#define SBPI_ECC_DISABLE		0x09
#define SBPI_ENABLE			BIT(0)
#define SBPI_ENABLE_MASK		GENMASK(16, 16)

#define OTPC_TIMEOUT			10000

struct rockchip_otp_platdata {
	void __iomem *base;
	unsigned long secure_conf_base;
	unsigned long otp_mask_base;
};

static int rockchip_otp_wait_status(struct rockchip_otp_platdata *otp,
				    u32 flag)
{
	int delay = OTPC_TIMEOUT;

	while (!(readl(otp->base + OTPC_INT_STATUS) & flag)) {
		udelay(1);
		delay--;
		if (delay <= 0) {
			printf("%s: wait init status timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	/* clean int status */
	writel(flag, otp->base + OTPC_INT_STATUS);

	return 0;
}

static int rockchip_otp_ecc_enable(struct rockchip_otp_platdata *otp,
				   bool enable)
{
	int ret = 0;

	writel(SBPI_DAP_ADDR_MASK | (SBPI_DAP_ADDR << SBPI_DAP_ADDR_SHIFT),
	       otp->base + OTPC_SBPI_CTRL);

	writel(SBPI_CMD_VALID_MASK | 0x1, otp->base + OTPC_SBPI_CMD_VALID_PRE);
	writel(SBPI_DAP_CMD_WRF | SBPI_DAP_REG_ECC,
	       otp->base + OTPC_SBPI_CMD0_OFFSET);

	if (enable)
		writel(SBPI_ECC_ENABLE, otp->base + OTPC_SBPI_CMD1_OFFSET);
	else
		writel(SBPI_ECC_DISABLE, otp->base + OTPC_SBPI_CMD1_OFFSET);

	writel(SBPI_ENABLE_MASK | SBPI_ENABLE, otp->base + OTPC_SBPI_CTRL);

	ret = rockchip_otp_wait_status(otp, OTPC_SBPI_DONE);
	if (ret < 0)
		printf("%s timeout during ecc_enable\n", __func__);

	return ret;
}

static int rockchip_px30_otp_read(struct udevice *dev, int offset,
				  void *buf, int size)
{
	struct rockchip_otp_platdata *otp = dev_get_platdata(dev);
	u8 *buffer = buf;
	int ret = 0;

	ret = rockchip_otp_ecc_enable(otp, false);
	if (ret < 0) {
		printf("%s rockchip_otp_ecc_enable err\n", __func__);
		return ret;
	}

	writel(OTPC_USE_USER | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);
	udelay(5);
	while (size--) {
		writel(offset++ | OTPC_USER_ADDR_MASK,
		       otp->base + OTPC_USER_ADDR);
		writel(OTPC_USER_FSM_ENABLE | OTPC_USER_FSM_ENABLE_MASK,
		       otp->base + OTPC_USER_ENABLE);

		ret = rockchip_otp_wait_status(otp, OTPC_USER_DONE);
		if (ret < 0) {
			printf("%s timeout during read setup\n", __func__);
			goto read_end;
		}

		*buffer++ = readb(otp->base + OTPC_USER_Q);
	}

read_end:
	writel(0x0 | OTPC_USE_USER_MASK, otp->base + OTPC_USER_CTRL);

	return ret;
}

static int rockchip_otp_read(struct udevice *dev, int offset,
			     void *buf, int size)
{
	return rockchip_px30_otp_read(dev, offset, buf, size);
}

static const struct misc_ops rockchip_otp_ops = {
	.read = rockchip_otp_read,
};

static int rockchip_otp_ofdata_to_platdata(struct udevice *dev)
{
	struct rockchip_otp_platdata *otp = dev_get_platdata(dev);

	otp->base = dev_read_addr_ptr(dev);

	return 0;
}

static const struct udevice_id rockchip_otp_ids[] = {
	{
		.compatible = "rockchip,px30-otp",
		.data = (ulong)&rockchip_px30_otp_read,
	},
	{
		.compatible = "rockchip,rk3308-otp",
		.data = (ulong)&rockchip_px30_otp_read,
	},
	{}
};

U_BOOT_DRIVER(rockchip_otp) = {
	.name = "rockchip_otp",
	.id = UCLASS_MISC,
	.of_match = rockchip_otp_ids,
	.ops = &rockchip_otp_ops,
	.ofdata_to_platdata = rockchip_otp_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct rockchip_otp_platdata),
};
