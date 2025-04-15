// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <stdlib.h>
#include <dm.h>
#include <input.h>
#include <keyboard.h>
#include <power/pmic.h>
#include <power/cpcap.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/input.h>

static const unsigned int cpcpap_to_reg[] = {
	CPCAP_REG_INT1,
	CPCAP_REG_INT2,
	CPCAP_REG_INT3,
	CPCAP_REG_INT4,
};

/**
 * struct cpcap_pwrbutton_priv
 *
 * @bank: id of interrupt bank co-responding to an IRQ register
 * @id: id of interrupt pin co-responding to the bit in IRQ register
 * @keycode: linux key code
 * @old_state: holder of last button state
 * @skip: holder of keycode skip state. This is required since both pressing
 *        and releasing generate same event and cause key send duplication.
 */
struct cpcap_pwrbutton_priv {
	u32 bank;
	u32 id;

	u32 keycode;

	bool old_state;
	bool skip;
};

static int cpcap_pwrbutton_read_keys(struct input_config *input)
{
	struct udevice *dev = input->dev;
	struct cpcap_pwrbutton_priv *priv = dev_get_priv(dev);
	u32 value, state_changed;
	bool state;

	value = pmic_reg_read(dev->parent, cpcpap_to_reg[priv->bank]) &
			      BIT(priv->id);

	/* Interrupt bit is cleared by writing it to interrupt reg */
	pmic_reg_write(dev->parent, cpcpap_to_reg[priv->bank], BIT(priv->id));

	state = value >> priv->id;
	state_changed = state != priv->old_state;

	if (state_changed && !priv->skip) {
		priv->old_state = state;
		input_add_keycode(input, priv->keycode, state);
	}

	if (state)
		priv->skip = !priv->skip;

	return 0;
}

static int cpcap_pwrbutton_of_to_plat(struct udevice *dev)
{
	struct cpcap_pwrbutton_priv *priv = dev_get_priv(dev);
	ofnode irq_parent;
	u32 irq_desc;
	int ret;

	/* Check interrupt parent, driver supports only CPCAP as parent */
	irq_parent = ofnode_parse_phandle(dev_ofnode(dev), "interrupt-parent", 0);
	if (!ofnode_device_is_compatible(irq_parent, "motorola,cpcap"))
		return -EINVAL;

	ret = dev_read_u32(dev, "interrupts", &irq_desc);
	if (ret)
		return ret;

	/* IRQ registers are 16 bit wide */
	priv->bank = irq_desc / 16;
	priv->id = irq_desc % 16;

	ret = dev_read_u32(dev, "linux,code", &priv->keycode);
	if (ret)
		return ret;

	priv->old_state = false;
	priv->skip = false;
	return 0;
}

static int cpcap_pwrbutton_probe(struct udevice *dev)
{
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	input_init(input, false);
	input_add_tables(input, false);

	/* Register the device */
	input->dev = dev;
	input->read_keys = cpcap_pwrbutton_read_keys;
	strcpy(sdev->name, "cpcap-pwrbutton");
	ret = input_stdio_register(sdev);
	if (ret) {
		log_debug("%s: input_stdio_register() failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct udevice_id cpcap_pwrbutton_ids[] = {
	{ .compatible = "motorola,cpcap-pwrbutton" },
	{ }
};

U_BOOT_DRIVER(cpcap_pwrbutton) = {
	.name		= "cpcap_pwrbutton",
	.id		= UCLASS_KEYBOARD,
	.of_match	= cpcap_pwrbutton_ids,
	.of_to_plat	= cpcap_pwrbutton_of_to_plat,
	.probe		= cpcap_pwrbutton_probe,
	.priv_auto	= sizeof(struct cpcap_pwrbutton_priv),
};
