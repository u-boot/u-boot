// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for the IMX SNVS ON/OFF Power Key
 * Copyright (C) 2015 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2026 Siddharth Menon <siddharth.menon@machinesoul.in>
 */

#include <button.h>
#include <dm.h>
#include <dt-bindings/input/linux-event-codes.h>
#include <log.h>
#include <regmap.h>
#include <syscon.h>
#include <time.h>
#include <linux/bitops.h>
#include <linux/err.h>

#define SNVS_HPVIDR1_REG	0xBF8
#define SNVS_LPSR_REG		0x4C	// LP Status Register
#define SNVS_LPCR_REG		0x38	// LP Control Register
#define SNVS_HPSR_REG		0x14
#define SNVS_HPSR_BTN		BIT(6)
#define SNVS_LPSR_SPO		BIT(18)
#define SNVS_LPCR_DEP_EN	BIT(5)
#define SNVS_LPCR_BPT_SHIFT	16
#define SNVS_LPCR_BPT_MASK	(3 << SNVS_LPCR_BPT_SHIFT)

#define DEBOUNCE_TIME		30000	// microseconds

struct pwrkey_drv_data {
	struct regmap *snvs;
	int keycode;
	ulong last_update_time;
	bool old_state;
	bool keystate;
	u8 minor_rev;
	bool click_pending;
};

/*
 * Only report a new event once it has been held for
 * DEBOUNCE_TIME, on both the press and release edge.
 */
static enum button_state_t imx_snvs_pwrkey_poll_state(struct pwrkey_drv_data *pdata)
{
	bool pressed;
	uint state;

	if (regmap_read(pdata->snvs, SNVS_HPSR_REG, &state))
		return pdata->keystate ? BUTTON_ON : BUTTON_OFF;

	pressed = !!(state & SNVS_HPSR_BTN);

	if (pressed != pdata->old_state) {
		pdata->old_state = pressed;
		pdata->last_update_time = get_timer_us(0);
	} else if (get_timer_us(0) - pdata->last_update_time >= DEBOUNCE_TIME) {
		pdata->keystate = pressed;
	}

	return pdata->keystate ? BUTTON_ON : BUTTON_OFF;
}

static enum button_state_t imx_snvs_pwrkey_get_state(struct udevice *dev)
{
	struct pwrkey_drv_data *pdata = dev_get_priv(dev);

	return imx_snvs_pwrkey_poll_state(pdata);
}

static int imx_snvs_pwrkey_get_code(struct udevice *dev)
{
	struct pwrkey_drv_data *pdata = dev_get_priv(dev);

	return pdata->keycode;
}

static int imx_snvs_pwrkey_bind(struct udevice *dev)
{
	struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->label = "Power Button";

	return 0;
}

static int imx_snvs_pwrkey_probe(struct udevice *dev)
{
	struct pwrkey_drv_data *pdata = dev_get_priv(dev);
	unsigned int val;
	unsigned int bpt;
	unsigned int vid;
	int error;

	pdata->snvs = syscon_regmap_lookup_by_phandle(dev, "regmap");
	if (IS_ERR(pdata->snvs)) {
		log_err("%s: can't get snvs syscon\n", dev->name);
		return PTR_ERR(pdata->snvs);
	}

	pdata->keycode = dev_read_u32_default(dev, "linux,keycode", KEY_POWER);

	error = dev_read_u32(dev, "power-off-time-sec", &val);
	if (!error) {
		switch (val) {
		case 0:
			bpt = 0x3;
			break;
		case 5:
		case 10:
		case 15:
			bpt = (val / 5) - 1;
			break;
		default:
			log_err("%s: power-off-time-sec %u out of range\n",
				dev->name, val);
			return -EINVAL;
		}

		regmap_update_bits(pdata->snvs, SNVS_LPCR_REG,
				   SNVS_LPCR_BPT_MASK,
				   bpt << SNVS_LPCR_BPT_SHIFT);
	}

	regmap_read(pdata->snvs, SNVS_HPVIDR1_REG, &vid);

	regmap_update_bits(pdata->snvs, SNVS_LPCR_REG, SNVS_LPCR_DEP_EN,
			   SNVS_LPCR_DEP_EN);

	/* clear the unexpected interrupt before driver ready */
	regmap_write(pdata->snvs, SNVS_LPSR_REG, SNVS_LPSR_SPO);

	return 0;
}

static const struct button_ops imx_snvs_pwrkey_ops = {
	.get_state	= imx_snvs_pwrkey_get_state,
	.get_code	= imx_snvs_pwrkey_get_code,
};

static const struct udevice_id imx_snvs_pwrkey_ids[] = {
	{ .compatible = "fsl,sec-v4.0-pwrkey" },
	{ }
};

U_BOOT_DRIVER(imx_snvs_pwrkey) = {
	.name		= "snvs_pwrkey",
	.id		= UCLASS_BUTTON,
	.of_match	= imx_snvs_pwrkey_ids,
	.bind		= imx_snvs_pwrkey_bind,
	.probe		= imx_snvs_pwrkey_probe,
	.ops		= &imx_snvs_pwrkey_ops,
	.priv_auto	= sizeof(struct pwrkey_drv_data),
};
