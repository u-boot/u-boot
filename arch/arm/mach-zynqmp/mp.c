// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <config.h>
#include <cpu_func.h>
#include <log.h>
#include <vsprintf.h>
#include <zynqmp_firmware.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/string.h>

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

#define ZYNQMP_R5_0_TCM_START_ADDR		0xFFE00000
#define ZYNQMP_R5_1_TCM_START_ADDR		0xFFE90000
#define ZYNQMP_TCM_BOTH_SIZE			0x40000

#define ZYNQMP_CORE_APU0	0
#define ZYNQMP_CORE_APU3	3
#define ZYNQMP_CORE_RPU0	4
#define ZYNQMP_CORE_RPU1	5

#define ZYNQMP_MAX_CORES	6

#define ZYNQMP_RPU0_USE_MASK BIT(1)
#define ZYNQMP_RPU1_USE_MASK BIT(2)

int is_core_valid(unsigned int core)
{
	if (core < ZYNQMP_MAX_CORES)
		return 1;

	return 0;
}

int cpu_reset(u32 nr)
{
	puts("Feature is not implemented.\n");
	return 0;
}

static void set_r5_halt_mode(u32 nr, u8 halt, enum tcm_mode mode)
{
	u32 tmp;

	if (mode == TCM_LOCK || nr == ZYNQMP_CORE_RPU0) {
		tmp = readl(&rpu_base->rpu0_cfg);
		if (halt == HALT)
			tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		writel(tmp, &rpu_base->rpu0_cfg);
	}

	if (mode == TCM_LOCK || nr == ZYNQMP_CORE_RPU1) {
		tmp = readl(&rpu_base->rpu1_cfg);
		if (halt == HALT)
			tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		writel(tmp, &rpu_base->rpu1_cfg);
	}
}

static void set_r5_tcm_mode(enum tcm_mode mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	if (mode == TCM_LOCK) {
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

static void set_r5_reset(u32 nr, enum tcm_mode mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	if (mode == TCM_LOCK) {
		tmp |= (ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
			ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK |
			ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK);
	} else {
		if (nr == ZYNQMP_CORE_RPU0) {
			tmp |= ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK;
			if (tmp & ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK)
				tmp |= ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK;
		} else {
			tmp |= ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK;
			if (tmp & ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK)
				tmp |= ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK;
		}
	}

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void release_r5_reset(u32 nr, enum tcm_mode mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	if (mode == TCM_LOCK || nr == ZYNQMP_CORE_RPU0)
		tmp &= ~(ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
			 ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK);

	if (mode == TCM_LOCK || nr == ZYNQMP_CORE_RPU1)
		tmp &= ~(ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
			 ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK);

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void enable_clock_r5(void)
{
	u32 tmp;

	tmp = readl(&crlapb_base->cpu_r5_ctrl);
	tmp |= ZYNQMP_CRLAPB_CPU_R5_CTRL_CLKACT_MASK;
	writel(tmp, &crlapb_base->cpu_r5_ctrl);

	/* Give some delay for clock
	 * to propagate */
	udelay(0x500);
}

static int check_r5_mode(void)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	if (tmp & ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK)
		return TCM_SPLIT;

	return TCM_LOCK;
}

int cpu_disable(u32 nr)
{
	if (nr <= ZYNQMP_CORE_APU3) {
		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val |= 1 << nr;
		writel(val, &crfapb_base->rst_fpd_apu);
	} else {
		set_r5_reset(nr, check_r5_mode());
	}

	return 0;
}

int cpu_status(u32 nr)
{
	if (nr <= ZYNQMP_CORE_APU3) {
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

static void write_tcm_boot_trampoline(u32 nr, u32 boot_addr)
{
	if (boot_addr) {
		u64 tcm_start_addr = ZYNQMP_R5_0_TCM_START_ADDR;

		if (nr == ZYNQMP_CORE_RPU1)
			tcm_start_addr = ZYNQMP_R5_1_TCM_START_ADDR;

		/*
		 * Boot trampoline is simple ASM code below.
		 *
		 *		b over;
		 *	label:
		 *	.word	0
		 *	over:	ldr	r0, =label
		 *		ldr	r1, [r0]
		 *		bx	r1
		 */
		debug("Write boot trampoline for %x\n", boot_addr);
		writel(0xea000000, tcm_start_addr);
		writel(boot_addr, tcm_start_addr + 0x4);
		writel(0xe59f0004, tcm_start_addr + 0x8);
		writel(0xe5901000, tcm_start_addr + 0xc);
		writel(0xe12fff11, tcm_start_addr + 0x10);
		writel(0x00000004, tcm_start_addr + 0x14);
	}
}

void initialize_tcm(enum tcm_mode mode)
{
	if (mode == TCM_LOCK) {
		set_r5_tcm_mode(TCM_LOCK);
		set_r5_halt_mode(ZYNQMP_CORE_RPU0, HALT, TCM_LOCK);
		enable_clock_r5();
		release_r5_reset(ZYNQMP_CORE_RPU0, TCM_LOCK);
	} else {
		set_r5_tcm_mode(TCM_SPLIT);
		set_r5_halt_mode(ZYNQMP_CORE_RPU0, HALT, TCM_SPLIT);
		set_r5_halt_mode(ZYNQMP_CORE_RPU1, HALT, TCM_SPLIT);
		enable_clock_r5();
		release_r5_reset(ZYNQMP_CORE_RPU0, TCM_SPLIT);
		release_r5_reset(ZYNQMP_CORE_RPU1, TCM_SPLIT);
	}
}

int check_tcm_mode(enum tcm_mode mode)
{
	u32 tmp, cpu_state;
	enum tcm_mode mode_prev;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	mode_prev = FIELD_GET(ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK, tmp);

	tmp = readl(&crlapb_base->rst_lpd_top);
	cpu_state = FIELD_GET(ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK |
			      ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK, tmp);
	cpu_state = cpu_state ? false : true;

	if ((mode_prev == TCM_SPLIT && mode == TCM_LOCK) && cpu_state)
		return -EACCES;

	if (mode_prev == mode)
		return -EAGAIN;

	return 0;
}

static void mark_r5_used(u32 nr, enum tcm_mode mode)
{
	u32 mask = 0;

	if (mode == TCM_LOCK) {
		mask = ZYNQMP_RPU0_USE_MASK | ZYNQMP_RPU1_USE_MASK;
	} else {
		switch (nr) {
		case ZYNQMP_CORE_RPU0:
			mask = ZYNQMP_RPU0_USE_MASK;
			break;
		case ZYNQMP_CORE_RPU1:
			mask = ZYNQMP_RPU1_USE_MASK;
			break;
		default:
			return;
		}
	}
	zynqmp_mmio_write((ulong)&pmu_base->gen_storage4, mask, mask);
}

int cpu_release(u32 nr, int argc, char *const argv[])
{
	if (nr <= ZYNQMP_CORE_APU3) {
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

		u32 boot_addr = hextoul(argv[0], NULL);
		u32 boot_addr_uniq = 0;
		if (!(boot_addr == ZYNQMP_R5_LOVEC_ADDR ||
		      boot_addr == ZYNQMP_R5_HIVEC_ADDR)) {
			printf("Using TCM jump trampoline for address 0x%x\n",
			       boot_addr);
			/* Save boot address for later usage */
			boot_addr_uniq = boot_addr;
			/*
			 * R5 needs to start from LOVEC at TCM
			 * OCM will be probably occupied by ATF
			 */
			boot_addr = ZYNQMP_R5_LOVEC_ADDR;
		}

		/*
		 * Since we don't know where the user may have loaded the image
		 * for an R5 we have to flush all the data cache to ensure
		 * the R5 sees it.
		 */
		flush_dcache_all();

		if (!strcmp(argv[1], "lockstep") || !strcmp(argv[1], "0")) {
			if (nr != ZYNQMP_CORE_RPU0) {
				printf("Lockstep mode should run on ZYNQMP_CORE_RPU0\n");
				return 1;
			}
			printf("R5 lockstep mode\n");
			set_r5_reset(nr, TCM_LOCK);
			set_r5_tcm_mode(TCM_LOCK);
			set_r5_halt_mode(nr, HALT, TCM_LOCK);
			set_r5_start(boot_addr);
			enable_clock_r5();
			release_r5_reset(nr, TCM_LOCK);
			dcache_disable();
			write_tcm_boot_trampoline(nr, boot_addr_uniq);
			dcache_enable();
			set_r5_halt_mode(nr, RELEASE, TCM_LOCK);
			mark_r5_used(nr, TCM_LOCK);
		} else if (!strcmp(argv[1], "split") || !strcmp(argv[1], "1")) {
			printf("R5 split mode\n");
			set_r5_reset(nr, TCM_SPLIT);
			set_r5_tcm_mode(TCM_SPLIT);
			set_r5_halt_mode(nr, HALT, TCM_SPLIT);
			set_r5_start(boot_addr);
			enable_clock_r5();
			release_r5_reset(nr, TCM_SPLIT);
			dcache_disable();
			write_tcm_boot_trampoline(nr, boot_addr_uniq);
			dcache_enable();
			set_r5_halt_mode(nr, RELEASE, TCM_SPLIT);
			mark_r5_used(nr, TCM_SPLIT);
		} else {
			printf("Unsupported mode\n");
			return 1;
		}
	}

	return 0;
}
