// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom.
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct sdhci_iproc_host {
	struct sdhci_host host;
	u32 shadow_cmd;
	u32 shadow_blk;
};

#define REG_OFFSET_IN_BITS(reg) ((reg) << 3 & 0x18)

static inline struct sdhci_iproc_host *to_iproc(struct sdhci_host *host)
{
	return (struct sdhci_iproc_host *)host;
}

#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
static u32 sdhci_iproc_readl(struct sdhci_host *host, int reg)
{
	u32 val = readl(host->ioaddr + reg);
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS_TRACE
	printf("%s %d: readl [0x%02x] 0x%08x\n",
	       host->name, host->index, reg, val);
#endif
	return val;
}

static u16 sdhci_iproc_readw(struct sdhci_host *host, int reg)
{
	u32 val = sdhci_iproc_readl(host, (reg & ~3));
	u16 word = val >> REG_OFFSET_IN_BITS(reg) & 0xffff;
	return word;
}

static u8 sdhci_iproc_readb(struct sdhci_host *host, int reg)
{
	u32 val = sdhci_iproc_readl(host, (reg & ~3));
	u8 byte = val >> REG_OFFSET_IN_BITS(reg) & 0xff;
	return byte;
}

static void sdhci_iproc_writel(struct sdhci_host *host, u32 val, int reg)
{
	u32 clock = 0;
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS_TRACE
	printf("%s %d: writel [0x%02x] 0x%08x\n",
	       host->name, host->index, reg, val);
#endif
	writel(val, host->ioaddr + reg);

	if (host->mmc)
		clock = host->mmc->clock;
	if (clock <= 400000) {
		/* Round up to micro-second four SD clock delay */
		if (clock)
			udelay((4 * 1000000 + clock - 1) / clock);
		else
			udelay(10);
	}
}

/*
 * The Arasan has a bugette whereby it may lose the content of successive
 * writes to the same register that are within two SD-card clock cycles of
 * each other (a clock domain crossing problem). The data
 * register does not have this problem, which is just as well - otherwise we'd
 * have to nobble the DMA engine too.
 *
 * This wouldn't be a problem with the code except that we can only write the
 * controller with 32-bit writes.  So two different 16-bit registers are
 * written back to back creates the problem.
 *
 * In reality, this only happens when SDHCI_BLOCK_SIZE and SDHCI_BLOCK_COUNT
 * are written followed by SDHCI_TRANSFER_MODE and SDHCI_COMMAND.
 * The BLOCK_SIZE and BLOCK_COUNT are meaningless until a command issued so
 * the work around can be further optimized. We can keep shadow values of
 * BLOCK_SIZE, BLOCK_COUNT, and TRANSFER_MODE until a COMMAND is issued.
 * Then, write the BLOCK_SIZE+BLOCK_COUNT in a single 32-bit write followed
 * by the TRANSFER+COMMAND in another 32-bit write.
 */
static void sdhci_iproc_writew(struct sdhci_host *host, u16 val, int reg)
{
	struct sdhci_iproc_host *iproc_host = to_iproc(host);
	u32 word_shift = REG_OFFSET_IN_BITS(reg);
	u32 mask = 0xffff << word_shift;
	u32 oldval, newval;

	if (reg == SDHCI_COMMAND) {
		/* Write the block now as we are issuing a command */
		if (iproc_host->shadow_blk != 0) {
			sdhci_iproc_writel(host, iproc_host->shadow_blk,
					   SDHCI_BLOCK_SIZE);
			iproc_host->shadow_blk = 0;
		}
		oldval = iproc_host->shadow_cmd;
	} else if (reg == SDHCI_BLOCK_SIZE || reg == SDHCI_BLOCK_COUNT) {
		/* Block size and count are stored in shadow reg */
		oldval = iproc_host->shadow_blk;
	} else {
		/* Read reg, all other registers are not shadowed */
		oldval = sdhci_iproc_readl(host, (reg & ~3));
	}
	newval = (oldval & ~mask) | (val << word_shift);

	if (reg == SDHCI_TRANSFER_MODE) {
		/* Save the transfer mode until the command is issued */
		iproc_host->shadow_cmd = newval;
	} else if (reg == SDHCI_BLOCK_SIZE || reg == SDHCI_BLOCK_COUNT) {
		/* Save the block info until the command is issued */
		iproc_host->shadow_blk = newval;
	} else {
		/* Command or other regular 32-bit write */
		sdhci_iproc_writel(host, newval, reg & ~3);
	}
}

static void sdhci_iproc_writeb(struct sdhci_host *host, u8 val, int reg)
{
	u32 oldval = sdhci_iproc_readl(host, (reg & ~3));
	u32 byte_shift = REG_OFFSET_IN_BITS(reg);
	u32 mask = 0xff << byte_shift;
	u32 newval = (oldval & ~mask) | (val << byte_shift);

	sdhci_iproc_writel(host, newval, reg & ~3);
}
#endif

static int sdhci_iproc_set_ios_post(struct sdhci_host *host)
{
	u32 ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	/* Reset UHS mode bits */
	ctrl &= ~SDHCI_CTRL_UHS_MASK;

	if (host->mmc->ddr_mode)
		ctrl |= UHS_DDR50_BUS_SPEED;

	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	return 0;
}

static struct sdhci_ops sdhci_platform_ops = {
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
	.read_l = sdhci_iproc_readl,
	.read_w = sdhci_iproc_readw,
	.read_b = sdhci_iproc_readb,
	.write_l = sdhci_iproc_writel,
	.write_w = sdhci_iproc_writew,
	.write_b = sdhci_iproc_writeb,
#endif
	.set_ios_post = sdhci_iproc_set_ios_post,
};

struct iproc_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int iproc_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct iproc_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct sdhci_iproc_host *iproc_host;
	int node = dev_of_offset(dev);
	u32 f_min_max[2];
	int ret;

	iproc_host = malloc(sizeof(struct sdhci_iproc_host));
	if (!iproc_host) {
		printf("%s: sdhci host malloc fail!\n", __func__);
		return -ENOMEM;
	}
	iproc_host->shadow_cmd = 0;
	iproc_host->shadow_blk = 0;

	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);
	host->voltages = MMC_VDD_165_195 |
			 MMC_VDD_32_33 | MMC_VDD_33_34;
	host->quirks = SDHCI_QUIRK_BROKEN_VOLTAGE | SDHCI_QUIRK_BROKEN_R1B;
	host->host_caps = MMC_MODE_DDR_52MHz;
	host->index = fdtdec_get_uint(gd->fdt_blob, node, "index", 0);
	host->ops = &sdhci_platform_ops;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
				   "clock-freq-min-max", f_min_max, 2);
	if (ret) {
		printf("sdhci: clock-freq-min-max not found\n");
		free(iproc_host);
		return ret;
	}
	host->max_clk = f_min_max[1];
	host->bus_width	= fdtdec_get_int(gd->fdt_blob,
					 dev_of_offset(dev), "bus-width", 4);

	/* Update host_caps for 8 bit bus width */
	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

	memcpy(&iproc_host->host, host, sizeof(struct sdhci_host));

	iproc_host->host.mmc = &plat->mmc;
	iproc_host->host.mmc->dev = dev;
	iproc_host->host.mmc->priv = &iproc_host->host;
	upriv->mmc = iproc_host->host.mmc;

	ret = sdhci_setup_cfg(&plat->cfg, &iproc_host->host,
			      f_min_max[1], f_min_max[0]);
	if (ret) {
		free(iproc_host);
		return ret;
	}

	return sdhci_probe(dev);
}

static int iproc_sdhci_bind(struct udevice *dev)
{
	struct iproc_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id iproc_sdhci_ids[] = {
	{ .compatible = "brcm,iproc-sdhci" },
	{ }
};

U_BOOT_DRIVER(iproc_sdhci_drv) = {
	.name = "iproc_sdhci",
	.id = UCLASS_MMC,
	.of_match = iproc_sdhci_ids,
	.ops = &sdhci_ops,
	.bind = iproc_sdhci_bind,
	.probe = iproc_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct iproc_sdhci_plat),
};
