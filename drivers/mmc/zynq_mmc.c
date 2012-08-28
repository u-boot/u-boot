/*
 * (C) Copyright 2012 Xilinx
 *
 * Zynq SD Host Controller Interface
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/mmc.h>
#include <asm/errno.h>
#include <malloc.h>
#include <mmc.h>

#include "zynq_mmc.h"

#define SD_BASEADDR XPSS_SDIO0_BASEADDR

static u32 *sd_dma_buffer;

/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO dmb()

static void sd_out32(u32 OutAddress, u32 Value)
{
	OutAddress += SD_BASEADDR;
	*(volatile u32 *) OutAddress = Value;
	SYNCHRONIZE_IO;
}   
static void sd_out16(u32 OutAddress, u16 Value)
{
	OutAddress += SD_BASEADDR;
	*(volatile u16 *) OutAddress = Value;
	SYNCHRONIZE_IO;
}   
static void sd_out8(u32 OutAddress, u8 Value)
{
	OutAddress += SD_BASEADDR;
	*(volatile u8 *) OutAddress = Value;
	SYNCHRONIZE_IO;
}   

static u32 sd_in32(u32 InAddress)
{
	SYNCHRONIZE_IO;
	InAddress += SD_BASEADDR;
	return *(volatile u32 *) InAddress;
}
static u16 sd_in16(u32 InAddress)
{
	SYNCHRONIZE_IO;
	InAddress += SD_BASEADDR;
	return *(volatile u16 *) InAddress;
}
static u8 sd_in8(u32 InAddress)
{
	SYNCHRONIZE_IO;
	InAddress += SD_BASEADDR;
	return *(volatile u8 *) InAddress;
}

static void connect_test_mode(void)
{
	volatile unsigned  statusreg;

	/* Fake out the card detect since it is also NAND CS */
	sd_out8(SD_HOST_CTRL_R, SD_CD_TEST | SD_CD_TEST_INS);

	/* Wait for card detected */
	statusreg = sd_in32(SD_PRES_STATE_R);
	while ((!(statusreg & SD_CARD_DPL))
		|| (!(statusreg & SD_CARD_DB))
		|| (!(statusreg & SD_CARD_INS)))
		statusreg = sd_in32(SD_PRES_STATE_R);
}

/* Initialize the SD controller */
static void init_port(void)
{
	unsigned clk;

	/* Power off the card */
	sd_out8(SD_PWR_CTRL_R, 0);

	/* Disable interrupts */
	sd_out32(SD_SIG_ENA_R, 0);

	/* Perform soft reset */
	sd_out8(SD_SOFT_RST_R, SD_RST_ALL);
	/* Wait for reset to comlete */
	while (sd_in8(SD_SOFT_RST_R)) {
		;
	}

	/* Power on the card */
	sd_out8(SD_PWR_CTRL_R, SD_POWER_33|SD_POWER_ON);

	connect_test_mode();

	/* Enable Internal clock and wait for it to stablilize */
	clk = (0x4 << SD_DIV_SHIFT) | SD_CLK_INT_EN;
	sd_out16(SD_CLK_CTL_R, clk);
	do {
		clk = sd_in16(SD_CLK_CTL_R);
	} while (!(clk & SD_CLK_INT_STABLE));

	/* Enable SD clock */
	clk |= SD_CLK_SD_EN;
	sd_out16(SD_CLK_CTL_R, clk);

	sd_out32(SD_SIG_ENA_R, 0xFFFFFFFF);
	sd_out32(SD_INT_ENA_R, 0xFFFFFFFF);
}

/* MMC/SD command (SPI mode) */
#define CMD0	(0)		/* GO_IDLE_STATE */
#define CMD1	(1)		/* SEND_OP_COND */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD2    (2)		/* SEND_CID */
#define CMD3    (3)		/* RELATIVE_ADDR */
#define CMD5	(5)		/* SLEEP_WAKE (SDC) */
#define CMD6	(6)		/* SWITCH_FUNC */
#define CMD7	(7)		/* SELECT */
#define CMD8	(8)		/* SEND_IF_COND */
#define CMD9	(9)		/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define CMD13	(13)		/* SEND_STATUS */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD41	(41)		/* SEND_OP_COND (ACMD) */
#define CMD51	(51)		/* SEND_SCR (ACMD) */
#define CMD52	(52)		/*  */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* make_command:
 * Determing the proper value for the command register for the indicated
 * command index.
 */
static int
make_command (unsigned cmd)
{
	unsigned retval;

	retval = cmd << 8;

#define RSP_NONE SD_CMD_RESP_NONE
#define RSP_R1   (SD_CMD_INDEX | SD_CMD_RESP_48 | SD_CMD_CRC)
#define RSP_R1b  (SD_CMD_INDEX | SD_CMD_RESP_48_BUSY | SD_CMD_CRC)
#define RSP_R2   (SD_CMD_CRC | SD_CMD_RESP_136)
#define RSP_R3   (SD_CMD_RESP_48)
#define RSP_R6   (SD_CMD_INDEX | SD_CMD_RESP_48_BUSY | SD_CMD_CRC)

	switch(cmd) {
	case CMD0:
		retval |= (SD_CMD_RESP_NONE);
		break;
	case CMD1:
		retval |= RSP_R3;
		break;
	case CMD2:
		retval |= RSP_R2;
		break;
	case CMD3:
		retval |= RSP_R6;
		break;
	case CMD5:
		retval |= RSP_R1b;
		break;
	case CMD7:
		retval |= RSP_R1;
		break;
	case CMD8:
		retval |= RSP_R1;
		break;
	case CMD9:
		retval |= RSP_R2;
		break;
	case CMD10:
	case CMD12:
	case CMD13:
	case ACMD13:
	case CMD16:
	case CMD23:
		retval |= RSP_R1;
		break;
	case CMD17:
	case CMD18:
		retval |= RSP_R1|SD_CMD_DATA;
		break;
	case ACMD23:
	case CMD24:
	case CMD25:
	case CMD41:
		retval |= RSP_R3;
		break;
	case CMD51:
		retval |= RSP_R1;
		break;
	case CMD52:
	case CMD55:
		retval |= RSP_R1;
		break;
	case CMD58:
	case CMD6:
		break;
	default:
		printf("Unknown command CMD%d\n", cmd);
		break;
	}

	return retval;
}

static int zynq_sdh_request(struct mmc *mmc, struct mmc_cmd *cmd,
                struct mmc_data *data)
{
	u32 status;
	u16 cmdreg;
	u16 modereg;
	int result = 0;

	debug("zynq_sdh_request: cmdidx: %d arg: 0x%x\n",
	    cmd->cmdidx, cmd->cmdarg);

	cmd->response[0] = 0;
	cmdreg = make_command(cmd->cmdidx);

	/* Wait until the device is willing to accept commands */
	do {
		status = sd_in32(SD_PRES_STATE_R);
	} while (status & (SD_CMD_INHIBIT|SD_DATA_INHIBIT));

	/*
	 * Set the DMA address to the DMA buffer.
	 * This is only relevant for data commands.
	 * u-boot performs a copy rather than DMA directly into the user
	 * buffer because the controller can't DMA into the first 512K
	 * of DDR.
	 */
	debug("data->dest = %p (%d) (%d * %d)\n", data->dest,
		(cmdreg & SD_CMD_DATA) ? data->blocks : 0, data->blocks,
		data->blocksize);

#ifdef CONFIG_ZYNQ_SD_DIRECT_DMA
	/*
	 * Added based on above comment.
	 * No evidence in the TRM supports this that I could find, though
	 */
	if (data->dest < 0x80000) {
		printf("Cannot DMA to lowest 512k of DDR\n");
		return COMM_ERR;
	}
	sd_out32(SD_DMA_ADDR_R, (u32)data->dest);
#else
	if ((cmdreg & SD_CMD_DATA) && data->blocksize * data->blocks > 512) {
		printf("MMC driver buffer too small\n");
		return COMM_ERR;
	}
	sd_out32(SD_DMA_ADDR_R, (u32)sd_dma_buffer);
#endif

	/*
	 * 512 bytes
	 * This is only relevant for data commands.
	 */
	if (cmdreg & SD_CMD_DATA) {
		sd_out16(SD_BLOCK_SZ_R, data->blocksize);
		sd_out16(SD_BLOCK_CNT_R, data->blocks);
	}

	sd_out8(SD_TIMEOUT_CTL_R, 0xA);

	sd_out32(SD_ARG_R, cmd->cmdarg);

	/*
	 * Set the transfer mode to read, simple DMA, single block
	 * (applicable only to data commands)
	 * This is all that this software supports.
	 */
	modereg = SD_TRNS_BLK_CNT_EN | SD_TRNS_READ | SD_TRNS_DMA;
	if (data->blocks > 1)
		modereg |= SD_TRNS_MULTI;
	sd_out16(SD_TRNS_MODE_R, modereg);

	/* Clear all pending interrupt status */
	sd_out32(SD_INT_STAT_R, 0xFFFFFFFF);

	/* Initiate the command */
	sd_out16(SD_CMD_R, cmdreg);

	/* Poll until operation complete */
	while (1) {
		status = sd_in32(SD_INT_STAT_R);
		if (status & SD_INT_ERROR) {
#ifdef DEBUG
			printf("send_cmd: Error: (0x%08x) cmd: %d arg: 0x%x\n",
				status, cmd->cmdidx, cmd->cmdarg);
#else
			if (cmd->cmdidx == CMD17) {
				printf("MMC: Error reading sector %d (0x%08x)\n",
					cmd->cmdarg, cmd->cmdarg);
			}
#endif
			sd_out8(SD_SOFT_RST_R,
				SD_RST_CMD|SD_RST_DATA);

			if (status & (SD_INT_ERR_CTIMEOUT|SD_INT_ERR_DTIMEOUT)) {
				result = TIMEOUT;
				goto exit;
			} else {
				result = COMM_ERR;
				goto exit;
			}
		}
		if (status & SD_INT_CMD_CMPL) {
			if (cmdreg & SD_CMD_DATA) {
				if (status & (SD_INT_DMA | SD_INT_TRNS_CMPL)) {
					break;
				}
			} else {
				break;
			}
		}
	} 

	if (cmd->resp_type == MMC_RSP_R2) {
		int i;
		
		/* RESP_136 */
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0;i < 4;i++) {
			cmd->response[i] = sd_in32(SD_RSP_R + (3-i)*4) << 8;
			if (i != 3) {
				cmd->response[i] |=
					sd_in8((SD_RSP_R + (3-i)*4)-1);
			}
		}
	} else {
		/* RESP_48 */
		cmd->response[0] = sd_in32(SD_RSP_R);
	}

#ifndef CONFIG_ZYNQ_SD_DIRECT_DMA
	if (cmdreg & SD_CMD_DATA) {
		memcpy(data->dest, sd_dma_buffer,
			data->blocks * data->blocksize);
	}
#endif

exit:
	/* Clear all pending interrupt status */
	sd_out32(SD_INT_STAT_R, 0xFFFFFFFF);

	return result;
}

static void zynq_sdh_set_ios(struct mmc *mmc)
{
	debug("%s: voltages: 0x%x clock: 0x%x bus_width: 0x%x\n",
		__func__, mmc->voltages, mmc->clock, mmc->bus_width);
}
static int zynq_sdh_init(struct mmc *mmc)
{
	debug(" zynq_sdh_init called\n");
	init_port();
	return 0;
}

int zynq_mmc_init(bd_t *bd)
{
	struct mmc *mmc;

	sd_dma_buffer = malloc(512);
	if (!sd_dma_buffer) {
		return -ENOMEM;
	}

	mmc = malloc(sizeof(struct mmc));
	if (!mmc) {
		return -ENOMEM;
	}
	sprintf(mmc->name, "SDHCI");
	mmc->send_cmd = zynq_sdh_request;
	mmc->set_ios = zynq_sdh_set_ios;
	mmc->init = zynq_sdh_init;

	mmc->host_caps = MMC_MODE_4BIT;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
        
        mmc->f_max = 52000000;
	mmc->f_min = mmc->f_max >> 9;

	mmc->block_dev.part_type = PART_TYPE_DOS;

	mmc_register(mmc);

	return 0;
}

