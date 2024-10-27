// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020
 * Tim Harvey, Gateworks Corporation
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <eth_phy.h>
#include <linux/delay.h>
#include <miiphy.h>
#if CONFIG_IS_ENABLED(DM_I2C)
# include <i2c.h>
#endif
#if CONFIG_IS_ENABLED(DM_SPI)
# include <spi.h>
#endif
#include <net/dsa.h>

#include <asm-generic/gpio.h>

/* Used with variable features to indicate capabilities. */
#define NEW_XMII			BIT(1)
#define IS_9893				BIT(2)

/* Global registers */

/* Chip ID */
#define REG_CHIP_ID0__1			0x0000

/* Operation control */
#define REG_SW_OPERATION		0x0300
#define SW_RESET			BIT(1)
#define SW_START			BIT(0)

/* Port Specific Registers */
#define PORT_CTRL_ADDR(port, addr) ((addr) | (((port) + 1) << 12))

/* Port Control */
#define REG_PORT_XMII_CTRL_1		0x0301
#define PORT_MII_NOT_1GBIT		BIT(6)
#define PORT_MII_SEL_EDGE		BIT(5)
#define PORT_RGMII_ID_IG_ENABLE		BIT(4)
#define PORT_RGMII_ID_EG_ENABLE		BIT(3)
#define PORT_MII_MAC_MODE		BIT(2)
#define PORT_MII_SEL_M			0x3
#define PORT_RGMII_SEL			0x0
#define PORT_RMII_SEL			0x1
#define PORT_GMII_SEL			0x2
#define PORT_MII_SEL			0x3
/* S1 */
#define PORT_MII_1000MBIT_S1		BIT(6)
/* S1 */
#define PORT_MII_SEL_S1			0x0
#define PORT_RMII_SEL_S1		0x1
#define PORT_GMII_SEL_S1		0x2
#define PORT_RGMII_SEL_S1		0x3

/* Port MSTP State Register */
#define REG_PORT_MSTP_STATE		0x0b04
#define PORT_TX_ENABLE			BIT(2)
#define PORT_RX_ENABLE			BIT(1)
#define PORT_LEARN_DISABLE		BIT(0)

/* MMD */
#define REG_PORT_PHY_MMD_SETUP		0x011A
#define PORT_MMD_OP_MODE_M		0x3
#define PORT_MMD_OP_MODE_S		14
#define PORT_MMD_OP_INDEX		0
#define PORT_MMD_OP_DATA_NO_INCR	1
#define PORT_MMD_OP_DATA_INCR_RW	2
#define PORT_MMD_OP_DATA_INCR_W		3
#define PORT_MMD_DEVICE_ID_M		0x1F
#define MMD_SETUP(mode, dev)		(((u16)(mode) << PORT_MMD_OP_MODE_S) | (dev))
#define REG_PORT_PHY_MMD_INDEX_DATA	0x011C

/* SPI specific define (opcodes) */
#define KSZ_SPI_OP_RD			3
#define KSZ_SPI_OP_WR			2

#define KSZ9477_SPI_ADDR_SHIFT		24
#define KSZ9477_SPI_ADDR_ALIGN		3
#define KSZ9477_SPI_TURNAROUND_SHIFT	5

/**
 * struct ksz_phy_ops - low-level KSZ bus operations
 */
struct ksz_phy_ops {
	/* read() - Read bytes from the device
	 *
	 * @udev: bus device
	 * @reg:  register offset
	 * @val:  data read
	 * @len:  Number of bytes to read
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*read)(struct udevice *udev, u32 reg, u8 *val, int len);

	/* write() - Write bytes to the device
	 *
	 * @udev: bus device
	 * @reg:  register offset
	 * @val:  data to write
	 * @len:  Number of bytes to write
	 *
	 * @return: 0 on success, negative on failure
	 */
	int (*write)(struct udevice *udev, u32 reg, u8 *val, int len);
};

struct ksz_dsa_priv {
	struct udevice *dev;
	struct ksz_phy_ops *phy_ops;

	u32 features;			/* chip specific features */
};

#if CONFIG_IS_ENABLED(DM_I2C)
static inline int ksz_i2c_read(struct udevice *dev, u32 reg, u8 *val, int len)
{
	return dm_i2c_read(dev, reg, val, len);
}

static inline int ksz_i2c_write(struct udevice *dev, u32 reg, u8 *val, int len)
{
	return dm_i2c_write(dev, reg, val, len);
}

static struct ksz_phy_ops phy_i2c_ops = {
	.read = ksz_i2c_read,
	.write = ksz_i2c_write,
};
#endif

#if CONFIG_IS_ENABLED(DM_SPI)
/**
 * ksz_spi_xfer() - only used for 8/16/32 bits bus access
 *
 * @dev:	The SPI slave device which will be sending/receiving the data.
 * @reg:	register address.
 * @out:	Pointer to a string of bits to send out.  The bits are
 *		held in a byte array and are sent MSB first.
 * @in:		Pointer to a string of bits that will be filled in.
 * @len:	number of bytes to read.
 *
 * Return: 0 on success, not 0 on failure
 */
static int ksz_spi_xfer(struct udevice *dev, u32 reg, const u8 *out,
			u8 *in, u16 len)
{
	int ret;
	u32 addr = 0;
	u8 opcode;

	if (in && out) {
		printf("%s: can't do full duplex\n", __func__);
		return -EINVAL;
	}

	if (len > 4 || len == 0) {
		printf("%s: only 8/16/32 bits bus access supported\n",
		       __func__);
		return -EINVAL;
	}

	ret = dm_spi_claim_bus(dev);
	if (ret < 0) {
		printf("%s: could not claim bus\n", __func__);
		return ret;
	}

	opcode = (in ? KSZ_SPI_OP_RD : KSZ_SPI_OP_WR);

	/* The actual device address space is 16 bits (A15 - A0),
	 * so the values of address bits A23 - A16 in the SPI
	 * command/address phase are “don't care”.
	 */
	addr |= opcode << (KSZ9477_SPI_ADDR_SHIFT + KSZ9477_SPI_TURNAROUND_SHIFT);
	addr |= reg << KSZ9477_SPI_TURNAROUND_SHIFT;

	addr = __swab32(addr);

	ret = dm_spi_xfer(dev, 32, &addr, NULL, SPI_XFER_BEGIN);
	if (ret) {
		printf("%s ERROR: dm_spi_xfer addr (%u)\n", __func__, ret);
		goto release_bus;
	}

	ret = dm_spi_xfer(dev, len * 8, out, in, SPI_XFER_END);
	if (ret) {
		printf("%s ERROR: dm_spi_xfer data (%u)\n", __func__, ret);
		goto release_bus;
	}

release_bus:
	/* If an error occurred, release the chip by deasserting the CS */
	if (ret < 0)
		dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_END);

	dm_spi_release_bus(dev);

	return ret;
}

static inline int ksz_spi_read(struct udevice *dev, u32 reg, u8 *val, int len)
{
	return ksz_spi_xfer(dev, reg, NULL, val, len);
}

static inline int ksz_spi_write(struct udevice *dev, u32 reg, u8 *val, int len)
{
	return ksz_spi_xfer(dev, reg, val, NULL, len);
}

static struct ksz_phy_ops phy_spi_ops = {
	.read = ksz_spi_read,
	.write = ksz_spi_write,
};
#endif

static inline int ksz_read8(struct udevice *dev, u32 reg, u8 *val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;

	int ret = phy_ops->read(dev, reg, val, 1);

	dev_dbg(dev, "%s 0x%04x<<0x%02x\n", __func__, reg, *val);

	return ret;
}

static inline int ksz_pread8(struct udevice *dev, int port, int reg, u8 *val)
{
	return ksz_read8(dev, PORT_CTRL_ADDR(port, reg), val);
}

static inline int ksz_write8(struct udevice *dev, u32 reg, u8 val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;

	dev_dbg(dev, "%s 0x%04x>>0x%02x\n", __func__, reg, val);
	return phy_ops->write(dev, reg, &val, 1);
}

static inline int ksz_pwrite8(struct udevice *dev, int port, int reg, u8 val)
{
	return ksz_write8(dev, PORT_CTRL_ADDR(port, reg), val);
}

static inline int ksz_write16(struct udevice *dev, u32 reg, u16 val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;
	u8 buf[2];

	buf[1] = val & 0xff;
	buf[0] = val >> 8;
	dev_dbg(dev, "%s 0x%04x>>0x%04x\n", __func__, reg, val);

	return phy_ops->write(dev, reg, buf, 2);
}

static inline int ksz_pwrite16(struct udevice *dev, int port, int reg, u16 val)
{
	return ksz_write16(dev, PORT_CTRL_ADDR(port, reg), val);
}

static inline int ksz_read16(struct udevice *dev, u32 reg, u16 *val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;
	u8 buf[2];
	int ret;

	ret = phy_ops->read(dev, reg, buf, 2);
	*val = (buf[0] << 8) | buf[1];
	dev_dbg(dev, "%s 0x%04x<<0x%04x\n", __func__, reg, *val);

	return ret;
}

static inline int ksz_pread16(struct udevice *dev, int port, int reg, u16 *val)
{
	return ksz_read16(dev, PORT_CTRL_ADDR(port, reg), val);
}

static inline int ksz_read32(struct udevice *dev, u32 reg, u32 *val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;

	return phy_ops->read(dev, reg, (u8 *)val, 4);
}

static inline int ksz_pread32(struct udevice *dev, int port, int reg, u32 *val)
{
	return ksz_read32(dev, PORT_CTRL_ADDR(port, reg), val);
}

static inline int ksz_write32(struct udevice *dev, u32 reg, u32 val)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	struct ksz_phy_ops *phy_ops = priv->phy_ops;
	u8 buf[4];

	buf[3] = val & 0xff;
	buf[2] = (val >> 24) & 0xff;
	buf[1] = (val >> 16) & 0xff;
	buf[0] = (val >> 8) & 0xff;
	dev_dbg(dev, "%s 0x%04x>>0x%04x\n", __func__, reg, val);

	return phy_ops->write(dev, reg, buf, 4);
}

static inline int ksz_pwrite32(struct udevice *dev, int port, int reg, u32 val)
{
	return ksz_write32(dev, PORT_CTRL_ADDR(port, reg), val);
}

static __maybe_unused void ksz_port_mmd_read(struct udevice *dev, int port,
					     u8 addr, u16 reg, u16 *val)
{
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_SETUP, MMD_SETUP(PORT_MMD_OP_INDEX, addr));
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_INDEX_DATA, reg);
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_SETUP, MMD_SETUP(PORT_MMD_OP_DATA_NO_INCR, addr));
	ksz_pread16(dev, port, REG_PORT_PHY_MMD_INDEX_DATA, val);
	dev_dbg(dev, "%s  P%d 0x%02x:0x%04x<<0x%04x\n", __func__, port + 1, addr, reg, *val);
}

static void ksz_port_mmd_write(struct udevice *dev, int port, u8 addr, u16 reg, u16 val)
{
	dev_dbg(dev, "%s P%d 0x%02x:0x%04x>>0x%04x\n", __func__, port + 1, addr, addr, val);
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_SETUP, MMD_SETUP(PORT_MMD_OP_INDEX, addr));
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_INDEX_DATA, addr);
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_SETUP, MMD_SETUP(PORT_MMD_OP_DATA_NO_INCR, addr));
	ksz_pwrite16(dev, port, REG_PORT_PHY_MMD_INDEX_DATA, val);
}

/* Apply PHY settings to address errata listed in KSZ9477, KSZ9897, KSZ9896, KSZ9567
 * Silicon Errata and Data Sheet Clarification documents
 */
static void ksz_phy_errata_setup(struct udevice *dev, int port)
{
	dev_dbg(dev, "%s P%d\n", __func__, port + 1);

	/* Register settings are needed to improve PHY receive performance */
	ksz_port_mmd_write(dev, port, 0x01, 0x6f, 0xdd0b);
	ksz_port_mmd_write(dev, port, 0x01, 0x8f, 0x6032);
	ksz_port_mmd_write(dev, port, 0x01, 0x9d, 0x248c);
	ksz_port_mmd_write(dev, port, 0x01, 0x75, 0x0060);
	ksz_port_mmd_write(dev, port, 0x01, 0xd3, 0x7777);
	ksz_port_mmd_write(dev, port, 0x1c, 0x06, 0x3008);
	ksz_port_mmd_write(dev, port, 0x1c, 0x08, 0x2001);

	/* Transmit waveform amplitude can be improved (1000BASE-T, 100BASE-TX, 10BASE-Te) */
	ksz_port_mmd_write(dev, port, 0x1c, 0x04, 0x00d0);

	/* Energy Efficient Ethernet (EEE) feature select must be manually disabled */
	ksz_port_mmd_write(dev, port, 0x07, 0x3c, 0x0000);

	/* Register settings are required to meet data sheet supply current specifications */
	ksz_port_mmd_write(dev, port, 0x1c, 0x13, 0x6eff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x14, 0xe6ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x15, 0x6eff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x16, 0xe6ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x17, 0x00ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x18, 0x43ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x19, 0xc3ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x1a, 0x6fff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x1b, 0x07ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x1c, 0x0fff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x1d, 0xe7ff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x1e, 0xefff);
	ksz_port_mmd_write(dev, port, 0x1c, 0x20, 0xeeee);
}

/*
 * mii bus driver
 */
#define KSZ_MDIO_CHILD_DRV_NAME	"ksz_mdio"

struct ksz_mdio_priv {
	struct ksz_dsa_priv *ksz;
};

static int dm_ksz_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct ksz_mdio_priv *priv = dev_get_priv(dev);
	struct ksz_dsa_priv *ksz = priv->ksz;
	u16 val = 0xffff;

	ksz_pread16(ksz->dev, addr, 0x100 + (reg << 1), &val);
	dev_dbg(ksz->dev, "%s P%d reg=0x%04x:0x%04x<<0x%04x\n", __func__,
		addr + 1, reg, 0x100 + (reg << 1), val);

	return val;
};

static int dm_ksz_mdio_write(struct udevice *dev, int addr, int devad, int reg, u16 val)
{
	struct ksz_mdio_priv *priv = dev_get_priv(dev);
	struct ksz_dsa_priv *ksz = priv->ksz;

	dev_dbg(ksz->dev, "%s P%d reg=0x%04x:%04x>>0x%04x\n",
		__func__, addr + 1, reg, 0x100 + (reg << 1), val);
	ksz_pwrite16(ksz->dev, addr, 0x100 + (reg << 1), val);

	return 0;
}

static const struct mdio_ops ksz_mdio_ops = {
	.read = dm_ksz_mdio_read,
	.write = dm_ksz_mdio_write,
};

static int ksz_mdio_bind(struct udevice *dev)
{
	char name[16];
	static int num_devices;

	dev_dbg(dev, "%s\n", __func__);
	sprintf(name, "ksz-mdio-%d", num_devices++);
	device_set_name(dev, name);

	return 0;
}

static int ksz_mdio_probe(struct udevice *dev)
{
	struct ksz_mdio_priv *priv = dev_get_priv(dev);

	dev_dbg(dev, "%s\n", __func__);
	priv->ksz = dev_get_priv(dev->parent);

	return 0;
}

static const struct udevice_id ksz_mdio_ids[] = {
	{ .compatible = "microchip,ksz-mdio" },
	{ }
};

U_BOOT_DRIVER(ksz_mdio) = {
	.name		= KSZ_MDIO_CHILD_DRV_NAME,
	.id		= UCLASS_MDIO,
	.of_match	= ksz_mdio_ids,
	.bind		= ksz_mdio_bind,
	.probe		= ksz_mdio_probe,
	.ops		= &ksz_mdio_ops,
	.priv_auto	= sizeof(struct ksz_mdio_priv),
	.plat_auto	= sizeof(struct mdio_perdev_priv),
};

static void ksz9477_set_gbit(struct ksz_dsa_priv *priv, bool gbit, u8 *data)
{
	if (priv->features & NEW_XMII) {
		if (gbit)
			*data &= ~PORT_MII_NOT_1GBIT;
		else
			*data |= PORT_MII_NOT_1GBIT;
	} else {
		if (gbit)
			*data |= PORT_MII_1000MBIT_S1;
		else
			*data &= ~PORT_MII_1000MBIT_S1;
	}
}

static void ksz9477_set_xmii(struct ksz_dsa_priv *priv, int mode, u8 *data)
{
	u8 xmii;

	if (priv->features & NEW_XMII) {
		switch (mode) {
		case 0:
			xmii = PORT_MII_SEL;
			break;
		case 1:
			xmii = PORT_RMII_SEL;
			break;
		case 2:
			xmii = PORT_GMII_SEL;
			break;
		default:
			xmii = PORT_RGMII_SEL;
			break;
		}
	} else {
		switch (mode) {
		case 0:
			xmii = PORT_MII_SEL_S1;
			break;
		case 1:
			xmii = PORT_RMII_SEL_S1;
			break;
		case 2:
			xmii = PORT_GMII_SEL_S1;
			break;
		default:
			xmii = PORT_RGMII_SEL_S1;
			break;
		}
	}
	*data &= ~PORT_MII_SEL_M;
	*data |= xmii;
}

static int ksz_port_setup(struct udevice *dev, int port,
			  phy_interface_t interface)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(dev);
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	u8 data8;

	dev_dbg(dev, "%s P%d %s\n", __func__, port + 1,
		(port == pdata->cpu_port) ? "cpu" : "");

	if (port != pdata->cpu_port) {
		if (priv->features & NEW_XMII)
			/* phy port: config errata and leds */
			ksz_phy_errata_setup(dev, port);
	} else {
		/* cpu port: configure MAC interface mode */
		ksz_pread8(dev, port, REG_PORT_XMII_CTRL_1, &data8);
		dev_dbg(dev, "%s P%d cpu interface %s\n", __func__, port + 1,
			phy_string_for_interface(interface));
		switch (interface) {
		case PHY_INTERFACE_MODE_MII:
			ksz9477_set_xmii(priv, 0, &data8);
			ksz9477_set_gbit(priv, false, &data8);
			break;
		case PHY_INTERFACE_MODE_RMII:
			ksz9477_set_xmii(priv, 1, &data8);
			ksz9477_set_gbit(priv, false, &data8);
			break;
		case PHY_INTERFACE_MODE_GMII:
			ksz9477_set_xmii(priv, 2, &data8);
			ksz9477_set_gbit(priv, true, &data8);
			break;
		default:
			ksz9477_set_xmii(priv, 3, &data8);
			ksz9477_set_gbit(priv, true, &data8);
			data8 &= ~PORT_RGMII_ID_IG_ENABLE;
			data8 &= ~PORT_RGMII_ID_EG_ENABLE;
			if (interface == PHY_INTERFACE_MODE_RGMII_ID ||
			    interface == PHY_INTERFACE_MODE_RGMII_RXID)
				data8 |= PORT_RGMII_ID_IG_ENABLE;
			if (interface == PHY_INTERFACE_MODE_RGMII_ID ||
			    interface == PHY_INTERFACE_MODE_RGMII_TXID)
				data8 |= PORT_RGMII_ID_EG_ENABLE;
			if (priv->features & IS_9893)
				data8 &= ~PORT_MII_MAC_MODE;
			break;
		}
		ksz_write8(dev, PORT_CTRL_ADDR(port, REG_PORT_XMII_CTRL_1), data8);
	}

	return 0;
}

static int ksz_port_probe(struct udevice *dev, int port, struct phy_device *phy)
{
	int supported = PHY_GBIT_FEATURES;

	/* configure phy */
	phy->supported &= supported;
	phy->advertising &= supported;

	return phy_config(phy);
}

static int ksz_port_enable(struct udevice *dev, int port, struct phy_device *phy)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(dev);
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	u8 data8;
	int ret;

	dev_dbg(dev, "%s P%d 0x%x %s\n", __func__, port + 1, phy->phy_id,
		phy_string_for_interface(phy->interface));

	/* setup this port */
	ret = ksz_port_setup(dev, port, phy->interface);
	if (ret) {
		dev_err(dev, "port setup failed: %d\n", ret);
		return ret;
	}

	/* enable port forwarding for this port */
	ksz_pread8(priv->dev, port, REG_PORT_MSTP_STATE, &data8);
	data8 &= ~(PORT_TX_ENABLE | PORT_RX_ENABLE | PORT_LEARN_DISABLE);
	data8 |= (PORT_TX_ENABLE | PORT_RX_ENABLE);
	ksz_pwrite8(priv->dev, port, REG_PORT_MSTP_STATE, data8);

	/* if cpu master we are done */
	if (port == pdata->cpu_port)
		return 0;

	/* start switch */
	ksz_read8(priv->dev, REG_SW_OPERATION, &data8);
	data8 |= SW_START;
	ksz_write8(priv->dev, REG_SW_OPERATION, data8);

	return phy_startup(phy);
}

static void ksz_port_disable(struct udevice *dev, int port, struct phy_device *phy)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(dev);
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	u8 data8;

	dev_dbg(dev, "%s P%d 0x%x\n", __func__, port + 1, phy->phy_id);

	/* can't disable CPU port without re-configuring/re-starting switch */
	if (port == pdata->cpu_port)
		return;

	/* disable port */
	ksz_pread8(priv->dev, port, REG_PORT_MSTP_STATE, &data8);
	data8 &= ~(PORT_TX_ENABLE | PORT_RX_ENABLE | PORT_LEARN_DISABLE);
	data8 |= PORT_LEARN_DISABLE;
	ksz_pwrite8(priv->dev, port, REG_PORT_MSTP_STATE, data8);

	/*
	 * we don't call phy_shutdown here to avoid waiting next time we use
	 * the port, but the downside is that remote side will think we're
	 * actively processing traffic although we are not.
	 */
}

static const struct dsa_ops ksz_dsa_ops = {
	.port_probe = ksz_port_probe,
	.port_enable = ksz_port_enable,
	.port_disable = ksz_port_disable,
};

static int ksz_probe_mdio(struct udevice *dev)
{
	ofnode node, mdios;
	int ret;

	mdios = dev_read_subnode(dev, "mdios");
	if (ofnode_valid(mdios)) {
		ofnode_for_each_subnode(node, mdios) {
			const char *name = ofnode_get_name(node);
			struct udevice *pdev;

			ret = device_bind_driver_to_node(dev,
							 KSZ_MDIO_CHILD_DRV_NAME,
							 name, node, &pdev);
			if (ret)
				dev_err(dev, "failed to probe %s: %d\n", name, ret);
		}
	}

	return 0;
}

static void ksz_ops_register(struct udevice *dev, struct ksz_phy_ops *ops)
{
	struct ksz_dsa_priv *priv = dev_get_priv(dev);

	priv->phy_ops = ops;
}

static bool dsa_ksz_check_ops(struct ksz_phy_ops *phy_ops)
{
	if (!phy_ops || !phy_ops->read || !phy_ops->write)
		return false;

	return true;
}

static int ksz_probe(struct udevice *dev)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(dev);
	struct ksz_dsa_priv *priv = dev_get_priv(dev);
	enum uclass_id parent_id = UCLASS_INVALID;
	int i, ret;
	u8 data8;
	u32 id;

	parent_id = device_get_uclass_id(dev_get_parent(dev));
	switch (parent_id) {
#if CONFIG_IS_ENABLED(DM_I2C)
	case UCLASS_I2C: {
		ksz_ops_register(dev, &phy_i2c_ops);

		ret = i2c_set_chip_offset_len(dev, 2);
		if (ret) {
			printf("i2c_set_chip_offset_len failed: %d\n", ret);
			return ret;
		}
		break;
	}
#endif
#if CONFIG_IS_ENABLED(DM_SPI)
	case UCLASS_SPI: {
		ksz_ops_register(dev, &phy_spi_ops);
		break;
	}
#endif
	default:
		dev_err(dev, "invalid parent bus (%s)\n",
			uclass_get_name(parent_id));
		return -EINVAL;
	}

	if (!dsa_ksz_check_ops(priv->phy_ops)) {
		printf("Driver bug. No bus ops defined\n");
		return -EINVAL;
	}

	/* default config */
	priv->dev = dev;

	/* chip level reset */
	ksz_read8(priv->dev, REG_SW_OPERATION, &data8);
	data8 |= SW_RESET;
	ksz_write8(priv->dev, REG_SW_OPERATION, data8);

	/* read chip id */
	ret = ksz_read32(dev, REG_CHIP_ID0__1, &id);
	if (ret)
		return ret;
	id = __swab32(id);
	dev_dbg(dev, "%s id=0x%08x\n", __func__, id);
	switch (id & 0xffffff00) {
	case 0x00947700:
		puts("KSZ9477S: ");
		break;
	case 0x00956700:
		puts("KSZ9567R: ");
		break;
	case 0x00989600:
		puts("KSZ9896C: ");
		break;
	case 0x00989700:
		puts("KSZ9897S: ");
		break;
	case 0x00989300:
		puts("KSZ9893R: ");
		break;
	default:
		dev_err(dev, "invalid chip id: 0x%08x\n", id);
		return -EINVAL;
	}
	if ((id & 0xf00) == 0x300)
		priv->features |= IS_9893;
	else
		priv->features |= NEW_XMII;

	/* probe mdio bus */
	ret = ksz_probe_mdio(dev);
	if (ret)
		return ret;

	/* disable ports by default */
	for (i = 0; i < pdata->num_ports; i++) {
		ksz_pread8(priv->dev, i, REG_PORT_MSTP_STATE, &data8);
		data8 &= ~(PORT_TX_ENABLE | PORT_RX_ENABLE | PORT_LEARN_DISABLE);
		ksz_pwrite8(priv->dev, i, REG_PORT_MSTP_STATE, data8);
	}

	return 0;
};

static const struct udevice_id ksz_ids[] = {
	{ .compatible = "microchip,ksz9897" },
	{ .compatible = "microchip,ksz9477" },
	{ .compatible = "microchip,ksz9567" },
	{ .compatible = "microchip,ksz9893" },
	{ .compatible = "microchip,ksz9896" },
	{ }
};

U_BOOT_DRIVER(ksz) = {
	.name		= "ksz-switch",
	.id		= UCLASS_DSA,
	.of_match	= ksz_ids,
	.probe		= ksz_probe,
	.ops		= &ksz_dsa_ops,
	.priv_auto	= sizeof(struct ksz_dsa_priv),
};
