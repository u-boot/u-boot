// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Renesas UFS host controller driver
 *
 * Copyright (C) 2022 Renesas Electronics Corporation
 */

#include <clk.h>
#include <dm.h>
#include <ufs.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/bug.h>
#include <linux/iopoll.h>

#include "ufs.h"

struct ufs_renesas_priv {
	struct clk_bulk clks;
	bool initialized;	/* The hardware needs initialization once */
};

enum {
	SET_PHY_INDEX_LO = 0,
	SET_PHY_INDEX_HI,
	TIMER_INDEX,
	MAX_INDEX
};

enum ufs_renesas_init_param_mode {
	MODE_RESTORE,
	MODE_SET,
	MODE_SAVE,
	MODE_POLL,
	MODE_WAIT,
	MODE_WRITE,
};

struct ufs_renesas_init_param {
	enum ufs_renesas_init_param_mode mode;
	u32 reg;
	union {
		u32 expected;
		u32 delay_us;
		u32 set;
		u32 val;
	} u;
	u32 mask;
	u32 index;
};

static void ufs_renesas_reg_control(struct ufs_hba *hba,
				    const struct ufs_renesas_init_param *p)
{
	static u32 save[MAX_INDEX];
	int ret;
	u32 val;

	WARN_ON(p->index >= MAX_INDEX);

	switch (p->mode) {
	case MODE_RESTORE:
		ufshcd_writel(hba, save[p->index], p->reg);
		break;
	case MODE_SET:
		save[p->index] |= p->u.set;
		break;
	case MODE_SAVE:
		save[p->index] = ufshcd_readl(hba, p->reg) & p->mask;
		break;
	case MODE_POLL:
		ret = readl_poll_timeout(hba->mmio_base + p->reg, val,
					 (val & p->mask) == p->u.expected,
					 10000);
		if (ret)
			dev_err(hba->dev, "%s: poll failed %d (%08x, %08x, %08x)\n",
				__func__, ret, val, p->mask, p->u.expected);
		break;
	case MODE_WAIT:
		if (p->u.delay_us > 1000)
			mdelay(DIV_ROUND_UP(p->u.delay_us, 1000));
		else
			udelay(p->u.delay_us);
		break;
	case MODE_WRITE:
		ufshcd_writel(hba, p->u.val, p->reg);
		break;
	default:
		break;
	}
}

static void ufs_renesas_poll(struct ufs_hba *hba, u32 reg, u32 expected, u32 mask)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_POLL,
		.reg = reg,
		.u.expected = expected,
		.mask = mask,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_restore(struct ufs_hba *hba, u32 reg, u32 index)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_RESTORE,
		.reg = reg,
		.index = index,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_save(struct ufs_hba *hba, u32 reg, u32 mask, u32 index)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_SAVE,
		.reg = reg,
		.mask = mask,
		.index = index,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_set(struct ufs_hba *hba, u32 index, u32 set)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_SAVE,
		.index = index,
		.u.set = set,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_wait(struct ufs_hba *hba, u32 delay_us)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_WAIT,
		.u.delay_us = delay_us,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_write(struct ufs_hba *hba, u32 reg, u32 value)
{
	struct ufs_renesas_init_param param = {
		.mode = MODE_WRITE,
		.reg = reg,
		.u.val = value,
	};

	ufs_renesas_reg_control(hba, &param);
}

static void ufs_renesas_write_d0_d4(struct ufs_hba *hba, u32 data_d0, u32 data_d4)
{
	ufs_renesas_write(hba, 0xd0, data_d0);
	ufs_renesas_write(hba, 0xd4, data_d4);
}

static void ufs_renesas_write_800_80c_poll(struct ufs_hba *hba, u32 addr,
					   u32 data_800)
{
	ufs_renesas_write_d0_d4(hba, 0x0000080c, 0x00000100);
	ufs_renesas_write_d0_d4(hba, 0x00000800, (data_800 << 16) | BIT(8) | addr);
	ufs_renesas_write(hba, 0xd0, 0x0000080c);
	ufs_renesas_poll(hba, 0xd4, BIT(8), BIT(8));
}

static void ufs_renesas_restore_800_80c_poll(struct ufs_hba *hba, u32 index)
{
	ufs_renesas_write_d0_d4(hba, 0x0000080c, 0x00000100);
	ufs_renesas_write(hba, 0xd0, 0x00000800);
	ufs_renesas_restore(hba, 0xd4, index);
	ufs_renesas_write(hba, 0xd0, 0x0000080c);
	ufs_renesas_poll(hba, 0xd4, BIT(8), BIT(8));
}

static void ufs_renesas_write_804_80c_poll(struct ufs_hba *hba, u32 addr, u32 data_804)
{
	ufs_renesas_write_d0_d4(hba, 0x0000080c, 0x00000100);
	ufs_renesas_write_d0_d4(hba, 0x00000804, (data_804 << 16) | BIT(8) | addr);
	ufs_renesas_write(hba, 0xd0, 0x0000080c);
	ufs_renesas_poll(hba, 0xd4, BIT(8), BIT(8));
}

static void ufs_renesas_write_828_82c_poll(struct ufs_hba *hba, u32 data_828)
{
	ufs_renesas_write_d0_d4(hba, 0x0000082c, 0x0f000000);
	ufs_renesas_write_d0_d4(hba, 0x00000828, data_828);
	ufs_renesas_write(hba, 0xd0, 0x0000082c);
	ufs_renesas_poll(hba, 0xd4, data_828, data_828);
}

static void ufs_renesas_write_phy(struct ufs_hba *hba, u32 addr16, u32 data16)
{
	ufs_renesas_write(hba, 0xf0, 1);
	ufs_renesas_write_800_80c_poll(hba, 0x16, addr16 & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x17, (addr16 >> 8) & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x18, data16 & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x19, (data16 >> 8) & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x1c, 0x01);
	ufs_renesas_write_828_82c_poll(hba, 0x0f000000);
	ufs_renesas_write(hba, 0xf0, 0);
}

static void ufs_renesas_set_phy(struct ufs_hba *hba, u32 addr16, u32 data16)
{
	ufs_renesas_write(hba, 0xf0, 1);
	ufs_renesas_write_800_80c_poll(hba, 0x16, addr16 & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x17, (addr16 >> 8) & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x1c, 0x01);
	ufs_renesas_write_828_82c_poll(hba, 0x0f000000);
	ufs_renesas_write_804_80c_poll(hba, 0x1a, 0);
	ufs_renesas_write(hba, 0xd0, 0x00000808);
	ufs_renesas_save(hba, 0xd4, 0xff, SET_PHY_INDEX_LO);
	ufs_renesas_write_804_80c_poll(hba, 0x1b, 0);
	ufs_renesas_write(hba, 0xd0, 0x00000808);
	ufs_renesas_save(hba, 0xd4, 0xff, SET_PHY_INDEX_HI);
	ufs_renesas_write_828_82c_poll(hba, 0x0f000000);
	ufs_renesas_write(hba, 0xf0, 0);
	ufs_renesas_write(hba, 0xf0, 1);
	ufs_renesas_write_800_80c_poll(hba, 0x16, addr16 & 0xff);
	ufs_renesas_write_800_80c_poll(hba, 0x17, (addr16 >> 8) & 0xff);
	ufs_renesas_set(hba, SET_PHY_INDEX_LO, ((data16 & 0xff) << 16) | BIT(8) | 0x18);
	ufs_renesas_restore_800_80c_poll(hba, SET_PHY_INDEX_LO);
	ufs_renesas_set(hba, SET_PHY_INDEX_HI, (((data16 >> 8) & 0xff) << 16) | BIT(8) | 0x19);
	ufs_renesas_restore_800_80c_poll(hba, SET_PHY_INDEX_HI);
	ufs_renesas_write_800_80c_poll(hba, 0x1c, 0x01);
	ufs_renesas_write_828_82c_poll(hba, 0x0f000000);
	ufs_renesas_write(hba, 0xf0, 0);
}

static void ufs_renesas_indirect_write(struct ufs_hba *hba, u32 gpio, u32 addr,
				       u32 data_800)
{
	ufs_renesas_write(hba, 0xf0, gpio);
	ufs_renesas_write_800_80c_poll(hba, addr, data_800);
	ufs_renesas_write_828_82c_poll(hba, 0x0f000000);
	ufs_renesas_write(hba, 0xf0, 0);
}

static void ufs_renesas_indirect_poll(struct ufs_hba *hba, u32 gpio, u32 addr,
				      u32 expected, u32 mask)
{
	ufs_renesas_write(hba, 0xf0, gpio);
	ufs_renesas_write_800_80c_poll(hba, addr, 0);
	ufs_renesas_write(hba, 0xd0, 0x00000808);
	ufs_renesas_poll(hba, 0xd4, expected, mask);
	ufs_renesas_write(hba, 0xf0, 0);
}

static void ufs_renesas_pre_init(struct ufs_hba *hba)
{
	/* This setting is for SERIES B */
	ufs_renesas_write(hba, 0xc0, 0x49425308);
	ufs_renesas_write_d0_d4(hba, 0x00000104, 0x00000002);
	ufs_renesas_wait(hba, 1);
	ufs_renesas_write_d0_d4(hba, 0x00000828, 0x00000200);
	ufs_renesas_wait(hba, 1);
	ufs_renesas_write_d0_d4(hba, 0x00000828, 0x00000000);
	ufs_renesas_write_d0_d4(hba, 0x00000104, 0x00000001);
	ufs_renesas_write_d0_d4(hba, 0x00000940, 0x00000001);
	ufs_renesas_wait(hba, 1);
	ufs_renesas_write_d0_d4(hba, 0x00000940, 0x00000000);

	ufs_renesas_write(hba, 0xc0, 0x49425308);
	ufs_renesas_write(hba, 0xc0, 0x41584901);

	ufs_renesas_write_d0_d4(hba, 0x0000080c, 0x00000100);
	ufs_renesas_write_d0_d4(hba, 0x00000804, 0x00000000);
	ufs_renesas_write(hba, 0xd0, 0x0000080c);
	ufs_renesas_poll(hba, 0xd4, BIT(8), BIT(8));

	ufs_renesas_write(hba, REG_CONTROLLER_ENABLE, 0x00000001);

	ufs_renesas_write(hba, 0xd0, 0x00000804);
	ufs_renesas_poll(hba, 0xd4, BIT(8) | BIT(6) | BIT(0), BIT(8) | BIT(6) | BIT(0));

	ufs_renesas_write(hba, 0xd0, 0x00000d00);
	ufs_renesas_save(hba, 0xd4, 0x0000ffff, TIMER_INDEX);
	ufs_renesas_write(hba, 0xd4, 0x00000000);
	ufs_renesas_write_d0_d4(hba, 0x0000082c, 0x0f000000);
	ufs_renesas_write_d0_d4(hba, 0x00000828, 0x08000000);
	ufs_renesas_write(hba, 0xd0, 0x0000082c);
	ufs_renesas_poll(hba, 0xd4, BIT(27), BIT(27));
	ufs_renesas_write(hba, 0xd0, 0x00000d2c);
	ufs_renesas_poll(hba, 0xd4, BIT(0), BIT(0));

	/* phy setup */
	ufs_renesas_indirect_write(hba, 1, 0x01, 0x001f);
	ufs_renesas_indirect_write(hba, 7, 0x5d, 0x0014);
	ufs_renesas_indirect_write(hba, 7, 0x5e, 0x0014);
	ufs_renesas_indirect_write(hba, 7, 0x0d, 0x0003);
	ufs_renesas_indirect_write(hba, 7, 0x0e, 0x0007);
	ufs_renesas_indirect_write(hba, 7, 0x5f, 0x0003);
	ufs_renesas_indirect_write(hba, 7, 0x60, 0x0003);
	ufs_renesas_indirect_write(hba, 7, 0x5b, 0x00a6);
	ufs_renesas_indirect_write(hba, 7, 0x5c, 0x0003);

	ufs_renesas_indirect_poll(hba, 7, 0x3c, 0, BIT(7));
	ufs_renesas_indirect_poll(hba, 7, 0x4c, 0, BIT(4));

	ufs_renesas_indirect_write(hba, 1, 0x32, 0x0080);
	ufs_renesas_indirect_write(hba, 1, 0x1f, 0x0001);
	ufs_renesas_indirect_write(hba, 0, 0x2c, 0x0001);
	ufs_renesas_indirect_write(hba, 0, 0x32, 0x0087);

	ufs_renesas_indirect_write(hba, 1, 0x4d, 0x0061);
	ufs_renesas_indirect_write(hba, 4, 0x9b, 0x0009);
	ufs_renesas_indirect_write(hba, 4, 0xa6, 0x0005);
	ufs_renesas_indirect_write(hba, 4, 0xa5, 0x0058);
	ufs_renesas_indirect_write(hba, 1, 0x39, 0x0027);
	ufs_renesas_indirect_write(hba, 1, 0x47, 0x004c);

	ufs_renesas_indirect_write(hba, 7, 0x0d, 0x0002);
	ufs_renesas_indirect_write(hba, 7, 0x0e, 0x0007);

	ufs_renesas_write_phy(hba, 0x0028, 0x0061);
	ufs_renesas_write_phy(hba, 0x4014, 0x0061);
	ufs_renesas_set_phy(hba, 0x401c, BIT(2));
	ufs_renesas_write_phy(hba, 0x4000, 0x0000);
	ufs_renesas_write_phy(hba, 0x4001, 0x0000);

	ufs_renesas_write_phy(hba, 0x10ae, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ad, 0x0000);
	ufs_renesas_write_phy(hba, 0x10af, 0x0001);
	ufs_renesas_write_phy(hba, 0x10b6, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ae, 0x0000);

	ufs_renesas_write_phy(hba, 0x10ae, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ad, 0x0000);
	ufs_renesas_write_phy(hba, 0x10af, 0x0002);
	ufs_renesas_write_phy(hba, 0x10b6, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ae, 0x0000);

	ufs_renesas_write_phy(hba, 0x10ae, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ad, 0x0080);
	ufs_renesas_write_phy(hba, 0x10af, 0x0000);
	ufs_renesas_write_phy(hba, 0x10b6, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ae, 0x0000);

	ufs_renesas_write_phy(hba, 0x10ae, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ad, 0x0080);
	ufs_renesas_write_phy(hba, 0x10af, 0x001a);
	ufs_renesas_write_phy(hba, 0x10b6, 0x0001);
	ufs_renesas_write_phy(hba, 0x10ae, 0x0000);

	ufs_renesas_indirect_write(hba, 7, 0x70, 0x0016);
	ufs_renesas_indirect_write(hba, 7, 0x71, 0x0016);
	ufs_renesas_indirect_write(hba, 7, 0x72, 0x0014);
	ufs_renesas_indirect_write(hba, 7, 0x73, 0x0014);
	ufs_renesas_indirect_write(hba, 7, 0x74, 0x0000);
	ufs_renesas_indirect_write(hba, 7, 0x75, 0x0000);
	ufs_renesas_indirect_write(hba, 7, 0x76, 0x0010);
	ufs_renesas_indirect_write(hba, 7, 0x77, 0x0010);
	ufs_renesas_indirect_write(hba, 7, 0x78, 0x00ff);
	ufs_renesas_indirect_write(hba, 7, 0x79, 0x0000);

	ufs_renesas_indirect_write(hba, 7, 0x19, 0x0007);
	ufs_renesas_indirect_write(hba, 7, 0x1a, 0x0007);
	ufs_renesas_indirect_write(hba, 7, 0x24, 0x000c);
	ufs_renesas_indirect_write(hba, 7, 0x25, 0x000c);
	ufs_renesas_indirect_write(hba, 7, 0x62, 0x0000);
	ufs_renesas_indirect_write(hba, 7, 0x63, 0x0000);
	ufs_renesas_indirect_write(hba, 7, 0x5d, 0x0014);
	ufs_renesas_indirect_write(hba, 7, 0x5e, 0x0017);
	ufs_renesas_indirect_write(hba, 7, 0x5d, 0x0004);
	ufs_renesas_indirect_write(hba, 7, 0x5e, 0x0017);
	ufs_renesas_indirect_poll(hba, 7, 0x55, 0, BIT(6));
	ufs_renesas_indirect_poll(hba, 7, 0x41, 0, BIT(7));
	/* end of phy setup */

	ufs_renesas_write(hba, 0xf0, 0);
	ufs_renesas_write(hba, 0xd0, 0x00000d00);
	ufs_renesas_restore(hba, 0xd4, TIMER_INDEX);
}

static int ufs_renesas_hce_enable_notify(struct ufs_hba *hba,
					 enum ufs_notify_change_status status)
{
	struct ufs_renesas_priv *priv = dev_get_priv(hba->dev);

	if (priv->initialized)
		return 0;

	if (status == PRE_CHANGE)
		ufs_renesas_pre_init(hba);

	priv->initialized = true;

	return 0;
}

static int ufs_renesas_init(struct ufs_hba *hba)
{
	hba->quirks |= UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS | UFSHCD_QUIRK_HIBERN_FASTAUTO;

	return 0;
}

static struct ufs_hba_ops ufs_renesas_vops = {
	.init		= ufs_renesas_init,
	.hce_enable_notify = ufs_renesas_hce_enable_notify,
};

static int ufs_renesas_pltfm_probe(struct udevice *dev)
{
	struct ufs_renesas_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_get_bulk(dev, &priv->clks);
	if (err < 0)
		return err;

	err = clk_enable_bulk(&priv->clks);
	if (err)
		goto err_clk_enable;

	err = ufshcd_probe(dev, &ufs_renesas_vops);
	if (err) {
		dev_err(dev, "ufshcd_probe() failed %d\n", err);
		goto err_ufshcd_probe;
	}

	return 0;

err_ufshcd_probe:
	clk_disable_bulk(&priv->clks);
err_clk_enable:
	clk_release_bulk(&priv->clks);
	return err;
}

static int ufs_renesas_pltfm_remove(struct udevice *dev)
{
	struct ufs_renesas_priv *priv = dev_get_priv(dev);

	clk_disable_bulk(&priv->clks);
	clk_release_bulk(&priv->clks);

	return 0;
}

static const struct udevice_id ufs_renesas_pltfm_ids[] = {
	{ .compatible = "renesas,r8a779f0-ufs" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ufs_renesas) = {
	.name		= "ufs-renesas",
	.id		= UCLASS_UFS,
	.of_match	= ufs_renesas_pltfm_ids,
	.probe		= ufs_renesas_pltfm_probe,
	.remove		= ufs_renesas_pltfm_remove,
	.priv_auto	= sizeof(struct ufs_renesas_priv),
};
