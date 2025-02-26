// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Analog Devices DMA controller driver
 *
 * (C) Copyright 2024 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 * Contact: Ian Roberts <ian.roberts@timesys.com>
 *
 */
#include <dm.h>
#include <dma.h>
#include <dma-uclass.h>
#include <dm/device_compat.h>
#include <linux/errno.h>
#include <linux/io.h>

#define HAS_MDMA	BIT(0)

#define REG_ADDRSTART	0x04
#define REG_CFG		0x08
#define REG_XCNT	0x0C
#define REG_XMOD	0x10
#define REG_STAT	0x30

#define BITP_DMA_CFG_MSIZE                        8
#define BITP_DMA_CFG_PSIZE                        4
#define BITM_DMA_CFG_WNR                 0x00000002
#define BITM_DMA_CFG_EN                  0x00000001
#define ENUM_DMA_CFG_XCNT_INT            0x00100000

#define BITP_DMA_STAT_PBWID                      12
#define BITP_DMA_STAT_ERRC                        4
#define BITM_DMA_STAT_PBWID              0x00003000
#define BITM_DMA_STAT_ERRC               0x00000070
#define BITM_DMA_STAT_PIRQ               0x00000004
#define BITM_DMA_STAT_IRQERR             0x00000002
#define BITM_DMA_STAT_IRQDONE            0x00000001

#define DMA_MDMA_SRC_DEFAULT_CONFIG(psize, msize) \
	(BITM_DMA_CFG_EN | ((psize) << BITP_DMA_CFG_PSIZE) | ((msize) << BITP_DMA_CFG_MSIZE))
#define DMA_MDMA_DST_DEFAULT_CONFIG(psize, msize) \
	(BITM_DMA_CFG_EN | BITM_DMA_CFG_WNR | ENUM_DMA_CFG_XCNT_INT | \
	((psize) << BITP_DMA_CFG_PSIZE) | ((msize) << BITP_DMA_CFG_MSIZE))

struct adi_dma_channel {
	int id;
	struct adi_dma *dma;
	void __iomem *iosrc;
	void __iomem *iodest;
};

struct adi_dma {
	struct udevice *dev;
	struct adi_dma_channel channels[1];
	void __iomem *ioaddr;
	unsigned long hw_cfg;
};

static const struct udevice_id dma_dt_ids[] = {
	{ .compatible = "adi,mdma-controller", .data = HAS_MDMA },
	{ }
};

static u8 adi_dma_get_msize(u32 n_bytecount, u32 n_address)
{
	/* Calculate MSIZE, PSIZE, XCNT and XMOD */
	u8 n_msize = 0;
	u32 n_value = n_bytecount | n_address;
	u32 n_mask = 0x1;

	for (n_msize = 0; n_msize < 5; n_msize++, n_mask <<= 1) {
		if ((n_value & n_mask) == n_mask)
			break;
	}

	return n_msize;
}

static int adi_dma_get_ch_error(void __iomem *ch)
{
	u32 cause = (ioread32(ch + REG_STAT) &  BITM_DMA_STAT_ERRC) >>
		    BITP_DMA_STAT_ERRC;
	switch (cause) {
	case 0:
		return -EINVAL;
	case 1:
		return -EBUSY;
	case 2:
		return -EFAULT;
	case 3:
		fallthrough;
	case 5:
		fallthrough;
	case 6:
		fallthrough;
	default:
		return -EIO;
	}
}

static int adi_mdma_transfer(struct udevice *dev, int direction,
			     dma_addr_t dst, dma_addr_t src, size_t len)
{
	struct adi_dma *priv = dev_get_priv(dev);
	void __iomem *chsrc = priv->channels[0].iosrc;
	void __iomem *chdst = priv->channels[0].iodest;

	int result = 0;
	u32 reg;
	u32 bytecount = len;

	u8 n_srcmsize;
	u8 n_dstmsize;
	u8 n_srcpsize;
	u8 n_dstpsize;
	u8 n_psize;
	u32 srcconfig;
	u32 dstconfig;
	u8 srcpsizemax = (ioread32(chsrc + REG_STAT) & BITM_DMA_STAT_PBWID) >>
			 BITP_DMA_STAT_PBWID;
	u8 dstpsizemax = (ioread32(chdst + REG_STAT) & BITM_DMA_STAT_PBWID) >>
			 BITP_DMA_STAT_PBWID;

	const u32 CLRSTAT = (BITM_DMA_STAT_IRQDONE | BITM_DMA_STAT_IRQERR |
			     BITM_DMA_STAT_PIRQ);

	if (len == 0)
		return -EINVAL;

	/* Clear DMA status */
	iowrite32(CLRSTAT, chsrc + REG_STAT);
	iowrite32(CLRSTAT, chdst + REG_STAT);

	/* Calculate MSIZE, PSIZE, XCNT and XMOD */
	n_srcmsize = adi_dma_get_msize(bytecount, src);
	n_dstmsize = adi_dma_get_msize(bytecount, dst);
	n_srcpsize = min(n_srcmsize, srcpsizemax);
	n_dstpsize = min(n_dstmsize, dstpsizemax);
	n_psize = min(n_srcpsize, n_dstpsize);

	srcconfig = DMA_MDMA_SRC_DEFAULT_CONFIG(n_psize, n_srcmsize);
	dstconfig = DMA_MDMA_DST_DEFAULT_CONFIG(n_psize, n_dstmsize);

	/* Load the DMA descriptors */
	iowrite32(src,			chsrc + REG_ADDRSTART);
	iowrite32(bytecount >> n_srcmsize,	chsrc + REG_XCNT);
	iowrite32(1 << n_srcmsize,		chsrc + REG_XMOD);
	iowrite32(dst,			chdst + REG_ADDRSTART);
	iowrite32(bytecount >> n_dstmsize,	chdst + REG_XCNT);
	iowrite32(1 << n_dstmsize,		chdst + REG_XMOD);

	iowrite32(dstconfig, chdst + REG_CFG);
	iowrite32(srcconfig, chsrc + REG_CFG);

	/* Wait for DMA to complete while checking for a DMA error */
	do {
		reg = ioread32(chsrc + REG_STAT);
		if ((reg & BITM_DMA_STAT_IRQERR) == BITM_DMA_STAT_IRQERR) {
			result = adi_dma_get_ch_error(chsrc);
			break;
		}
		reg = ioread32(chdst + REG_STAT);
		if ((reg & BITM_DMA_STAT_IRQERR) == BITM_DMA_STAT_IRQERR) {
			result = adi_dma_get_ch_error(chdst);
			break;
		}
	} while ((reg & BITM_DMA_STAT_IRQDONE) == 0);

	clrbits_32(chsrc + REG_CFG, 1);
	clrbits_32(chdst + REG_CFG, 1);

	return result;
}

static int adi_dma_init_channel(struct adi_dma *dma,
				struct adi_dma_channel *channel, ofnode node)
{
	u32 offset;

	if (ofnode_read_u32(node, "adi,id", &channel->id)) {
		dev_err(dma->dev, "Missing adi,id for channel %s\n",
			ofnode_get_name(node));
		return -ENOENT;
	}

	if (ofnode_read_u32(node, "adi,src-offset", &offset)) {
		dev_err(dma->dev, "Missing adi,src-offset for channel %s\n",
			ofnode_get_name(node));
		return -ENOENT;
	}

	channel->iosrc = dma->ioaddr + offset;
	channel->dma = dma;

	if (dma->hw_cfg & HAS_MDMA) {
		if (ofnode_read_u32(node, "adi,dest-offset", &offset)) {
			dev_err(dma->dev,
				"Missing adi,dest-offset for channel %s\n",
				ofnode_get_name(node));
			return -ENOENT;
		}
		channel->iodest = dma->ioaddr + offset;
	}

	return 0;
}

static int adi_dma_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct adi_dma *priv = dev_get_priv(dev);
	ofnode node, child;

	priv->hw_cfg = dev_get_driver_data(dev);
	if (priv->hw_cfg & HAS_MDMA)
		uc_priv->supported = DMA_SUPPORTS_MEM_TO_MEM;

	priv->ioaddr = dev_remap_addr(dev);
	if (!priv->ioaddr)
		return -EINVAL;

	node = dev_read_first_subnode(dev);
	if (!ofnode_valid(node)) {
		dev_err(dev,
			"Error: device tree DMA channel config missing!\n");
		return -ENODEV;
	}

	node = dev_ofnode(dev);
	ofnode_for_each_subnode(child, node) {
		adi_dma_init_channel(priv, priv->channels, child);
		break; //Only 1 channel supported for now
	}

	return 0;
}

static const struct dma_ops adi_dma_ops = {
	.transfer = adi_mdma_transfer,
};

U_BOOT_DRIVER(adi_dma) = {
	.name = "adi_dma",
	.id = UCLASS_DMA,
	.of_match = dma_dt_ids,
	.ops = &adi_dma_ops,
	.probe = adi_dma_probe,
	.priv_auto = sizeof(struct adi_dma),
};
