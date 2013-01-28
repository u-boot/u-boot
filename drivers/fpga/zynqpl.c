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
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#define DEVCFG_CTRL_PCFG_PROG_B		0x40000000
#define DEVCFG_ISR_FATAL_ERROR_MASK	0x00740040
#define DEVCFG_ISR_ERROR_FLAGS_MASK	0x00340840
#define DEVCFG_ISR_RX_FIFO_OV		0x00040000
#define DEVCFG_ISR_DMA_DONE		0x00002000
#define DEVCFG_ISR_PCFG_DONE		0x00000004
#define DEVCFG_STATUS_DMA_CMD_Q_F	0x80000000
#define DEVCFG_STATUS_DMA_CMD_Q_E	0x40000000
#define DEVCFG_STATUS_DMA_DONE_CNT_MASK	0x30000000
#define DEVCFG_STATUS_PCFG_INIT		0x00000010
#define DEVCFG_MCTRL_RFIFO_FLUSH	0x00000002
#define DEVCFG_MCTRL_WFIFO_FLUSH	0x00000001

#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/100	/* 10 ms */
#endif

#ifndef CONFIG_SYS_FPGA_PROG_TIME
#define CONFIG_SYS_FPGA_PROG_TIME CONFIG_SYS_HZ	/* 1 s */
#endif

int zynq_info(Xilinx_desc *desc)
{
	return FPGA_SUCCESS;
}

int zynq_load(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	unsigned long ts; /* Timestamp */
	u32 control;
	u32 isr_status;
	u32 status;

	/* FIXME Add checking that passing bin is not a bitstream */

	zynq_slcr_devcfg_disable();

	/* Setting PCFG_PROG_B signal to high */
	control = readl(&devcfg_base->ctrl);
	writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);
	/* Setting PCFG_PROG_B signal to low */
	writel(control & ~DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

	/* Polling the PCAP_INIT status for Reset */
	ts = get_timer(0);
	while (readl(&devcfg_base->status) & DEVCFG_STATUS_PCFG_INIT) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for INIT to clear.\n");
			return FPGA_FAIL;
		}
	}

	/* Setting PCFG_PROG_B signal to high */
	writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(readl(&devcfg_base->status) & DEVCFG_STATUS_PCFG_INIT)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for INIT to set.\n");
			return FPGA_FAIL;
		}
	}

	isr_status = readl(&devcfg_base->int_sts);

	/* Clear it all, so if Boot ROM comes back, it can proceed */
	writel(0xFFFFFFFF, &devcfg_base->int_sts);

	if (isr_status & DEVCFG_ISR_FATAL_ERROR_MASK) {
		debug("Fatal errors in PCAP 0x%X\n", isr_status);

		/* If RX FIFO overflow, need to flush RX FIFO first */
		if (isr_status & DEVCFG_ISR_RX_FIFO_OV) {
			writel(DEVCFG_MCTRL_RFIFO_FLUSH, &devcfg_base->mctrl);
			writel(0xFFFFFFFF, &devcfg_base->int_sts);
		}
		return FPGA_FAIL;
	}

	status = readl(&devcfg_base->status);

	debug("status = 0x%08X\n", status);

	if (status & DEVCFG_STATUS_DMA_CMD_Q_F) {
		debug("Error: device busy\n");
		return FPGA_FAIL;
	}

	debug("device ready\n");

	if (!(status & DEVCFG_STATUS_DMA_CMD_Q_E)) {
		if (!(readl(&devcfg_base->int_sts) & DEVCFG_ISR_DMA_DONE)) {
			/* Error state, transfer cannot occur */
			debug("isr indicates error\n");
			return FPGA_FAIL;
		} else {
			/* Clear out the status */
			writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);
		}
	}

	if (status & DEVCFG_STATUS_DMA_DONE_CNT_MASK) {
		/* Clear the count of completed DMA transfers */
		writel(DEVCFG_STATUS_DMA_DONE_CNT_MASK, &devcfg_base->status);
	}

	debug("Source = 0x%08X\n", (u32)buf);
	debug("Size = %zu\n", bsize);

	/* Set up the transfer */
	writel((u32)buf | 1, &devcfg_base->dma_src_addr);
	writel(0xFFFFFFFF, &devcfg_base->dma_dst_addr);
	writel(bsize >> 2, &devcfg_base->dma_src_len);
	writel(0, &devcfg_base->dma_dst_len);

	isr_status = readl(&devcfg_base->int_sts);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_DMA_DONE)) {
		if (isr_status & DEVCFG_ISR_ERROR_FLAGS_MASK) {
			debug("Error: isr = 0x%08X\n", isr_status);
			debug("Write count = 0x%08X\n",
				readl(&devcfg_base->write_count));
			debug("Read count = 0x%08X\n",
				readl(&devcfg_base->read_count));

			return FPGA_FAIL;
		}
		if (get_timer(ts) > CONFIG_SYS_FPGA_PROG_TIME) {
			puts("Error: Timeout waiting for DMA to complete.\n");
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("DMA transfer is done\n");

	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			puts("Error: Timeout waiting for FPGA to config.\n");
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("FPGA config done\n");

	/* Clear out the DMA status */
	writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);

	zynq_slcr_devcfg_enable();

	return FPGA_SUCCESS;
}

int zynq_dump(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	return FPGA_FAIL;
}
