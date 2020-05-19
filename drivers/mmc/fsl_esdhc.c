// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc
 * Copyright 2019-2020 NXP
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <errno.h>
#include <hwconfig.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <fsl_esdhc.h>
#include <fdt_support.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct fsl_esdhc {
	uint    dsaddr;		/* SDMA system address register */
	uint    blkattr;	/* Block attributes register */
	uint    cmdarg;		/* Command argument register */
	uint    xfertyp;	/* Transfer type register */
	uint    cmdrsp0;	/* Command response 0 register */
	uint    cmdrsp1;	/* Command response 1 register */
	uint    cmdrsp2;	/* Command response 2 register */
	uint    cmdrsp3;	/* Command response 3 register */
	uint    datport;	/* Buffer data port register */
	uint    prsstat;	/* Present state register */
	uint    proctl;		/* Protocol control register */
	uint    sysctl;		/* System Control Register */
	uint    irqstat;	/* Interrupt status register */
	uint    irqstaten;	/* Interrupt status enable register */
	uint    irqsigen;	/* Interrupt signal enable register */
	uint    autoc12err;	/* Auto CMD error status register */
	uint    hostcapblt;	/* Host controller capabilities register */
	uint    wml;		/* Watermark level register */
	char    reserved1[8];	/* reserved */
	uint    fevt;		/* Force event register */
	uint    admaes;		/* ADMA error status register */
	uint    adsaddr;	/* ADMA system address register */
	char    reserved2[160];
	uint    hostver;	/* Host controller version register */
	char    reserved3[4];	/* reserved */
	uint    dmaerraddr;	/* DMA error address register */
	char    reserved4[4];	/* reserved */
	uint    dmaerrattr;	/* DMA error attribute register */
	char    reserved5[4];	/* reserved */
	uint    hostcapblt2;	/* Host controller capabilities register 2 */
	char    reserved6[756];	/* reserved */
	uint    esdhcctl;	/* eSDHC control register */
};

struct fsl_esdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

/**
 * struct fsl_esdhc_priv
 *
 * @esdhc_regs: registers of the sdhc controller
 * @sdhc_clk: Current clk of the sdhc controller
 * @bus_width: bus width, 1bit, 4bit or 8bit
 * @cfg: mmc config
 * @mmc: mmc
 * Following is used when Driver Model is enabled for MMC
 * @dev: pointer for the device
 * @cd_gpio: gpio for card detection
 * @wp_gpio: gpio for write protection
 */
struct fsl_esdhc_priv {
	struct fsl_esdhc *esdhc_regs;
	unsigned int sdhc_clk;
	bool is_sdhc_per_clk;
	unsigned int clock;
#if !CONFIG_IS_ENABLED(DM_MMC)
	struct mmc *mmc;
#endif
	struct udevice *dev;
};

/* Return the XFERTYP flags for a given command and data packet */
static uint esdhc_xfertyp(struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint xfertyp = 0;

	if (data) {
		xfertyp |= XFERTYP_DPSEL;
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
		xfertyp |= XFERTYP_DMAEN;
#endif
		if (data->blocks > 1) {
			xfertyp |= XFERTYP_MSBSEL;
			xfertyp |= XFERTYP_BCEN;
#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC111
			xfertyp |= XFERTYP_AC12EN;
#endif
		}

		if (data->flags & MMC_DATA_READ)
			xfertyp |= XFERTYP_DTDSEL;
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		xfertyp |= XFERTYP_CCCEN;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		xfertyp |= XFERTYP_CICEN;
	if (cmd->resp_type & MMC_RSP_136)
		xfertyp |= XFERTYP_RSPTYP_136;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		xfertyp |= XFERTYP_RSPTYP_48_BUSY;
	else if (cmd->resp_type & MMC_RSP_PRESENT)
		xfertyp |= XFERTYP_RSPTYP_48;

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		xfertyp |= XFERTYP_CMDTYP_ABORT;

	return XFERTYP_CMD(cmd->cmdidx) | xfertyp;
}

#ifdef CONFIG_SYS_FSL_ESDHC_USE_PIO
/*
 * PIO Read/Write Mode reduce the performace as DMA is not used in this mode.
 */
static void esdhc_pio_read_write(struct fsl_esdhc_priv *priv,
				 struct mmc_data *data)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	uint blocks;
	char *buffer;
	uint databuf;
	uint size;
	uint irqstat;
	ulong start;

	if (data->flags & MMC_DATA_READ) {
		blocks = data->blocks;
		buffer = data->dest;
		while (blocks) {
			start = get_timer(0);
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BREN)) {
				if (get_timer(start) > PIO_TIMEOUT) {
					printf("\nData Read Failed in PIO Mode.");
					return;
				}
			}
			while (size && (!(irqstat & IRQSTAT_TC))) {
				udelay(100); /* Wait before last byte transfer complete */
				irqstat = esdhc_read32(&regs->irqstat);
				databuf = in_le32(&regs->datport);
				*((uint *)buffer) = databuf;
				buffer += 4;
				size -= 4;
			}
			blocks--;
		}
	} else {
		blocks = data->blocks;
		buffer = (char *)data->src;
		while (blocks) {
			start = get_timer(0);
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BWEN)) {
				if (get_timer(start) > PIO_TIMEOUT) {
					printf("\nData Write Failed in PIO Mode.");
					return;
				}
			}
			while (size && (!(irqstat & IRQSTAT_TC))) {
				udelay(100); /* Wait before last byte transfer complete */
				databuf = *((uint *)buffer);
				buffer += 4;
				size -= 4;
				irqstat = esdhc_read32(&regs->irqstat);
				out_le32(&regs->datport, databuf);
			}
			blocks--;
		}
	}
}
#endif

static int esdhc_setup_data(struct fsl_esdhc_priv *priv, struct mmc *mmc,
			    struct mmc_data *data)
{
	int timeout;
	struct fsl_esdhc *regs = priv->esdhc_regs;
#if defined(CONFIG_FSL_LAYERSCAPE)
	dma_addr_t addr;
#endif
	uint wml_value;

	wml_value = data->blocksize/4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > WML_RD_WML_MAX)
			wml_value = WML_RD_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_RD_WML_MASK, wml_value);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#if defined(CONFIG_FSL_LAYERSCAPE)
		addr = virt_to_phys((void *)(data->dest));
		if (upper_32_bits(addr))
			printf("Error found for upper 32 bits\n");
		else
			esdhc_write32(&regs->dsaddr, lower_32_bits(addr));
#else
		esdhc_write32(&regs->dsaddr, (u32)data->dest);
#endif
#endif
	} else {
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
		flush_dcache_range((ulong)data->src,
				   (ulong)data->src+data->blocks
					 *data->blocksize);
#endif
		if (wml_value > WML_WR_WML_MAX)
			wml_value = WML_WR_WML_MAX_VAL;

		if (!(esdhc_read32(&regs->prsstat) & PRSSTAT_WPSPL)) {
			printf("Can not write to locked SD card.\n");
			return -EINVAL;
		}

		esdhc_clrsetbits32(&regs->wml, WML_WR_WML_MASK,
					wml_value << 16);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#if defined(CONFIG_FSL_LAYERSCAPE)
		addr = virt_to_phys((void *)(data->src));
		if (upper_32_bits(addr))
			printf("Error found for upper 32 bits\n");
		else
			esdhc_write32(&regs->dsaddr, lower_32_bits(addr));
#else
		esdhc_write32(&regs->dsaddr, (u32)data->src);
#endif
#endif
	}

	esdhc_write32(&regs->blkattr, data->blocks << 16 | data->blocksize);

	/* Calculate the timeout period for data transactions */
	/*
	 * 1)Timeout period = (2^(timeout+13)) SD Clock cycles
	 * 2)Timeout period should be minimum 0.250sec as per SD Card spec
	 *  So, Number of SD Clock cycles for 0.25sec should be minimum
	 *		(SD Clock/sec * 0.25 sec) SD Clock cycles
	 *		= (mmc->clock * 1/4) SD Clock cycles
	 * As 1) >=  2)
	 * => (2^(timeout+13)) >= mmc->clock * 1/4
	 * Taking log2 both the sides
	 * => timeout + 13 >= log2(mmc->clock/4)
	 * Rounding up to next power of 2
	 * => timeout + 13 = log2(mmc->clock/4) + 1
	 * => timeout + 13 = fls(mmc->clock/4)
	 *
	 * However, the MMC spec "It is strongly recommended for hosts to
	 * implement more than 500ms timeout value even if the card
	 * indicates the 250ms maximum busy length."  Even the previous
	 * value of 300ms is known to be insufficient for some cards.
	 * So, we use
	 * => timeout + 13 = fls(mmc->clock/2)
	 */
	timeout = fls(mmc->clock/2);
	timeout -= 13;

	if (timeout > 14)
		timeout = 14;

	if (timeout < 0)
		timeout = 0;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC_A001
	if ((timeout == 4) || (timeout == 8) || (timeout == 12))
		timeout++;
#endif

#ifdef ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE
	timeout = 0xE;
#endif
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, timeout << 16);

	return 0;
}

static void check_and_invalidate_dcache_range
	(struct mmc_cmd *cmd,
	 struct mmc_data *data) {
	unsigned start = 0;
	unsigned end = 0;
	unsigned size = roundup(ARCH_DMA_MINALIGN,
				data->blocks*data->blocksize);
#if defined(CONFIG_FSL_LAYERSCAPE)
	dma_addr_t addr;

	addr = virt_to_phys((void *)(data->dest));
	if (upper_32_bits(addr))
		printf("Error found for upper 32 bits\n");
	else
		start = lower_32_bits(addr);
#else
	start = (unsigned)data->dest;
#endif
	end = start + size;
	invalidate_dcache_range(start, end);
}

/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int esdhc_send_cmd_common(struct fsl_esdhc_priv *priv, struct mmc *mmc,
				 struct mmc_cmd *cmd, struct mmc_data *data)
{
	int	err = 0;
	uint	xfertyp;
	uint	irqstat;
	u32	flags = IRQSTAT_CC | IRQSTAT_CTOE;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	unsigned long start;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC111
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		return 0;
#endif

	esdhc_write32(&regs->irqstat, -1);

	sync();

	/* Wait for the bus to be idle */
	while ((esdhc_read32(&regs->prsstat) & PRSSTAT_CICHB) ||
			(esdhc_read32(&regs->prsstat) & PRSSTAT_CIDHB))
		;

	while (esdhc_read32(&regs->prsstat) & PRSSTAT_DLA)
		;

	/* Wait at least 8 SD clock cycles before the next command */
	/*
	 * Note: This is way more than 8 cycles, but 1ms seems to
	 * resolve timing issues with some cards
	 */
	udelay(1000);

	/* Set up for a data transfer if we have one */
	if (data) {
		err = esdhc_setup_data(priv, mmc, data);
		if(err)
			return err;

		if (data->flags & MMC_DATA_READ)
			check_and_invalidate_dcache_range(cmd, data);
	}

	/* Figure out the transfer arguments */
	xfertyp = esdhc_xfertyp(cmd, data);

	/* Mask all irqs */
	esdhc_write32(&regs->irqsigen, 0);

	/* Send the command */
	esdhc_write32(&regs->cmdarg, cmd->cmdarg);
	esdhc_write32(&regs->xfertyp, xfertyp);

	/* Wait for the command to complete */
	start = get_timer(0);
	while (!(esdhc_read32(&regs->irqstat) & flags)) {
		if (get_timer(start) > 1000) {
			err = -ETIMEDOUT;
			goto out;
		}
	}

	irqstat = esdhc_read32(&regs->irqstat);

	if (irqstat & CMD_ERR) {
		err = -ECOMM;
		goto out;
	}

	if (irqstat & IRQSTAT_CTOE) {
		err = -ETIMEDOUT;
		goto out;
	}

	/* Workaround for ESDHC errata ENGcm03648 */
	if (!data && (cmd->resp_type & MMC_RSP_BUSY)) {
		int timeout = 6000;

		/* Poll on DATA0 line for cmd with busy signal for 600 ms */
		while (timeout > 0 && !(esdhc_read32(&regs->prsstat) &
					PRSSTAT_DAT0)) {
			udelay(100);
			timeout--;
		}

		if (timeout <= 0) {
			printf("Timeout waiting for DAT0 to go high!\n");
			err = -ETIMEDOUT;
			goto out;
		}
	}

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		u32 cmdrsp3, cmdrsp2, cmdrsp1, cmdrsp0;

		cmdrsp3 = esdhc_read32(&regs->cmdrsp3);
		cmdrsp2 = esdhc_read32(&regs->cmdrsp2);
		cmdrsp1 = esdhc_read32(&regs->cmdrsp1);
		cmdrsp0 = esdhc_read32(&regs->cmdrsp0);
		cmd->response[0] = (cmdrsp3 << 8) | (cmdrsp2 >> 24);
		cmd->response[1] = (cmdrsp2 << 8) | (cmdrsp1 >> 24);
		cmd->response[2] = (cmdrsp1 << 8) | (cmdrsp0 >> 24);
		cmd->response[3] = (cmdrsp0 << 8);
	} else
		cmd->response[0] = esdhc_read32(&regs->cmdrsp0);

	/* Wait until all of the blocks are transferred */
	if (data) {
#ifdef CONFIG_SYS_FSL_ESDHC_USE_PIO
		esdhc_pio_read_write(priv, data);
#else
		do {
			irqstat = esdhc_read32(&regs->irqstat);

			if (irqstat & IRQSTAT_DTOE) {
				err = -ETIMEDOUT;
				goto out;
			}

			if (irqstat & DATA_ERR) {
				err = -ECOMM;
				goto out;
			}
		} while ((irqstat & DATA_COMPLETE) != DATA_COMPLETE);

		/*
		 * Need invalidate the dcache here again to avoid any
		 * cache-fill during the DMA operations such as the
		 * speculative pre-fetching etc.
		 */
		if (data->flags & MMC_DATA_READ) {
			check_and_invalidate_dcache_range(cmd, data);
		}
#endif
	}

out:
	/* Reset CMD and DATA portions on error */
	if (err) {
		esdhc_write32(&regs->sysctl, esdhc_read32(&regs->sysctl) |
			      SYSCTL_RSTC);
		while (esdhc_read32(&regs->sysctl) & SYSCTL_RSTC)
			;

		if (data) {
			esdhc_write32(&regs->sysctl,
				      esdhc_read32(&regs->sysctl) |
				      SYSCTL_RSTD);
			while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTD))
				;
		}
	}

	esdhc_write32(&regs->irqstat, -1);

	return err;
}

static void set_sysctl(struct fsl_esdhc_priv *priv, struct mmc *mmc, uint clock)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int div = 1;
	int pre_div = 2;
	unsigned int sdhc_clk = priv->sdhc_clk;
	u32 time_out;
	u32 value;
	uint clk;

	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	while (sdhc_clk / (16 * pre_div) > clock && pre_div < 256)
		pre_div *= 2;

	while (sdhc_clk / (div * pre_div) > clock && div < 16)
		div++;

	pre_div >>= 1;
	div -= 1;

	clk = (pre_div << 8) | (div << 4);

	esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	time_out = 20;
	value = PRSSTAT_SDSTB;
	while (!(esdhc_read32(&regs->prsstat) & value)) {
		if (time_out == 0) {
			printf("fsl_esdhc: Internal clock never stabilised.\n");
			break;
		}
		time_out--;
		mdelay(1);
	}

	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);
}

static void esdhc_clock_control(struct fsl_esdhc_priv *priv, bool enable)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 value;
	u32 time_out;

	value = esdhc_read32(&regs->sysctl);

	if (enable)
		value |= SYSCTL_CKEN;
	else
		value &= ~SYSCTL_CKEN;

	esdhc_write32(&regs->sysctl, value);

	time_out = 20;
	value = PRSSTAT_SDSTB;
	while (!(esdhc_read32(&regs->prsstat) & value)) {
		if (time_out == 0) {
			printf("fsl_esdhc: Internal clock never stabilised.\n");
			break;
		}
		time_out--;
		mdelay(1);
	}
}

static int esdhc_set_ios_common(struct fsl_esdhc_priv *priv, struct mmc *mmc)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;

	if (priv->is_sdhc_per_clk) {
		/* Select to use peripheral clock */
		esdhc_clock_control(priv, false);
		esdhc_setbits32(&regs->esdhcctl, ESDHCCTL_PCS);
		esdhc_clock_control(priv, true);
	}

	/* Set the clock speed */
	if (priv->clock != mmc->clock)
		set_sysctl(priv, mmc, mmc->clock);

	/* Set the bus width */
	esdhc_clrbits32(&regs->proctl, PROCTL_DTW_4 | PROCTL_DTW_8);

	if (mmc->bus_width == 4)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_4);
	else if (mmc->bus_width == 8)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_8);

	return 0;
}

static void esdhc_enable_cache_snooping(struct fsl_esdhc *regs)
{
#ifdef CONFIG_ARCH_MPC830X
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;

	setbits_be32(&sysconf->sdhccr, 0x02000000);
#else
	esdhc_write32(&regs->esdhcctl, 0x00000040);
#endif
}

static int esdhc_init_common(struct fsl_esdhc_priv *priv, struct mmc *mmc)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	ulong start;

	/* Reset the entire host controller */
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);

	/* Wait until the controller is available */
	start = get_timer(0);
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA)) {
		if (get_timer(start) > 1000)
			return -ETIMEDOUT;
	}

	esdhc_enable_cache_snooping(regs);

	esdhc_setbits32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

	/* Set the initial clock speed */
	mmc_set_clock(mmc, 400000, MMC_CLK_ENABLE);

	/* Disable the BRR and BWR bits in IRQSTAT */
	esdhc_clrbits32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

	/* Put the PROCTL reg back to the default */
	esdhc_write32(&regs->proctl, PROCTL_INIT);

	/* Set timout to the maximum value */
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

	return 0;
}

static int esdhc_getcd_common(struct fsl_esdhc_priv *priv)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;

#ifdef CONFIG_ESDHC_DETECT_QUIRK
	if (CONFIG_ESDHC_DETECT_QUIRK)
		return 1;
#endif
	if (esdhc_read32(&regs->prsstat) & PRSSTAT_CINS)
		return 1;

	return 0;
}

static void fsl_esdhc_get_cfg_common(struct fsl_esdhc_priv *priv,
				     struct mmc_config *cfg)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 caps;

	caps = esdhc_read32(&regs->hostcapblt);
#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC135
	caps &= ~(HOSTCAPBLT_SRS | HOSTCAPBLT_VS18 | HOSTCAPBLT_VS30);
#endif
#ifdef CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
	caps |= HOSTCAPBLT_VS33;
#endif
	if (caps & HOSTCAPBLT_VS18)
		cfg->voltages |= MMC_VDD_165_195;
	if (caps & HOSTCAPBLT_VS30)
		cfg->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & HOSTCAPBLT_VS33)
		cfg->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;

	cfg->name = "FSL_SDHC";

	if (caps & HOSTCAPBLT_HSS)
		cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	cfg->f_min = 400000;
	cfg->f_max = min(priv->sdhc_clk, (u32)200000000);
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
}

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
void mmc_adapter_card_type_ident(void)
{
	u8 card_id;
	u8 value;

	card_id = QIXIS_READ(present) & QIXIS_SDID_MASK;
	gd->arch.sdhc_adapter = card_id;

	switch (card_id) {
	case QIXIS_ESDHC_ADAPTER_TYPE_EMMC45:
		value = QIXIS_READ(brdcfg[5]);
		value |= (QIXIS_DAT4 | QIXIS_DAT5_6_7);
		QIXIS_WRITE(brdcfg[5], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_SDMMC_LEGACY:
		value = QIXIS_READ(pwr_ctl[1]);
		value |= QIXIS_EVDD_BY_SDHC_VS;
		QIXIS_WRITE(pwr_ctl[1], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_EMMC44:
		value = QIXIS_READ(brdcfg[5]);
		value |= (QIXIS_SDCLKIN | QIXIS_SDCLKOUT);
		QIXIS_WRITE(brdcfg[5], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_RSV:
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_MMC:
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_SD:
		break;
	case QIXIS_ESDHC_NO_ADAPTER:
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_OF_LIBFDT
__weak int esdhc_status_fixup(void *blob, const char *compat)
{
#ifdef CONFIG_FSL_ESDHC_PIN_MUX
	if (!hwconfig("esdhc")) {
		do_fixup_by_compat(blob, compat, "status", "disabled",
				sizeof("disabled"), 1);
		return 1;
	}
#endif
	return 0;
}

#ifdef CONFIG_FSL_ESDHC_33V_IO_RELIABILITY_WORKAROUND
static int fsl_esdhc_get_cd(struct udevice *dev);

static void esdhc_disable_for_no_card(void *blob)
{
	struct udevice *dev;

	for (uclass_first_device(UCLASS_MMC, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		char esdhc_path[50];

		if (fsl_esdhc_get_cd(dev))
			continue;

		snprintf(esdhc_path, sizeof(esdhc_path), "/soc/esdhc@%lx",
			 (unsigned long)dev_read_addr(dev));
		do_fixup_by_path(blob, esdhc_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}
}
#endif

void fdt_fixup_esdhc(void *blob, bd_t *bd)
{
	const char *compat = "fsl,esdhc";

	if (esdhc_status_fixup(blob, compat))
		return;
#ifdef CONFIG_FSL_ESDHC_33V_IO_RELIABILITY_WORKAROUND
	esdhc_disable_for_no_card(blob);
#endif
	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->arch.sdhc_clk, 1);
}
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
static int esdhc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_getcd_common(priv);
}

static int esdhc_init(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_init_common(priv, mmc);
}

static int esdhc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_send_cmd_common(priv, mmc, cmd, data);
}

static int esdhc_set_ios(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_set_ios_common(priv, mmc);
}

static const struct mmc_ops esdhc_ops = {
	.getcd		= esdhc_getcd,
	.init		= esdhc_init,
	.send_cmd	= esdhc_send_cmd,
	.set_ios	= esdhc_set_ios,
};

int fsl_esdhc_initialize(bd_t *bis, struct fsl_esdhc_cfg *cfg)
{
	struct fsl_esdhc_plat *plat;
	struct fsl_esdhc_priv *priv;
	struct mmc_config *mmc_cfg;
	struct mmc *mmc;

	if (!cfg)
		return -EINVAL;

	priv = calloc(sizeof(struct fsl_esdhc_priv), 1);
	if (!priv)
		return -ENOMEM;
	plat = calloc(sizeof(struct fsl_esdhc_plat), 1);
	if (!plat) {
		free(priv);
		return -ENOMEM;
	}

	priv->esdhc_regs = (struct fsl_esdhc *)(unsigned long)(cfg->esdhc_base);
	priv->sdhc_clk = cfg->sdhc_clk;
	if (gd->arch.sdhc_per_clk)
		priv->is_sdhc_per_clk = true;

	mmc_cfg = &plat->cfg;

	if (cfg->max_bus_width == 8) {
		mmc_cfg->host_caps |= MMC_MODE_1BIT | MMC_MODE_4BIT |
				      MMC_MODE_8BIT;
	} else if (cfg->max_bus_width == 4) {
		mmc_cfg->host_caps |= MMC_MODE_1BIT | MMC_MODE_4BIT;
	} else if (cfg->max_bus_width == 1) {
		mmc_cfg->host_caps |= MMC_MODE_1BIT;
	} else {
		mmc_cfg->host_caps |= MMC_MODE_1BIT | MMC_MODE_4BIT |
				      MMC_MODE_8BIT;
		printf("No max bus width provided. Assume 8-bit supported.\n");
	}

#ifdef CONFIG_ESDHC_DETECT_8_BIT_QUIRK
	if (CONFIG_ESDHC_DETECT_8_BIT_QUIRK)
		mmc_cfg->host_caps &= ~MMC_MODE_8BIT;
#endif
	mmc_cfg->ops = &esdhc_ops;

	fsl_esdhc_get_cfg_common(priv, mmc_cfg);

	mmc = mmc_create(mmc_cfg, priv);
	if (!mmc)
		return -EIO;

	priv->mmc = mmc;
	return 0;
}

int fsl_esdhc_mmc_init(bd_t *bis)
{
	struct fsl_esdhc_cfg *cfg;

	cfg = calloc(sizeof(struct fsl_esdhc_cfg), 1);
	cfg->esdhc_base = CONFIG_SYS_FSL_ESDHC_ADDR;
	/* Prefer peripheral clock which provides higher frequency. */
	if (gd->arch.sdhc_per_clk)
		cfg->sdhc_clk = gd->arch.sdhc_per_clk;
	else
		cfg->sdhc_clk = gd->arch.sdhc_clk;
	return fsl_esdhc_initialize(bis, cfg);
}
#else /* DM_MMC */
static int fsl_esdhc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	struct mmc *mmc;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
#ifdef CONFIG_PPC
	priv->esdhc_regs = (struct fsl_esdhc *)lower_32_bits(addr);
#else
	priv->esdhc_regs = (struct fsl_esdhc *)addr;
#endif
	priv->dev = dev;

	if (gd->arch.sdhc_per_clk) {
		priv->sdhc_clk = gd->arch.sdhc_per_clk;
		priv->is_sdhc_per_clk = true;
	} else {
		priv->sdhc_clk = gd->arch.sdhc_clk;
	}

	if (priv->sdhc_clk <= 0) {
		dev_err(dev, "Unable to get clk for %s\n", dev->name);
		return -EINVAL;
	}

	fsl_esdhc_get_cfg_common(priv, &plat->cfg);

	mmc_of_parse(dev, &plat->cfg);

	mmc = &plat->mmc;
	mmc->cfg = &plat->cfg;
	mmc->dev = dev;

	upriv->mmc = mmc;

	ret = esdhc_init_common(priv, mmc);
	if (ret)
		return ret;

#ifdef CONFIG_FSL_ESDHC_33V_IO_RELIABILITY_WORKAROUND
	if (!fsl_esdhc_get_cd(dev))
		esdhc_setbits32(&priv->esdhc_regs->proctl, PROCTL_VOLT_SEL);
#endif
	return 0;
}

static int fsl_esdhc_get_cd(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	if (plat->cfg.host_caps & MMC_CAP_NONREMOVABLE)
		return 1;

	return esdhc_getcd_common(priv);
}

static int fsl_esdhc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_send_cmd_common(priv, &plat->mmc, cmd, data);
}

static int fsl_esdhc_set_ios(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_set_ios_common(priv, &plat->mmc);
}

static const struct dm_mmc_ops fsl_esdhc_ops = {
	.get_cd		= fsl_esdhc_get_cd,
	.send_cmd	= fsl_esdhc_send_cmd,
	.set_ios	= fsl_esdhc_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning = fsl_esdhc_execute_tuning,
#endif
};

static const struct udevice_id fsl_esdhc_ids[] = {
	{ .compatible = "fsl,esdhc", },
	{ /* sentinel */ }
};

static int fsl_esdhc_bind(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

U_BOOT_DRIVER(fsl_esdhc) = {
	.name	= "fsl-esdhc-mmc",
	.id	= UCLASS_MMC,
	.of_match = fsl_esdhc_ids,
	.ops	= &fsl_esdhc_ops,
	.bind	= fsl_esdhc_bind,
	.probe	= fsl_esdhc_probe,
	.platdata_auto_alloc_size = sizeof(struct fsl_esdhc_plat),
	.priv_auto_alloc_size = sizeof(struct fsl_esdhc_priv),
};
#endif
