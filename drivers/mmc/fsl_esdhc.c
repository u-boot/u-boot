/*
 * Copyright 2007, Freescale Semiconductor, Inc
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
	char	reserved1[8];
	uint	fevt;
	char	reserved2[168];
	uint	hostver;
	char	reserved3[780];
	uint	scr;
};

/* Return the XFERTYP flags for a given command and data packet */
uint esdhc_xfertyp(struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint xfertyp = 0;

	if (data) {
		xfertyp |= XFERTYP_DPSEL | XFERTYP_DMAEN;

		if (data->blocks > 1) {
			xfertyp |= XFERTYP_MSBSEL;
			xfertyp |= XFERTYP_BCEN;
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

	return XFERTYP_CMD(cmd->cmdidx) | xfertyp;
}

static int esdhc_setup_data(struct mmc *mmc, struct mmc_data *data)
{
	uint wml_value;
	int timeout;
	struct fsl_esdhc *regs = mmc->priv;

	wml_value = data->blocksize/4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > 0x10)
			wml_value = 0x10;

		wml_value = 0x100000 | wml_value;

		out_be32(&regs->dsaddr, (u32)data->dest);
	} else {
		if (wml_value > 0x80)
			wml_value = 0x80;
		if ((in_be32(&regs->prsstat) & PRSSTAT_WPSPL) == 0) {
			printf("\nThe SD card is locked. Can not write to a locked card.\n\n");
			return TIMEOUT;
		}
		wml_value = wml_value << 16 | 0x10;
		out_be32(&regs->dsaddr, (u32)data->src);
	}

	out_be32(&regs->wml, wml_value);

	out_be32(&regs->blkattr, data->blocks << 16 | data->blocksize);

	/* Calculate the timeout period for data transactions */
	timeout = __ilog2(mmc->tran_speed/10);
	timeout -= 13;

	if (timeout > 14)
		timeout = 14;

	if (timeout < 0)
		timeout = 0;

	clrsetbits_be32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, timeout << 16);

	return 0;
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
	volatile struct fsl_esdhc *regs = mmc->priv;

	out_be32(&regs->irqstat, -1);

	sync();

	/* Wait for the bus to be idle */
	while ((in_be32(&regs->prsstat) & PRSSTAT_CICHB) ||
			(in_be32(&regs->prsstat) & PRSSTAT_CIDHB));

	while (in_be32(&regs->prsstat) & PRSSTAT_DLA);

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
	out_be32(&regs->cmdarg, cmd->cmdarg);
	out_be32(&regs->xfertyp, xfertyp);

	/* Wait for the command to complete */
	while (!(in_be32(&regs->irqstat) & IRQSTAT_CC));

	irqstat = in_be32(&regs->irqstat);
	out_be32(&regs->irqstat, irqstat);

	if (irqstat & CMD_ERR)
		return COMM_ERR;

	if (irqstat & IRQSTAT_CTOE)
		return TIMEOUT;

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		u32 cmdrsp3, cmdrsp2, cmdrsp1, cmdrsp0;

		cmdrsp3 = in_be32(&regs->cmdrsp3);
		cmdrsp2 = in_be32(&regs->cmdrsp2);
		cmdrsp1 = in_be32(&regs->cmdrsp1);
		cmdrsp0 = in_be32(&regs->cmdrsp0);
		cmd->response[0] = (cmdrsp3 << 8) | (cmdrsp2 >> 24);
		cmd->response[1] = (cmdrsp2 << 8) | (cmdrsp1 >> 24);
		cmd->response[2] = (cmdrsp1 << 8) | (cmdrsp0 >> 24);
		cmd->response[3] = (cmdrsp0 << 8);
	} else
		cmd->response[0] = in_be32(&regs->cmdrsp0);

	/* Wait until all of the blocks are transferred */
	if (data) {
		do {
			irqstat = in_be32(&regs->irqstat);

			if (irqstat & DATA_ERR)
				return COMM_ERR;

			if (irqstat & IRQSTAT_DTOE)
				return TIMEOUT;
		} while (!(irqstat & IRQSTAT_TC) &&
				(in_be32(&regs->prsstat) & PRSSTAT_DLA));
	}

	out_be32(&regs->irqstat, -1);

	return 0;
}

void set_sysctl(struct mmc *mmc, uint clock)
{
	int sdhc_clk = gd->sdhc_clk;
	int div, pre_div;
	volatile struct fsl_esdhc *regs = mmc->priv;
	uint clk;

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

	clrsetbits_be32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	udelay(10000);

	setbits_be32(&regs->sysctl, SYSCTL_PEREN);
}

static void esdhc_set_ios(struct mmc *mmc)
{
	struct fsl_esdhc *regs = mmc->priv;

	/* Set the clock speed */
	set_sysctl(mmc, mmc->clock);

	/* Set the bus width */
	clrbits_be32(&regs->proctl, PROCTL_DTW_4 | PROCTL_DTW_8);

	if (mmc->bus_width == 4)
		setbits_be32(&regs->proctl, PROCTL_DTW_4);
	else if (mmc->bus_width == 8)
		setbits_be32(&regs->proctl, PROCTL_DTW_8);
}

static int esdhc_init(struct mmc *mmc)
{
	struct fsl_esdhc *regs = mmc->priv;
	int timeout = 1000;

	/* Enable cache snooping */
	out_be32(&regs->scr, 0x00000040);

	out_be32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

	/* Set the initial clock speed */
	set_sysctl(mmc, 400000);

	/* Disable the BRR and BWR bits in IRQSTAT */
	clrbits_be32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

	/* Put the PROCTL reg back to the default */
	out_be32(&regs->proctl, PROCTL_INIT);

	while (!(in_be32(&regs->prsstat) & PRSSTAT_CINS) && --timeout)
		udelay(1000);

	if (timeout <= 0)
		return NO_CARD_ERR;

	return 0;
}

static int esdhc_initialize(bd_t *bis)
{
	struct fsl_esdhc *regs = (struct fsl_esdhc *)CONFIG_SYS_FSL_ESDHC_ADDR;
	struct mmc *mmc;
	u32 caps;

	mmc = malloc(sizeof(struct mmc));

	sprintf(mmc->name, "FSL_ESDHC");
	mmc->priv = regs;
	mmc->send_cmd = esdhc_send_cmd;
	mmc->set_ios = esdhc_set_ios;
	mmc->init = esdhc_init;

	caps = regs->hostcapblt;

	if (caps & ESDHC_HOSTCAPBLT_VS18)
		mmc->voltages |= MMC_VDD_165_195;
	if (caps & ESDHC_HOSTCAPBLT_VS30)
		mmc->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & ESDHC_HOSTCAPBLT_VS33)
		mmc->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;

	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;

	if (caps & ESDHC_HOSTCAPBLT_HSS)
		mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	mmc->f_min = 400000;
	mmc->f_max = MIN(gd->sdhc_clk, 50000000);

	mmc_register(mmc);

	return 0;
}

int fsl_esdhc_mmc_init(bd_t *bis)
{
	return esdhc_initialize(bis);
}

void fdt_fixup_esdhc(void *blob, bd_t *bd)
{
	const char *compat = "fsl,esdhc";
	const char *status = "okay";

	if (!hwconfig("esdhc")) {
		status = "disabled";
		goto out;
	}

	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->sdhc_clk, 1);
out:
	do_fixup_by_compat(blob, compat, "status", status,
			   strlen(status) + 1, 1);
}
