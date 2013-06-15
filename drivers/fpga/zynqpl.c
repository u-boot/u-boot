/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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
#define DEVCFG_MCTRL_PCAP_LPBK		0x00000010
#define DEVCFG_MCTRL_RFIFO_FLUSH	0x00000002
#define DEVCFG_MCTRL_WFIFO_FLUSH	0x00000001

#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/100	/* 10 ms */
#endif

#ifndef CONFIG_SYS_FPGA_PROG_TIME
#define CONFIG_SYS_FPGA_PROG_TIME	(CONFIG_SYS_HZ * 4) /* 4 s */
#endif

int zynq_info(Xilinx_desc *desc)
{
	return FPGA_SUCCESS;
}

#define DUMMY_WORD	0xffffffff

/* Xilinx binary format header */
static const u32 bin_format[] = {
	DUMMY_WORD, /* Dummy words */
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	0x000000bb, /* Sync word */
	0x11220044, /* Sync word */
	DUMMY_WORD,
	DUMMY_WORD,
	0xaa995566, /* Sync word */
};

#define SWAP_NO		1
#define SWAP_DONE	2

/*
 * Load the whole word from unaligned buffer
 * Keep in your mind that it is byte loading on little-endian system
 */
static u32 load_word(const void *buf, u32 swap)
{
	u32 word = 0;
	u8 *bitc = (u8 *)buf;
	int p;

	if (swap == SWAP_NO) {
		for (p = 0; p < 4; p++) {
			word <<= 8;
			word |= bitc[p];
		}
	} else {
		for (p = 3; p >= 0; p--) {
			word <<= 8;
			word |= bitc[p];
		}
	}

	return word;
}

static u32 check_header(const void *buf)
{
	u32 i, pattern;
	int swap = SWAP_NO;
	u32 *test = (u32 *)buf;

	debug("%s: Let's check bitstream header\n", __func__);

	/* Checking that passing bin is not a bitstream */
	for (i = 0; i < ARRAY_SIZE(bin_format); i++) {
		pattern = load_word(&test[i], swap);

		/*
		 * Bitstreams in binary format are swapped
		 * compare to regular bistream.
		 * Do not swap dummy word but if swap is done assume
		 * that parsing buffer is binary format
		 */
		if ((__swab32(pattern) != DUMMY_WORD) &&
		    (__swab32(pattern) == bin_format[i])) {
			pattern = __swab32(pattern);
			swap = SWAP_DONE;
			debug("%s: data swapped - let's swap\n", __func__);
		}

		debug("%s: %d/%x: pattern %x/%x bin_format\n", __func__, i,
		      (u32)&test[i], pattern, bin_format[i]);
		if (pattern != bin_format[i]) {
			debug("%s: Bitstream is not recognized\n", __func__);
			return 0;
		}
	}
	debug("%s: Found bitstream header at %x %s swapinng\n", __func__,
	      (u32)buf, swap == SWAP_NO ? "without" : "with");

	return swap;
}

static void *check_data(u8 *buf, size_t bsize, u32 *swap)
{
	u32 word, p = 0; /* possition */

	/* Because buf doesn't need to be aligned let's read it by chars */
	for (p = 0; p < bsize; p++) {
		word = load_word(&buf[p], SWAP_NO);
		debug("%s: word %x %x/%x\n", __func__, word, p, (u32)&buf[p]);

		/* Find the first bitstream dummy word */
		if (word == DUMMY_WORD) {
			debug("%s: Found dummy word at position %x/%x\n",
			      __func__, p, (u32)&buf[p]);
			*swap = check_header(&buf[p]);
			if (*swap) {
				/* FIXME add full bitstream checking here */
				return &buf[p];
			}
		}
		/* Loop can be huge - support CTRL + C */
		if (ctrlc())
			return 0;
	}
	return 0;
}


int zynq_load(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	unsigned long ts; /* Timestamp */
	u32 partialbit = 0;
	u32 i, control, isr_status, status, swap, diff;
	u32 *buf_start;

	/* Detect if we are going working with partial or full bitstream */
	if (bsize != desc->size) {
		printf("%s: Working with partial bitstream\n", __func__);
		partialbit = 1;
	}

	buf_start = check_data((u8 *)buf, bsize, &swap);
	if (!buf_start)
		return FPGA_FAIL;

	/* Check if data is postpone from start */
	diff = (u32)buf_start - (u32)buf;
	if (diff) {
		printf("%s: Bitstream is not validated yet (diff %x)\n",
		       __func__, diff);
		return FPGA_FAIL;
	}

	if ((u32)buf_start & 0x3) {
		u32 *new_buf = (u32 *)((u32)buf & ~0x3);

		printf("%s: Align buffer at %x to %x(swap %d)\n", __func__,
		       (u32)buf_start, (u32)new_buf, swap);

		for (i = 0; i < (bsize/4); i++)
			new_buf[i] = load_word(&buf_start[i], swap);

		swap = SWAP_DONE;
		buf = new_buf;
	} else if (swap != SWAP_DONE) {
		/* For bitstream which are aligned */
		u32 *new_buf = (u32 *)buf;

		printf("%s: Bitstream is not swapped(%d) - swap it\n", __func__,
		       swap);

		for (i = 0; i < (bsize/4); i++)
			new_buf[i] = load_word(&buf_start[i], swap);

		swap = SWAP_DONE;
	}

	/* Clear loopback bit */
	clrbits_le32(&devcfg_base->mctrl, DEVCFG_MCTRL_PCAP_LPBK);

	if (!partialbit) {
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
				printf("%s: Timeout wait for INIT to clear\n",
				       __func__);
				return FPGA_FAIL;
			}
		}

		/* Setting PCFG_PROG_B signal to high */
		writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/* Polling the PCAP_INIT status for Set */
		ts = get_timer(0);
		while (!(readl(&devcfg_base->status) &
			DEVCFG_STATUS_PCFG_INIT)) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for INIT to set\n",
				       __func__);
				return FPGA_FAIL;
			}
		}
	}

	isr_status = readl(&devcfg_base->int_sts);

	/* Clear it all, so if Boot ROM comes back, it can proceed */
	writel(0xFFFFFFFF, &devcfg_base->int_sts);

	if (isr_status & DEVCFG_ISR_FATAL_ERROR_MASK) {
		debug("%s: Fatal errors in PCAP 0x%X\n", __func__, isr_status);

		/* If RX FIFO overflow, need to flush RX FIFO first */
		if (isr_status & DEVCFG_ISR_RX_FIFO_OV) {
			writel(DEVCFG_MCTRL_RFIFO_FLUSH, &devcfg_base->mctrl);
			writel(0xFFFFFFFF, &devcfg_base->int_sts);
		}
		return FPGA_FAIL;
	}

	status = readl(&devcfg_base->status);

	debug("%s: Status = 0x%08X\n", __func__, status);

	if (status & DEVCFG_STATUS_DMA_CMD_Q_F) {
		debug("%s: Error: device busy\n", __func__);
		return FPGA_FAIL;
	}

	debug("%s: Device ready\n", __func__);

	if (!(status & DEVCFG_STATUS_DMA_CMD_Q_E)) {
		if (!(readl(&devcfg_base->int_sts) & DEVCFG_ISR_DMA_DONE)) {
			/* Error state, transfer cannot occur */
			debug("%s: ISR indicates error\n", __func__);
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

	debug("%s: Source = 0x%08X\n", __func__, (u32)buf);
	debug("%s: Size = %zu\n", __func__, bsize);

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
			debug("%s: Error: isr = 0x%08X\n", __func__,
			      isr_status);
			debug("%s: Write count = 0x%08X\n", __func__,
			      readl(&devcfg_base->write_count));
			debug("%s: Read count = 0x%08X\n", __func__,
			      readl(&devcfg_base->read_count));

			return FPGA_FAIL;
		}
		if (get_timer(ts) > CONFIG_SYS_FPGA_PROG_TIME) {
			printf("%s: Timeout wait for DMA to complete\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: DMA transfer is done\n", __func__);

	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			printf("%s: Timeout wait for FPGA to config\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: FPGA config done\n", __func__);

	/* Clear out the DMA status */
	writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);

	if (!partialbit)
		zynq_slcr_devcfg_enable();

	return FPGA_SUCCESS;
}

int zynq_dump(Xilinx_desc *desc, const void *buf, size_t bsize)
{
	return FPGA_FAIL;
}
