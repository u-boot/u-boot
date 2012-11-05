/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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

DECLARE_GLOBAL_DATA_PTR;

struct fsl_esdhc {
	uint	dsaddr;
	uint	blkattr;
	uint	cmdarg;
	uint	xfertyp;
	uint	cmdrsp0;
	uint	cmdrsp1;
	uint	cmdrsp2;
	uint	cmdrsp3;
	uint	datport;
	uint	prsstat;
	uint	proctl;
	uint	sysctl;
	uint	irqstat;
	uint	irqstaten;
	uint	irqsigen;
	uint	autoc12err;
	uint	hostcapblt;
	uint	wml;
	uint    mixctrl;
	char    reserved1[4];
	uint	fevt;
	char	reserved2[168];
	uint	hostver;
	char	reserved3[780];
	uint	scr;
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

#ifdef CONFIG_MX53
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		xfertyp |= XFERTYP_CMDTYP_ABORT;
#endif
	return XFERTYP_CMD(cmd->cmdidx) | xfertyp;
}

#ifdef CONFIG_SYS_FSL_ESDHC_USE_PIO
/*
 * PIO Read/Write Mode reduce the performace as DMA is not used in this mode.
 */
static void
esdhc_pio_read_write(struct mmc *mmc, struct mmc_data *data)
{
	struct fsl_esdhc_cfg *cfg = mmc->priv;
	struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;
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
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
	uint wml_value;

	wml_value = data->blocksize/4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > WML_RD_WML_MAX)
			wml_value = WML_RD_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_RD_WML_MASK, wml_value);
		esdhc_write32(&regs->dsaddr, (u32)data->dest);
	} else {
		flush_dcache_range((ulong)data->src,
				   (ulong)data->src+data->blocks
					 *data->blocksize);

		if (wml_value > WML_WR_WML_MAX)
			wml_value = WML_WR_WML_MAX_VAL;
		if ((esdhc_read32(&regs->prsstat) & PRSSTAT_WPSPL) == 0) {
			printf("\nThe SD card is locked. Can not write to a locked card.\n\n");
			return TIMEOUT;
		}

		esdhc_clrsetbits32(&regs->wml, WML_WR_WML_MASK,
					wml_value << 16);
		esdhc_write32(&regs->dsaddr, (u32)data->src);
	}
#else	/* CONFIG_SYS_FSL_ESDHC_USE_PIO */
	if (!(data->flags & MMC_DATA_READ)) {
		if ((esdhc_read32(&regs->prsstat) & PRSSTAT_WPSPL) == 0) {
			printf("\nThe SD card is locked. "
				"Can not write to a locked card.\n\n");
			return TIMEOUT;
		}
		esdhc_write32(&regs->dsaddr, (u32)data->src);
	} else
		esdhc_write32(&regs->dsaddr, (u32)data->dest);
#endif	/* CONFIG_SYS_FSL_ESDHC_USE_PIO */

	esdhc_write32(&regs->blkattr, data->blocks << 16 | data->blocksize);

	/* Calculate the timeout period for data transactions */
	/*
	 * 1)Timeout period = (2^(timeout+13)) SD Clock cycles
	 * 2)Timeout period should be minimum 0.250sec as per SD Card spec
	 *  So, Number of SD Clock cycles for 0.25sec should be minimum
	 *		(SD Clock/sec * 0.25 sec) SD Clock cycles
	 *		= (mmc->tran_speed * 1/4) SD Clock cycles
	 * As 1) >=  2)
	 * => (2^(timeout+13)) >= mmc->tran_speed * 1/4
	 * Taking log2 both the sides
	 * => timeout + 13 >= log2(mmc->tran_speed/4)
	 * Rounding up to next power of 2
	 * => timeout + 13 = log2(mmc->tran_speed/4) + 1
	 * => timeout + 13 = fls(mmc->tran_speed/4)
	 */
	timeout = fls(mmc->tran_speed/4);
	timeout -= 13;

	if (timeout > 14)
		timeout = 14;

	if (timeout < 0)
		timeout = 0;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC_A001
	if ((timeout == 4) || (timeout == 8) || (timeout == 12))
		timeout++;
#endif

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, timeout << 16);

	return 0;
}

static void check_and_invalidate_dcache_range
	(struct mmc_cmd *cmd,
	 struct mmc_data *data) {
	unsigned start = (unsigned)data->dest ;
	unsigned size = roundup(ARCH_DMA_MINALIGN,
				data->blocks*data->blocksize);
	unsigned end = start+size ;
	invalidate_dcache_range(start, end);
}
/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int
esdhc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint	xfertyp;
	uint	irqstat;
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	volatile struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;

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
		int err;

		err = esdhc_setup_data(mmc, data);
		if(err)
			return err;
	}

	/* Figure out the transfer arguments */
	xfertyp = esdhc_xfertyp(cmd, data);

	/* Send the command */
	esdhc_write32(&regs->cmdarg, cmd->cmdarg);
#if defined(CONFIG_FSL_USDHC)
	esdhc_write32(&regs->mixctrl,
	(esdhc_read32(&regs->mixctrl) & 0xFFFFFF80) | (xfertyp & 0x7F));
	esdhc_write32(&regs->xfertyp, xfertyp & 0xFFFF0000);
#else
	esdhc_write32(&regs->xfertyp, xfertyp);
#endif

	/* Mask all irqs */
	esdhc_write32(&regs->irqsigen, 0);

	/* Wait for the command to complete */
	while (!(esdhc_read32(&regs->irqstat) & (IRQSTAT_CC | IRQSTAT_CTOE)))
		;

	if (data && (data->flags & MMC_DATA_READ))
		check_and_invalidate_dcache_range(cmd, data);

	irqstat = esdhc_read32(&regs->irqstat);
	esdhc_write32(&regs->irqstat, irqstat);

	/* Reset CMD and DATA portions on error */
	if (irqstat & (CMD_ERR | IRQSTAT_CTOE)) {
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

	if (irqstat & CMD_ERR)
		return COMM_ERR;

	if (irqstat & IRQSTAT_CTOE)
		return TIMEOUT;

	/* Workaround for ESDHC errata ENGcm03648 */
	if (!data && (cmd->resp_type & MMC_RSP_BUSY)) {
		int timeout = 2500;

		/* Poll on DATA0 line for cmd with busy signal for 250 ms */
		while (timeout > 0 && !(esdhc_read32(&regs->prsstat) &
					PRSSTAT_DAT0)) {
			udelay(100);
			timeout--;
		}

		if (timeout <= 0) {
			printf("Timeout waiting for DAT0 to go high!\n");
			return TIMEOUT;
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

			if (irqstat & IRQSTAT_DTOE)
				return TIMEOUT;

			if (irqstat & DATA_ERR)
				return COMM_ERR;
		} while (!(irqstat & IRQSTAT_TC) &&
				(esdhc_read32(&regs->prsstat) & PRSSTAT_DLA));
#endif
	}

	esdhc_write32(&regs->irqstat, -1);

	return 0;
}

static void set_sysctl(struct mmc *mmc, uint clock)
{
	int div, pre_div;
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	volatile struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;
	int sdhc_clk = cfg->sdhc_clk;
	uint clk;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	if (sdhc_clk / 16 > clock) {
		for (pre_div = 2; pre_div < 256; pre_div *= 2)
			if ((sdhc_clk / pre_div) <= (clock * 16))
				break;
	} else
		pre_div = 2;

	for (div = 1; div <= 16; div++)
		if ((sdhc_clk / (div * pre_div)) <= clock)
			break;

	pre_div >>= 1;
	div -= 1;

	clk = (pre_div << 8) | (div << 4);

	esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	udelay(10000);

	clk = SYSCTL_PEREN | SYSCTL_CKEN;

	esdhc_setbits32(&regs->sysctl, clk);
}

static void esdhc_set_ios(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;

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
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;
	int timeout = 1000;

	/* Reset the entire host controller */
	esdhc_write32(&regs->sysctl, SYSCTL_RSTA);

	/* Wait until the controller is available */
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA) && --timeout)
		udelay(1000);

#ifndef ARCH_MXC
	/* Enable cache snooping */
	esdhc_write32(&regs->scr, 0x00000040);
#endif

	esdhc_write32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

	/* Set the initial clock speed */
	mmc_set_clock(mmc, 400000);

	/* Disable the BRR and BWR bits in IRQSTAT */
	esdhc_clrbits32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

	/* Put the PROCTL reg back to the default */
	esdhc_write32(&regs->proctl, PROCTL_INIT);

	/* Set timout to the maximum value */
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

	return 0;
}

static int esdhc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	struct fsl_esdhc *regs = (struct fsl_esdhc *)cfg->esdhc_base;
	int timeout = 1000;

	while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_CINS) && --timeout)
		udelay(1000);

	return timeout > 0;
}

static void esdhc_reset(struct fsl_esdhc *regs)
{
	unsigned long timeout = 100; /* wait max 100 ms */

	/* reset the controller */
	esdhc_write32(&regs->sysctl, SYSCTL_RSTA);

	/* hardware clears the bit when it is done */
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA) && --timeout)
		udelay(1000);
	if (!timeout)
		printf("MMC/SD: Reset never completed.\n");
}

int fsl_esdhc_initialize(bd_t *bis, struct fsl_esdhc_cfg *cfg)
{
	struct fsl_esdhc *regs;
	struct mmc *mmc;
	u32 caps, voltage_caps;

	if (!cfg)
		return -1;

	mmc = malloc(sizeof(struct mmc));

	sprintf(mmc->name, "FSL_SDHC");
	regs = (struct fsl_esdhc *)cfg->esdhc_base;

	/* First reset the eSDHC controller */
	esdhc_reset(regs);

	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_HCKEN
				| SYSCTL_IPGEN | SYSCTL_CKEN);

	mmc->priv = cfg;
	mmc->send_cmd = esdhc_send_cmd;
	mmc->set_ios = esdhc_set_ios;
	mmc->init = esdhc_init;
	mmc->getcd = esdhc_getcd;

	voltage_caps = 0;
	caps = regs->hostcapblt;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC135
	caps = caps & ~(ESDHC_HOSTCAPBLT_SRS |
			ESDHC_HOSTCAPBLT_VS18 | ESDHC_HOSTCAPBLT_VS30);
#endif
	if (caps & ESDHC_HOSTCAPBLT_VS18)
		voltage_caps |= MMC_VDD_165_195;
	if (caps & ESDHC_HOSTCAPBLT_VS30)
		voltage_caps |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & ESDHC_HOSTCAPBLT_VS33)
		voltage_caps |= MMC_VDD_32_33 | MMC_VDD_33_34;

#ifdef CONFIG_SYS_SD_VOLTAGE
	mmc->voltages = CONFIG_SYS_SD_VOLTAGE;
#else
	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
#endif
	if ((mmc->voltages & voltage_caps) == 0) {
		printf("voltage not supported by controller\n");
		return -1;
	}

	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;

	if (caps & ESDHC_HOSTCAPBLT_HSS)
		mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	mmc->f_min = 400000;
	mmc->f_max = MIN(gd->sdhc_clk, 52000000);

	mmc->b_max = 0;
	mmc_register(mmc);

	return 0;
}

int fsl_esdhc_mmc_init(bd_t *bis)
{
	struct fsl_esdhc_cfg *cfg;

	cfg = malloc(sizeof(struct fsl_esdhc_cfg));
	memset(cfg, 0, sizeof(struct fsl_esdhc_cfg));
	cfg->esdhc_base = CONFIG_SYS_FSL_ESDHC_ADDR;
	cfg->sdhc_clk = gd->sdhc_clk;
	return fsl_esdhc_initialize(bis, cfg);
}

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

	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->sdhc_clk, 1);

	do_fixup_by_compat(blob, compat, "status", "okay",
			   4 + 1, 1);
}
#endif
