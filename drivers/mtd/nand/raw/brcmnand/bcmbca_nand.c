// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <memalign.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm.h>
#include <linux/printk.h>

#include "brcmnand.h"

struct bcmbca_nand_soc {
	struct brcmnand_soc soc;
	void __iomem *base;
};

#define BCMBCA_NAND_INT			0x00
#define BCMBCA_NAND_STATUS_SHIFT	0
#define BCMBCA_NAND_STATUS_MASK		(0xfff << BCMBCA_NAND_STATUS_SHIFT)

#define BCMBCA_NAND_INT_EN		0x04
#define BCMBCA_NAND_ENABLE_SHIFT	0
#define BCMBCA_NAND_ENABLE_MASK		(0xffff << BCMBCA_NAND_ENABLE_SHIFT)

enum {
	BCMBCA_NP_READ		= BIT(0),
	BCMBCA_BLOCK_ERASE	= BIT(1),
	BCMBCA_COPY_BACK	= BIT(2),
	BCMBCA_PAGE_PGM		= BIT(3),
	BCMBCA_CTRL_READY	= BIT(4),
	BCMBCA_DEV_RBPIN	= BIT(5),
	BCMBCA_ECC_ERR_UNC	= BIT(6),
	BCMBCA_ECC_ERR_CORR	= BIT(7),
};

#if defined(CONFIG_ARM64)
#define ALIGN_REQ		8
#else
#define ALIGN_REQ		4
#endif

static inline bool bcmbca_nand_is_buf_aligned(void *flash_cache,  void *buffer)
{
	return IS_ALIGNED((uintptr_t)buffer, ALIGN_REQ) &&
				IS_ALIGNED((uintptr_t)flash_cache, ALIGN_REQ);
}

static bool bcmbca_nand_intc_ack(struct brcmnand_soc *soc)
{
	struct bcmbca_nand_soc *priv =
			container_of(soc, struct bcmbca_nand_soc, soc);
	void __iomem *mmio = priv->base + BCMBCA_NAND_INT;
	u32 val = brcmnand_readl(mmio);

	if (val & (BCMBCA_CTRL_READY << BCMBCA_NAND_STATUS_SHIFT)) {
		/* Ack interrupt */
		val &= ~BCMBCA_NAND_STATUS_MASK;
		val |= BCMBCA_CTRL_READY << BCMBCA_NAND_STATUS_SHIFT;
		brcmnand_writel(val, mmio);
		return true;
	}

	return false;
}

static void bcmbca_nand_intc_set(struct brcmnand_soc *soc, bool en)
{
	struct bcmbca_nand_soc *priv =
			container_of(soc, struct bcmbca_nand_soc, soc);
	void __iomem *mmio = priv->base + BCMBCA_NAND_INT_EN;
	u32 val = brcmnand_readl(mmio);

	/* Don't ack any interrupts */
	val &= ~BCMBCA_NAND_STATUS_MASK;

	if (en)
		val |= BCMBCA_CTRL_READY << BCMBCA_NAND_ENABLE_SHIFT;
	else
		val &= ~(BCMBCA_CTRL_READY << BCMBCA_NAND_ENABLE_SHIFT);

	brcmnand_writel(val, mmio);
}

static void bcmbca_read_data_bus(struct brcmnand_soc *soc,
				 void __iomem *flash_cache,  u32 *buffer, int fc_words)
{
	/*
	 * memcpy can do unaligned aligned access depending on source
	 * and dest address, which is incompatible with nand cache. Fallback
	 * to the memcpy_fromio in such case
	 */
	if (bcmbca_nand_is_buf_aligned((void __force *)flash_cache, buffer))
		memcpy((void *)buffer, (void __force *)flash_cache, fc_words * 4);
	else
		memcpy_fromio((void *)buffer, flash_cache, fc_words * 4);
}

static int bcmbca_nand_probe(struct udevice *dev)
{
	struct udevice *pdev = dev;
	struct bcmbca_nand_soc *priv = dev_get_priv(dev);
	struct brcmnand_soc *soc;
	struct resource res;

	soc = &priv->soc;

	dev_read_resource_byname(pdev, "nand-int-base", &res);
	priv->base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	soc->ctlrdy_ack = bcmbca_nand_intc_ack;
	soc->ctlrdy_set_enabled = bcmbca_nand_intc_set;
	soc->read_data_bus = bcmbca_read_data_bus;

	/* Disable and ack all interrupts  */
	brcmnand_writel(0, priv->base + BCMBCA_NAND_INT_EN);
	brcmnand_writel(0, priv->base + BCMBCA_NAND_INT);

	return brcmnand_probe(pdev, soc);
}

static const struct udevice_id bcmbca_nand_dt_ids[] = {
	{
		.compatible = "brcm,nand-bcm63138",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcmbca_nand) = {
	.name = "bcmbca-nand",
	.id = UCLASS_MTD,
	.of_match = bcmbca_nand_dt_ids,
	.probe = bcmbca_nand_probe,
	.priv_auto	= sizeof(struct bcmbca_nand_soc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(bcmbca_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name,
		       ret);
}
