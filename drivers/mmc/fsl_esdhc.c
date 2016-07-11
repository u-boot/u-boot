/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <hwconfig.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <dm.h>
#include <asm-generic/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#define SDHCI_IRQ_EN_BITS		(IRQSTATEN_CC | IRQSTATEN_TC | \
				IRQSTATEN_CINT | \
				IRQSTATEN_CTOE | IRQSTATEN_CCE | IRQSTATEN_CEBE | \
				IRQSTATEN_CIE | IRQSTATEN_DTOE | IRQSTATEN_DCE | \
				IRQSTATEN_DEBE | IRQSTATEN_BRR | IRQSTATEN_BWR | \
				IRQSTATEN_DINT)

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
	uint    mixctrl;	/* For USDHC */
	char    reserved1[4];	/* reserved */
	uint    fevt;		/* Force event register */
	uint    admaes;		/* ADMA error status register */
	uint    adsaddr;	/* ADMA system address register */
	char    reserved2[100];	/* reserved */
	uint    vendorspec;	/* Vendor Specific register */
	char    reserved3[56];	/* reserved */
	uint    hostver;	/* Host controller version register */
	char    reserved4[4];	/* reserved */
	uint    dmaerraddr;	/* DMA error address register */
	char    reserved5[4];	/* reserved */
	uint    dmaerrattr;	/* DMA error attribute register */
	char    reserved6[4];	/* reserved */
	uint    hostcapblt2;	/* Host controller capabilities register 2 */
	char    reserved7[8];	/* reserved */
	uint    tcr;		/* Tuning control register */
	char    reserved8[28];	/* reserved */
	uint    sddirctl;	/* SD direction control register */
	char    reserved9[712];	/* reserved */
	uint    scr;		/* eSDHC control register */
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
 * @non_removable: 0: removable; 1: non-removable
 * @cd_gpio: gpio for card detection
 */
struct fsl_esdhc_priv {
	struct fsl_esdhc *esdhc_regs;
	unsigned int sdhc_clk;
	unsigned int bus_width;
	struct mmc_config cfg;
	struct mmc *mmc;
	struct udevice *dev;
	int non_removable;
	struct gpio_desc cd_gpio;
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
static void
esdhc_pio_read_write(struct mmc *mmc, struct mmc_data *data)
{
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	uint blocks;
	char *buffer;
	uint databuf;
	uint size;
	uint irqstat;
	uint timeout;

	if (data->flags & MMC_DATA_READ) {
		blocks = data->blocks;
		buffer = data->dest;
		while (blocks) {
			timeout = PIO_TIMEOUT;
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BREN)
				&& --timeout);
			if (timeout <= 0) {
				printf("\nData Read Failed in PIO Mode.");
				return;
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
			timeout = PIO_TIMEOUT;
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BWEN)
				&& --timeout);
			if (timeout <= 0) {
				printf("\nData Write Failed in PIO Mode.");
				return;
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

static int esdhc_setup_data(struct mmc *mmc, struct mmc_data *data)
{
	int timeout;
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;
#ifdef CONFIG_FSL_LAYERSCAPE
	dma_addr_t addr;
#endif
	uint wml_value;

	wml_value = data->blocksize/4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > WML_RD_WML_MAX)
			wml_value = WML_RD_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_RD_WML_MASK, wml_value);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#ifdef CONFIG_FSL_LAYERSCAPE
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
		if ((esdhc_read32(&regs->prsstat) & PRSSTAT_WPSPL) == 0) {
			printf("\nThe SD card is locked. Can not write to a locked card.\n\n");
			return TIMEOUT;
		}

		esdhc_clrsetbits32(&regs->wml, WML_WR_WML_MASK,
					wml_value << 16);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#ifdef CONFIG_FSL_LAYERSCAPE
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
#ifdef CONFIG_FSL_LAYERSCAPE
	unsigned start = 0;
#else
	unsigned start = (unsigned)data->dest ;
#endif
	unsigned size = roundup(ARCH_DMA_MINALIGN,
				data->blocks*data->blocksize);
	unsigned end = start+size ;
#ifdef CONFIG_FSL_LAYERSCAPE
	dma_addr_t addr;

	addr = virt_to_phys((void *)(data->dest));
	if (upper_32_bits(addr))
		printf("Error found for upper 32 bits\n");
	else
		start = lower_32_bits(addr);
#endif
	invalidate_dcache_range(start, end);
}

/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int
esdhc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int	err = 0;
	uint	xfertyp;
	uint	irqstat;
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;

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
		err = esdhc_setup_data(mmc, data);
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
#if defined(CONFIG_FSL_USDHC)
	esdhc_write32(&regs->mixctrl,
	(esdhc_read32(&regs->mixctrl) & 0xFFFFFF80) | (xfertyp & 0x7F)
			| (mmc->ddr_mode ? XFERTYP_DDREN : 0));
	esdhc_write32(&regs->xfertyp, xfertyp & 0xFFFF0000);
#else
	esdhc_write32(&regs->xfertyp, xfertyp);
#endif

	/* Wait for the command to complete */
	while (!(esdhc_read32(&regs->irqstat) & (IRQSTAT_CC | IRQSTAT_CTOE)))
		;

	irqstat = esdhc_read32(&regs->irqstat);

	if (irqstat & CMD_ERR) {
		err = COMM_ERR;
		goto out;
	}

	if (irqstat & IRQSTAT_CTOE) {
		err = TIMEOUT;
		goto out;
	}

	/* Switch voltage to 1.8V if CMD11 succeeded */
	if (cmd->cmdidx == SD_CMD_SWITCH_UHS18V) {
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);

		printf("Run CMD11 1.8V switch\n");
		/* Sleep for 5 ms - max time for card to switch to 1.8V */
		udelay(5000);
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
			err = TIMEOUT;
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
		esdhc_pio_read_write(mmc, data);
#else
		do {
			irqstat = esdhc_read32(&regs->irqstat);

			if (irqstat & IRQSTAT_DTOE) {
				err = TIMEOUT;
				goto out;
			}

			if (irqstat & DATA_ERR) {
				err = COMM_ERR;
				goto out;
			}
		} while ((irqstat & DATA_COMPLETE) != DATA_COMPLETE);

		/*
		 * Need invalidate the dcache here again to avoid any
		 * cache-fill during the DMA operations such as the
		 * speculative pre-fetching etc.
		 */
		if (data->flags & MMC_DATA_READ)
			check_and_invalidate_dcache_range(cmd, data);
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

		/* If this was CMD11, then notify that power cycle is needed */
		if (cmd->cmdidx == SD_CMD_SWITCH_UHS18V)
			printf("CMD11 to switch to 1.8V mode failed, card requires power cycle.\n");
	}

	esdhc_write32(&regs->irqstat, -1);

	return err;
}

static void set_sysctl(struct mmc *mmc, uint clock)
{
	int div, pre_div;
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int sdhc_clk = priv->sdhc_clk;
	uint clk;

	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	if (sdhc_clk / 16 > clock) {
		for (pre_div = 2; pre_div < 256; pre_div *= 2)
			if ((sdhc_clk / pre_div) <= (clock * 16))
				break;
	} else
		pre_div = 2;

	for (div = 1; div <= 16; div++)
		if ((sdhc_clk / (div * pre_div)) <= clock)
			break;

	pre_div >>= mmc->ddr_mode ? 2 : 1;
	div -= 1;

	clk = (pre_div << 8) | (div << 4);

#ifdef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);
#else
	esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);
#endif

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	udelay(10000);

#ifdef CONFIG_FSL_USDHC
	esdhc_clrbits32(&regs->sysctl, SYSCTL_RSTA);
#else
	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);
#endif

}

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
static void esdhc_clock_control(struct mmc *mmc, bool enable)
{
	struct fsl_esdhc_priv *priv = mmc->priv;
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
#endif

static void esdhc_set_ios(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
	/* Select to use peripheral clock */
	esdhc_clock_control(mmc, false);
	esdhc_setbits32(&regs->scr, ESDHCCTL_PCS);
	esdhc_clock_control(mmc, true);
#endif
	/* Set the clock speed */
	set_sysctl(mmc, mmc->clock);

	/* Set the bus width */
	esdhc_clrbits32(&regs->proctl, PROCTL_DTW_4 | PROCTL_DTW_8);

	if (mmc->bus_width == 4)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_4);
	else if (mmc->bus_width == 8)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_8);

}

static int esdhc_init(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int timeout = 1000;

	/* Reset the entire host controller */
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);

	/* Wait until the controller is available */
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA) && --timeout)
		udelay(1000);

#ifndef ARCH_MXC
	/* Enable cache snooping */
	esdhc_write32(&regs->scr, 0x00000040);
#endif

#ifndef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);
#endif

	/* Set the initial clock speed */
	mmc_set_clock(mmc, 400000);

	/* Disable the BRR and BWR bits in IRQSTAT */
	esdhc_clrbits32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

	/* Put the PROCTL reg back to the default */
	esdhc_write32(&regs->proctl, PROCTL_INIT);

	/* Set timout to the maximum value */
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

#ifdef CONFIG_SYS_FSL_ESDHC_FORCE_VSELECT
	esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);
#endif

	return 0;
}

static int esdhc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int timeout = 1000;

#ifdef CONFIG_ESDHC_DETECT_QUIRK
	if (CONFIG_ESDHC_DETECT_QUIRK)
		return 1;
#endif

#ifdef CONFIG_DM_MMC
	if (priv->non_removable)
		return 1;

	if (dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);
#endif

	while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_CINS) && --timeout)
		udelay(1000);

	return timeout > 0;
}

static void esdhc_reset(struct fsl_esdhc *regs)
{
	unsigned long timeout = 100; /* wait max 100 ms */

	/* reset the controller */
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);

	/* hardware clears the bit when it is done */
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA) && --timeout)
		udelay(1000);
	if (!timeout)
		printf("MMC/SD: Reset never completed.\n");
}

static const struct mmc_ops esdhc_ops = {
	.send_cmd	= esdhc_send_cmd,
	.set_ios	= esdhc_set_ios,
	.init		= esdhc_init,
	.getcd		= esdhc_getcd,
};

static int fsl_esdhc_cfg_to_priv(struct fsl_esdhc_cfg *cfg,
				 struct fsl_esdhc_priv *priv)
{
	if (!cfg || !priv)
		return -EINVAL;

	priv->esdhc_regs = (struct fsl_esdhc *)(unsigned long)(cfg->esdhc_base);
	priv->bus_width = cfg->max_bus_width;
	priv->sdhc_clk = cfg->sdhc_clk;

	return 0;
};

static int fsl_esdhc_init(struct fsl_esdhc_priv *priv)
{
	struct fsl_esdhc *regs;
	struct mmc *mmc;
	u32 caps, voltage_caps;

	if (!priv)
		return -EINVAL;

	regs = priv->esdhc_regs;

	/* First reset the eSDHC controller */
	esdhc_reset(regs);

#ifndef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_HCKEN
				| SYSCTL_IPGEN | SYSCTL_CKEN);
#endif

	writel(SDHCI_IRQ_EN_BITS, &regs->irqstaten);
	memset(&priv->cfg, 0, sizeof(priv->cfg));

	voltage_caps = 0;
	caps = esdhc_read32(&regs->hostcapblt);

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC135
	caps = caps & ~(ESDHC_HOSTCAPBLT_SRS |
			ESDHC_HOSTCAPBLT_VS18 | ESDHC_HOSTCAPBLT_VS30);
#endif

/* T4240 host controller capabilities register should have VS33 bit */
#ifdef CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
	caps = caps | ESDHC_HOSTCAPBLT_VS33;
#endif

	if (caps & ESDHC_HOSTCAPBLT_VS18)
		voltage_caps |= MMC_VDD_165_195;
	if (caps & ESDHC_HOSTCAPBLT_VS30)
		voltage_caps |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & ESDHC_HOSTCAPBLT_VS33)
		voltage_caps |= MMC_VDD_32_33 | MMC_VDD_33_34;

	priv->cfg.name = "FSL_SDHC";
	priv->cfg.ops = &esdhc_ops;
#ifdef CONFIG_SYS_SD_VOLTAGE
	priv->cfg.voltages = CONFIG_SYS_SD_VOLTAGE;
#else
	priv->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
#endif
	if ((priv->cfg.voltages & voltage_caps) == 0) {
		printf("voltage not supported by controller\n");
		return -1;
	}

	if (priv->bus_width == 8)
		priv->cfg.host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;
	else if (priv->bus_width == 4)
		priv->cfg.host_caps = MMC_MODE_4BIT;

	priv->cfg.host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;
#ifdef CONFIG_SYS_FSL_ESDHC_HAS_DDR_MODE
	priv->cfg.host_caps |= MMC_MODE_DDR_52MHz;
#endif

	if (priv->bus_width > 0) {
		if (priv->bus_width < 8)
			priv->cfg.host_caps &= ~MMC_MODE_8BIT;
		if (priv->bus_width < 4)
			priv->cfg.host_caps &= ~MMC_MODE_4BIT;
	}

	if (caps & ESDHC_HOSTCAPBLT_HSS)
		priv->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

#ifdef CONFIG_ESDHC_DETECT_8_BIT_QUIRK
	if (CONFIG_ESDHC_DETECT_8_BIT_QUIRK)
		priv->cfg.host_caps &= ~MMC_MODE_8BIT;
#endif

	priv->cfg.f_min = 400000;
	priv->cfg.f_max = min(priv->sdhc_clk, (u32)52000000);

	priv->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	mmc = mmc_create(&priv->cfg, priv);
	if (mmc == NULL)
		return -1;

	priv->mmc = mmc;

	return 0;
}

int fsl_esdhc_initialize(bd_t *bis, struct fsl_esdhc_cfg *cfg)
{
	struct fsl_esdhc_priv *priv;
	int ret;

	if (!cfg)
		return -EINVAL;

	priv = calloc(sizeof(struct fsl_esdhc_priv), 1);
	if (!priv)
		return -ENOMEM;

	ret = fsl_esdhc_cfg_to_priv(cfg, priv);
	if (ret) {
		debug("%s xlate failure\n", __func__);
		free(priv);
		return ret;
	}

	ret = fsl_esdhc_init(priv);
	if (ret) {
		debug("%s init failure\n", __func__);
		free(priv);
		return ret;
	}

	return 0;
}

int fsl_esdhc_mmc_init(bd_t *bis)
{
	struct fsl_esdhc_cfg *cfg;

	cfg = calloc(sizeof(struct fsl_esdhc_cfg), 1);
	cfg->esdhc_base = CONFIG_SYS_FSL_ESDHC_ADDR;
	cfg->sdhc_clk = gd->arch.sdhc_clk;
	return fsl_esdhc_initialize(bis, cfg);
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
void fdt_fixup_esdhc(void *blob, bd_t *bd)
{
	const char *compat = "fsl,esdhc";

#ifdef CONFIG_FSL_ESDHC_PIN_MUX
	if (!hwconfig("esdhc")) {
		do_fixup_by_compat(blob, compat, "status", "disabled",
				8 + 1, 1);
		return;
	}
#endif

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
	do_fixup_by_compat_u32(blob, compat, "peripheral-frequency",
			       gd->arch.sdhc_clk, 1);
#else
	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->arch.sdhc_clk, 1);
#endif
#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
	do_fixup_by_compat_u32(blob, compat, "adapter-type",
			       (u32)(gd->arch.sdhc_adapter), 1);
#endif
	do_fixup_by_compat(blob, compat, "status", "okay",
			   4 + 1, 1);
}
#endif

#ifdef CONFIG_DM_MMC
#include <asm/arch/clock.h>
static int fsl_esdhc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev->of_offset;
	fdt_addr_t addr;
	unsigned int val;
	int ret;

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->esdhc_regs = (struct fsl_esdhc *)addr;
	priv->dev = dev;

	val = fdtdec_get_int(fdt, node, "bus-width", -1);
	if (val == 8)
		priv->bus_width = 8;
	else if (val == 4)
		priv->bus_width = 4;
	else
		priv->bus_width = 1;

	if (fdt_get_property(fdt, node, "non-removable", NULL)) {
		priv->non_removable = 1;
	 } else {
		priv->non_removable = 0;
		gpio_request_by_name_nodev(fdt, node, "cd-gpios", 0,
					   &priv->cd_gpio, GPIOD_IS_IN);
	}

	/*
	 * TODO:
	 * Because lack of clk driver, if SDHC clk is not enabled,
	 * need to enable it first before this driver is invoked.
	 *
	 * we use MXC_ESDHC_CLK to get clk freq.
	 * If one would like to make this function work,
	 * the aliases should be provided in dts as this:
	 *
	 *  aliases {
	 *	mmc0 = &usdhc1;
	 *	mmc1 = &usdhc2;
	 *	mmc2 = &usdhc3;
	 *	mmc3 = &usdhc4;
	 *	};
	 * Then if your board only supports mmc2 and mmc3, but we can
	 * correctly get the seq as 2 and 3, then let mxc_get_clock
	 * work as expected.
	 */
	priv->sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK + dev->seq);
	if (priv->sdhc_clk <= 0) {
		dev_err(dev, "Unable to get clk for %s\n", dev->name);
		return -EINVAL;
	}

	ret = fsl_esdhc_init(priv);
	if (ret) {
		dev_err(dev, "fsl_esdhc_init failure\n");
		return ret;
	}

	upriv->mmc = priv->mmc;

	return 0;
}

static const struct udevice_id fsl_esdhc_ids[] = {
	{ .compatible = "fsl,imx6ul-usdhc", },
	{ .compatible = "fsl,imx6sx-usdhc", },
	{ .compatible = "fsl,imx6sl-usdhc", },
	{ .compatible = "fsl,imx6q-usdhc", },
	{ .compatible = "fsl,imx7d-usdhc", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(fsl_esdhc) = {
	.name	= "fsl-esdhc-mmc",
	.id	= UCLASS_MMC,
	.of_match = fsl_esdhc_ids,
	.probe	= fsl_esdhc_probe,
	.priv_auto_alloc_size = sizeof(struct fsl_esdhc_priv),
};
#endif
