/*
 * Copyright (C) 2010
 * Rob Emanuele <rob@emanuele.us>
 * Reinhard Meyer, EMK Elektronik <reinhard.meyer@emk-elektronik.de>
 *
 * Original Driver:
 * Copyright (C) 2004-2006 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include "atmel_mci.h"

#ifndef CONFIG_SYS_MMC_CLK_OD
# define CONFIG_SYS_MMC_CLK_OD	150000
#endif

#define MMC_DEFAULT_BLKLEN	512

#if defined(CONFIG_ATMEL_MCI_PORTB)
# define MCI_BUS 1
#else
# define MCI_BUS 0
#endif

static int initialized = 0;

/*
 * Print command and status:
 *
 * - always when DEBUG is defined
 * - on command errors
 */
static void dump_cmd(u32 cmdr, u32 arg, u32 status, const char* msg)
{
	printf("gen_atmel_mci: CMDR %08x (%2u) ARGR %08x (SR: %08x) %s\n",
		cmdr, cmdr&0x3F, arg, status, msg);
}

/* Setup for MCI Clock and Block Size */
static void mci_set_mode(struct mmc *mmc, u32 hz, u32 blklen)
{
	atmel_mci_t *mci = (atmel_mci_t *)mmc->priv;
	u32 bus_hz = get_mci_clk_rate();
	u32 clkdiv = 255;

	debug("mci: bus_hz is %u, setting clock %u Hz, block size %u\n",
		bus_hz, hz, blklen);
	if (hz > 0) {
		/* find lowest clkdiv yielding a rate <= than requested */
		for (clkdiv=0; clkdiv<255; clkdiv++) {
			if ((bus_hz / (clkdiv+1) / 2) <= hz)
				break;
		}
	}
	printf("mci: setting clock %u Hz, block size %u\n",
		(bus_hz / (clkdiv+1)) / 2, blklen);

	blklen &= 0xfffc;
	/* On some platforms RDPROOF and WRPROOF are ignored */
	writel((MMCI_BF(CLKDIV, clkdiv)
		 | MMCI_BF(BLKLEN, blklen)
		 | MMCI_BIT(RDPROOF)
		 | MMCI_BIT(WRPROOF)), &mci->mr);
	initialized = 1;
}

/* Return the CMDR with flags for a given command and data packet */
static u32 mci_encode_cmd(
	struct mmc_cmd *cmd, struct mmc_data *data, u32* error_flags)
{
	u32 cmdr = 0;

	/* Default Flags for Errors */
	*error_flags |= (MMCI_BIT(DTOE) | MMCI_BIT(RDIRE) | MMCI_BIT(RENDE) |
		MMCI_BIT(RINDE) | MMCI_BIT(RTOE));

	/* Default Flags for the Command */
	cmdr |= MMCI_BIT(MAXLAT);

	if (data) {
		cmdr |= MMCI_BF(TRCMD, 1);
		if (data->blocks > 1)
			cmdr |= MMCI_BF(TRTYP, 1);
		if (data->flags & MMC_DATA_READ)
			cmdr |= MMCI_BIT(TRDIR);
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		*error_flags |= MMCI_BIT(RCRCE);
	if (cmd->resp_type & MMC_RSP_136)
		cmdr |= MMCI_BF(RSPTYP, 2);
	else if (cmd->resp_type & MMC_RSP_BUSY)
		cmdr |= MMCI_BF(RSPTYP, 3);
	else if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdr |= MMCI_BF(RSPTYP, 1);

	return cmdr | MMCI_BF(CMDNB, cmd->cmdidx);
}

/* Entered into function pointer in mci_send_cmd */
static u32 mci_data_read(atmel_mci_t *mci, u32* data, u32 error_flags)
{
	u32 status;

	do {
		status = readl(&mci->sr);
		if (status & (error_flags | MMCI_BIT(OVRE)))
			goto io_fail;
	} while (!(status & MMCI_BIT(RXRDY)));

	if (status & MMCI_BIT(RXRDY)) {
		*data = readl(&mci->rdr);
		status = 0;
	}
io_fail:
	return status;
}

/* Entered into function pointer in mci_send_cmd */
static u32 mci_data_write(atmel_mci_t *mci, u32* data, u32 error_flags)
{
	u32 status;

	do {
		status = readl(&mci->sr);
		if (status & (error_flags | MMCI_BIT(UNRE)))
			goto io_fail;
	} while (!(status & MMCI_BIT(TXRDY)));

	if (status & MMCI_BIT(TXRDY)) {
		writel(*data, &mci->tdr);
		status = 0;
	}
io_fail:
	return status;
}

/*
 * Entered into mmc structure during driver init
 *
 * Sends a command out on the bus and deals with the block data.
 * Takes the mmc pointer, a command pointer, and an optional data pointer.
 */
static int
mci_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	atmel_mci_t *mci = (atmel_mci_t *)mmc->priv;
	u32 cmdr;
	u32 error_flags = 0;
	u32 status;

	if (!initialized) {
		puts ("MCI not initialized!\n");
		return COMM_ERR;
	}

	/* Figure out the transfer arguments */
	cmdr = mci_encode_cmd(cmd, data, &error_flags);

	if (data)
		writel(MMCI_BF(BCNT, data->blocks) |
			MMCI_BF(BLKLEN,	mmc->read_bl_len), &mci->blkr);

	/* Send the command */
	writel(cmd->cmdarg, &mci->argr);
	writel(cmdr, &mci->cmdr);

#ifdef DEBUG
	dump_cmd(cmdr, cmd->cmdarg, 0, "DEBUG");
#endif

	/* Wait for the command to complete */
	while (!((status = readl(&mci->sr)) & MMCI_BIT(CMDRDY)));

	if (status & error_flags) {
		dump_cmd(cmdr, cmd->cmdarg, status, "Command Failed");
		return COMM_ERR;
	}

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mci->rspr);
		cmd->response[1] = readl(&mci->rspr1);
		cmd->response[2] = readl(&mci->rspr2);
		cmd->response[3] = readl(&mci->rspr3);
	} else
		cmd->response[0] = readl(&mci->rspr);

	/* transfer all of the blocks */
	if (data) {
		u32 word_count, block_count;
		u32* ioptr;
		u32 sys_blocksize, dummy, i;
		u32 (*mci_data_op)
			(atmel_mci_t *mci, u32* data, u32 error_flags);

		if (data->flags & MMC_DATA_READ) {
			mci_data_op = mci_data_read;
			sys_blocksize = mmc->read_bl_len;
			ioptr = (u32*)data->dest;
		} else {
			mci_data_op = mci_data_write;
			sys_blocksize = mmc->write_bl_len;
			ioptr = (u32*)data->src;
		}

		status = 0;
		for (block_count = 0;
				block_count < data->blocks && !status;
				block_count++) {
			word_count = 0;
			do {
				status = mci_data_op(mci, ioptr, error_flags);
				word_count++;
				ioptr++;
			} while (!status && word_count < (data->blocksize/4));
#ifdef DEBUG
			if (data->flags & MMC_DATA_READ)
			{
				printf("Read Data:\n");
				print_buffer(0, data->dest, 1,
					word_count*4, 0);
			}
#endif
#ifdef DEBUG
			if (!status && word_count < (sys_blocksize / 4))
				printf("filling rest of block...\n");
#endif
			/* fill the rest of a full block */
			while (!status && word_count < (sys_blocksize / 4)) {
				status = mci_data_op(mci, &dummy,
					error_flags);
				word_count++;
			}
			if (status) {
				dump_cmd(cmdr, cmd->cmdarg, status,
					"Data Transfer Failed");
				return COMM_ERR;
			}
		}

		/* Wait for Transfer End */
		i = 0;
		do {
			status = readl(&mci->sr);

			if (status & error_flags) {
				dump_cmd(cmdr, cmd->cmdarg, status,
					"DTIP Wait Failed");
				return COMM_ERR;
			}
			i++;
		} while ((status & MMCI_BIT(DTIP)) && i < 10000);
		if (status & MMCI_BIT(DTIP)) {
			dump_cmd(cmdr, cmd->cmdarg, status,
				"XFER DTIP never unset, ignoring");
		}
	}

	return 0;
}

/* Entered into mmc structure during driver init */
static void mci_set_ios(struct mmc *mmc)
{
	atmel_mci_t *mci = (atmel_mci_t *)mmc->priv;
	int busw = (mmc->bus_width == 4) ? 1 : 0;

	/* Set the clock speed */
	mci_set_mode(mmc, mmc->clock, MMC_DEFAULT_BLKLEN);

	/*
	 * set the bus width and select slot for this interface
	 * there is no capability for multiple slots on the same interface yet
	 * Bitfield SCDBUS needs to be expanded to 2 bits for 8-bit buses
	 */
	writel(MMCI_BF(SCDBUS, busw) | MMCI_BF(SCDSEL, MCI_BUS), &mci->sdcr);
}

/* Entered into mmc structure during driver init */
static int mci_init(struct mmc *mmc)
{
	atmel_mci_t *mci = (atmel_mci_t *)mmc->priv;

	/* Initialize controller */
	writel(MMCI_BIT(SWRST), &mci->cr);	/* soft reset */
	writel(MMCI_BIT(PWSDIS), &mci->cr);	/* disable power save */
	writel(MMCI_BIT(MCIEN), &mci->cr);	/* enable mci */
	writel(MMCI_BF(SCDSEL, MCI_BUS), &mci->sdcr);	/* select port */

	/* Initial Time-outs */
	writel(0x5f, &mci->dtor);
	/* Disable Interrupts */
	writel(~0UL, &mci->idr);

	/* Set default clocks and blocklen */
	mci_set_mode(mmc, CONFIG_SYS_MMC_CLK_OD, MMC_DEFAULT_BLKLEN);

	return 0;
}

/*
 * This is the only exported function
 *
 * Call it with the MCI register base address
 */
int atmel_mci_init(void *regs)
{
	struct mmc *mmc = malloc(sizeof(struct mmc));

	if (!mmc)
		return -1;
	strcpy(mmc->name, "mci");
	mmc->priv = regs;
	mmc->send_cmd = mci_send_cmd;
	mmc->set_ios = mci_set_ios;
	mmc->init = mci_init;

	/* need to be able to pass these in on a board by board basis */
	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT;
	/*
	 * min and max frequencies determined by
	 * max and min of clock divider
	 */
	mmc->f_min = get_mci_clk_rate() / (2*256);
	mmc->f_max = get_mci_clk_rate() / (2*1);

	mmc->b_max = 0;

	mmc_register(mmc);

	return 0;
}
