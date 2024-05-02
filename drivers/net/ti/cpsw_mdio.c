// SPDX-License-Identifier: GPL-2.0+
/*
 * CPSW MDIO generic driver for TI AMxx/K2x/EMAC devices.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <clk.h>
#include <dm/device_compat.h>
#include <log.h>
#include <malloc.h>
#include <phy.h>
#include <asm/io.h>
#include <miiphy.h>
#include <soc.h>
#include <wait_bit.h>
#include <linux/bitops.h>
#include <linux/delay.h>

struct cpsw_mdio_regs {
	u32	version;
	u32	control;
#define CONTROL_IDLE		BIT(31)
#define CONTROL_ENABLE		BIT(30)
#define CONTROL_FAULT		BIT(19)
#define CONTROL_FAULT_ENABLE	BIT(18)
#define CONTROL_DIV_MASK	GENMASK(15, 0)
#define CONTROL_MAX_DIV		CONTROL_DIV_MASK

#define MDIO_MAN_MDCLK_O        BIT(2)
#define MDIO_MAN_OE             BIT(1)
#define MDIO_MAN_PIN            BIT(0)
#define MDIO_MANUALMODE         BIT(31)

	u32	alive;
	u32	link;
	u32	linkintraw;
	u32	linkintmasked;
	u32	__reserved_0[2];
	u32	userintraw;
	u32	userintmasked;
	u32	userintmaskset;
	u32	userintmaskclr;
	u32	manualif;
	u32	poll;
	u32	__reserved_1[18];

	struct {
		u32		access;
		u32		physel;
#define USERACCESS_GO		BIT(31)
#define USERACCESS_WRITE	BIT(30)
#define USERACCESS_ACK		BIT(29)
#define USERACCESS_READ		(0)
#define USERACCESS_PHY_REG_SHIFT	(21)
#define USERACCESS_PHY_ADDR_SHIFT	(16)
#define USERACCESS_DATA		GENMASK(15, 0)
	} user[2];
};

#define CPSW_MDIO_DIV_DEF	0xff
#define PHY_REG_MASK		0x1f
#define PHY_ID_MASK		0x1f

#define MDIO_BITRANGE           0x8000
#define C22_READ_PATTERN        0x6
#define C22_WRITE_PATTERN       0x5
#define C22_BITRANGE            0x8
#define PHY_BITRANGE            0x10
#define PHY_DATA_BITRANGE       0x8000

/*
 * This timeout definition is a worst-case ultra defensive measure against
 * unexpected controller lock ups.  Ideally, we should never ever hit this
 * scenario in practice.
 */
#define CPSW_MDIO_TIMEOUT            100 /* msecs */

#define CPSW_MDIO_DEF_BUS_FREQ		2200000 /* 2.2 MHz */

enum cpsw_mdio_manual {
	MDIO_PIN = 0,
	MDIO_OE,
	MDIO_MDCLK,
};

struct cpsw_mdio {
	struct cpsw_mdio_regs *regs;
	struct mii_dev *bus;
	int div;
	bool manual_mode;
	struct clk clk;
	unsigned long bus_freq;
};

static int cpsw_mdio_enable(struct cpsw_mdio *data)
{
	int ret;

	/* set enable and clock divider */
	writel(data->div | CONTROL_ENABLE, &data->regs->control);
	ret = wait_for_bit_le32(&data->regs->control,
				CONTROL_IDLE, false, CPSW_MDIO_TIMEOUT, true);
	if (ret)
		return ret;

	/*
	 * wait for scan logic to settle:
	 * the scan time consists of (a) a large fixed component, and (b) a
	 * small component that varies with the mii bus frequency.  These
	 * were estimated using measurements at 1.1 and 2.2 MHz on tnetv107x
	 * silicon.  Since the effect of (b) was found to be largely
	 * negligible, we keep things simple here.
	 */
	mdelay(1);

	return 0;
}

static void cpsw_mdio_disable(struct cpsw_mdio *mdio)
{
	u32 reg;
	/* Disable MDIO state machine */
	reg = readl(&mdio->regs->control);
	reg &= ~CONTROL_ENABLE;

	writel(reg, &mdio->regs->control);
}

static void cpsw_mdio_enable_manual_mode(struct cpsw_mdio *mdio)
{
	u32 reg;

	/* set manual mode */
	reg = readl(&mdio->regs->poll);
	reg |= MDIO_MANUALMODE;

	writel(reg, &mdio->regs->poll);
}

static void cpsw_mdio_sw_set_bit(struct cpsw_mdio *mdio,
				 enum cpsw_mdio_manual bit)
{
	u32 reg;

	reg = readl(&mdio->regs->manualif);

	switch (bit) {
	case MDIO_OE:
		reg |= MDIO_MAN_OE;
		writel(reg, &mdio->regs->manualif);
		break;
	case MDIO_PIN:
		reg |= MDIO_MAN_PIN;
		writel(reg, &mdio->regs->manualif);
		break;
	case MDIO_MDCLK:
		reg |= MDIO_MAN_MDCLK_O;
		writel(reg, &mdio->regs->manualif);
		break;
	default:
		break;
	};
}

static void cpsw_mdio_sw_clr_bit(struct cpsw_mdio *mdio,
				 enum cpsw_mdio_manual bit)
{
	u32 reg;

	reg = readl(&mdio->regs->manualif);

	switch (bit) {
	case MDIO_OE:
		reg &= ~MDIO_MAN_OE;
		writel(reg, &mdio->regs->manualif);
		break;
	case MDIO_PIN:
		reg &= ~MDIO_MAN_PIN;
		writel(reg, &mdio->regs->manualif);
		break;
	case MDIO_MDCLK:
		reg = readl(&mdio->regs->manualif);
		reg &= ~MDIO_MAN_MDCLK_O;
		writel(reg, &mdio->regs->manualif);
		break;
	default:
		break;
	};
}

static int cpsw_mdio_test_man_bit(struct cpsw_mdio *mdio,
				  enum cpsw_mdio_manual bit)
{
	u32 reg;

	reg = readl(&mdio->regs->manualif);
	return test_bit(bit, &reg);
}

static void cpsw_mdio_toggle_man_bit(struct cpsw_mdio *mdio,
				     enum cpsw_mdio_manual bit)
{
	cpsw_mdio_sw_clr_bit(mdio, bit);
	cpsw_mdio_sw_set_bit(mdio, bit);
}

static void cpsw_mdio_man_send_pattern(struct cpsw_mdio *mdio,
				       u32 bitrange, u32 val)
{
	u32 i;

	for (i = bitrange; i; i = i >> 1) {
		if (i & val)
			cpsw_mdio_sw_set_bit(mdio, MDIO_PIN);
		else
			cpsw_mdio_sw_clr_bit(mdio, MDIO_PIN);

		cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);
	}
}

static void cpsw_mdio_sw_preamble(struct cpsw_mdio *mdio)
{
	u32 i;

	cpsw_mdio_sw_clr_bit(mdio, MDIO_OE);

	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_set_bit(mdio, MDIO_MDCLK);

	for (i = 0; i < 32; i++) {
		cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
		cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
		cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
		cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);
	}
}

#if defined(CONFIG_DM_MDIO)
#define MII_TO_CPSW_MDIO(bus)	(dev_get_priv((struct udevice *)(bus)->priv))
#else
#define MII_TO_CPSW_MDIO(bus)	((bus)->priv)
#endif

static int cpsw_mdio_sw_read(struct mii_dev *bus, int phy_id,
			     int dev_addr, int phy_reg)
{
	struct cpsw_mdio *mdio = MII_TO_CPSW_MDIO(bus);
	u32 reg, i;
	u8 ack;

	if (phy_reg & ~PHY_REG_MASK || phy_id & ~PHY_ID_MASK)
		return -EINVAL;

	cpsw_mdio_disable(mdio);
	cpsw_mdio_enable_manual_mode(mdio);
	cpsw_mdio_sw_preamble(mdio);

	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_set_bit(mdio, MDIO_OE);

	/* Issue clause 22 MII read function {0,1,1,0} */
	cpsw_mdio_man_send_pattern(mdio, C22_BITRANGE, C22_READ_PATTERN);

	/* Send the device number MSB first */
	cpsw_mdio_man_send_pattern(mdio, PHY_BITRANGE, phy_id);

	/* Send the register number MSB first */
	cpsw_mdio_man_send_pattern(mdio, PHY_BITRANGE, phy_reg);

	/* Send turn around cycles */
	cpsw_mdio_sw_clr_bit(mdio, MDIO_OE);

	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

	ack = cpsw_mdio_test_man_bit(mdio, MDIO_PIN);
	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

	reg = 0;
	if (ack == 0) {
		for (i = MDIO_BITRANGE; i; i = i >> 1) {
			if (cpsw_mdio_test_man_bit(mdio, MDIO_PIN))
				reg |= i;

			cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);
		}
	} else {
		for (i = MDIO_BITRANGE; i; i = i >> 1)
			cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

		reg = 0xFFFF;
	}

	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_set_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_set_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

	return reg;
}

static int cpsw_mdio_sw_write(struct mii_dev *bus, int phy_id,
			      int dev_addr, int phy_reg, u16 phy_data)
{
	struct cpsw_mdio *mdio = MII_TO_CPSW_MDIO(bus);

	if ((phy_reg & ~PHY_REG_MASK) || (phy_id & ~PHY_ID_MASK))
		return -EINVAL;

	cpsw_mdio_disable(mdio);
	cpsw_mdio_enable_manual_mode(mdio);
	cpsw_mdio_sw_preamble(mdio);

	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_set_bit(mdio, MDIO_OE);

	/* Issue clause 22 MII write function {0,1,0,1} */
	cpsw_mdio_man_send_pattern(mdio, C22_BITRANGE, C22_WRITE_PATTERN);

	/* Send the device number MSB first */
	cpsw_mdio_man_send_pattern(mdio, PHY_BITRANGE, phy_id);

	/* Send the register number MSB first */
	cpsw_mdio_man_send_pattern(mdio, PHY_BITRANGE, phy_reg);

	/* set turn-around cycles */
	cpsw_mdio_sw_set_bit(mdio, MDIO_PIN);
	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_PIN);
	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

	/* Send Register data MSB first */
	cpsw_mdio_man_send_pattern(mdio, PHY_DATA_BITRANGE, phy_data);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_OE);

	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_sw_clr_bit(mdio, MDIO_MDCLK);
	cpsw_mdio_toggle_man_bit(mdio, MDIO_MDCLK);

	return 0;
}

/* wait until hardware is ready for another user access */
static int cpsw_mdio_wait_for_user_access(struct cpsw_mdio *mdio)
{
	return wait_for_bit_le32(&mdio->regs->user[0].access,
				 USERACCESS_GO, false,
				 CPSW_MDIO_TIMEOUT, false);
}

static int cpsw_mdio_read(struct mii_dev *bus, int phy_id,
			  int dev_addr, int phy_reg)
{
	struct cpsw_mdio *mdio = MII_TO_CPSW_MDIO(bus);
	int data, ret;
	u32 reg;

	if (phy_reg & ~PHY_REG_MASK || phy_id & ~PHY_ID_MASK)
		return -EINVAL;

	ret = cpsw_mdio_wait_for_user_access(mdio);
	if (ret)
		return ret;
	reg = (USERACCESS_GO | USERACCESS_READ |
	       (phy_reg << USERACCESS_PHY_REG_SHIFT) |
	       (phy_id << USERACCESS_PHY_ADDR_SHIFT));
	writel(reg, &mdio->regs->user[0].access);
	ret = cpsw_mdio_wait_for_user_access(mdio);
	if (ret)
		return ret;

	reg = readl(&mdio->regs->user[0].access);
	data = (reg & USERACCESS_ACK) ? (reg & USERACCESS_DATA) : -1;
	return data;
}

static int cpsw_mdio_write(struct mii_dev *bus, int phy_id, int dev_addr,
			   int phy_reg, u16 data)
{
	struct cpsw_mdio *mdio = MII_TO_CPSW_MDIO(bus);
	u32 reg;
	int ret;

	if (phy_reg & ~PHY_REG_MASK || phy_id & ~PHY_ID_MASK)
		return -EINVAL;

	ret = cpsw_mdio_wait_for_user_access(mdio);
	if (ret)
		return ret;
	reg = (USERACCESS_GO | USERACCESS_WRITE |
	       (phy_reg << USERACCESS_PHY_REG_SHIFT) |
	       (phy_id << USERACCESS_PHY_ADDR_SHIFT) |
	       (data & USERACCESS_DATA));
	writel(reg, &mdio->regs->user[0].access);

	return cpsw_mdio_wait_for_user_access(mdio);
}

#if !defined(CONFIG_MDIO_TI_CPSW)
u32 cpsw_mdio_get_alive(struct mii_dev *bus)
{
	struct cpsw_mdio *mdio = MII_TO_CPSW_MDIO(bus);
	u32 val;

	val = readl(&mdio->regs->alive);
	return val & GENMASK(7, 0);
}

struct mii_dev *cpsw_mdio_init(const char *name, phys_addr_t mdio_base,
			       u32 bus_freq, int fck_freq, bool manual_mode)
{
	struct cpsw_mdio *cpsw_mdio;
	int ret;

	cpsw_mdio = calloc(1, sizeof(*cpsw_mdio));
	if (!cpsw_mdio) {
		debug("failed to alloc cpsw_mdio\n");
		return NULL;
	}

	cpsw_mdio->bus = mdio_alloc();
	if (!cpsw_mdio->bus) {
		debug("failed to alloc mii bus\n");
		free(cpsw_mdio);
		return NULL;
	}

	cpsw_mdio->regs = (struct cpsw_mdio_regs *)(uintptr_t)mdio_base;

	if (!bus_freq || !fck_freq)
		cpsw_mdio->div = CPSW_MDIO_DIV_DEF;
	else
		cpsw_mdio->div = (fck_freq / bus_freq) - 1;
	cpsw_mdio->div &= CONTROL_DIV_MASK;
	ret = cpsw_mdio_enable(cpsw_mdio);
	if (ret) {
		debug("mdio_enable failed: %d\n", ret);
		goto free_bus;
	}

	if (manual_mode) {
		cpsw_mdio->bus->read = cpsw_mdio_sw_read;
		cpsw_mdio->bus->write = cpsw_mdio_sw_write;
	} else {
		cpsw_mdio->bus->read = cpsw_mdio_read;
		cpsw_mdio->bus->write = cpsw_mdio_write;
	}

	cpsw_mdio->bus->priv = cpsw_mdio;
	snprintf(cpsw_mdio->bus->name, sizeof(cpsw_mdio->bus->name), name);

	ret = mdio_register(cpsw_mdio->bus);
	if (ret < 0) {
		debug("failed to register mii bus\n");
		goto free_bus;
	}

	return cpsw_mdio->bus;

free_bus:
	mdio_free(cpsw_mdio->bus);
	free(cpsw_mdio);
	return NULL;
}

void cpsw_mdio_free(struct mii_dev *bus)
{
	struct cpsw_mdio *mdio = bus->priv;
	u32 reg;

	/* disable mdio */
	reg = readl(&mdio->regs->control);
	reg &= ~CONTROL_ENABLE;
	writel(reg, &mdio->regs->control);

	mdio_unregister(bus);
	mdio_free(bus);
	free(mdio);
}

#else

static int cpsw_mdio_init_clk(struct cpsw_mdio *data)
{
	u32 mdio_in, div;

	mdio_in = clk_get_rate(&data->clk);
	div = (mdio_in / data->bus_freq) - 1;
	if (div > CONTROL_MAX_DIV)
		div = CONTROL_MAX_DIV;

	data->div = div;
	return cpsw_mdio_enable(data);
}

static int cpsw_mdio_bus_read(struct udevice *dev, int addr,
			      int devad, int reg)
{
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;
	struct cpsw_mdio *priv = dev_get_priv(dev);

	if (pdata && pdata->mii_bus) {
		if (priv->manual_mode)
			return cpsw_mdio_sw_read(pdata->mii_bus, addr, devad, reg);
		else
			return cpsw_mdio_read(pdata->mii_bus, addr, devad, reg);
	}

	return -1;
}

static int cpsw_mdio_bus_write(struct udevice *dev, int addr,
			       int devad, int reg, u16 val)
{
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;
	struct cpsw_mdio *priv = dev_get_priv(dev);

	if (pdata && pdata->mii_bus) {
		if (priv->manual_mode)
			return cpsw_mdio_sw_write(pdata->mii_bus, addr, devad, reg, val);
		else
			return cpsw_mdio_write(pdata->mii_bus, addr, devad, reg, val);
	}

	return -1;
}

static const struct mdio_ops cpsw_mdio_ops = {
	.read = cpsw_mdio_bus_read,
	.write = cpsw_mdio_bus_write,
};

static const struct soc_attr k3_mdio_soc_data[] = {
	{ .family = "AM62X", .revision = "SR1.0" },
	{ .family = "AM64X", .revision = "SR1.0" },
	{ .family = "AM64X", .revision = "SR2.0" },
	{ .family = "AM65X", .revision = "SR1.0" },
	{ .family = "AM65X", .revision = "SR2.0" },
	{ .family = "J7200", .revision = "SR1.0" },
	{ .family = "J7200", .revision = "SR2.0" },
	{ .family = "J721E", .revision = "SR1.0" },
	{ .family = "J721E", .revision = "SR1.1" },
	{ .family = "J721S2", .revision = "SR1.0" },
	{ /* sentinel */ },
};

static const struct udevice_id cpsw_mdio_ids[] = {
	{ .compatible = "ti,davinci_mdio", },
	{ .compatible = "ti,cpsw-mdio", },
	{ /* sentinel */ },
};

static int cpsw_mdio_probe(struct udevice *dev)
{
	struct cpsw_mdio *priv = dev_get_priv(dev);
	int ret;

	if (!priv) {
		dev_err(dev, "dev_get_priv(dev %p) = NULL\n", dev);
		return -ENOMEM;
	}

	priv->regs = dev_remap_addr(dev);

	if (soc_device_match(k3_mdio_soc_data))
		priv->manual_mode = true;

	ret = clk_get_by_name(dev, "fck", &priv->clk);
	if (ret) {
		dev_err(dev, "failed to get clock %d\n", ret);
		return ret;
	}

	priv->bus_freq = dev_read_u32_default(dev, "bus_freq",
					      CPSW_MDIO_DEF_BUS_FREQ);
	ret = cpsw_mdio_init_clk(priv);
	if (ret) {
		dev_err(dev, "init clock failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static int cpsw_mdio_remove(struct udevice *dev)
{
	struct cpsw_mdio *priv = dev_get_priv(dev);

	cpsw_mdio_disable(priv);

	return 0;
}

U_BOOT_DRIVER(cpsw_mdio) = {
	.name = "cpsw_mdio",
	.id = UCLASS_MDIO,
	.of_match = cpsw_mdio_ids,
	.probe = cpsw_mdio_probe,
	.remove = cpsw_mdio_remove,
	.ops = &cpsw_mdio_ops,
	.priv_auto = sizeof(struct cpsw_mdio),
};
#endif /* CONFIG_MDIO_TI_CPSW */
