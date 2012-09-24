/*
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <asm/io.h>
#include <zynqpl.h>

#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/100	/* 10 ms */
#endif

#ifndef CONFIG_SYS_FPGA_PROG_TIME
#define CONFIG_SYS_FPGA_PROG_TIME CONFIG_SYS_HZ	/* 1 s */
#endif

#define SLCR_BASEADDR 0xF8000000
#define SLCR_LOCK (SLCR_BASEADDR + 0x04)
#define SLCR_LOCK_VALUE 0x767B
#define SLCR_UNLOCK (SLCR_BASEADDR + 0x08)
#define SLCR_UNLOCK_VALUE 0xDF0D
#define SLCR_FPGA_RST_CTRL (SLCR_BASEADDR + 0x240)
#define SLCR_LVL_SHFTR_EN (SLCR_BASEADDR + 0x900)

#define DEVCFG_BASEADDR 0xF8007000
#define DEVCFG_CTRL (DEVCFG_BASEADDR + 0x00)
#define DEVCFG_CTRL_PCFG_PROG_B 0x40000000
#define DEVCFG_LOCK (DEVCFG_BASEADDR + 0x04)
#define DEVCFG_CFG (DEVCFG_BASEADDR + 0x08)
#define DEVCFG_ISR (DEVCFG_BASEADDR + 0x0C)
#define DEVCFG_ISR_FATAL_ERROR_MASK 0x00740040
#define DEVCFG_ISR_ERROR_FLAGS_MASK 0x00340840
#define DEVCFG_ISR_RX_FIFO_OV 0x00040000
#define DEVCFG_ISR_DMA_DONE 0x00002000
#define DEVCFG_ISR_PCFG_DONE 0x00000004
#define DEVCFG_STATUS (DEVCFG_BASEADDR + 0x14)
#define DEVCFG_STATUS_DMA_CMD_Q_F 0x80000000
#define DEVCFG_STATUS_DMA_CMD_Q_E 0x40000000
#define DEVCFG_STATUS_DMA_DONE_CNT_MASK 0x30000000
#define DEVCFG_STATUS_PCFG_INIT 0x00000010
#define DEVCFG_DMA_SRC_ADDR (DEVCFG_BASEADDR + 0x18)
#define DEVCFG_DMA_DST_ADDR (DEVCFG_BASEADDR + 0x1C)
#define DEVCFG_DMA_SRC_LEN (DEVCFG_BASEADDR + 0x20)
#define DEVCFG_DMA_DEST_LEN (DEVCFG_BASEADDR + 0x24)
#define DEVCFG_MCTRL (DEVCFG_BASEADDR + 0x80)
#define DEVCFG_MCTRL_RFIFO_FLUSH 0x00000002
#define DEVCFG_MCTRL_WFIFO_FLUSH 0x00000001
#define DEVCFG_DEBUG_XFER_WRITE_COUNT (DEVCFG_BASEADDR + 0x88)
#define DEVCFG_DEBUG_XFER_READ_COUNT (DEVCFG_BASEADDR + 0x8C)

/* ------------------------------------------------------------------------- */
/* Zynq Implementation */

int zynq_info(Xilinx_desc *desc)
{
	return FPGA_SUCCESS;
}

int zynq_load(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	unsigned long ts;		/* timestamp */
	u32 control;
	u32 isr_status;
	u32 status;

	out_le32(SLCR_UNLOCK, SLCR_UNLOCK_VALUE);
	out_le32(SLCR_FPGA_RST_CTRL, 0xFFFFFFFF); /* Disable AXI interface */
	/* Set Level Shifters DT618760*/
	out_le32(SLCR_LVL_SHFTR_EN, 0x0000000A);
	/* Setting PCFG_PROG_B signal to high */
	control = in_le32(DEVCFG_CTRL);
	out_le32(DEVCFG_CTRL, control | DEVCFG_CTRL_PCFG_PROG_B);
	/* Setting PCFG_PROG_B signal to low */
	out_le32(DEVCFG_CTRL, control & ~DEVCFG_CTRL_PCFG_PROG_B);

	/* Polling the PCAP_INIT status for Reset */
	ts = get_timer(0);
	while (in_le32(DEVCFG_STATUS) & DEVCFG_STATUS_PCFG_INIT) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for INIT to clear.\n");
			return FPGA_FAIL;
		}
	}

	/* Setting PCFG_PROG_B signal to high */
	out_le32(DEVCFG_CTRL, control | DEVCFG_CTRL_PCFG_PROG_B);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(in_le32(DEVCFG_STATUS) & DEVCFG_STATUS_PCFG_INIT)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for INIT to set.\n");
			return FPGA_FAIL;
		}
	}

	out_le32(SLCR_LOCK, SLCR_LOCK_VALUE);

	isr_status = in_le32(DEVCFG_ISR);

	/* Clear it all, so if Boot ROM comes back, it can proceed */
	out_le32(DEVCFG_ISR, 0xFFFFFFFF);

	if (isr_status & DEVCFG_ISR_FATAL_ERROR_MASK) {
		debug("Fatal errors in PCAP 0x%X\n", isr_status);

		/* If RX FIFO overflow, need to flush RX FIFO first */
		if (isr_status & DEVCFG_ISR_RX_FIFO_OV) {
			out_le32(DEVCFG_MCTRL, DEVCFG_MCTRL_RFIFO_FLUSH);
			out_le32(DEVCFG_ISR, 0xFFFFFFFF);
		}
		return FPGA_FAIL;
	}

	status = in_le32(DEVCFG_STATUS);

	debug("status = 0x%08X\n", status);

	if (status & DEVCFG_STATUS_DMA_CMD_Q_F) {
		debug("Error: device busy\n");
		return FPGA_FAIL;
	}

	debug("device ready\n");

	if (!(status & DEVCFG_STATUS_DMA_CMD_Q_E)) {
		if (!(in_le32(DEVCFG_ISR) & DEVCFG_ISR_DMA_DONE)) {
			/* error state, transfer cannot occur */
			debug("isr indicates error\n");
			return FPGA_FAIL;
		} else {
			/* clear out the status */
			out_le32(DEVCFG_ISR, DEVCFG_ISR_DMA_DONE);
		}
	}

	if (status & DEVCFG_STATUS_DMA_DONE_CNT_MASK) {
		/* Clear the count of completed DMA transfers */
		out_le32(DEVCFG_STATUS, DEVCFG_STATUS_DMA_DONE_CNT_MASK);
	}

	debug("Source = 0x%08X\n", (u32)buf);
	debug("Size = %zu\n", bsize);

	/* set up the transfer */
	out_le32(DEVCFG_DMA_SRC_ADDR, (u32)buf | 1);
	out_le32(DEVCFG_DMA_DST_ADDR, 0xFFFFFFFF);
	out_le32(DEVCFG_DMA_SRC_LEN, bsize >> 2);
	out_le32(DEVCFG_DMA_DEST_LEN, 0);

	isr_status = in_le32(DEVCFG_ISR);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_DMA_DONE)) {
		if (isr_status & DEVCFG_ISR_ERROR_FLAGS_MASK) {
			debug("Error: isr = 0x%08X\n", isr_status);
			debug("Write count = 0x%08X\n",
				in_le32(DEVCFG_DEBUG_XFER_WRITE_COUNT));
			debug("Read count = 0x%08X\n",
				in_le32(DEVCFG_DEBUG_XFER_READ_COUNT));

			return FPGA_FAIL;
		}
		if (get_timer(ts) > CONFIG_SYS_FPGA_PROG_TIME) {
			puts("Error: Timeout waiting for DMA to complete.\n");
			return FPGA_FAIL;
		}
		isr_status = in_le32(DEVCFG_ISR);
	}

	debug("DMA transfer is done\n");

	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for FPGA to config.\n");
			return FPGA_FAIL;
		}
		isr_status = in_le32(DEVCFG_ISR);
	}

	debug("FPGA config done\n");

	/* clear out the DMA status */
	out_le32(DEVCFG_ISR, DEVCFG_ISR_DMA_DONE);

	out_le32(SLCR_UNLOCK, SLCR_UNLOCK_VALUE);

	/* Set Level Shifters DT618760*/
	out_le32(SLCR_LVL_SHFTR_EN, 0x0000000F);
	/* Disable AXI interface */
	out_le32(SLCR_FPGA_RST_CTRL, 0x00000000);

	out_le32(SLCR_LOCK, SLCR_LOCK_VALUE);

	return FPGA_SUCCESS;
}

int zynq_dump(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	return FPGA_FAIL;
}
