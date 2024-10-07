// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <asm/gpio.h>
#include <linux/io.h>

#define MAX_NR_HW_SGPIO		64
#define	NPCM_CLK_MHZ		8000000

#define  NPCM_IOXCFG1		0x2A

#define	NPCM_IOXCTS			0x28
#define	NPCM_IOXCTS_IOXIF_EN		BIT(7)
#define	NPCM_IOXCTS_RD_MODE			GENMASK(2, 1)
#define	NPCM_IOXCTS_RD_MODE_PERIODIC	BIT(2)

#define	NPCM_IOXCFG2		0x2B
#define	NPCM_IOXCFG2_PORT	GENMASK(3, 0)

#define GPIO_BANK(x)    ((x) / 8)
#define GPIO_BIT(x)     ((x) % 8)

struct npcm_sgpio_priv {
	void __iomem *base;
	u32 nin_sgpio;
	u32 nout_sgpio;
	u32 in_port;
	u32 out_port;
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
	return npcm_sgpio_direction_output(dev, offset, value);
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

static int npcm_sgpio_init_port(struct udevice *dev)
{
	struct npcm_sgpio_priv *priv = dev_get_priv(dev);
	u8 in_port, out_port, set_port, reg, set_clk;

	npcm_sgpio_setup_enable(priv, false);

	in_port = GPIO_BANK(priv->nin_sgpio);
	if (GPIO_BIT(priv->nin_sgpio) > 0)
		in_port += 1;

	out_port = GPIO_BANK(priv->nout_sgpio);
	if (GPIO_BIT(priv->nout_sgpio) > 0)
		out_port += 1;

	priv->in_port = in_port;
	priv->out_port = out_port;

	set_port = (out_port & NPCM_IOXCFG2_PORT) << 4 | (in_port & NPCM_IOXCFG2_PORT);
	set_clk = 0x07;

	iowrite8(set_port, priv->base + NPCM_IOXCFG2);
	iowrite8(set_clk, priv->base + NPCM_IOXCFG1);

	reg = ioread8(priv->base + NPCM_IOXCFG2);

	return reg == set_port ? 0 : -EINVAL;
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
	int rc;

	priv->base = dev_read_addr_ptr(dev);
	ofnode_read_u32(dev_ofnode(dev), "nuvoton,input-ngpios", &priv->nin_sgpio);
	ofnode_read_u32(dev_ofnode(dev), "nuvoton,output-ngpios", &priv->nout_sgpio);

	if (priv->nin_sgpio > MAX_NR_HW_SGPIO || priv->nout_sgpio > MAX_NR_HW_SGPIO)
		return -EINVAL;

	rc = npcm_sgpio_init_port(dev);
	if (rc < 0)
		return rc;

	uc_priv->gpio_count = priv->nin_sgpio + priv->nout_sgpio;
	uc_priv->bank_name = dev->name;

	npcm_sgpio_setup_enable(priv, true);

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
