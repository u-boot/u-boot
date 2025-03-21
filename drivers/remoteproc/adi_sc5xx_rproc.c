// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 *
 * Analog Devices SC5xx remoteproc driver for loading code onto SHARC cores
 */

#include <dm.h>
#include <regmap.h>
#include <remoteproc.h>
#include <syscon.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/io.h>

/* Register offsets */
#ifdef CONFIG_SC58X
#define ADI_RCU_REG_CTL		0x00
#define ADI_RCU_REG_STAT	0x04
#define ADI_RCU_REG_CRCTL	0x08
#define ADI_RCU_REG_CRSTAT	0x0c
#define ADI_RCU_REG_SIDIS	0x10
#define ADI_RCU_REG_SISTAT	0x14
#define ADI_RCU_REG_BCODE	0x1c
#define ADI_RCU_REG_SVECT0	0x20
#define ADI_RCU_REG_SVECT1	0x24
#define ADI_RCU_REG_SVECT2	0x28
#define ADI_RCU_REG_MSG		0x60
#define ADI_RCU_REG_MSG_SET	0x64
#define ADI_RCU_REG_MSG_CLR	0x68
#else
#define ADI_RCU_REG_CTL		0x00
#define ADI_RCU_REG_STAT	0x04
#define ADI_RCU_REG_CRCTL	0x08
#define ADI_RCU_REG_CRSTAT	0x0c
#define ADI_RCU_REG_SRRQSTAT	0x18
#define ADI_RCU_REG_SIDIS	0x1c
#define ADI_RCU_REG_SISTAT	0x20
#define ADI_RCU_REG_SVECT_LCK	0x24
#define ADI_RCU_REG_BCODE	0x28
#define ADI_RCU_REG_SVECT0	0x2c
#define ADI_RCU_REG_SVECT1	0x30
#define ADI_RCU_REG_SVECT2	0x34
#define ADI_RCU_REG_MSG		0x6c
#define ADI_RCU_REG_MSG_SET	0x70
#define ADI_RCU_REG_MSG_CLR	0x74
#endif /* CONFIG_SC58X */

/* Register bit definitions */
#define ADI_RCU_CTL_SYSRST		BIT(0)

/* Bit values for the RCU0_MSG register */
#define RCU0_MSG_C0IDLE			0x00000100		/* Core 0 Idle */
#define RCU0_MSG_C1IDLE			0x00000200		/* Core 1 Idle */
#define RCU0_MSG_C2IDLE			0x00000400		/* Core 2 Idle */
#define RCU0_MSG_CRR0			0x00001000		/* Core 0 reset request */
#define RCU0_MSG_CRR1			0x00002000		/* Core 1 reset request */
#define RCU0_MSG_CRR2			0x00004000		/* Core 2 reset request */
#define RCU0_MSG_C1ACTIVATE		0x00080000		/* Core 1 Activated */
#define RCU0_MSG_C2ACTIVATE		0x00100000		/* Core 2 Activated */

struct sc5xx_rproc_data {
	/* Address to load to svect when rebooting core */
	u32 load_addr;

	/* RCU parameters */
	struct regmap *rcu;
	u32 svect_offset;
	u32 coreid;
};

struct block_code_flag {
	u32 bcode:4,		/* 0-3 */
	    bflag_save:1,	/* 4 */
	    bflag_aux:1,	/* 5 */
	    breserved:1,	/* 6 */
	    bflag_forward:1,	/* 7 */
	    bflag_fill:1,	/* 8 */
	    bflag_quickboot:1,	/* 9 */
	    bflag_callback:1,	/* 10 */
	    bflag_init:1,	/* 11 */
	    bflag_ignore:1,	/* 12 */
	    bflag_indirect:1,	/* 13 */
	    bflag_first:1,	/* 14 */
	    bflag_final:1,	/* 15 */
	    bhdrchk:8,		/* 16-23 */
	    bhdrsign:8;		/* 0xAD, 0xAC or 0xAB */
};

struct ldr_hdr {
	struct block_code_flag bcode_flag;
	u32 target_addr;
	u32 byte_count;
	u32 argument;
};

static int is_final(struct ldr_hdr *hdr)
{
	return hdr->bcode_flag.bflag_final;
}

static int is_empty(struct ldr_hdr *hdr)
{
	return hdr->bcode_flag.bflag_ignore || (hdr->byte_count == 0);
}

static int adi_valid_firmware(struct ldr_hdr *adi_ldr_hdr)
{
	if (!adi_ldr_hdr->byte_count &&
	    (adi_ldr_hdr->bcode_flag.bhdrsign == 0xAD ||
	     adi_ldr_hdr->bcode_flag.bhdrsign == 0xAC ||
	     adi_ldr_hdr->bcode_flag.bhdrsign == 0xAB))
		return 1;

	return 0;
}

static int sharc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct sc5xx_rproc_data *priv = dev_get_priv(dev);
	size_t offset;
	u8 *buf = (u8 *)addr;
	struct ldr_hdr *ldr = (struct ldr_hdr *)addr;
	struct ldr_hdr *block_hdr;
	struct ldr_hdr *next_hdr;

	if (!adi_valid_firmware(ldr)) {
		dev_err(dev, "Firmware at 0x%lx does not appear to be an LDR image\n", addr);
		dev_err(dev, "Note: Signed firmware is not currently supported\n");
		return -EINVAL;
	}

	do {
		block_hdr = (struct ldr_hdr *)buf;
		offset = sizeof(struct ldr_hdr) + (block_hdr->bcode_flag.bflag_fill ?
							0 : block_hdr->byte_count);
		next_hdr = (struct ldr_hdr *)(buf + offset);

		if (block_hdr->bcode_flag.bflag_first)
			priv->load_addr = (unsigned long)block_hdr->target_addr;

		if (!is_empty(block_hdr)) {
			if (block_hdr->bcode_flag.bflag_fill) {
				memset_io((void *)(phys_addr_t)block_hdr->target_addr,
					  block_hdr->argument,
					  block_hdr->byte_count);
			} else {
				memcpy_toio((void *)(phys_addr_t)block_hdr->target_addr,
					  buf + sizeof(struct ldr_hdr),
					  block_hdr->byte_count);
			}
		}

		if (is_final(block_hdr))
			break;

		buf += offset;
	} while (1);

	return 0;
}

static void sharc_reset(struct sc5xx_rproc_data *priv)
{
	u32 coreid = priv->coreid;
	u32 val;

	/* First put core in reset.
	 * Clear CRSTAT bit for given coreid.
	 */
	regmap_write(priv->rcu, ADI_RCU_REG_CRSTAT, 1 << coreid);

	/* Set SIDIS to disable the system interface */
	regmap_read(priv->rcu, ADI_RCU_REG_SIDIS, &val);
	regmap_write(priv->rcu, ADI_RCU_REG_SIDIS, val | (1 << (coreid - 1)));

	/*
	 * Wait for access to coreX have been disabled and all the pending
	 * transactions have completed
	 */
	udelay(50);

	/* Set CRCTL bit to put core in reset */
	regmap_read(priv->rcu, ADI_RCU_REG_CRCTL, &val);
	regmap_write(priv->rcu, ADI_RCU_REG_CRCTL, val | (1 << coreid));

	/* Poll until Core is in reset */
	while (!(regmap_read(priv->rcu, ADI_RCU_REG_CRSTAT, &val), val & (1 << coreid)))
		;

	/* Clear SIDIS to reenable the system interface */
	regmap_read(priv->rcu, ADI_RCU_REG_SIDIS, &val);
	regmap_write(priv->rcu, ADI_RCU_REG_SIDIS, val & ~(1 << (coreid - 1)));

	udelay(50);

	/* Take Core out of reset */
	regmap_read(priv->rcu, ADI_RCU_REG_CRCTL, &val);
	regmap_write(priv->rcu, ADI_RCU_REG_CRCTL, val & ~(1 << coreid));

	/* Wait for done */
	udelay(50);
}

static int sharc_start(struct udevice *dev)
{
	struct sc5xx_rproc_data *priv = dev_get_priv(dev);

	/* Write load address to appropriate SVECT for core */
	regmap_write(priv->rcu, priv->svect_offset, priv->load_addr);

	sharc_reset(priv);

	/* Clear the IDLE bit when start the SHARC core */
	regmap_write(priv->rcu, ADI_RCU_REG_MSG_CLR, RCU0_MSG_C0IDLE << priv->coreid);

	/* Notify CCES */
	regmap_write(priv->rcu, ADI_RCU_REG_MSG_SET, RCU0_MSG_C1ACTIVATE << (priv->coreid - 1));
	return 0;
}

static const struct dm_rproc_ops sc5xx_ops = {
	.load = sharc_load,
	.start = sharc_start,
};

static int sc5xx_probe(struct udevice *dev)
{
	struct sc5xx_rproc_data *priv = dev_get_priv(dev);
	u32 coreid;

	if (dev_read_u32(dev, "coreid", &coreid)) {
		dev_err(dev, "Missing property coreid\n");
		return -ENOENT;
	}

	priv->coreid = coreid;
	switch (coreid) {
	case 1:
		priv->svect_offset = ADI_RCU_REG_SVECT1;
		break;
	case 2:
		priv->svect_offset = ADI_RCU_REG_SVECT2;
		break;
	default:
		dev_err(dev, "Invalid value %d for coreid, must be 1 or 2\n", coreid);
		return -EINVAL;
	}

	priv->rcu = syscon_regmap_lookup_by_phandle(dev, "adi,rcu");
	if (IS_ERR(priv->rcu))
		return PTR_ERR(priv->rcu);

	dev_err(dev, "sc5xx remoteproc core %d available\n", priv->coreid);

	return 0;
}

static const struct udevice_id sc5xx_ids[] = {
	{ .compatible = "adi,sc5xx-rproc" },
	{ }
};

U_BOOT_DRIVER(adi_sc5xx_rproc) = {
	.name = "adi_sc5xx_rproc",
	.of_match = sc5xx_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &sc5xx_ops,
	.probe = sc5xx_probe,
	.priv_auto = sizeof(struct sc5xx_rproc_data),
	.flags = 0,
};
