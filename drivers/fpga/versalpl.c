// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019, Xilinx, Inc,
 * Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/arch/sys_proto.h>
#include <memalign.h>
#include <versalpl.h>
#include <zynqmp_firmware.h>

static ulong versal_align_dma_buffer(ulong *buf, u32 len)
{
	ulong *new_buf;

	if ((ulong)buf != ALIGN((ulong)buf, ARCH_DMA_MINALIGN)) {
		new_buf = (ulong *)ALIGN((ulong)buf, ARCH_DMA_MINALIGN);
		memcpy(new_buf, buf, len);
		buf = new_buf;
	}

	return (ulong)buf;
}

static int versal_load(xilinx_desc *desc, const void *buf, size_t bsize,
		       bitstream_type bstype)
{
	ulong bin_buf;
	int ret;
	u32 buf_lo, buf_hi;
	u32 ret_payload[5];

	bin_buf = versal_align_dma_buffer((ulong *)buf, bsize);

	debug("%s called!\n", __func__);
	flush_dcache_range(bin_buf, bin_buf + bsize);

	buf_lo = lower_32_bits(bin_buf);
	buf_hi = upper_32_bits(bin_buf);

	ret = xilinx_pm_request(VERSAL_PM_LOAD_PDI, VERSAL_PM_PDI_TYPE, buf_lo,
				buf_hi, 0, ret_payload);
	if (ret)
		puts("PL FPGA LOAD fail\n");

	return ret;
}

struct xilinx_fpga_op versal_op = {
	.load = versal_load,
};
