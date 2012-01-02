/*
 * ARM PrimeCell MultiMedia Card Interface - PL180
 *
 * Copyright (C) ST-Ericsson SA 2010
 *
 * Author: Ulf Hansson <ulf.hansson@stericsson.com>
 * Author: Martin Lundholm <martin.xa.lundholm@stericsson.com>
 * Ported to drivers/mmc/ by: Matt Waddel <matt.waddel@linaro.org>
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

/* #define DEBUG */

#include <asm/io.h>
#include "common.h"
#include <errno.h>
#include <mmc.h>
#include "arm_pl180_mmci.h"
#include <malloc.h>

struct mmc_host {
	struct sdi_registers *base;
};

static int wait_for_command_end(struct mmc *dev, struct mmc_cmd *cmd)
{
	u32 hoststatus, statusmask;
	struct mmc_host *host = dev->priv;

	statusmask = SDI_STA_CTIMEOUT | SDI_STA_CCRCFAIL;
	if ((cmd->resp_type & MMC_RSP_PRESENT))
		statusmask |= SDI_STA_CMDREND;
	else
		statusmask |= SDI_STA_CMDSENT;

	do
		hoststatus = readl(&host->base->status) & statusmask;
	while (!hoststatus);

	writel(statusmask, &host->base->status_clear);
	if (hoststatus & SDI_STA_CTIMEOUT) {
		printf("CMD%d time out\n", cmd->cmdidx);
		return -ETIMEDOUT;
	} else if ((hoststatus & SDI_STA_CCRCFAIL) &&
		   (cmd->flags & MMC_RSP_CRC)) {
		printf("CMD%d CRC error\n", cmd->cmdidx);
		return -EILSEQ;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		cmd->response[0] = readl(&host->base->response0);
		cmd->response[1] = readl(&host->base->response1);
		cmd->response[2] = readl(&host->base->response2);
		cmd->response[3] = readl(&host->base->response3);
		debug("CMD%d response[0]:0x%08X, response[1]:0x%08X, "
			"response[2]:0x%08X, response[3]:0x%08X\n",
			cmd->cmdidx, cmd->response[0], cmd->response[1],
			cmd->response[2], cmd->response[3]);
	}

	return 0;
}

/* send command to the mmc card and wait for results */
static int do_command(struct mmc *dev, struct mmc_cmd *cmd)
{
	int result;
	u32 sdi_cmd = 0;
	struct mmc_host *host = dev->priv;

	sdi_cmd = ((cmd->cmdidx & SDI_CMD_CMDINDEX_MASK) | SDI_CMD_CPSMEN);

	if (cmd->resp_type) {
		sdi_cmd |= SDI_CMD_WAITRESP;
		if (cmd->resp_type & MMC_RSP_136)
			sdi_cmd |= SDI_CMD_LONGRESP;
	}

	writel((u32)cmd->cmdarg, &host->base->argument);
	udelay(COMMAND_REG_DELAY);
	writel(sdi_cmd, &host->base->command);
	result = wait_for_command_end(dev, cmd);

	/* After CMD2 set RCA to a none zero value. */
	if ((result == 0) && (cmd->cmdidx == MMC_CMD_ALL_SEND_CID))
		dev->rca = 10;

	/* After CMD3 open drain is switched off and push pull is used. */
	if ((result == 0) && (cmd->cmdidx == MMC_CMD_SET_RELATIVE_ADDR)) {
		u32 sdi_pwr = readl(&host->base->power) & ~SDI_PWR_OPD;
		writel(sdi_pwr, &host->base->power);
	}

	return result;
}

static int read_bytes(struct mmc *dev, u32 *dest, u32 blkcount, u32 blksize)
{
	u32 *tempbuff = dest;
	u64 xfercount = blkcount * blksize;
	struct mmc_host *host = dev->priv;
	u32 status, status_err;

	debug("read_bytes: blkcount=%u blksize=%u\n", blkcount, blksize);

	status = readl(&host->base->status);
	status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT |
			       SDI_STA_RXOVERR);
	while ((!status_err) && (xfercount >= sizeof(u32))) {
		if (status & SDI_STA_RXDAVL) {
			*(tempbuff) = readl(&host->base->fifo);
			tempbuff++;
			xfercount -= sizeof(u32);
		}
		status = readl(&host->base->status);
		status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT |
				       SDI_STA_RXOVERR);
	}

	status_err = status &
		(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND |
		 SDI_STA_RXOVERR);
	while (!status_err) {
		status = readl(&host->base->status);
		status_err = status &
			(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND |
			 SDI_STA_RXOVERR);
	}

	if (status & SDI_STA_DTIMEOUT) {
		printf("Read data timed out, xfercount: %llu, status: 0x%08X\n",
			xfercount, status);
		return -ETIMEDOUT;
	} else if (status & SDI_STA_DCRCFAIL) {
		printf("Read data bytes CRC error: 0x%x\n", status);
		return -EILSEQ;
	} else if (status & SDI_STA_RXOVERR) {
		printf("Read data RX overflow error\n");
		return -EIO;
	}

	writel(SDI_ICR_MASK, &host->base->status_clear);

	if (xfercount) {
		printf("Read data error, xfercount: %llu\n", xfercount);
		return -ENOBUFS;
	}

	return 0;
}

static int write_bytes(struct mmc *dev, u32 *src, u32 blkcount, u32 blksize)
{
	u32 *tempbuff = src;
	int i;
	u64 xfercount = blkcount * blksize;
	struct mmc_host *host = dev->priv;
	u32 status, status_err;

	debug("write_bytes: blkcount=%u blksize=%u\n", blkcount, blksize);

	status = readl(&host->base->status);
	status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT);
	while (!status_err && xfercount) {
		if (status & SDI_STA_TXFIFOBW) {
			if (xfercount >= SDI_FIFO_BURST_SIZE * sizeof(u32)) {
				for (i = 0; i < SDI_FIFO_BURST_SIZE; i++)
					writel(*(tempbuff + i),
						&host->base->fifo);
				tempbuff += SDI_FIFO_BURST_SIZE;
				xfercount -= SDI_FIFO_BURST_SIZE * sizeof(u32);
			} else {
				while (xfercount >= sizeof(u32)) {
					writel(*(tempbuff), &host->base->fifo);
					tempbuff++;
					xfercount -= sizeof(u32);
				}
			}
		}
		status = readl(&host->base->status);
		status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT);
	}

	status_err = status &
		(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND);
	while (!status_err) {
		status = readl(&host->base->status);
		status_err = status &
			(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND);
	}

	if (status & SDI_STA_DTIMEOUT) {
		printf("Write data timed out, xfercount:%llu,status:0x%08X\n",
		       xfercount, status);
		return -ETIMEDOUT;
	} else if (status & SDI_STA_DCRCFAIL) {
		printf("Write data CRC error\n");
		return -EILSEQ;
	}

	writel(SDI_ICR_MASK, &host->base->status_clear);

	if (xfercount) {
		printf("Write data error, xfercount:%llu", xfercount);
		return -ENOBUFS;
	}

	return 0;
}

static int do_data_transfer(struct mmc *dev,
			    struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	int error = -ETIMEDOUT;
	struct mmc_host *host = dev->priv;
	u32 blksz = 0;
	u32 data_ctrl = 0;
	u32 data_len = (u32) (data->blocks * data->blocksize);

	blksz = (ffs(data->blocksize) - 1);
	data_ctrl |= ((blksz << 4) & SDI_DCTRL_DBLKSIZE_MASK);
	data_ctrl |= SDI_DCTRL_DTEN;

	writel(SDI_DTIMER_DEFAULT, &host->base->datatimer);
	writel(data_len, &host->base->datalength);
	udelay(DATA_REG_DELAY);

	if (data->flags & MMC_DATA_READ) {
		data_ctrl |= SDI_DCTRL_DTDIR_IN;
		writel(data_ctrl, &host->base->datactrl);

		error = do_command(dev, cmd);
		if (error)
			return error;

		error = read_bytes(dev, (u32 *)data->dest, (u32)data->blocks,
				   (u32)data->blocksize);
	} else if (data->flags & MMC_DATA_WRITE) {
		error = do_command(dev, cmd);
		if (error)
			return error;

		writel(data_ctrl, &host->base->datactrl);
		error = write_bytes(dev, (u32 *)data->src, (u32)data->blocks,
				    (u32)data->blocksize);
	}

	return error;
}

static int host_request(struct mmc *dev,
			struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	int result;

	if (data)
		result = do_data_transfer(dev, cmd, data);
	else
		result = do_command(dev, cmd);

	return result;
}

/* MMC uses open drain drivers in the enumeration phase */
static int mmc_host_reset(struct mmc *dev)
{
	struct mmc_host *host = dev->priv;
	u32 sdi_u32 = SDI_PWR_OPD | SDI_PWR_PWRCTRL_ON;

	writel(sdi_u32, &host->base->power);

	return 0;
}

static void host_set_ios(struct mmc *dev)
{
	struct mmc_host *host = dev->priv;
	u32 sdi_clkcr;

	sdi_clkcr = readl(&host->base->clock);

	/* Ramp up the clock rate */
	if (dev->clock) {
		u32 clkdiv = 0;

		if (dev->clock >= dev->f_max)
			dev->clock = dev->f_max;

		clkdiv = ((ARM_MCLK / dev->clock) / 2) - 1;

		if (clkdiv > SDI_CLKCR_CLKDIV_MASK)
			clkdiv = SDI_CLKCR_CLKDIV_MASK;

		sdi_clkcr &= ~(SDI_CLKCR_CLKDIV_MASK);
		sdi_clkcr |= clkdiv;
	}

	/* Set the bus width */
	if (dev->bus_width) {
		u32 buswidth = 0;

		switch (dev->bus_width) {
		case 1:
			buswidth |= SDI_CLKCR_WIDBUS_1;
			break;
		case 4:
			buswidth |= SDI_CLKCR_WIDBUS_4;
			break;
		default:
			printf("Invalid bus width\n");
			break;
		}
		sdi_clkcr &= ~(SDI_CLKCR_WIDBUS_MASK);
		sdi_clkcr |= buswidth;
	}

	writel(sdi_clkcr, &host->base->clock);
	udelay(CLK_CHANGE_DELAY);
}

struct mmc *alloc_mmc_struct(void)
{
	struct mmc_host *host = NULL;
	struct mmc *mmc_device = NULL;

	host = malloc(sizeof(struct mmc_host));
	if (!host)
		return NULL;

	mmc_device = malloc(sizeof(struct mmc));
	if (!mmc_device)
		goto err;

	mmc_device->priv = host;
	return mmc_device;

err:
	free(host);
	return NULL;
}

/*
 * mmc_host_init - initialize the mmc controller.
 * Set initial clock and power for mmc slot.
 * Initialize mmc struct and register with mmc framework.
 */
static int arm_pl180_mmci_host_init(struct mmc *dev)
{
	struct mmc_host *host = dev->priv;
	u32 sdi_u32;

	host->base = (struct sdi_registers *)CONFIG_ARM_PL180_MMCI_BASE;

	/* Initially set power-on, full voltage & MMCI read */
	sdi_u32 = INIT_PWR;
	writel(sdi_u32, &host->base->power);

	/* setting clk freq 505KHz */
	sdi_u32 = SDI_CLKCR_CLKDIV_INIT | SDI_CLKCR_CLKEN;
	writel(sdi_u32, &host->base->clock);
	udelay(CLK_CHANGE_DELAY);

	/* Disable mmc interrupts */
	sdi_u32 = readl(&host->base->mask0) & ~SDI_MASK0_MASK;
	writel(sdi_u32, &host->base->mask0);

	sprintf(dev->name, "MMC");
	dev->clock = ARM_MCLK / (2 * (SDI_CLKCR_CLKDIV_INIT + 1));
	dev->send_cmd = host_request;
	dev->set_ios = host_set_ios;
	dev->init = mmc_host_reset;
	dev->getcd = NULL;
	dev->host_caps = 0;
	dev->voltages = VOLTAGE_WINDOW_MMC;
	dev->f_min = dev->clock;
	dev->f_max = CONFIG_ARM_PL180_MMCI_CLOCK_FREQ;

	return 0;
}

int arm_pl180_mmci_init(void)
{
	int error;
	struct mmc *dev;

	dev = alloc_mmc_struct();
	if (!dev)
		return -1;

	error = arm_pl180_mmci_host_init(dev);
	if (error) {
		printf("mmci_host_init error - %d\n", error);
		return -1;
	}

	dev->b_max = 0;

	mmc_register(dev);
	debug("registered mmc interface number is:%d\n", dev->block_dev.dev);

	return 0;
}
