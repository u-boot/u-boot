// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <linux/err.h>
#include <linux/io.h>
#include <asm/arch/rst.h>

#define MAX_NR_HW_SGPIO		64
#define NPCM_SIOX1			24
#define NPCM_SIOX2			25

#define	NPCM_IOXCTS						0x28
#define	NPCM_IOXCTS_IOXIF_EN			BIT(7)
#define	NPCM_IOXCTS_RD_MODE				GENMASK(2, 1)
#define	NPCM_IOXCTS_RD_MODE_PERIODIC	BIT(2)

#define NPCM_IOXCFG1		0x2A
#define	NPCM_IOXCFG2		0x2B
#define	NPCM_IOXCFG2_PORT	GENMASK(3, 0)

#define GPIO_BANK(x)    ((x) / 8)
#define GPIO_BIT(x)     ((x) % 8)

#define WD0RCR          0x38
#define WD1RCR          0x3c
#define WD2RCR          0x40
#define SWRSTC1         0x44
#define SWRSTC2         0x48
#define SWRSTC3         0x4c
#define TIPRSTC         0x50
#define CORSTC          0x5c

struct npcm_sgpio_priv {
	void __iomem *base;
	struct regmap *rst_regmap;
	u32 nin_sgpio;
	u32 nout_sgpio;
	u32 in_port;
	u32 out_port;
	u8 persist[8];
	u8 siox_num;
};

struct npcm_sgpio_bank {
	u8 rdata_reg;
	u8 wdata_reg;
	u8 event_config;
	u8 event_status;
};

enum npcm_sgpio_reg {
	READ_DATA,
	WRITE_DATA,
	EVENT_CFG,
	EVENT_STS,
};

static const struct npcm_sgpio_bank npcm_sgpio_banks[] = {
	{
		.wdata_reg = 0x00,
		.rdata_reg = 0x08,
		.event_config = 0x10,
		.event_status = 0x20,
	},
	{
		.wdata_reg = 0x01,
		.rdata_reg = 0x09,
		.event_config = 0x12,
		.event_status = 0x21,
	},
	{
		.wdata_reg = 0x02,
		.rdata_reg = 0x0a,
		.event_config = 0x14,
		.event_status = 0x22,
	},
	{
		.wdata_reg = 0x03,
		.rdata_reg = 0x0b,
		.event_config = 0x16,
		.event_status = 0x23,
	},
	{
		.wdata_reg = 0x04,
		.rdata_reg = 0x0c,
		.event_config = 0x18,
		.event_status = 0x24,
	},
	{
		.wdata_reg = 0x05,
		.rdata_reg = 0x0d,
		.event_config = 0x1a,
		.event_status = 0x25,
	},
	{
		.wdata_reg = 0x06,
		.rdata_reg = 0x0e,
		.event_config = 0x1c,
		.event_status = 0x26,
	},
	{
		.wdata_reg = 0x07,
		.rdata_reg = 0x0f,
		.event_config = 0x1e,
		.event_status = 0x27,
	},
};

static void __iomem *bank_reg(struct npcm_sgpio_priv *gpio,
			      const struct npcm_sgpio_bank *bank,
			      const enum npcm_sgpio_reg reg)
{
	switch (reg) {
	case READ_DATA:
		return gpio->base + bank->rdata_reg;
	case WRITE_DATA:
		return gpio->base + bank->wdata_reg;
	case EVENT_CFG:
		return gpio->base + bank->event_config;
	case EVENT_STS:
		return gpio->base + bank->event_status;
	default:
		/* actually if code runs to here, it's an error case */
		printf("Getting here is an error condition\n");
		return NULL;
	}
}

static const struct npcm_sgpio_bank *offset_to_bank(unsigned int offset)
{
	unsigned int bank = GPIO_BANK(offset);

	return &npcm_sgpio_banks[bank];
}

static int npcm_sgpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);

	if (offset < priv->nout_sgpio) {
		printf("Error: Offset %d is a output pin\n", offset);
		return -EINVAL;
	}

	return 0;
}

static int npcm_sgpio_direction_output(struct udevice *dev, unsigned int offset,
				       int value)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	const struct  npcm_sgpio_bank *bank = offset_to_bank(offset);
	void __iomem *addr;
	u8 reg = 0;

	if (offset >= priv->nout_sgpio) {
		printf("Error: Offset %d is a input pin\n", offset);
		return -EINVAL;
	}

	addr = bank_reg(priv, bank, WRITE_DATA);
	reg = ioread8(addr);

	if (value)
		reg |= BIT(GPIO_BIT(offset));
	else
		reg &= ~BIT(GPIO_BIT(offset));

	iowrite8(reg, addr);

	return 0;
}

static int npcm_sgpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	const struct  npcm_sgpio_bank *bank;
	void __iomem *addr;
	u8 reg;

	if (offset < priv->nout_sgpio) {
		bank = offset_to_bank(offset);
		addr = bank_reg(priv, bank, WRITE_DATA);
	} else {
		offset -= priv->nout_sgpio;
		bank = offset_to_bank(offset);
		addr = bank_reg(priv, bank, READ_DATA);
	}

	reg = ioread8(addr);

	return !!(reg & BIT(GPIO_BIT(offset)));
}

static int npcm_sgpio_set_value(struct udevice *dev, unsigned int offset,
				int value)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u8 check = priv->persist[GPIO_BANK(offset)];

	if (!!(check & BIT(GPIO_BIT(offset))) == 0)
		return npcm_sgpio_direction_output(dev, offset, value);
	else
		return -EINVAL;
}

static int npcm_sgpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);

	if (offset < priv->nout_sgpio)
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static void npcm_sgpio_setup_enable(struct npcm_sgpio_priv *gpio, bool enable)
{
	u8 reg;

	reg = ioread8(gpio->base + NPCM_IOXCTS);
	reg = (reg & ~NPCM_IOXCTS_RD_MODE) | NPCM_IOXCTS_RD_MODE_PERIODIC;

	if (enable)
		reg |= NPCM_IOXCTS_IOXIF_EN;
	else
		reg &= ~NPCM_IOXCTS_IOXIF_EN;

	iowrite8(reg, gpio->base + NPCM_IOXCTS);
}

static void npcm_sgpio_set_port(struct udevice *dev)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u8 in_port, out_port;

	in_port = GPIO_BANK(priv->nin_sgpio);
	if (GPIO_BIT(priv->nin_sgpio) > 0)
		in_port += 1;

	out_port = GPIO_BANK(priv->nout_sgpio);
	if (GPIO_BIT(priv->nout_sgpio) > 0)
		out_port += 1;

	priv->in_port = in_port;
	priv->out_port = out_port;
}

static int npcm_sgpio_init_port(struct udevice *dev)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u8 set_port, reg, set_clk;

	npcm_sgpio_setup_enable(priv, false);

	set_port = (priv->out_port & NPCM_IOXCFG2_PORT) << 4 | (priv->in_port & NPCM_IOXCFG2_PORT);
	set_clk = 0x07;

	iowrite8(set_port, priv->base + NPCM_IOXCFG2);
	iowrite8(set_clk, priv->base + NPCM_IOXCFG1);

	reg = ioread8(priv->base + NPCM_IOXCFG2);

	return reg == set_port ? 0 : -EINVAL;
}

static void npcm_sgpio_reset_persist(struct udevice *dev, uint enable)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u8 num;

	if (priv->siox_num == 1)
		num = NPCM_SIOX2;
	else
		num = NPCM_SIOX1;

	if (enable) {
		regmap_update_bits(priv->rst_regmap, WD0RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, WD1RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, WD2RCR, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, CORSTC, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC1, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC2, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, SWRSTC3, BIT(num), 0);
		regmap_update_bits(priv->rst_regmap, TIPRSTC, BIT(num), 0);
	}
}

static bool is_gpio_persist(struct udevice *dev)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u32 val;
	int status;

	status = npcm_get_reset_status();

	if (status & PORST)
		return false;
	if (status & CORST)
		regmap_read(priv->rst_regmap, CORSTC, &val);
	else if (status & WD0RST)
		regmap_read(priv->rst_regmap, WD0RCR, &val);
	else if (status & WD1RST)
		regmap_read(priv->rst_regmap, WD1RCR, &val);
	else if (status & WD2RST)
		regmap_read(priv->rst_regmap, WD2RCR, &val);
	else if (status & SW1RST)
		regmap_read(priv->rst_regmap, SWRSTC1, &val);
	else if (status & SW2RST)
		regmap_read(priv->rst_regmap, SWRSTC2, &val);
	else if (status & SW3RST)
		regmap_read(priv->rst_regmap, SWRSTC3, &val);
	else if (status & TIPRST)
		regmap_read(priv->rst_regmap, TIPRSTC, &val);

	if (priv->siox_num == 1)
		return (val && BIT(NPCM_SIOX2));
	else
		return (val && BIT(NPCM_SIOX1));
}

static const struct dm_gpio_ops npcm_sgpio_ops = {
	.direction_input	= npcm_sgpio_direction_input,
	.direction_output	= npcm_sgpio_direction_output,
	.get_value		= npcm_sgpio_get_value,
	.set_value		= npcm_sgpio_set_value,
	.get_function		= npcm_sgpio_get_function,
};

static int npcm_sgpio_probe(struct udevice *dev)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int rc, i;
	ofnode node;
	u32 val[2];

	priv->base = dev_read_addr_ptr(dev);
	priv->rst_regmap = syscon_regmap_lookup_by_phandle(dev, "syscon-rst");
	if (IS_ERR(priv->rst_regmap))
		return -EINVAL;

	ofnode_read_u32(dev_ofnode(dev), "nuvoton,input-ngpios", &priv->nin_sgpio);
	ofnode_read_u32(dev_ofnode(dev), "nuvoton,output-ngpios", &priv->nout_sgpio);

	if (priv->nin_sgpio > MAX_NR_HW_SGPIO || priv->nout_sgpio > MAX_NR_HW_SGPIO)
		return -EINVAL;

	if (!strcmp(ofnode_get_name(dev_ofnode(dev)), "sgpio2@102000"))
		priv->siox_num = 1;
	else if (!strcmp(ofnode_get_name(dev_ofnode(dev)), "sgpio1@101000"))
		priv->siox_num = 0;
	else
		return -EINVAL;

	npcm_sgpio_set_port(dev);
	uc_priv->gpio_count = priv->nin_sgpio + priv->nout_sgpio;
	uc_priv->bank_name = dev->name;

	if (is_gpio_persist(dev)) {
		ofnode_for_each_subnode(node, dev_ofnode(dev)) {
			if (ofnode_read_bool(node, "persist-enable")) {
				rc = ofnode_read_u32_array(node, "gpios", val, 2);
				if (rc == 0)
					priv->persist[GPIO_BANK(val[0])] = priv->persist[GPIO_BANK(val[0])] | BIT(GPIO_BIT(val[0]));
			}
		}
		for (i = 0; i < priv->nout_sgpio; i++)
			npcm_sgpio_set_value(dev, i, 0);
	} else {
		rc = npcm_sgpio_init_port(dev);
		if (rc < 0)
			return rc;

		ofnode_for_each_subnode(node, dev_ofnode(dev)) {
			if (ofnode_read_bool(node, "persist-enable"))
				npcm_sgpio_reset_persist(dev, 1);
		}

		for (i = 0; i < priv->nout_sgpio; i++)
			npcm_sgpio_set_value(dev, i, 0);

		npcm_sgpio_setup_enable(priv, true);
	}

	return 0;
}

static const struct udevice_id npcm_sgpio_match[] = {
	{ .compatible = "nuvoton,npcm845-sgpio" },
	{ .compatible = "nuvoton,npcm750-sgpio" },
	{ }
};

U_BOOT_DRIVER(npcm_sgpio) = {
	.name	= "npcm_sgpio",
	.id	= UCLASS_GPIO,
	.of_match = npcm_sgpio_match,
	.probe	= npcm_sgpio_probe,
	.priv_auto = sizeof(struct npcm_sgpio_priv),
	.ops	= &npcm_sgpio_ops,
};
