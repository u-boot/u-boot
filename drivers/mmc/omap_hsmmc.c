/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Sukumar Ghorai <s-ghorai@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
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
#include <mmc.h>
#include <part.h>
#include <i2c.h>
#include <twl4030.h>
#include <twl6030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>

/* If we fail after 1 second wait, something is really bad */
#define MAX_RETRY_MS	1000

static int mmc_read_data(struct hsmmc *mmc_base, char *buf, unsigned int size);
static int mmc_write_data(struct hsmmc *mmc_base, const char *buf,
			unsigned int siz);
static struct mmc hsmmc_dev[2];

#if defined(CONFIG_OMAP44XX) && defined(CONFIG_TWL6030_POWER)
static void omap4_vmmc_pbias_config(struct mmc *mmc)
{
	u32 value = 0;
	struct omap4_sys_ctrl_regs *const ctrl =
		(struct omap4_sys_ctrl_regs *)SYSCTRL_GENERAL_CORE_BASE;


	value = readl(&ctrl->control_pbiaslite);
	value &= ~(MMC1_PBIASLITE_PWRDNZ | MMC1_PWRDNZ);
	writel(value, &ctrl->control_pbiaslite);
	/* set VMMC to 3V */
	twl6030_power_mmc_init();
	value = readl(&ctrl->control_pbiaslite);
	value |= MMC1_PBIASLITE_VMODE | MMC1_PBIASLITE_PWRDNZ | MMC1_PWRDNZ;
	writel(value, &ctrl->control_pbiaslite);
}
#endif

unsigned char mmc_board_init(struct mmc *mmc)
{
#if defined(CONFIG_TWL4030_POWER)
	twl4030_power_mmc_init();
#endif

#if defined(CONFIG_OMAP34XX)
	t2_t *t2_base = (t2_t *)T2_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	writel(readl(&t2_base->pbias_lite) | PBIASLITEPWRDNZ1 |
		PBIASSPEEDCTRL0 | PBIASLITEPWRDNZ0,
		&t2_base->pbias_lite);

	writel(readl(&t2_base->devconf0) | MMCSDIO1ADPCLKISEL,
		&t2_base->devconf0);

	writel(readl(&t2_base->devconf1) | MMCSDIO2ADPCLKISEL,
		&t2_base->devconf1);

	writel(readl(&prcm_base->fclken1_core) |
		EN_MMC1 | EN_MMC2 | EN_MMC3,
		&prcm_base->fclken1_core);

	writel(readl(&prcm_base->iclken1_core) |
		EN_MMC1 | EN_MMC2 | EN_MMC3,
		&prcm_base->iclken1_core);
#endif

#if defined(CONFIG_OMAP44XX) && defined(CONFIG_TWL6030_POWER)
	/* PBIAS config needed for MMC1 only */
	if (mmc->block_dev.dev == 0)
		omap4_vmmc_pbias_config(mmc);
#endif

	return 0;
}

void mmc_init_stream(struct hsmmc *mmc_base)
{
	ulong start;

	writel(readl(&mmc_base->con) | INIT_INITSTREAM, &mmc_base->con);

	writel(MMC_CMD0, &mmc_base->cmd);
	start = get_timer(0);
	while (!(readl(&mmc_base->stat) & CC_MASK)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc!\n", __func__);
			return;
		}
	}
	writel(CC_MASK, &mmc_base->stat)
		;
	writel(MMC_CMD0, &mmc_base->cmd)
		;
	start = get_timer(0);
	while (!(readl(&mmc_base->stat) & CC_MASK)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc2!\n", __func__);
			return;
		}
	}
	writel(readl(&mmc_base->con) & ~INIT_INITSTREAM, &mmc_base->con);
}


static int mmc_init_setup(struct mmc *mmc)
{
	struct hsmmc *mmc_base = (struct hsmmc *)mmc->priv;
	unsigned int reg_val;
	unsigned int dsor;
	ulong start;

	mmc_board_init(mmc);

	writel(readl(&mmc_base->sysconfig) | MMC_SOFTRESET,
		&mmc_base->sysconfig);
	start = get_timer(0);
	while ((readl(&mmc_base->sysstatus) & RESETDONE) == 0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc2!\n", __func__);
			return TIMEOUT;
		}
	}
	writel(readl(&mmc_base->sysctl) | SOFTRESETALL, &mmc_base->sysctl);
	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & SOFTRESETALL) != 0x0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for softresetall!\n",
				__func__);
			return TIMEOUT;
		}
	}
	writel(DTW_1_BITMODE | SDBP_PWROFF | SDVS_3V0, &mmc_base->hctl);
	writel(readl(&mmc_base->capa) | VS30_3V0SUP | VS18_1V8SUP,
		&mmc_base->capa);

	reg_val = readl(&mmc_base->con) & RESERVED_MASK;

	writel(CTPL_MMC_SD | reg_val | WPP_ACTIVEHIGH | CDP_ACTIVEHIGH |
		MIT_CTO | DW8_1_4BITMODE | MODE_FUNC | STR_BLOCK |
		HR_NOHOSTRESP | INIT_NOINIT | NOOPENDRAIN, &mmc_base->con);

	dsor = 240;
	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK | CEN_MASK),
		(ICE_STOP | DTO_15THDTO | CEN_DISABLE));
	mmc_reg_out(&mmc_base->sysctl, ICE_MASK | CLKD_MASK,
		(dsor << CLKD_OFFSET) | ICE_OSCILLATE);
	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for ics!\n", __func__);
			return TIMEOUT;
		}
	}
	writel(readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);

	writel(readl(&mmc_base->hctl) | SDBP_PWRON, &mmc_base->hctl);

	writel(IE_BADA | IE_CERR | IE_DEB | IE_DCRC | IE_DTO | IE_CIE |
		IE_CEB | IE_CCRC | IE_CTO | IE_BRR | IE_BWR | IE_TC | IE_CC,
		&mmc_base->ie);

	mmc_init_stream(mmc_base);

	return 0;
}


static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct hsmmc *mmc_base = (struct hsmmc *)mmc->priv;
	unsigned int flags, mmc_stat;
	ulong start;

	start = get_timer(0);
	while ((readl(&mmc_base->pstate) & (DATI_MASK | CMDI_MASK)) != 0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting on cmd inhibit to clear\n",
					__func__);
			return TIMEOUT;
		}
	}
	writel(0xFFFFFFFF, &mmc_base->stat);
	start = get_timer(0);
	while (readl(&mmc_base->stat)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for stat!\n", __func__);
			return TIMEOUT;
		}
	}
	/*
	 * CMDREG
	 * CMDIDX[13:8]	: Command index
	 * DATAPRNT[5]	: Data Present Select
	 * ENCMDIDX[4]	: Command Index Check Enable
	 * ENCMDCRC[3]	: Command CRC Check Enable
	 * RSPTYP[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	/* Delay added before checking the status of frq change
	 * retry not supported by mmc.c(core file)
	 */
	if (cmd->cmdidx == SD_CMD_APP_SEND_SCR)
		udelay(50000); /* wait 50 ms */

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = 0;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = RSP_TYPE_LGHT136 | CICE_NOCHECK;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = RSP_TYPE_LGHT48B;
	else
		flags = RSP_TYPE_LGHT48;

	/* enable default flags */
	flags =	flags | (CMD_TYPE_NORMAL | CICE_NOCHECK | CCCE_NOCHECK |
			MSBS_SGLEBLK | ACEN_DISABLE | BCE_DISABLE | DE_DISABLE);

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= CCCE_CHECK;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= CICE_CHECK;

	if (data) {
		if ((cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK) ||
			 (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK)) {
			flags |= (MSBS_MULTIBLK | BCE_ENABLE);
			data->blocksize = 512;
			writel(data->blocksize | (data->blocks << 16),
							&mmc_base->blk);
		} else
			writel(data->blocksize | NBLK_STPCNT, &mmc_base->blk);

		if (data->flags & MMC_DATA_READ)
			flags |= (DP_DATA | DDIR_READ);
		else
			flags |= (DP_DATA | DDIR_WRITE);
	}

	writel(cmd->cmdarg, &mmc_base->arg);
	writel((cmd->cmdidx << 24) | flags, &mmc_base->cmd);

	start = get_timer(0);
	do {
		mmc_stat = readl(&mmc_base->stat);
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s : timeout: No status update\n", __func__);
			return TIMEOUT;
		}
	} while (!mmc_stat);

	if ((mmc_stat & IE_CTO) != 0)
		return TIMEOUT;
	else if ((mmc_stat & ERRI_MASK) != 0)
		return -1;

	if (mmc_stat & CC_MASK) {
		writel(CC_MASK, &mmc_base->stat);
		if (cmd->resp_type & MMC_RSP_PRESENT) {
			if (cmd->resp_type & MMC_RSP_136) {
				/* response type 2 */
				cmd->response[3] = readl(&mmc_base->rsp10);
				cmd->response[2] = readl(&mmc_base->rsp32);
				cmd->response[1] = readl(&mmc_base->rsp54);
				cmd->response[0] = readl(&mmc_base->rsp76);
			} else
				/* response types 1, 1b, 3, 4, 5, 6 */
				cmd->response[0] = readl(&mmc_base->rsp10);
		}
	}

	if (data && (data->flags & MMC_DATA_READ)) {
		mmc_read_data(mmc_base,	data->dest,
				data->blocksize * data->blocks);
	} else if (data && (data->flags & MMC_DATA_WRITE)) {
		mmc_write_data(mmc_base, data->src,
				data->blocksize * data->blocks);
	}
	return 0;
}

static int mmc_read_data(struct hsmmc *mmc_base, char *buf, unsigned int size)
{
	unsigned int *output_buf = (unsigned int *)buf;
	unsigned int mmc_stat;
	unsigned int count;

	/*
	 * Start Polled Read
	 */
	count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
	count /= 4;

	while (size) {
		ulong start = get_timer(0);
		do {
			mmc_stat = readl(&mmc_base->stat);
			if (get_timer(0) - start > MAX_RETRY_MS) {
				printf("%s: timedout waiting for status!\n",
						__func__);
				return TIMEOUT;
			}
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return 1;

		if (mmc_stat & BRR_MASK) {
			unsigned int k;

			writel(readl(&mmc_base->stat) | BRR_MASK,
				&mmc_base->stat);
			for (k = 0; k < count; k++) {
				*output_buf = readl(&mmc_base->data);
				output_buf++;
			}
			size -= (count*4);
		}

		if (mmc_stat & BWR_MASK)
			writel(readl(&mmc_base->stat) | BWR_MASK,
				&mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			writel(readl(&mmc_base->stat) | TC_MASK,
				&mmc_base->stat);
			break;
		}
	}
	return 0;
}

static int mmc_write_data(struct hsmmc *mmc_base, const char *buf,
				unsigned int size)
{
	unsigned int *input_buf = (unsigned int *)buf;
	unsigned int mmc_stat;
	unsigned int count;

	/*
	 * Start Polled Read
	 */
	count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
	count /= 4;

	while (size) {
		ulong start = get_timer(0);
		do {
			mmc_stat = readl(&mmc_base->stat);
			if (get_timer(0) - start > MAX_RETRY_MS) {
				printf("%s: timedout waiting for status!\n",
						__func__);
				return TIMEOUT;
			}
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return 1;

		if (mmc_stat & BWR_MASK) {
			unsigned int k;

			writel(readl(&mmc_base->stat) | BWR_MASK,
					&mmc_base->stat);
			for (k = 0; k < count; k++) {
				writel(*input_buf, &mmc_base->data);
				input_buf++;
			}
			size -= (count*4);
		}

		if (mmc_stat & BRR_MASK)
			writel(readl(&mmc_base->stat) | BRR_MASK,
				&mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			writel(readl(&mmc_base->stat) | TC_MASK,
				&mmc_base->stat);
			break;
		}
	}
	return 0;
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct hsmmc *mmc_base = (struct hsmmc *)mmc->priv;
	unsigned int dsor = 0;
	ulong start;

	/* configue bus width */
	switch (mmc->bus_width) {
	case 8:
		writel(readl(&mmc_base->con) | DTW_8_BITMODE,
			&mmc_base->con);
		break;

	case 4:
		writel(readl(&mmc_base->con) & ~DTW_8_BITMODE,
			&mmc_base->con);
		writel(readl(&mmc_base->hctl) | DTW_4_BITMODE,
			&mmc_base->hctl);
		break;

	case 1:
	default:
		writel(readl(&mmc_base->con) & ~DTW_8_BITMODE,
			&mmc_base->con);
		writel(readl(&mmc_base->hctl) & ~DTW_4_BITMODE,
			&mmc_base->hctl);
		break;
	}

	/* configure clock with 96Mhz system clock.
	 */
	if (mmc->clock != 0) {
		dsor = (MMC_CLOCK_REFERENCE * 1000000 / mmc->clock);
		if ((MMC_CLOCK_REFERENCE * 1000000) / dsor > mmc->clock)
			dsor++;
	}

	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK | CEN_MASK),
				(ICE_STOP | DTO_15THDTO | CEN_DISABLE));

	mmc_reg_out(&mmc_base->sysctl, ICE_MASK | CLKD_MASK,
				(dsor << CLKD_OFFSET) | ICE_OSCILLATE);

	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for ics!\n", __func__);
			return;
		}
	}
	writel(readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);
}

int omap_mmc_init(int dev_index)
{
	struct mmc *mmc;

	mmc = &hsmmc_dev[dev_index];

	sprintf(mmc->name, "OMAP SD/MMC");
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_init_setup;
	mmc->getcd = NULL;

	switch (dev_index) {
	case 0:
		mmc->priv = (struct hsmmc *)OMAP_HSMMC1_BASE;
		break;
#ifdef OMAP_HSMMC2_BASE
	case 1:
		mmc->priv = (struct hsmmc *)OMAP_HSMMC2_BASE;
		break;
#endif
#ifdef OMAP_HSMMC3_BASE
	case 2:
		mmc->priv = (struct hsmmc *)OMAP_HSMMC3_BASE;
		break;
#endif
	default:
		mmc->priv = (struct hsmmc *)OMAP_HSMMC1_BASE;
		return 1;
	}
	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS |
				MMC_MODE_HC;

	mmc->f_min = 400000;
	mmc->f_max = 52000000;

	mmc->b_max = 0;

#if defined(CONFIG_OMAP34XX)
	/*
	 * Silicon revs 2.1 and older do not support multiblock transfers.
	 */
	if ((get_cpu_family() == CPU_OMAP34XX) && (get_cpu_rev() <= CPU_3XX_ES21))
		mmc->b_max = 1;
#endif

	mmc_register(mmc);

	return 0;
}
