/*
 * Copyright (c) 2013 Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/arch/hardware.h>

#define SLCR_LOCK_MAGIC		0x767B
#define SLCR_UNLOCK_MAGIC	0xDF0D

#define SLCR_IDCODE_MASK	0x1F000
#define SLCR_IDCODE_SHIFT	12

static int slcr_lock = 1; /* 1 means locked, 0 means unlocked */

void zynq_slcr_lock(void)
{
	if (!slcr_lock)
		writel(SLCR_LOCK_MAGIC, &slcr_base->slcr_lock);
}

void zynq_slcr_unlock(void)
{
	if (slcr_lock)
		writel(SLCR_UNLOCK_MAGIC, &slcr_base->slcr_unlock);
}

/* Reset the entire system */
void zynq_slcr_cpu_reset(void)
{
	/*
	 * Unlock the SLCR then reset the system.
	 * Note that this seems to require raw i/o
	 * functions or there's a lockup?
	 */
	zynq_slcr_unlock();

	/*
	 * Clear 0x0F000000 bits of reboot status register to workaround
	 * the FSBL not loading the bitstream after soft-reboot
	 * This is a temporary solution until we know more.
	 */
	clrbits_le32(&slcr_base->reboot_status, 0xF000000);

	writel(1, &slcr_base->pss_rst_ctrl);
}

/* Setup clk for network */
void zynq_slcr_gem_clk_setup(u32 gem_id, u32 rclk, u32 clk)
{
	zynq_slcr_unlock();

	if (gem_id > 1) {
		printf("Non existing GEM id %d\n", gem_id);
		goto out;
	}

	if (gem_id) {
		/* Set divisors for appropriate frequency in GEM_CLK_CTRL */
		writel(clk, &slcr_base->gem1_clk_ctrl);
		/* Configure GEM_RCLK_CTRL */
		writel(rclk, &slcr_base->gem1_rclk_ctrl);
	} else {
		/* Set divisors for appropriate frequency in GEM_CLK_CTRL */
		writel(clk, &slcr_base->gem0_clk_ctrl);
		/* Configure GEM_RCLK_CTRL */
		writel(rclk, &slcr_base->gem0_rclk_ctrl);
	}
	udelay(100000);
out:
	zynq_slcr_lock();
}

void zynq_slcr_devcfg_disable(void)
{
	zynq_slcr_unlock();

	/* Disable AXI interface */
	writel(0xFFFFFFFF, &slcr_base->fpga_rst_ctrl);

	/* Set Level Shifters DT618760 */
	writel(0xA, &slcr_base->lvl_shftr_en);

	zynq_slcr_lock();
}

void zynq_slcr_devcfg_enable(void)
{
	zynq_slcr_unlock();

	/* Set Level Shifters DT618760 */
	writel(0xF, &slcr_base->lvl_shftr_en);

	/* Disable AXI interface */
	writel(0x0, &slcr_base->fpga_rst_ctrl);

	zynq_slcr_lock();
}

u32 zynq_slcr_get_idcode(void)
{
	return (readl(&slcr_base->pss_idcode) & SLCR_IDCODE_MASK) >>
							SLCR_IDCODE_SHIFT;
}
