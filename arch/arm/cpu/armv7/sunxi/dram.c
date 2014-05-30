/*
 * sunxi DRAM controller initialization
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 *
 * Based on sun4i Linux kernel sources mach-sunxi/pm/standby/dram*.c
 * and earlier U-Boot Allwiner A10 SPL work
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Unfortunately the only documentation we have on the sun7i DRAM
 * controller is Allwinner boot0 + boot1 code, and that code uses
 * magic numbers & shifts with no explanations. Hence this code is
 * rather undocumented and full of magic.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/timer.h>
#include <asm/arch/sys_proto.h>

#define CPU_CFG_CHIP_VER(n) ((n) << 6)
#define CPU_CFG_CHIP_VER_MASK CPU_CFG_CHIP_VER(0x3)
#define CPU_CFG_CHIP_REV_A 0x0
#define CPU_CFG_CHIP_REV_C1 0x1
#define CPU_CFG_CHIP_REV_C2 0x2
#define CPU_CFG_CHIP_REV_B 0x3

/*
 * Wait up to 1s for mask to be clear in given reg.
 */
static void await_completion(u32 *reg, u32 mask)
{
	unsigned long tmo = timer_get_us() + 1000000;

	while (readl(reg) & mask) {
		if (timer_get_us() > tmo)
			panic("Timeout initialising DRAM\n");
	}
}

static void mctl_ddr3_reset(void)
{
	struct sunxi_dram_reg *dram =
			(struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	clrbits_le32(&dram->mcr, DRAM_MCR_RESET);
	udelay(2);
	setbits_le32(&dram->mcr, DRAM_MCR_RESET);
}

static void mctl_set_drive(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	clrsetbits_le32(&dram->mcr, DRAM_MCR_MODE_NORM(0x3) | (0x3 << 28),
			DRAM_MCR_MODE_EN(0x3) |
			0xffc);
}

static void mctl_itm_disable(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	clrsetbits_le32(&dram->ccr, DRAM_CCR_INIT, DRAM_CCR_ITM_OFF);
}

static void mctl_itm_enable(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	clrbits_le32(&dram->ccr, DRAM_CCR_ITM_OFF);
}

static void mctl_enable_dll0(u32 phase)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	clrsetbits_le32(&dram->dllcr[0], 0x3f << 6,
			((phase >> 16) & 0x3f) << 6);
	clrsetbits_le32(&dram->dllcr[0], DRAM_DLLCR_NRESET, DRAM_DLLCR_DISABLE);
	udelay(2);

	clrbits_le32(&dram->dllcr[0], DRAM_DLLCR_NRESET | DRAM_DLLCR_DISABLE);
	udelay(22);

	clrsetbits_le32(&dram->dllcr[0], DRAM_DLLCR_DISABLE, DRAM_DLLCR_NRESET);
	udelay(22);
}

/*
 * Note: This differs from pm/standby in that it checks the bus width
 */
static void mctl_enable_dllx(u32 phase)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	u32 i, n, bus_width;

	bus_width = readl(&dram->dcr);

	if ((bus_width & DRAM_DCR_BUS_WIDTH_MASK) ==
	    DRAM_DCR_BUS_WIDTH(DRAM_DCR_BUS_WIDTH_32BIT))
		n = DRAM_DCR_NR_DLLCR_32BIT;
	else
		n = DRAM_DCR_NR_DLLCR_16BIT;

	for (i = 1; i < n; i++) {
		clrsetbits_le32(&dram->dllcr[i], 0xf << 14,
				(phase & 0xf) << 14);
		clrsetbits_le32(&dram->dllcr[i], DRAM_DLLCR_NRESET,
				DRAM_DLLCR_DISABLE);
		phase >>= 4;
	}
	udelay(2);

	for (i = 1; i < n; i++)
		clrbits_le32(&dram->dllcr[i], DRAM_DLLCR_NRESET |
			     DRAM_DLLCR_DISABLE);
	udelay(22);

	for (i = 1; i < n; i++)
		clrsetbits_le32(&dram->dllcr[i], DRAM_DLLCR_DISABLE,
				DRAM_DLLCR_NRESET);
	udelay(22);
}

static u32 hpcr_value[32] = {
#ifdef CONFIG_SUN7I
	0x0301, 0x0301, 0x0301, 0x0301,
	0x0301, 0x0301, 0x0301, 0x0301,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0x1031, 0x1031, 0x0735, 0x1035,
	0x1035, 0x0731, 0x1031, 0x0735,
	0x1035, 0x1031, 0x0731, 0x1035,
	0x0001, 0x1031, 0, 0x1031
	/* last row differs from boot0 source table
	 * 0x1031, 0x0301, 0x0301, 0x0731
	 * but boot0 code skips #28 and #30, and sets #29 and #31 to the
	 * value from #28 entry (0x1031)
	 */
#endif
};

static void mctl_configure_hostport(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	u32 i;

	for (i = 0; i < 32; i++)
		writel(hpcr_value[i], &dram->hpcr[i]);
}

static void mctl_setup_dram_clock(u32 clk)
{
	u32 reg_val;
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* setup DRAM PLL */
	reg_val = readl(&ccm->pll5_cfg);
	reg_val &= ~CCM_PLL5_CTRL_M_MASK;		/* set M to 0 (x1) */
	reg_val &= ~CCM_PLL5_CTRL_K_MASK;		/* set K to 0 (x1) */
	reg_val &= ~CCM_PLL5_CTRL_N_MASK;		/* set N to 0 (x0) */
	reg_val &= ~CCM_PLL5_CTRL_P_MASK;		/* set P to 0 (x1) */
	if (clk >= 540 && clk < 552) {
		/* dram = 540MHz, pll5p = 540MHz */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(2));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(3));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(15));
		reg_val |= CCM_PLL5_CTRL_P(1);
	} else if (clk >= 512 && clk < 528) {
		/* dram = 512MHz, pll5p = 384MHz */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(3));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(4));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(16));
		reg_val |= CCM_PLL5_CTRL_P(2);
	} else if (clk >= 496 && clk < 504) {
		/* dram = 496MHz, pll5p = 372MHz */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(3));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(2));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(31));
		reg_val |= CCM_PLL5_CTRL_P(2);
	} else if (clk >= 468 && clk < 480) {
		/* dram = 468MHz, pll5p = 468MHz */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(2));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(3));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(13));
		reg_val |= CCM_PLL5_CTRL_P(1);
	} else if (clk >= 396 && clk < 408) {
		/* dram = 396MHz, pll5p = 396MHz */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(2));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(3));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(11));
		reg_val |= CCM_PLL5_CTRL_P(1);
	} else 	{
		/* any other frequency that is a multiple of 24 */
		reg_val |= CCM_PLL5_CTRL_M(CCM_PLL5_CTRL_M_X(2));
		reg_val |= CCM_PLL5_CTRL_K(CCM_PLL5_CTRL_K_X(2));
		reg_val |= CCM_PLL5_CTRL_N(CCM_PLL5_CTRL_N_X(clk / 24));
		reg_val |= CCM_PLL5_CTRL_P(CCM_PLL5_CTRL_P_X(2));
	}
	reg_val &= ~CCM_PLL5_CTRL_VCO_GAIN;		/* PLL VCO Gain off */
	reg_val |= CCM_PLL5_CTRL_EN;			/* PLL On */
	writel(reg_val, &ccm->pll5_cfg);
	udelay(5500);

	setbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_DDR_CLK);

#if defined(CONFIG_SUN4I) || defined(CONFIG_SUN7I)
	/* reset GPS */
	clrbits_le32(&ccm->gps_clk_cfg, CCM_GPS_CTRL_RESET | CCM_GPS_CTRL_GATE);
	setbits_le32(&ccm->ahb_gate0, CCM_AHB_GATE_GPS);
	udelay(1);
	clrbits_le32(&ccm->ahb_gate0, CCM_AHB_GATE_GPS);
#endif

	/* setup MBUS clock */
	reg_val = CCM_MBUS_CTRL_GATE |
		  CCM_MBUS_CTRL_CLK_SRC(CCM_MBUS_CTRL_CLK_SRC_PLL6) |
		  CCM_MBUS_CTRL_N(CCM_MBUS_CTRL_N_X(2)) |
		  CCM_MBUS_CTRL_M(CCM_MBUS_CTRL_M_X(2));
	writel(reg_val, &ccm->mbus_clk_cfg);

	/*
	 * open DRAMC AHB & DLL register clock
	 * close it first
	 */
	clrbits_le32(&ccm->ahb_gate0, CCM_AHB_GATE_SDRAM | CCM_AHB_GATE_DLL);
	udelay(22);

	/* then open it */
	setbits_le32(&ccm->ahb_gate0, CCM_AHB_GATE_SDRAM | CCM_AHB_GATE_DLL);
	udelay(22);
}

static int dramc_scan_readpipe(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	u32 reg_val;

	/* data training trigger */
#ifdef CONFIG_SUN7I
	clrbits_le32(&dram->csr, DRAM_CSR_FAILED);
#endif
	setbits_le32(&dram->ccr, DRAM_CCR_DATA_TRAINING);

	/* check whether data training process has completed */
	await_completion(&dram->ccr, DRAM_CCR_DATA_TRAINING);

	/* check data training result */
	reg_val = readl(&dram->csr);
	if (reg_val & DRAM_CSR_FAILED)
		return -1;

	return 0;
}

static int dramc_scan_dll_para(void)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	const u32 dqs_dly[7] = {0x3, 0x2, 0x1, 0x0, 0xe, 0xd, 0xc};
	const u32 clk_dly[15] = {0x07, 0x06, 0x05, 0x04, 0x03,
				 0x02, 0x01, 0x00, 0x08, 0x10,
				 0x18, 0x20, 0x28, 0x30, 0x38};
	u32 clk_dqs_count[15];
	u32 dqs_i, clk_i, cr_i;
	u32 max_val, min_val;
	u32 dqs_index, clk_index;

	/* Find DQS_DLY Pass Count for every CLK_DLY */
	for (clk_i = 0; clk_i < 15; clk_i++) {
		clk_dqs_count[clk_i] = 0;
		clrsetbits_le32(&dram->dllcr[0], 0x3f << 6,
				(clk_dly[clk_i] & 0x3f) << 6);
		for (dqs_i = 0; dqs_i < 7; dqs_i++) {
			for (cr_i = 1; cr_i < 5; cr_i++) {
				clrsetbits_le32(&dram->dllcr[cr_i],
						0x4f << 14,
						(dqs_dly[dqs_i] & 0x4f) << 14);
			}
			udelay(2);
			if (dramc_scan_readpipe() == 0)
				clk_dqs_count[clk_i]++;
		}
	}
	/* Test DQS_DLY Pass Count for every CLK_DLY from up to down */
	for (dqs_i = 15; dqs_i > 0; dqs_i--) {
		max_val = 15;
		min_val = 15;
		for (clk_i = 0; clk_i < 15; clk_i++) {
			if (clk_dqs_count[clk_i] == dqs_i) {
				max_val = clk_i;
				if (min_val == 15)
					min_val = clk_i;
			}
		}
		if (max_val < 15)
			break;
	}

	/* Check if Find a CLK_DLY failed */
	if (!dqs_i)
		goto fail;

	/* Find the middle index of CLK_DLY */
	clk_index = (max_val + min_val) >> 1;
	if ((max_val == (15 - 1)) && (min_val > 0))
		/* if CLK_DLY[MCTL_CLK_DLY_COUNT] is very good, then the middle
		 * value can be more close to the max_val
		 */
		clk_index = (15 + clk_index) >> 1;
	else if ((max_val < (15 - 1)) && (min_val == 0))
		/* if CLK_DLY[0] is very good, then the middle value can be more
		 * close to the min_val
		 */
		clk_index >>= 1;
	if (clk_dqs_count[clk_index] < dqs_i)
		clk_index = min_val;

	/* Find the middle index of DQS_DLY for the CLK_DLY got above, and Scan
	 * read pipe again
	 */
	clrsetbits_le32(&dram->dllcr[0], 0x3f << 6,
			(clk_dly[clk_index] & 0x3f) << 6);
	max_val = 7;
	min_val = 7;
	for (dqs_i = 0; dqs_i < 7; dqs_i++) {
		clk_dqs_count[dqs_i] = 0;
		for (cr_i = 1; cr_i < 5; cr_i++) {
			clrsetbits_le32(&dram->dllcr[cr_i],
					0x4f << 14,
					(dqs_dly[dqs_i] & 0x4f) << 14);
		}
		udelay(2);
		if (dramc_scan_readpipe() == 0) {
			clk_dqs_count[dqs_i] = 1;
			max_val = dqs_i;
			if (min_val == 7)
				min_val = dqs_i;
		}
	}

	if (max_val < 7) {
		dqs_index = (max_val + min_val) >> 1;
		if ((max_val == (7-1)) && (min_val > 0))
			dqs_index = (7 + dqs_index) >> 1;
		else if ((max_val < (7-1)) && (min_val == 0))
			dqs_index >>= 1;
		if (!clk_dqs_count[dqs_index])
			dqs_index = min_val;
		for (cr_i = 1; cr_i < 5; cr_i++) {
			clrsetbits_le32(&dram->dllcr[cr_i],
					0x4f << 14,
					(dqs_dly[dqs_index] & 0x4f) << 14);
		}
		udelay(2);
		return dramc_scan_readpipe();
	}

fail:
	clrbits_le32(&dram->dllcr[0], 0x3f << 6);
	for (cr_i = 1; cr_i < 5; cr_i++)
		clrbits_le32(&dram->dllcr[cr_i], 0x4f << 14);
	udelay(2);

	return dramc_scan_readpipe();
}

static void dramc_clock_output_en(u32 on)
{
#if defined(CONFIG_SUN5I) || defined(CONFIG_SUN7I)
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;

	if (on)
		setbits_le32(&dram->mcr, DRAM_MCR_DCLK_OUT);
	else
		clrbits_le32(&dram->mcr, DRAM_MCR_DCLK_OUT);
#endif
}

static const u16 tRFC_table[2][6] = {
	/*       256Mb    512Mb    1Gb      2Gb      4Gb      8Gb      */
	/* DDR2  75ns     105ns    127.5ns  195ns    327.5ns  invalid  */
	{        77,      108,     131,     200,     336,     336 },
	/* DDR3  invalid  90ns     110ns    160ns    300ns    350ns    */
	{        93,      93,      113,     164,     308,     359 }
};

static void dramc_set_autorefresh_cycle(u32 clk, u32 type, u32 density)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	u32 tRFC, tREFI;

	tRFC = (tRFC_table[type][density] * clk + 1023) >> 10;
	tREFI = (7987 * clk) >> 10;	/* <= 7.8us */

	writel(DRAM_DRR_TREFI(tREFI) | DRAM_DRR_TRFC(tRFC), &dram->drr);
}

unsigned long dramc_init(struct dram_para *para)
{
	struct sunxi_dram_reg *dram = (struct sunxi_dram_reg *)SUNXI_DRAMC_BASE;
	u32 reg_val;
	u32 density;
	int ret_val;

	/* check input dram parameter structure */
	if (!para)
		return 0;

	/* setup DRAM relative clock */
	mctl_setup_dram_clock(para->clock);

	/* reset external DRAM */
	mctl_set_drive();

	/* dram clock off */
	dramc_clock_output_en(0);

	mctl_itm_disable();
	mctl_enable_dll0(para->tpr3);

	/* configure external DRAM */
	reg_val = 0x0;
	if (para->type == DRAM_MEMORY_TYPE_DDR3)
		reg_val |= DRAM_DCR_TYPE_DDR3;
	reg_val |= DRAM_DCR_IO_WIDTH(para->io_width >> 3);

	if (para->density == 256)
		density = DRAM_DCR_CHIP_DENSITY_256M;
	else if (para->density == 512)
		density = DRAM_DCR_CHIP_DENSITY_512M;
	else if (para->density == 1024)
		density = DRAM_DCR_CHIP_DENSITY_1024M;
	else if (para->density == 2048)
		density = DRAM_DCR_CHIP_DENSITY_2048M;
	else if (para->density == 4096)
		density = DRAM_DCR_CHIP_DENSITY_4096M;
	else if (para->density == 8192)
		density = DRAM_DCR_CHIP_DENSITY_8192M;
	else
		density = DRAM_DCR_CHIP_DENSITY_256M;

	reg_val |= DRAM_DCR_CHIP_DENSITY(density);
	reg_val |= DRAM_DCR_BUS_WIDTH((para->bus_width >> 3) - 1);
	reg_val |= DRAM_DCR_RANK_SEL(para->rank_num - 1);
	reg_val |= DRAM_DCR_CMD_RANK_ALL;
	reg_val |= DRAM_DCR_MODE(DRAM_DCR_MODE_INTERLEAVE);
	writel(reg_val, &dram->dcr);

#ifdef CONFIG_SUN7I
	setbits_le32(&dram->zqcr1, (0x1 << 24) | (0x1 << 1));
	if (para->tpr4 & 0x2)
		clrsetbits_le32(&dram->zqcr1, (0x1 << 24), (0x1 << 1));
	dramc_clock_output_en(1);
#endif

#if (defined(CONFIG_SUN5I) || defined(CONFIG_SUN7I))
	/* set odt impendance divide ratio */
	reg_val = ((para->zq) >> 8) & 0xfffff;
	reg_val |= ((para->zq) & 0xff) << 20;
	reg_val |= (para->zq) & 0xf0000000;
	writel(reg_val, &dram->zqcr0);
#endif

#ifdef CONFIG_SUN7I
	/* Set CKE Delay to about 1ms */
	setbits_le32(&dram->idcr, 0x1ffff);
#endif

#ifdef CONFIG_SUN7I
	if ((readl(&dram->ppwrsctl) & 0x1) != 0x1)
		mctl_ddr3_reset();
	else
		setbits_le32(&dram->mcr, DRAM_MCR_RESET);
#endif

	udelay(1);

	await_completion(&dram->ccr, DRAM_CCR_INIT);

	mctl_enable_dllx(para->tpr3);

	/* set refresh period */
	dramc_set_autorefresh_cycle(para->clock, para->type - 2, density);

	/* set timing parameters */
	writel(para->tpr0, &dram->tpr0);
	writel(para->tpr1, &dram->tpr1);
	writel(para->tpr2, &dram->tpr2);

	if (para->type == DRAM_MEMORY_TYPE_DDR3) {
		reg_val = DRAM_MR_BURST_LENGTH(0x0);
#if (defined(CONFIG_SUN5I) || defined(CONFIG_SUN7I))
		reg_val |= DRAM_MR_POWER_DOWN;
#endif
		reg_val |= DRAM_MR_CAS_LAT(para->cas - 4);
		reg_val |= DRAM_MR_WRITE_RECOVERY(0x5);
	} else if (para->type == DRAM_MEMORY_TYPE_DDR2) {
		reg_val = DRAM_MR_BURST_LENGTH(0x2);
		reg_val |= DRAM_MR_CAS_LAT(para->cas);
		reg_val |= DRAM_MR_WRITE_RECOVERY(0x5);
	}
	writel(reg_val, &dram->mr);

	writel(para->emr1, &dram->emr);
	writel(para->emr2, &dram->emr2);
	writel(para->emr3, &dram->emr3);

	/* set DQS window mode */
	clrsetbits_le32(&dram->ccr, DRAM_CCR_DQS_DRIFT_COMP, DRAM_CCR_DQS_GATE);

#ifdef CONFIG_SUN7I
	/* Command rate timing mode 2T & 1T */
	if (para->tpr4 & 0x1)
		setbits_le32(&dram->ccr, DRAM_CCR_COMMAND_RATE_1T);
#endif
	/* reset external DRAM */
	setbits_le32(&dram->ccr, DRAM_CCR_INIT);
	await_completion(&dram->ccr, DRAM_CCR_INIT);

#ifdef CONFIG_SUN7I
	/* setup zq calibration manual */
	reg_val = readl(&dram->ppwrsctl);
	if ((reg_val & 0x1) == 1) {
		/* super_standby_flag = 1 */

		reg_val = readl(0x01c20c00 + 0x120); /* rtc */
		reg_val &= 0x000fffff;
		reg_val |= 0x17b00000;
		writel(reg_val, &dram->zqcr0);

		/* exit self-refresh state */
		clrsetbits_le32(&dram->dcr, 0x1f << 27, 0x12 << 27);
		/* check whether command has been executed */
		await_completion(&dram->dcr, 0x1 << 31);

		udelay(2);

		/* dram pad hold off */
		setbits_le32(&dram->ppwrsctl, 0x16510000);

		await_completion(&dram->ppwrsctl, 0x1);

		/* exit self-refresh state */
		clrsetbits_le32(&dram->dcr, 0x1f << 27, 0x12 << 27);

		/* check whether command has been executed */
		await_completion(&dram->dcr, 0x1 << 31);

		udelay(2);

		/* issue a refresh command */
		clrsetbits_le32(&dram->dcr, 0x1f << 27, 0x13 << 27);
		await_completion(&dram->dcr, 0x1 << 31);

		udelay(2);
	}
#endif

	/* scan read pipe value */
	mctl_itm_enable();
	if (para->tpr3 & (0x1 << 31)) {
		ret_val = dramc_scan_dll_para();
		if (ret_val == 0)
			para->tpr3 =
				(((readl(&dram->dllcr[0]) >> 6) & 0x3f) << 16) |
				(((readl(&dram->dllcr[1]) >> 14) & 0xf) << 0) |
				(((readl(&dram->dllcr[2]) >> 14) & 0xf) << 4) |
				(((readl(&dram->dllcr[3]) >> 14) & 0xf) << 8) |
				(((readl(&dram->dllcr[4]) >> 14) & 0xf) << 12
				);
	} else {
		ret_val = dramc_scan_readpipe();
	}

	if (ret_val < 0)
		return 0;

	/* configure all host port */
	mctl_configure_hostport();

	return get_ram_size((long *)PHYS_SDRAM_0, PHYS_SDRAM_0_SIZE);
}
