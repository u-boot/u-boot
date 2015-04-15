/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

#define LOCK		0
#define SPLIT		1

#define HALT		0
#define RELEASE		1

#define ZYNQMP_BOOTADDR_HIGH_MASK		0xFFFFFFFF
#define ZYNQMP_R5_HIVEC_ADDR			0xFFFF0000
#define ZYNQMP_R5_LOVEC_ADDR			0x0
#define ZYNQMP_RPU_CFG_CPU_HALT_MASK		0x01
#define ZYNQMP_RPU_CFG_HIVEC_MASK		0x04
#define ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK	0x08
#define ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK	0x40
#define ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK	0x10

#define ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK	0x04
#define ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK	0x01
#define ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK	0x02
#define ZYNQMP_CRLAPB_CPU_R5_CTRL_CLKACT_MASK	0x1000000

#define ZYNQMP_TCM_START_ADDRESS		0xFFE00000
#define ZYNQMP_TCM_BOTH_SIZE			0x40000

#define ZYNQMP_CORE_APU0	0
#define ZYNQMP_CORE_APU3	3

#define ZYNQMP_MAX_CORES	6

int is_core_valid(unsigned int core)
{
	if (core < ZYNQMP_MAX_CORES)
		return 1;

	return 0;
}

int cpu_reset(int nr)
{
	puts("Feature is not implemented.\n");
	return 0;
}

static void set_r5_halt_mode(u8 halt, u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu0_cfg);
	if (halt == HALT)
		tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
	else
		tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
	writel(tmp, &rpu_base->rpu0_cfg);

	if (mode == LOCK) {
		tmp = readl(&rpu_base->rpu1_cfg);
		if (halt == HALT)
			tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		writel(tmp, &rpu_base->rpu1_cfg);
	}
}

static void set_r5_tcm_mode(u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	if (mode == LOCK) {
		tmp &= ~ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp |= ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK;
	} else {
		tmp |= ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp &= ~(ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK);
	}

	writel(tmp, &rpu_base->rpu_glbl_ctrl);
}

static void set_r5_reset(u8 mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	tmp |= (ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
	       ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK);

	if (mode == LOCK)
		tmp |= ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK;

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void release_r5_reset(u8 mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	tmp &= ~(ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
	       ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK);

	if (mode == LOCK)
		tmp &= ~ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK;

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void enable_clock_r5(void)
{
	u32 tmp;

	tmp = readl(&crlapb_base->cpu_r5_ctrl);
	tmp |= ZYNQMP_CRLAPB_CPU_R5_CTRL_CLKACT_MASK;
	writel(tmp, &crlapb_base->cpu_r5_ctrl);

	/* Give some delay for clock
	 * to propogate */
	udelay(0x500);
}

int cpu_disable(int nr)
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val |= 1 << nr;
		writel(val, &crfapb_base->rst_fpd_apu);
	} else {
		set_r5_reset(LOCK);
	}

	return 0;
}

int cpu_status(int nr)
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u32 addr_low = readl(((u8 *)&apu_base->rvbar_addr0_l) + nr * 8);
		u32 addr_high = readl(((u8 *)&apu_base->rvbar_addr0_h) +
				      nr * 8);
		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val &= 1 << nr;
		printf("APU CPU%d %s - starting address HI: %x, LOW: %x\n",
		       nr, val ? "OFF" : "ON" , addr_high, addr_low);
	} else {
		u32 val = readl(&crlapb_base->rst_lpd_top);
		val &= 1 << (nr - 4);
		printf("RPU CPU%d %s\n", nr - 4, val ? "OFF" : "ON");
	}

	return 0;
}

static void set_r5_start(u8 high)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu0_cfg);
	if (high)
		tmp |= ZYNQMP_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~ZYNQMP_RPU_CFG_HIVEC_MASK;
	writel(tmp, &rpu_base->rpu0_cfg);

	tmp = readl(&rpu_base->rpu1_cfg);
	if (high)
		tmp |= ZYNQMP_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~ZYNQMP_RPU_CFG_HIVEC_MASK;
	writel(tmp, &rpu_base->rpu1_cfg);
}

int cpu_release(int nr, int argc, char * const argv[])
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u64 boot_addr = simple_strtoull(argv[0], NULL, 16);
		/* HIGH */
		writel((u32)(boot_addr >> 32),
		       ((u8 *)&apu_base->rvbar_addr0_h) + nr * 8);
		/* LOW */
		writel((u32)(boot_addr & ZYNQMP_BOOTADDR_HIGH_MASK),
		       ((u8 *)&apu_base->rvbar_addr0_l) + nr * 8);

		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val &= ~(1 << nr);
		writel(val, &crfapb_base->rst_fpd_apu);
	} else {
		if (argc != 2) {
			printf("Invalid number of arguments to release.\n");
			printf("<addr> <mode>-Start addr lockstep or split\n");
			return 1;
		}

		u32 boot_addr = simple_strtoul(argv[0], NULL, 16);
		if (!(boot_addr == ZYNQMP_R5_LOVEC_ADDR ||
		      boot_addr == ZYNQMP_R5_HIVEC_ADDR)) {
			printf("Invalid starting address 0x%x\n", boot_addr);
			printf("0 or 0xffff0000 are permitted\n");
			return 1;
		}

		if (!strncmp(argv[1], "lockstep", 8)) {
			printf("R5 lockstep mode\n");
			set_r5_tcm_mode(LOCK);
			set_r5_halt_mode(HALT, LOCK);

			if (boot_addr == 0)
				set_r5_start(0);
			else
				set_r5_start(1);

			enable_clock_r5();
			release_r5_reset(LOCK);
			set_r5_halt_mode(RELEASE, LOCK);
		} else if (!strncmp(argv[1], "split", 5)) {
			printf("R5 split mode\n");
			set_r5_tcm_mode(SPLIT);
			set_r5_halt_mode(HALT, SPLIT);
			enable_clock_r5();
			release_r5_reset(SPLIT);
			set_r5_halt_mode(RELEASE, SPLIT);
		} else {
			printf("Unsupported mode\n");
			return 1;
		}
	}

	return 0;
}
