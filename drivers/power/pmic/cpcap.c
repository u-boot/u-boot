// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <log.h>
#include <power/pmic.h>
#include <power/cpcap.h>
#include <spi.h>
#include <linux/delay.h>
#include <linux/err.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "sw", .driver = CPCAP_SW_DRIVER },
	{ .prefix = "v", .driver = CPCAP_LDO_DRIVER },
	{ },
};

static int cpcap_write(struct udevice *dev, uint reg, const uint8_t *buff, int len)
{
	u8 buf[4];
	u16 data = *(u16 *)buff;
	int ret;

	buf[0] = ((reg >> 8) & 0xff) | 0x80;
	buf[1] = reg & 0xff;
	buf[2] = data >> 8 & 0xff;
	buf[3] = data & 0xff;

	ret = dm_spi_xfer(dev, 32, buf, NULL, SPI_XFER_ONCE);

	log_debug("%s: reg 0x%x, data 0x%04x, ret %d\n", __func__, reg, data, ret);

	return ret;
}

static int cpcap_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	u8 buf[4];
	int ret;

	buf[0] = (reg >> 8) & 0xff;
	buf[1] = reg & 0xff;
	buf[2] = 0;
	buf[3] = 0;

	ret = dm_spi_xfer(dev, 32, buf, buf, SPI_XFER_ONCE);
	*buff = (buf[2] << 8) | buf[3];

	log_debug("%s: reg 0x%x, data 0x%04x, ret %d\n", __func__, reg, *buff, ret);
	return ret;
}

static int cpcap_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	/* Regulator device node of PMIC */
	regulators_node = dev_read_subnode(dev, "regulator");
	if (!ofnode_valid(regulators_node)) {
		log_err("%s regulator subnode not found!\n", dev->name);
		return -ENXIO;
	}

	/* Actual regulators container */
	regulators_node = ofnode_find_subnode(regulators_node, "regulators");
	if (!ofnode_valid(regulators_node)) {
		log_err("%s regulators subnode not found!\n", dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		log_err("%s - no child found\n", dev->name);

	return dm_scan_fdt_dev(dev);
}

static int cpcap_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int ret;

	ret = spi_claim_bus(slave);
	if (ret) {
		log_err("SPI bus allocation failed (%d)\n", ret);
		return ret;
	}

	u16 id = pmic_reg_read(dev, CPCAP_REG_VERSC1);

	u16 ven = (id >> 6) & 0x7;
	u16 rev = ((id >> 3) & 0x7) | ((id << 3) & 0x38);

	log_debug("%s: vendor %s rev: %i.%i (%x)\n", __func__,
		  ven == CPCAP_VENDOR_ST ? "ST" : "TI",
		  CPCAP_REVISION_MAJOR(rev), CPCAP_REVISION_MINOR(rev),
		  rev);
	return 0;
}

static struct dm_pmic_ops cpcap_ops = {
	.read = cpcap_read,
	.write = cpcap_write,
};

static const struct udevice_id cpcap_ids[] = {
	{ .compatible = "motorola,cpcap" },
	{ .compatible = "st,6556002" },
	{ }
};

U_BOOT_DRIVER(pmic_cpcap) = {
	.name = "cpcap_pmic",
	.id = UCLASS_PMIC,
	.of_match = cpcap_ids,
	.bind = cpcap_bind,
	.probe = cpcap_probe,
	.ops = &cpcap_ops,
};
