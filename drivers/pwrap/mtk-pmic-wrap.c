// SPDX-License-Identifier: GPL-2.0-only

#include <common.h>
#include <clk.h>
#include <time.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <pwrap/pwrap.h>
#include <reset.h>

DECLARE_GLOBAL_DATA_PTR;

struct pmic_wrapper {
    void __iomem *base;
    struct clk clk_spi;
    struct clk clk_wrap;
};

u32 pwrap_readl(struct udevice *dev, u32 reg)
{
    struct pmic_wrapper *priv = dev_get_priv(dev);
	mdelay(10);
	return readl(priv->base + reg);
}
void pwrap_writel(struct udevice *dev, u32 val, u32 reg)
{
    struct pmic_wrapper *priv = dev_get_priv(dev);
	mdelay(10);
	writel(val, priv->base + reg);
}

static const struct udevice_id mtk_pwrap_ids[] = {
	{
		.compatible = "mediatek,mtk-pwrap",
	}, {

	}
};

static int pwrap_probe(struct udevice *dev)
{
    struct pmic_wrapper *priv = dev_get_priv(dev);
	int ret;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return log_msg_ret("pwrap_base", -EINVAL);

	priv->base = ioremap(addr, 0x1000);

	ret = clk_get_by_name(dev, "spi", &priv->clk_spi);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "wrap", &priv->clk_wrap);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk_spi);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk_wrap);
	if (ret)
		goto err_out1;
    
	mtk_pwrap_init(dev);
    
	return 0;

err_out1:
	clk_disable_unprepare(&priv->clk_spi);
    clk_disable_unprepare(&priv->clk_wrap);

	return 0;

}

static const struct dm_pwrap_ops mtk_pwrap_ops = {
    .read = pwrap_read,
    .write = pwrap_write,
};

U_BOOT_DRIVER(mtk_pwrap) = {
	.name = "mtk_pwrap",
	.id = UCLASS_PWRAP,
	.of_match = mtk_pwrap_ids,
	.ops = &mtk_pwrap_ops,
	.probe = pwrap_probe,
	.priv_auto	= sizeof(struct pmic_wrapper),
    .flags = DM_FLAG_PRE_RELOC
};
