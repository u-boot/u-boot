/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * based on commit 21b6e480f92ccc38fe0502e3116411d6509d3bf2 of Diag by:
 * Copyright (C) 2015 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "../soc-info.h"
#include "ddrmphy-regs.h"
#include "umc-regs.h"

#define CH_NR	3

enum dram_freq {
	FREQ_1866M,
	FREQ_2133M,
	FREQ_NR,
};

enum dram_size {
	SIZE_0,
	SIZE_512M,
	SIZE_1G,
	SIZE_NR,
};

static u32 ddrphy_pgcr2[FREQ_NR] = {0x00FC7E5D, 0x00FC90AB};
static u32 ddrphy_ptr0[FREQ_NR] = {0x0EA09205, 0x10C0A6C6};
static u32 ddrphy_ptr1[FREQ_NR] = {0x0DAC041B, 0x0FA104B1};
static u32 ddrphy_ptr3[FREQ_NR] = {0x15171e45, 0x18182357};
static u32 ddrphy_ptr4[FREQ_NR] = {0x0e9ad8e9, 0x10b34157};
static u32 ddrphy_dtpr0[FREQ_NR] = {0x35a00d88, 0x39e40e88};
static u32 ddrphy_dtpr1[FREQ_NR] = {0x2288cc2c, 0x228a04d0};
static u32 ddrphy_dtpr2[FREQ_NR] = {0x50005e00, 0x50006a00};
static u32 ddrphy_dtpr3[FREQ_NR] = {0x0010cb49, 0x0010ec89};
static u32 ddrphy_mr0[FREQ_NR] = {0x00000115, 0x00000125};
static u32 ddrphy_mr2[FREQ_NR] = {0x000002a0, 0x000002a8};

/* dependent on package and board design */
static u32 ddrphy_acbdlr0[CH_NR] = {0x0000000c, 0x0000000c, 0x00000009};

static u32 umc_cmdctla[FREQ_NR] = {0x66DD131D, 0x77EE1722};
/*
 * The ch2 is a different generation UMC core.
 * The register spec is different, unfortunately.
 */
static u32 umc_cmdctlb_ch01[FREQ_NR] = {0x13E87C44, 0x18F88C44};
static u32 umc_cmdctlb_ch2[FREQ_NR] = {0x19E8DC44, 0x1EF8EC44};
static u32 umc_spcctla[FREQ_NR][SIZE_NR] = {
	{0x00000000, 0x004A071D, 0x0078071D},
	{0x00000000, 0x0055081E, 0x0089081E},
};

static u32 umc_spcctlb[] = {0x00FF000A, 0x00FF000B};
/* The ch2 is different for some reason only hardware guys know... */
static u32 umc_flowctla_ch01[] = {0x0800001E, 0x08000022};
static u32 umc_flowctla_ch2[] = {0x0800001E, 0x0800001E};

/* DDR multiPHY */
static inline int ddrphy_get_rank(int dx)
{
	return dx / 2;
}

static void ddrphy_fifo_reset(void __iomem *phy_base)
{
	u32 tmp;

	tmp = readl(phy_base + DMPHY_PGCR0);
	tmp &= ~DMPHY_PGCR0_PHYFRST;
	writel(tmp, phy_base + DMPHY_PGCR0);

	udelay(1);

	tmp |= DMPHY_PGCR0_PHYFRST;
	writel(tmp, phy_base + DMPHY_PGCR0);

	udelay(1);
}

static void ddrphy_vt_ctrl(void __iomem *phy_base, int enable)
{
	u32 tmp;

	tmp = readl(phy_base + DMPHY_PGCR1);

	if (enable)
		tmp &= ~DMPHY_PGCR1_INHVT;
	else
		tmp |= DMPHY_PGCR1_INHVT;

	writel(tmp, phy_base + DMPHY_PGCR1);

	if (!enable) {
		while (!(readl(phy_base + DMPHY_PGSR1) & DMPHY_PGSR1_VTSTOP))
			cpu_relax();
	}
}

static void ddrphy_dqs_delay_fixup(void __iomem *phy_base, int nr_dx, int step)
{
	int dx;
	u32 lcdlr1, rdqsd;
	void __iomem *dx_base = phy_base + DMPHY_DX_BASE;

	ddrphy_vt_ctrl(phy_base, 0);

	for (dx = 0; dx < nr_dx; dx++) {
		lcdlr1 = readl(dx_base + DMPHY_DX_LCDLR1);
		rdqsd = (lcdlr1 >> 8) & 0xff;
		rdqsd = clamp(rdqsd + step, 0U, 0xffU);
		lcdlr1 = (lcdlr1 & ~(0xff << 8)) | (rdqsd << 8);
		writel(lcdlr1, dx_base + DMPHY_DX_LCDLR1);
		readl(dx_base + DMPHY_DX_LCDLR1); /* relax */
		dx_base += DMPHY_DX_STRIDE;
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static int ddrphy_get_system_latency(void __iomem *phy_base, int width)
{
	void __iomem *dx_base = phy_base + DMPHY_DX_BASE;
	const int nr_dx = width / 8;
	int dx, rank;
	u32 gtr;
	int dgsl, dgsl_min = INT_MAX, dgsl_max = 0;

	for (dx = 0; dx < nr_dx; dx++) {
		gtr = readl(dx_base + DMPHY_DX_GTR);
		for (rank = 0; rank < 4; rank++) {
			dgsl = gtr & 0x7;
			/* if dgsl is zero, this rank was not trained. skip. */
			if (dgsl) {
				dgsl_min = min(dgsl_min, dgsl);
				dgsl_max = max(dgsl_max, dgsl);
			}
			gtr >>= 3;
		}
		dx_base += DMPHY_DX_STRIDE;
	}

	if (dgsl_min != dgsl_max)
		printf("DQS Gateing System Latencies are not all leveled.\n");

	return dgsl_max;
}

static void ddrphy_init(void __iomem *phy_base, enum dram_freq freq, int width,
			int ch)
{
	u32 tmp;
	void __iomem *zq_base, *dx_base;
	int zq, dx;
	int nr_dx;

	nr_dx = width / 8;

	writel(DMPHY_PIR_ZCALBYP,        phy_base + DMPHY_PIR);
	/*
	 * Disable RGLVT bit (Read DQS Gating LCDL Delay VT Compensation)
	 * to avoid read error issue.
	 */
	writel(0x07d81e37,         phy_base + DMPHY_PGCR0);
	writel(0x0200c4e0,         phy_base + DMPHY_PGCR1);

	tmp = ddrphy_pgcr2[freq];
	if (width >= 32)
		tmp |= DMPHY_PGCR2_DUALCHN | DMPHY_PGCR2_ACPDDC;
	writel(tmp, phy_base + DMPHY_PGCR2);

	writel(ddrphy_ptr0[freq],  phy_base + DMPHY_PTR0);
	writel(ddrphy_ptr1[freq],  phy_base + DMPHY_PTR1);
	writel(0x00083def,         phy_base + DMPHY_PTR2);
	writel(ddrphy_ptr3[freq],  phy_base + DMPHY_PTR3);
	writel(ddrphy_ptr4[freq],  phy_base + DMPHY_PTR4);

	writel(ddrphy_acbdlr0[ch], phy_base + DMPHY_ACBDLR0);

	writel(0x55555555, phy_base + DMPHY_ACIOCR1);
	writel(0x00000000, phy_base + DMPHY_ACIOCR2);
	writel(0x55555555, phy_base + DMPHY_ACIOCR3);
	writel(0x00000000, phy_base + DMPHY_ACIOCR4);
	writel(0x00000055, phy_base + DMPHY_ACIOCR5);
	writel(0x00181aa4, phy_base + DMPHY_DXCCR);

	writel(0x0024641e, phy_base + DMPHY_DSGCR);
	writel(0x0000040b, phy_base + DMPHY_DCR);
	writel(ddrphy_dtpr0[freq], phy_base + DMPHY_DTPR0);
	writel(ddrphy_dtpr1[freq], phy_base + DMPHY_DTPR1);
	writel(ddrphy_dtpr2[freq], phy_base + DMPHY_DTPR2);
	writel(ddrphy_dtpr3[freq], phy_base + DMPHY_DTPR3);
	writel(ddrphy_mr0[freq], phy_base + DMPHY_MR0);
	writel(0x00000006,       phy_base + DMPHY_MR1);
	writel(ddrphy_mr2[freq], phy_base + DMPHY_MR2);
	writel(0x00000000,       phy_base + DMPHY_MR3);

	tmp = 0;
	for (dx = 0; dx < nr_dx; dx++)
		tmp |= BIT(DMPHY_DTCR_RANKEN_SHIFT + ddrphy_get_rank(dx));
	writel(0x90003087 | tmp, phy_base + DMPHY_DTCR);

	writel(0x00000000, phy_base + DMPHY_DTAR0);
	writel(0x00000008, phy_base + DMPHY_DTAR1);
	writel(0x00000010, phy_base + DMPHY_DTAR2);
	writel(0x00000018, phy_base + DMPHY_DTAR3);
	writel(0xdd22ee11, phy_base + DMPHY_DTDR0);
	writel(0x7788bb44, phy_base + DMPHY_DTDR1);

	/* impedance control settings */
	writel(0x04048900, phy_base + DMPHY_ZQCR);

	zq_base = phy_base + DMPHY_ZQ_BASE;
	for (zq = 0; zq < 4; zq++) {
		/*
		 * board-dependent
		 * PXS2: CH0ZQ0=0x5B, CH1ZQ0=0x5B, CH2ZQ0=0x59, others=0x5D
		 */
		writel(0x0007BB5D, zq_base + DMPHY_ZQ_PR);
		zq_base += DMPHY_ZQ_STRIDE;
	}

	/* DATX8 settings */
	dx_base = phy_base + DMPHY_DX_BASE;
	for (dx = 0; dx < 4; dx++) {
		tmp = readl(dx_base + DMPHY_DX_GCR0);
		tmp &= ~DMPHY_DX_GCR0_WLRKEN_MASK;
		tmp |= BIT(DMPHY_DX_GCR0_WLRKEN_SHIFT + ddrphy_get_rank(dx)) &
						DMPHY_DX_GCR0_WLRKEN_MASK;
		writel(tmp, dx_base + DMPHY_DX_GCR0);

		writel(0x00000000, dx_base + DMPHY_DX_GCR1);
		writel(0x00000000, dx_base + DMPHY_DX_GCR2);
		writel(0x00000000, dx_base + DMPHY_DX_GCR3);
		dx_base += DMPHY_DX_STRIDE;
	}

	while (!(readl(phy_base + DMPHY_PGSR0) & DMPHY_PGSR0_IDONE))
		cpu_relax();

	ddrphy_dqs_delay_fixup(phy_base, nr_dx, -4);
}

struct ddrphy_init_sequence {
	char *description;
	u32 init_flag;
	u32 done_flag;
	u32 err_flag;
};

static const struct ddrphy_init_sequence impedance_calibration_sequence[] = {
	{
		"Impedance Calibration",
		DMPHY_PIR_ZCAL,
		DMPHY_PGSR0_ZCDONE,
		DMPHY_PGSR0_ZCERR,
	},
	{ /* sentinel */ }
};

static const struct ddrphy_init_sequence dram_init_sequence[] = {
	{
		"DRAM Initialization",
		DMPHY_PIR_DRAMRST | DMPHY_PIR_DRAMINIT,
		DMPHY_PGSR0_DIDONE,
		0,
	},
	{ /* sentinel */ }
};

static const struct ddrphy_init_sequence training_sequence[] = {
	{
		"Write Leveling",
		DMPHY_PIR_WL,
		DMPHY_PGSR0_WLDONE,
		DMPHY_PGSR0_WLERR,
	},
	{
		"Read DQS Gate Training",
		DMPHY_PIR_QSGATE,
		DMPHY_PGSR0_QSGDONE,
		DMPHY_PGSR0_QSGERR,
	},
	{
		"Write Leveling Adjustment",
		DMPHY_PIR_WLADJ,
		DMPHY_PGSR0_WLADONE,
		DMPHY_PGSR0_WLAERR,
	},
	{
		"Read Bit Deskew",
		DMPHY_PIR_RDDSKW,
		DMPHY_PGSR0_RDDONE,
		DMPHY_PGSR0_RDERR,
	},
	{
		"Write Bit Deskew",
		DMPHY_PIR_WRDSKW,
		DMPHY_PGSR0_WDDONE,
		DMPHY_PGSR0_WDERR,
	},
	{
		"Read Eye Training",
		DMPHY_PIR_RDEYE,
		DMPHY_PGSR0_REDONE,
		DMPHY_PGSR0_REERR,
	},
	{
		"Write Eye Training",
		DMPHY_PIR_WREYE,
		DMPHY_PGSR0_WEDONE,
		DMPHY_PGSR0_WEERR,
	},
	{ /* sentinel */ }
};

static int __ddrphy_training(void __iomem *phy_base,
			     const struct ddrphy_init_sequence *seq)
{
	const struct ddrphy_init_sequence *s;
	u32 pgsr0;
	u32 init_flag = DMPHY_PIR_INIT;
	u32 done_flag = DMPHY_PGSR0_IDONE;
	int timeout = 50000; /* 50 msec is long enough */
#ifdef DISPLAY_ELAPSED_TIME
	ulong start = get_timer(0);
#endif

	for (s = seq; s->description; s++) {
		init_flag |= s->init_flag;
		done_flag |= s->done_flag;
	}

	writel(init_flag, phy_base + DMPHY_PIR);

	do {
		if (--timeout < 0) {
			printf("%s: error: timeout during DDR training\n",
			       __func__);
			return -ETIMEDOUT;
		}
		udelay(1);
		pgsr0 = readl(phy_base + DMPHY_PGSR0);
	} while ((pgsr0 & done_flag) != done_flag);

	for (s = seq; s->description; s++) {
		if (pgsr0 & s->err_flag) {
			printf("%s: error: %s failed\n", __func__,
			       s->description);
			return -EIO;
		}
	}

#ifdef DISPLAY_ELAPSED_TIME
	printf("%s: info: elapsed time %ld msec\n", get_timer(start));
#endif

	return 0;
}

static int ddrphy_impedance_calibration(void __iomem *phy_base)
{
	int ret;
	u32 tmp;

	ret = __ddrphy_training(phy_base, impedance_calibration_sequence);
	if (ret)
		return ret;

	/*
	 * Because of a hardware bug, IDONE flag is set when the first ZQ block
	 * is calibrated.  The flag does not guarantee the completion for all
	 * the ZQ blocks.  Wait a little more just in case.
	 */
	udelay(1);

	/* reflect ZQ settings and enable average algorithm*/
	tmp = readl(phy_base + DMPHY_ZQCR);
	tmp |= DMPHY_ZQCR_FORCE_ZCAL_VT_UPDATE;
	writel(tmp, phy_base + DMPHY_ZQCR);
	tmp &= ~DMPHY_ZQCR_FORCE_ZCAL_VT_UPDATE;
	tmp |= DMPHY_ZQCR_AVGEN;
	writel(tmp, phy_base + DMPHY_ZQCR);

	return 0;
}

static int ddrphy_dram_init(void __iomem *phy_base)
{
	return __ddrphy_training(phy_base, dram_init_sequence);
}

static int ddrphy_training(void __iomem *phy_base)
{
	return __ddrphy_training(phy_base, training_sequence);
}

/* UMC */
static void umc_set_system_latency(void __iomem *umc_dc_base, int phy_latency)
{
	u32 val;
	int latency;

	val = readl(umc_dc_base + UMC_RDATACTL_D0);
	latency = (val & UMC_RDATACTL_RADLTY_MASK) >> UMC_RDATACTL_RADLTY_SHIFT;
	latency += (val & UMC_RDATACTL_RAD2LTY_MASK) >>
						UMC_RDATACTL_RAD2LTY_SHIFT;
	/*
	 * UMC works at the half clock rate of the PHY.
	 * The LSB of latency is ignored
	 */
	latency += phy_latency & ~1;

	val &= ~(UMC_RDATACTL_RADLTY_MASK | UMC_RDATACTL_RAD2LTY_MASK);
	if (latency > 0xf) {
		val |= 0xf << UMC_RDATACTL_RADLTY_SHIFT;
		val |= (latency - 0xf) << UMC_RDATACTL_RAD2LTY_SHIFT;
	} else {
		val |= latency << UMC_RDATACTL_RADLTY_SHIFT;
	}

	writel(val, umc_dc_base + UMC_RDATACTL_D0);
	writel(val, umc_dc_base + UMC_RDATACTL_D1);

	readl(umc_dc_base + UMC_RDATACTL_D1); /* relax */
}

/* enable/disable auto refresh */
void umc_refresh_ctrl(void __iomem *umc_dc_base, int enable)
{
	u32 tmp;

	tmp = readl(umc_dc_base + UMC_SPCSETB);
	tmp &= ~UMC_SPCSETB_AREFMD_MASK;

	if (enable)
		tmp |= UMC_SPCSETB_AREFMD_ARB;
	else
		tmp |= UMC_SPCSETB_AREFMD_REG;

	writel(tmp, umc_dc_base + UMC_SPCSETB);
	udelay(1);
}

static void umc_ud_init(void __iomem *umc_base, int ch)
{
	writel(0x00000003, umc_base + UMC_BITPERPIXELMODE_D0);

	if (ch == 2)
		writel(0x00000033, umc_base + UMC_PAIR1DOFF_D0);
}

static void umc_dc_init(void __iomem *umc_dc_base, enum dram_freq freq,
			enum dram_size size, int ch, int width)
{
	int latency;
	u32 val;

	writel(umc_cmdctla[freq], umc_dc_base + UMC_CMDCTLA);

	writel(ch == 2 ? umc_cmdctlb_ch2[freq] : umc_cmdctlb_ch01[freq],
	       umc_dc_base + UMC_CMDCTLB);

	writel(umc_spcctla[freq][size / (width / 16)],
	       umc_dc_base + UMC_SPCCTLA);
	writel(umc_spcctlb[freq], umc_dc_base + UMC_SPCCTLB);

	val = 0x000e000e;
	latency = 12;
	/* ES2 inserted one more FF to the logic. */
	if (uniphier_get_soc_model() >= 2)
		latency += 2;

	if (latency > 0xf) {
		val |= 0xf << UMC_RDATACTL_RADLTY_SHIFT;
		val |= (latency - 0xf) << UMC_RDATACTL_RAD2LTY_SHIFT;
	} else {
		val |= latency << UMC_RDATACTL_RADLTY_SHIFT;
	}

	writel(val, umc_dc_base + UMC_RDATACTL_D0);
	if (width >= 32)
		writel(val, umc_dc_base + UMC_RDATACTL_D1);

	writel(0x04060A02, umc_dc_base + UMC_WDATACTL_D0);
	if (width >= 32)
		writel(0x04060A02, umc_dc_base + UMC_WDATACTL_D1);
	writel(0x04000000, umc_dc_base + UMC_DATASET);
	writel(0x00400020, umc_dc_base + UMC_DCCGCTL);
	writel(0x00000084, umc_dc_base + UMC_FLOWCTLG);
	writel(0x00000000, umc_dc_base + UMC_ACSSETA);

	writel(ch == 2 ? umc_flowctla_ch2[freq] : umc_flowctla_ch01[freq],
	       umc_dc_base + UMC_FLOWCTLA);

	writel(0x00004400, umc_dc_base + UMC_FLOWCTLC);
	writel(0x200A0A00, umc_dc_base + UMC_SPCSETB);
	writel(0x00000520, umc_dc_base + UMC_DFICUPDCTLA);
	writel(0x0000000D, umc_dc_base + UMC_RESPCTL);

	if (ch != 2) {
		writel(0x00202000, umc_dc_base + UMC_FLOWCTLB);
		writel(0xFDBFFFFF, umc_dc_base + UMC_FLOWCTLOB0);
		writel(0xFFFFFFFF, umc_dc_base + UMC_FLOWCTLOB1);
		writel(0x00080700, umc_dc_base + UMC_BSICMAPSET);
	} else {
		writel(0x00200000, umc_dc_base + UMC_FLOWCTLB);
		writel(0x00000000, umc_dc_base + UMC_BSICMAPSET);
	}

	writel(0x00000000, umc_dc_base + UMC_ERRMASKA);
	writel(0x00000000, umc_dc_base + UMC_ERRMASKB);
}

static int umc_init(void __iomem *umc_base, enum dram_freq freq, int ch,
		    enum dram_size size, int width)
{
	void __iomem *umc_dc_base = umc_base + 0x00011000;
	void __iomem *phy_base = umc_base + 0x00030000;
	int ret;

	writel(0x00000002, umc_dc_base + UMC_INITSET);
	while (readl(umc_dc_base + UMC_INITSTAT) & BIT(2))
		cpu_relax();

	/* deassert PHY reset signals */
	writel(UMC_DIOCTLA_CTL_NRST | UMC_DIOCTLA_CFG_NRST,
	       umc_dc_base + UMC_DIOCTLA);

	ddrphy_init(phy_base, freq, width, ch);

	ret = ddrphy_impedance_calibration(phy_base);
	if (ret)
		return ret;

	ddrphy_dram_init(phy_base);
	if (ret)
		return ret;

	umc_dc_init(umc_dc_base, freq, size, ch, width);

	umc_ud_init(umc_base, ch);

	if (size) {
		ret = ddrphy_training(phy_base);
		if (ret)
			return ret;
	}

	udelay(1);

	/* match the system latency between UMC and PHY */
	umc_set_system_latency(umc_dc_base,
			       ddrphy_get_system_latency(phy_base, width));

	udelay(1);

	/* stop auto refresh before clearing FIFO in PHY */
	umc_refresh_ctrl(umc_dc_base, 0);
	ddrphy_fifo_reset(phy_base);
	umc_refresh_ctrl(umc_dc_base, 1);

	udelay(10);

	return 0;
}

static void um_init(void __iomem *um_base)
{
	writel(0x000000ff, um_base + UMC_MBUS0);
	writel(0x000000ff, um_base + UMC_MBUS1);
	writel(0x000000ff, um_base + UMC_MBUS2);
	writel(0x000000ff, um_base + UMC_MBUS3);
}

int proxstream2_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5b600000;
	void __iomem *umc_ch0_base = (void __iomem *)0x5b800000;
	void __iomem *umc_ch1_base = (void __iomem *)0x5ba00000;
	void __iomem *umc_ch2_base = (void __iomem *)0x5bc00000;
	enum dram_freq freq;
	int ret;

	switch (bd->dram_freq) {
	case 1866:
		freq = FREQ_1866M;
		break;
	case 2133:
		freq = FREQ_2133M;
		break;
	default:
		printf("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	ret = umc_init(umc_ch0_base, freq, 0, bd->dram_ch0_size / SZ_256M,
		       bd->dram_ch0_width);
	if (ret) {
		printf("failed to initialize UMC ch0\n");
		return ret;
	}

	ret = umc_init(umc_ch1_base, freq, 1, bd->dram_ch1_size / SZ_256M,
		       bd->dram_ch1_width);
	if (ret) {
		printf("failed to initialize UMC ch1\n");
		return ret;
	}

	ret = umc_init(umc_ch2_base, freq, 2, bd->dram_ch2_size / SZ_256M,
		       bd->dram_ch2_width);
	if (ret) {
		printf("failed to initialize UMC ch2\n");
		return ret;
	}

	um_init(um_base);

	return 0;
}
