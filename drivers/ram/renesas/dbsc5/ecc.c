// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <linux/sizes.h>

#define DBSC5_DBSC_CNT			8

#define DBSC_D_BASE(n)			(0xe9900000 + ((n) * 0x4000))
#define DBSC_A_BASE(n)			(0xe9800000 + ((n) * 0x8000))
#define DBSYSCNT			0x100
#define DBACEN				0x200
#define DBFSINTENB02A			0x7088
#define DBFSINTENB04A			0x7090
#define DBFSCONFAXI0			0x7400
#define DBFSDRAMECCAREA00		0x7450
#define DBFSCTRL01A			0x7604
#define DBFSCONF00A			0x7640
#define DBFSCONF01A			0x7644
#define DBFSCONF02A			0x7648
#define DBFSSTAT00A			0x7680
#define DBFSSTAT01A			0x7684

#define DBSYSCNT_ENABLE			0x1234
#define DBSYSCNT_DISABLE		0x0
#define DBACEN_ACCESS_DISABLE		0
#define DBACEN_ACCESS_ENABLE		1

#define ECM_BASE			0xb89a0000
#define ECMERRCTLR0			(ECM_BASE + 0x0)
#define ECMERRINCR0			(ECM_BASE + 0x200)
#define ECMERROMKR0			(ECM_BASE + 0x600)
#define ECMERRCTLR6			(ECM_BASE + (0x4 * 6))
#define ECMERRINCR6			(ECM_BASE + 0x200U + (0x4 * 6))
#define ECMERROMKR6			(ECM_BASE + 0x600U + (0x4 * 6))
#define ECMWPCNTR			(ECM_BASE + 0xa00)
#define ECMWACNTR			(ECM_BASE + 0xa04)

struct renesas_dbsc5_ecc_priv {
	void __iomem		*regs;
};

static void ecm_reg_unlock(void)
{
	writel(0xacce0001, ECMWPCNTR);
}

static void ecm_reg_lock(void)
{
	writel(0xacce0000, ECMWPCNTR);
}

static void ecm_reg_write(u32 adr, u32 val)
{
	writel(0xacce0000 | (adr & 0xffff), ECMWACNTR);
	writel(val, adr);
}

static int renesas_dbsc5_ecc_probe(struct udevice *dev)
{
	int i;

	ecm_reg_unlock();

	for (i = 0; i < DBSC5_DBSC_CNT; i++) {
		writel(DBSYSCNT_ENABLE, DBSC_D_BASE(i) + DBSYSCNT);
		writel(DBSYSCNT_ENABLE, DBSC_A_BASE(i) + DBSYSCNT);
	}

	ecm_reg_write(ECMERRINCR0, readl(ECMERRINCR0) | 0xAAA);
	ecm_reg_write(ECMERROMKR0, readl(ECMERROMKR0) | 0xAAA);
	ecm_reg_write(ECMERRCTLR0, readl(ECMERRCTLR0) | 0xAAA);
	ecm_reg_write(ECMERRINCR6, readl(ECMERRINCR6) | 0xA000);
	ecm_reg_write(ECMERROMKR6, readl(ECMERROMKR6) | 0xA000);
	ecm_reg_write(ECMERRCTLR6, readl(ECMERRCTLR6) | 0xA000);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0xcccc, DBSC_A_BASE(i) + DBFSDRAMECCAREA00);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(DBACEN_ACCESS_DISABLE, DBSC_A_BASE(i) + DBACEN);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0, DBSC_A_BASE(i) + DBFSCONF00A);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0, DBSC_A_BASE(i) + DBFSCONF01A);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0xcccb, DBSC_A_BASE(i) + DBFSCONF02A);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0x1, DBSC_A_BASE(i) + DBFSCTRL01A);

	u32 fsstat;
	do {
		fsstat = 0x1;
		for (i = 0; i < DBSC5_DBSC_CNT; i++)
			fsstat &= readl(DBSC_A_BASE(i) + DBFSSTAT01A);
	} while (!(fsstat & 0x1));

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(DBACEN_ACCESS_ENABLE, DBSC_A_BASE(i) + DBACEN);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		setbits_le32(DBSC_A_BASE(i) + DBFSCONFAXI0, 0x100);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0xff00ff00, DBSC_A_BASE(i) + DBFSINTENB02A);

	for (i = 0; i < DBSC5_DBSC_CNT; i++)
		writel(0xffffffff, DBSC_A_BASE(i) + DBFSINTENB04A);

	ecm_reg_lock();

	for (i = 0; i < DBSC5_DBSC_CNT; i++) {
		writel(DBSYSCNT_DISABLE, DBSC_D_BASE(i) + DBSYSCNT);
		writel(DBSYSCNT_DISABLE, DBSC_A_BASE(i) + DBSYSCNT);
	}

	return 0;
}

static int renesas_dbsc5_ecc_of_to_plat(struct udevice *dev)
{
	struct renesas_dbsc5_ecc_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static int renesas_dbsc5_ecc_get_info(struct udevice *dev,
				      struct ram_info *info)
{
	struct renesas_dbsc5_ecc_priv *priv = dev_get_priv(dev);

	info->base = (phys_addr_t)priv->regs;
	info->size = 32 * SZ_1M;

	return 0;
}

static const struct ram_ops renesas_dbsc5_ecc_ops = {
	.get_info = renesas_dbsc5_ecc_get_info,
};

static const struct udevice_id renesas_dbsc5_ecc_ids[] = {
	{ .compatible = "renesas,r8a78000-ecc" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_dbsc5_ecc) = {
	.name		= "dbsc5_ecc",
	.id		= UCLASS_RAM,
	.of_match	= renesas_dbsc5_ecc_ids,
	.of_to_plat	= renesas_dbsc5_ecc_of_to_plat,
	.ops		= &renesas_dbsc5_ecc_ops,
	.probe		= renesas_dbsc5_ecc_probe,
	.priv_auto	= sizeof(struct renesas_dbsc5_ecc_priv),
};
