/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bootm.h>
#include <common.h>
#include <netdev.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <imx_thermal.h>
#include <ipu_pixfmt.h>
#include <thermal.h>
#include <sata.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
static u32 reset_cause = -1;

static char *get_reset_cause(void)
{
	u32 cause;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);
	reset_cause = cause;

	switch (cause) {
	case 0x00001:
	case 0x00011:
		return "POR";
	case 0x00004:
		return "CSU";
	case 0x00008:
		return "IPP USER";
	case 0x00010:
#ifdef	CONFIG_MX7
		return "WDOG1";
#else
		return "WDOG";
#endif
	case 0x00020:
		return "JTAG HIGH-Z";
	case 0x00040:
		return "JTAG SW";
	case 0x00080:
		return "WDOG3";
#ifdef CONFIG_MX7
	case 0x00100:
		return "WDOG4";
	case 0x00200:
		return "TEMPSENSE";
#else
	case 0x00100:
		return "TEMPSENSE";
	case 0x10000:
		return "WARM BOOT";
#endif
	default:
		return "unknown reset";
	}
}

u32 get_imx_reset_cause(void)
{
	return reset_cause;
}
#endif

#if defined(CONFIG_MX53) || defined(CONFIG_MX6)
#if defined(CONFIG_MX53)
#define MEMCTL_BASE	ESDCTL_BASE_ADDR
#else
#define MEMCTL_BASE	MMDC_P0_BASE_ADDR
#endif
static const unsigned char col_lookup[] = {9, 10, 11, 8, 12, 9, 9, 9};
static const unsigned char bank_lookup[] = {3, 2};

/* these MMDC registers are common to the IMX53 and IMX6 */
struct esd_mmdc_regs {
	uint32_t	ctl;
	uint32_t	pdc;
	uint32_t	otc;
	uint32_t	cfg0;
	uint32_t	cfg1;
	uint32_t	cfg2;
	uint32_t	misc;
};

#define ESD_MMDC_CTL_GET_ROW(mdctl)	((ctl >> 24) & 7)
#define ESD_MMDC_CTL_GET_COLUMN(mdctl)	((ctl >> 20) & 7)
#define ESD_MMDC_CTL_GET_WIDTH(mdctl)	((ctl >> 16) & 3)
#define ESD_MMDC_CTL_GET_CS1(mdctl)	((ctl >> 30) & 1)
#define ESD_MMDC_MISC_GET_BANK(mdmisc)	((misc >> 5) & 1)

/*
 * imx_ddr_size - return size in bytes of DRAM according MMDC config
 * The MMDC MDCTL register holds the number of bits for row, col, and data
 * width and the MMDC MDMISC register holds the number of banks. Combine
 * all these bits to determine the meme size the MMDC has been configured for
 */
unsigned imx_ddr_size(void)
{
	struct esd_mmdc_regs *mem = (struct esd_mmdc_regs *)MEMCTL_BASE;
	unsigned ctl = readl(&mem->ctl);
	unsigned misc = readl(&mem->misc);
	int bits = 11 + 0 + 0 + 1;      /* row + col + bank + width */

	bits += ESD_MMDC_CTL_GET_ROW(ctl);
	bits += col_lookup[ESD_MMDC_CTL_GET_COLUMN(ctl)];
	bits += bank_lookup[ESD_MMDC_MISC_GET_BANK(misc)];
	bits += ESD_MMDC_CTL_GET_WIDTH(ctl);
	bits += ESD_MMDC_CTL_GET_CS1(ctl);

	/* The MX6 can do only 3840 MiB of DRAM */
	if (bits == 32)
		return 0xf0000000;

	return 1 << bits;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)

const char *get_imx_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_MX7D:
		return "7D";	/* Dual-core version of the mx7 */
	case MXC_CPU_MX6QP:
		return "6QP";	/* Quad-Plus version of the mx6 */
	case MXC_CPU_MX6DP:
		return "6DP";	/* Dual-Plus version of the mx6 */
	case MXC_CPU_MX6Q:
		return "6Q";	/* Quad-core version of the mx6 */
	case MXC_CPU_MX6D:
		return "6D";	/* Dual-core version of the mx6 */
	case MXC_CPU_MX6DL:
		return "6DL";	/* Dual Lite version of the mx6 */
	case MXC_CPU_MX6SOLO:
		return "6SOLO";	/* Solo version of the mx6 */
	case MXC_CPU_MX6SL:
		return "6SL";	/* Solo-Lite version of the mx6 */
	case MXC_CPU_MX6SX:
		return "6SX";   /* SoloX version of the mx6 */
	case MXC_CPU_MX6UL:
		return "6UL";   /* Ultra-Lite version of the mx6 */
	case MXC_CPU_MX51:
		return "51";
	case MXC_CPU_MX53:
		return "53";
	default:
		return "??";
	}
}

int print_cpuinfo(void)
{
	u32 cpurev;
	__maybe_unused u32 max_freq;

	cpurev = get_cpu_rev();

#if defined(CONFIG_IMX_THERMAL)
	struct udevice *thermal_dev;
	int cpu_tmp, minc, maxc, ret;

	printf("CPU:   Freescale i.MX%s rev%d.%d",
	       get_imx_type((cpurev & 0xFF000) >> 12),
	       (cpurev & 0x000F0) >> 4,
	       (cpurev & 0x0000F) >> 0);
	max_freq = get_cpu_speed_grade_hz();
	if (!max_freq || max_freq == mxc_get_clock(MXC_ARM_CLK)) {
		printf(" at %dMHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	} else {
		printf(" %d MHz (running at %d MHz)\n", max_freq / 1000000,
		       mxc_get_clock(MXC_ARM_CLK) / 1000000);
	}
#else
	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
		get_imx_type((cpurev & 0xFF000) >> 12),
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
#endif

#if defined(CONFIG_IMX_THERMAL)
	puts("CPU:   ");
	switch (get_cpu_temp_grade(&minc, &maxc)) {
	case TEMP_AUTOMOTIVE:
		puts("Automotive temperature grade ");
		break;
	case TEMP_INDUSTRIAL:
		puts("Industrial temperature grade ");
		break;
	case TEMP_EXTCOMMERCIAL:
		puts("Extended Commercial temperature grade ");
		break;
	default:
		puts("Commercial temperature grade ");
		break;
	}
	printf("(%dC to %dC)", minc, maxc);
	ret = uclass_get_device(UCLASS_THERMAL, 0, &thermal_dev);
	if (!ret) {
		ret = thermal_get_temp(thermal_dev, &cpu_tmp);

		if (!ret)
			printf(" at %dC\n", cpu_tmp);
		else
			debug(" - invalid sensor data\n");
	} else {
		debug(" - invalid sensor device\n");
	}
#endif

	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

#ifndef CONFIG_MX7
u32 get_ahb_clk(void)
{
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg, ahb_podf;

	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= MXC_CCM_CBCDR_AHB_PODF_MASK;
	ahb_podf = reg >> MXC_CCM_CBCDR_AHB_PODF_OFFSET;

	return get_periph_clk() / (ahb_podf + 1);
}
#endif

void arch_preboot_os(void)
{
#if defined(CONFIG_CMD_SATA)
	sata_stop();
#if defined(CONFIG_MX6)
	disable_sata_clock();
#endif
#endif
#if defined(CONFIG_VIDEO_IPUV3)
	/* disable video before launching O/S */
	ipuv3_fb_shutdown();
#endif
#if defined(CONFIG_VIDEO_MXS)
	lcdif_power_down();
#endif
}

void set_chipselect_size(int const cs_size)
{
	unsigned int reg;
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	reg = readl(&iomuxc_regs->gpr[1]);

	switch (cs_size) {
	case CS0_128:
		reg &= ~0x7;	/* CS0=128MB, CS1=0, CS2=0, CS3=0 */
		reg |= 0x5;
		break;
	case CS0_64M_CS1_64M:
		reg &= ~0x3F;	/* CS0=64MB, CS1=64MB, CS2=0, CS3=0 */
		reg |= 0x1B;
		break;
	case CS0_64M_CS1_32M_CS2_32M:
		reg &= ~0x1FF;	/* CS0=64MB, CS1=32MB, CS2=32MB, CS3=0 */
		reg |= 0x4B;
		break;
	case CS0_32M_CS1_32M_CS2_32M_CS3_32M:
		reg &= ~0xFFF;  /* CS0=32MB, CS1=32MB, CS2=32MB, CS3=32MB */
		reg |= 0x249;
		break;
	default:
		printf("Unknown chip select size: %d\n", cs_size);
		break;
	}

	writel(reg, &iomuxc_regs->gpr[1]);
}
