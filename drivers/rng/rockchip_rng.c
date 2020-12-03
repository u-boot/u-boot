// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
 */
#include <asm/arch-rockchip/hardware.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/string.h>
#include <rng.h>

#define RK_HW_RNG_MAX 32

#define _SBF(s, v)	((v) << (s))

/* start of CRYPTO V1 register define */
#define CRYPTO_V1_CTRL				0x0008
#define CRYPTO_V1_RNG_START			BIT(8)
#define CRYPTO_V1_RNG_FLUSH			BIT(9)

#define CRYPTO_V1_TRNG_CTRL			0x0200
#define CRYPTO_V1_OSC_ENABLE			BIT(16)
#define CRYPTO_V1_TRNG_SAMPLE_PERIOD(x)		(x)

#define CRYPTO_V1_TRNG_DOUT_0			0x0204
/* end of CRYPTO V1 register define */

/* start of CRYPTO V2 register define */
#define CRYPTO_V2_RNG_CTL			0x0400
#define CRYPTO_V2_RNG_64_BIT_LEN		_SBF(4, 0x00)
#define CRYPTO_V2_RNG_128_BIT_LEN		_SBF(4, 0x01)
#define CRYPTO_V2_RNG_192_BIT_LEN		_SBF(4, 0x02)
#define CRYPTO_V2_RNG_256_BIT_LEN		_SBF(4, 0x03)
#define CRYPTO_V2_RNG_FATESY_SOC_RING		_SBF(2, 0x00)
#define CRYPTO_V2_RNG_SLOWER_SOC_RING_0		_SBF(2, 0x01)
#define CRYPTO_V2_RNG_SLOWER_SOC_RING_1		_SBF(2, 0x02)
#define CRYPTO_V2_RNG_SLOWEST_SOC_RING		_SBF(2, 0x03)
#define CRYPTO_V2_RNG_ENABLE			BIT(1)
#define CRYPTO_V2_RNG_START			BIT(0)
#define CRYPTO_V2_RNG_SAMPLE_CNT		0x0404
#define CRYPTO_V2_RNG_DOUT_0			0x0410
/* end of CRYPTO V2 register define */

#define RK_RNG_TIME_OUT	50000  /* max 50ms */

struct rk_rng_soc_data {
	int (*rk_rng_read)(struct udevice *dev, void *data, size_t len);
};

struct rk_rng_plat {
	fdt_addr_t base;
	struct rk_rng_soc_data *soc_data;
};

static int rk_rng_read_regs(fdt_addr_t addr, void *buf, size_t size)
{
	u32 count = RK_HW_RNG_MAX / sizeof(u32);
	u32 reg, tmp_len;

	if (size > RK_HW_RNG_MAX)
		return -EINVAL;

	while (size && count) {
		reg = readl(addr);
		tmp_len = min(size, sizeof(u32));
		memcpy(buf, &reg, tmp_len);
		addr += sizeof(u32);
		buf += tmp_len;
		size -= tmp_len;
		count--;
	}

	return 0;
}

static int rk_v1_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_plat *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	/* enable osc_ring to get entropy, sample period is set as 100 */
	writel(CRYPTO_V1_OSC_ENABLE | CRYPTO_V1_TRNG_SAMPLE_PERIOD(100),
	       pdata->base + CRYPTO_V1_TRNG_CTRL);

	rk_clrsetreg(pdata->base + CRYPTO_V1_CTRL, CRYPTO_V1_RNG_START,
		     CRYPTO_V1_RNG_START);

	retval = readl_poll_timeout(pdata->base + CRYPTO_V1_CTRL, reg,
				    !(reg & CRYPTO_V1_RNG_START),
				    RK_RNG_TIME_OUT);
	if (retval)
		goto exit;

	rk_rng_read_regs(pdata->base + CRYPTO_V1_TRNG_DOUT_0, data, len);

exit:
	/* close TRNG */
	rk_clrreg(pdata->base + CRYPTO_V1_CTRL, CRYPTO_V1_RNG_START);

	return 0;
}

static int rk_v2_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct rk_rng_plat *pdata = dev_get_priv(dev);
	u32 reg = 0;
	int retval;

	if (len > RK_HW_RNG_MAX)
		return -EINVAL;

	/* enable osc_ring to get entropy, sample period is set as 100 */
	writel(100, pdata->base + CRYPTO_V2_RNG_SAMPLE_CNT);

	reg |= CRYPTO_V2_RNG_256_BIT_LEN;
	reg |= CRYPTO_V2_RNG_SLOWER_SOC_RING_0;
	reg |= CRYPTO_V2_RNG_ENABLE;
	reg |= CRYPTO_V2_RNG_START;

	rk_clrsetreg(pdata->base + CRYPTO_V2_RNG_CTL, 0xffff, reg);

	retval = readl_poll_timeout(pdata->base + CRYPTO_V2_RNG_CTL, reg,
				    !(reg & CRYPTO_V2_RNG_START),
				    RK_RNG_TIME_OUT);
	if (retval)
		goto exit;

	rk_rng_read_regs(pdata->base + CRYPTO_V2_RNG_DOUT_0, data, len);

exit:
	/* close TRNG */
	rk_clrreg(pdata->base + CRYPTO_V2_RNG_CTL, 0xffff);

	return retval;
}

static int rockchip_rng_read(struct udevice *dev, void *data, size_t len)
{
	unsigned char *buf = data;
	unsigned int i;
	int ret = -EIO;

	struct rk_rng_plat *pdata = dev_get_priv(dev);

	if (!len)
		return 0;

	if (!pdata->soc_data || !pdata->soc_data->rk_rng_read)
		return -EINVAL;

	for (i = 0; i < len / RK_HW_RNG_MAX; i++, buf += RK_HW_RNG_MAX) {
		ret = pdata->soc_data->rk_rng_read(dev, buf, RK_HW_RNG_MAX);
		if (ret)
			goto exit;
	}

	if (len % RK_HW_RNG_MAX)
		ret = pdata->soc_data->rk_rng_read(dev, buf,
						   len % RK_HW_RNG_MAX);

exit:
	return ret;
}

static int rockchip_rng_of_to_plat(struct udevice *dev)
{
	struct rk_rng_plat *pdata = dev_get_priv(dev);

	memset(pdata, 0x00, sizeof(*pdata));

	pdata->base = (fdt_addr_t)dev_read_addr_ptr(dev);
	if (!pdata->base)
		return -ENOMEM;

	return 0;
}

static int rockchip_rng_probe(struct udevice *dev)
{
	struct rk_rng_plat *pdata = dev_get_priv(dev);

	pdata->soc_data = (struct rk_rng_soc_data *)dev_get_driver_data(dev);

	return 0;
}

static const struct rk_rng_soc_data rk_rng_v1_soc_data = {
	.rk_rng_read = rk_v1_rng_read,
};

static const struct rk_rng_soc_data rk_rng_v2_soc_data = {
	.rk_rng_read = rk_v2_rng_read,
};

static const struct dm_rng_ops rockchip_rng_ops = {
	.read = rockchip_rng_read,
};

static const struct udevice_id rockchip_rng_match[] = {
	{
		.compatible = "rockchip,cryptov1-rng",
		.data = (ulong)&rk_rng_v1_soc_data,
	},
	{
		.compatible = "rockchip,cryptov2-rng",
		.data = (ulong)&rk_rng_v2_soc_data,
	},
	{},
};

U_BOOT_DRIVER(rockchip_rng) = {
	.name = "rockchip-rng",
	.id = UCLASS_RNG,
	.of_match = rockchip_rng_match,
	.ops = &rockchip_rng_ops,
	.probe = rockchip_rng_probe,
	.of_to_plat = rockchip_rng_of_to_plat,
	.priv_auto	= sizeof(struct rk_rng_plat),
};
