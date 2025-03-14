// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on Linux drivers/clk/clk-en7523.c reworked
 * and detached to a dedicated driver
 *
 * Author: Lorenzo Bianconi <lorenzo@kernel.org> (original driver)
 *	   Christian Marangi <ansuelsmth@gmail.com>
 */

#include <dm.h>
#include <linux/io.h>
#include <reset-uclass.h>

#include <dt-bindings/reset/airoha,en7581-reset.h>

#define RST_NR_PER_BANK			32

#define REG_RESET_CONTROL2		0x830
#define REG_RESET_CONTROL1		0x834

struct airoha_reset_priv {
	const u16 *bank_ofs;
	const u16 *idx_map;
	void __iomem *base;
};

static const u16 en7581_rst_ofs[] = {
	REG_RESET_CONTROL2,
	REG_RESET_CONTROL1,
};

static const u16 en7581_rst_map[] = {
	/* RST_CTRL2 */
	[EN7581_XPON_PHY_RST]		= 0,
	[EN7581_CPU_TIMER2_RST]		= 2,
	[EN7581_HSUART_RST]		= 3,
	[EN7581_UART4_RST]		= 4,
	[EN7581_UART5_RST]		= 5,
	[EN7581_I2C2_RST]		= 6,
	[EN7581_XSI_MAC_RST]		= 7,
	[EN7581_XSI_PHY_RST]		= 8,
	[EN7581_NPU_RST]		= 9,
	[EN7581_I2S_RST]		= 10,
	[EN7581_TRNG_RST]		= 11,
	[EN7581_TRNG_MSTART_RST]	= 12,
	[EN7581_DUAL_HSI0_RST]		= 13,
	[EN7581_DUAL_HSI1_RST]		= 14,
	[EN7581_HSI_RST]		= 15,
	[EN7581_DUAL_HSI0_MAC_RST]	= 16,
	[EN7581_DUAL_HSI1_MAC_RST]	= 17,
	[EN7581_HSI_MAC_RST]		= 18,
	[EN7581_WDMA_RST]		= 19,
	[EN7581_WOE0_RST]		= 20,
	[EN7581_WOE1_RST]		= 21,
	[EN7581_HSDMA_RST]		= 22,
	[EN7581_TDMA_RST]		= 24,
	[EN7581_EMMC_RST]		= 25,
	[EN7581_SOE_RST]		= 26,
	[EN7581_PCIE2_RST]		= 27,
	[EN7581_XFP_MAC_RST]		= 28,
	[EN7581_USB_HOST_P1_RST]	= 29,
	[EN7581_USB_HOST_P1_U3_PHY_RST]	= 30,
	/* RST_CTRL1 */
	[EN7581_PCM1_ZSI_ISI_RST]	= RST_NR_PER_BANK + 0,
	[EN7581_FE_PDMA_RST]		= RST_NR_PER_BANK + 1,
	[EN7581_FE_QDMA_RST]		= RST_NR_PER_BANK + 2,
	[EN7581_PCM_SPIWP_RST]		= RST_NR_PER_BANK + 4,
	[EN7581_CRYPTO_RST]		= RST_NR_PER_BANK + 6,
	[EN7581_TIMER_RST]		= RST_NR_PER_BANK + 8,
	[EN7581_PCM1_RST]		= RST_NR_PER_BANK + 11,
	[EN7581_UART_RST]		= RST_NR_PER_BANK + 12,
	[EN7581_GPIO_RST]		= RST_NR_PER_BANK + 13,
	[EN7581_GDMA_RST]		= RST_NR_PER_BANK + 14,
	[EN7581_I2C_MASTER_RST]		= RST_NR_PER_BANK + 16,
	[EN7581_PCM2_ZSI_ISI_RST]	= RST_NR_PER_BANK + 17,
	[EN7581_SFC_RST]		= RST_NR_PER_BANK + 18,
	[EN7581_UART2_RST]		= RST_NR_PER_BANK + 19,
	[EN7581_GDMP_RST]		= RST_NR_PER_BANK + 20,
	[EN7581_FE_RST]			= RST_NR_PER_BANK + 21,
	[EN7581_USB_HOST_P0_RST]	= RST_NR_PER_BANK + 22,
	[EN7581_GSW_RST]		= RST_NR_PER_BANK + 23,
	[EN7581_SFC2_PCM_RST]		= RST_NR_PER_BANK + 25,
	[EN7581_PCIE0_RST]		= RST_NR_PER_BANK + 26,
	[EN7581_PCIE1_RST]		= RST_NR_PER_BANK + 27,
	[EN7581_CPU_TIMER_RST]		= RST_NR_PER_BANK + 28,
	[EN7581_PCIE_HB_RST]		= RST_NR_PER_BANK + 29,
	[EN7581_XPON_MAC_RST]		= RST_NR_PER_BANK + 31,
};

static int airoha_reset_update(struct airoha_reset_priv *priv,
			       unsigned long id, bool assert)
{
	void __iomem *addr = priv->base + priv->bank_ofs[id / RST_NR_PER_BANK];
	u32 val;

	val = readl(addr);
	if (assert)
		val |= BIT(id % RST_NR_PER_BANK);
	else
		val &= ~BIT(id % RST_NR_PER_BANK);
	writel(val, addr);

	return 0;
}

static int airoha_reset_assert(struct reset_ctl *reset_ctl)
{
	struct airoha_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;

	return airoha_reset_update(priv, id, true);
}

static int airoha_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct airoha_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;

	return airoha_reset_update(priv, id, false);
}

static int airoha_reset_status(struct reset_ctl *reset_ctl)
{
	struct airoha_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;
	void __iomem *addr;

	addr = priv->base + priv->bank_ofs[id / RST_NR_PER_BANK];

	return !!(readl(addr) & BIT(id % RST_NR_PER_BANK));
}

static int airoha_reset_xlate(struct reset_ctl *reset_ctl,
			      struct ofnode_phandle_args *args)
{
	struct airoha_reset_priv *priv = dev_get_priv(reset_ctl->dev);

	if (args->args[0] >= ARRAY_SIZE(en7581_rst_map))
		return -EINVAL;

	reset_ctl->id = priv->idx_map[args->args[0]];

	return 0;
}

static struct reset_ops airoha_reset_ops = {
	.of_xlate = airoha_reset_xlate,
	.rst_assert = airoha_reset_assert,
	.rst_deassert = airoha_reset_deassert,
	.rst_status = airoha_reset_status,
};

static int airoha_reset_probe(struct udevice *dev)
{
	struct airoha_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -ENOMEM;

	priv->bank_ofs = en7581_rst_ofs;
	priv->idx_map = en7581_rst_map;

	return 0;
}

U_BOOT_DRIVER(airoha_reset) = {
	.name = "airoha-reset",
	.id = UCLASS_RESET,
	.probe = airoha_reset_probe,
	.ops = &airoha_reset_ops,
	.priv_auto = sizeof(struct airoha_reset_priv),
};
