/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/sizes.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/iomux-v3.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static iomux_v3_cfg_t const uart4_pads[] = {
	IOMUX_PADS(PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

int board_early_init_f(void)
{
	SETUP_IOMUX_PADS(uart4_pads);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

#ifdef CONFIG_ENV_IS_IN_MMC
int board_mmc_get_env_dev(int devno)
{
	/* dev 0 for SD/eSD, dev 1 for MMC/eMMC */
	return (devno == 3) ? 1 : 0;
}

static void mmc_late_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	setenv_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw", dev_no);
	setenv("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
#endif

int board_late_init(void)
{
	switch ((imx6_src_get_boot_mode() & IMX6_BMODE_MASK) >>
			IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
#ifdef CONFIG_ENV_IS_IN_MMC
		mmc_late_init();
#endif
		setenv("modeboot", "mmcboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <libfdt.h>
#include <spl.h>

#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>

/* MMC board initialization is needed till adding DM support in SPL */
#if defined(CONFIG_FSL_ESDHC) && !defined(CONFIG_DM_MMC)
#include <mmc.h>
#include <fsl_esdhc.h>

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |             \
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_HIGH |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC3_BASE_ADDR, 1, 4},
	{USDHC4_BASE_ADDR, 1, 8},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
	case USDHC4_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;

	/*
	* According to the board_mmc_init() the following map is done:
	* (U-boot device node)    (Physical Port)
	* mmc0			USDHC3
	* mmc1			USDHC4
	*/
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			SETUP_IOMUX_PADS(usdhc3_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		case 1:
			SETUP_IOMUX_PADS(usdhc4_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
		default:
			printf("Warning - USDHC%d controller not supporting\n",
			       i + 1);
			return 0;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

#ifdef CONFIG_ENV_IS_IN_MMC
void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 boot_dev = BOOT_DEVICE_MMC1;

	switch ((bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC1 */
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC */
		boot_dev = BOOT_DEVICE_MMC2;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		printf("Wrong board boot order\n");
		break;
	}

	spl_boot_list[0] = boot_dev;
}
#endif
#endif

/*
 * Driving strength:
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */

#define IMX6DQ_DRIVE_STRENGTH		0x30
#define IMX6SDL_DRIVE_STRENGTH		0x28

/* configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	.dram_sdqs0 = 0x28,
	.dram_sdqs1 = 0x28,
	.dram_sdqs2 = 0x28,
	.dram_sdqs3 = 0x28,
	.dram_sdqs4 = 0x28,
	.dram_sdqs5 = 0x28,
	.dram_sdqs6 = 0x28,
	.dram_sdqs7 = 0x28,
	.dram_dqm0 = 0x28,
	.dram_dqm1 = 0x28,
	.dram_dqm2 = 0x28,
	.dram_dqm3 = 0x28,
	.dram_dqm4 = 0x28,
	.dram_dqm5 = 0x28,
	.dram_dqm6 = 0x28,
	.dram_dqm7 = 0x28,
	.dram_cas = 0x30,
	.dram_ras = 0x30,
	.dram_sdclk_0 = 0x30,
	.dram_sdclk_1 = 0x30,
	.dram_reset = 0x30,
	.dram_sdcke0 = 0x3000,
	.dram_sdcke1 = 0x3000,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = 0x30,
	.dram_sdodt1 = 0x30,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	.grp_b0ds = 0x30,
	.grp_b1ds = 0x30,
	.grp_b2ds = 0x30,
	.grp_b3ds = 0x30,
	.grp_b4ds = 0x30,
	.grp_b5ds = 0x30,
	.grp_b6ds = 0x30,
	.grp_b7ds = 0x30,
	.grp_addds = 0x30,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ctlds = 0x30,
	.grp_ddr_type = 0x000c0000,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	.dram_sdclk_0 = 0x30,
	.dram_sdclk_1 = 0x30,
	.dram_cas = 0x30,
	.dram_ras = 0x30,
	.dram_reset = 0x30,
	.dram_sdcke0 = 0x30,
	.dram_sdcke1 = 0x30,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = 0x30,
	.dram_sdodt1 = 0x30,
	.dram_sdqs0 = 0x28,
	.dram_sdqs1 = 0x28,
	.dram_sdqs2 = 0x28,
	.dram_sdqs3 = 0x28,
	.dram_sdqs4 = 0x28,
	.dram_sdqs5 = 0x28,
	.dram_sdqs6 = 0x28,
	.dram_sdqs7 = 0x28,
	.dram_dqm0 = 0x28,
	.dram_dqm1 = 0x28,
	.dram_dqm2 = 0x28,
	.dram_dqm3 = 0x28,
	.dram_dqm4 = 0x28,
	.dram_dqm5 = 0x28,
	.dram_dqm6 = 0x28,
	.dram_dqm7 = 0x28,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x30,
	.grp_ctlds = 0x30,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x28,
	.grp_b1ds = 0x28,
	.grp_b2ds = 0x28,
	.grp_b3ds = 0x28,
	.grp_b4ds = 0x28,
	.grp_b5ds = 0x28,
	.grp_b6ds = 0x28,
	.grp_b7ds = 0x28,
};

/* mt41j256 */
static struct mx6_ddr3_cfg mt41j256 = {
	.mem_speed = 1066,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 13,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT = 0,
};

static struct mx6_mmdc_calibration mx6dq_mmdc_calib = {
	.p0_mpwldectrl0 = 0x000E0009,
	.p0_mpwldectrl1 = 0x0018000E,
	.p1_mpwldectrl0 = 0x00000007,
	.p1_mpwldectrl1 = 0x00000000,
	.p0_mpdgctrl0 = 0x43280334,
	.p0_mpdgctrl1 = 0x031C0314,
	.p1_mpdgctrl0 = 0x4318031C,
	.p1_mpdgctrl1 = 0x030C0258,
	.p0_mprddlctl = 0x3E343A40,
	.p1_mprddlctl = 0x383C3844,
	.p0_mpwrdlctl = 0x40404440,
	.p1_mpwrdlctl = 0x4C3E4446,
};

/* DDR 64bit */
static struct mx6_ddr_sysinfo mem_q = {
	.ddr_type	= DDR_TYPE_DDR3,
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 2,
	.rtt_wr		= 2,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static struct mx6_mmdc_calibration mx6dl_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001F0024,
	.p0_mpwldectrl1 = 0x00110018,
	.p1_mpwldectrl0 = 0x001F0024,
	.p1_mpwldectrl1 = 0x00110018,
	.p0_mpdgctrl0 = 0x4230022C,
	.p0_mpdgctrl1 = 0x02180220,
	.p1_mpdgctrl0 = 0x42440248,
	.p1_mpdgctrl1 = 0x02300238,
	.p0_mprddlctl = 0x44444A48,
	.p1_mprddlctl = 0x46484A42,
	.p0_mpwrdlctl = 0x38383234,
	.p1_mpwrdlctl = 0x3C34362E,
};

/* DDR 64bit 1GB */
static struct mx6_ddr_sysinfo mem_dl = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 1,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

/* DDR 32bit 512MB */
static struct mx6_ddr_sysinfo mem_s = {
	.dsize		= 1,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 1,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00003F3F, &ccm->CCGR0);
	writel(0x0030FC00, &ccm->CCGR1);
	writel(0x000FC000, &ccm->CCGR2);
	writel(0x3F300000, &ccm->CCGR3);
	writel(0xFF00F300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003CC, &ccm->CCGR6);
}

static void gpr_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* enable AXI cache for VDOA/VPU/IPU */
	writel(0xF00000CF, &iomux->gpr[4]);
	/* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
	writel(0x007F007F, &iomux->gpr[6]);
	writel(0x007F007F, &iomux->gpr[7]);
}

static void spl_dram_init(void)
{
	if (is_mx6solo()) {
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_s, &mx6dl_mmdc_calib, &mt41j256);
	} else if (is_mx6dl()) {
		mx6sdl_dram_iocfg(64, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_dl, &mx6dl_mmdc_calib, &mt41j256);
	} else if (is_mx6dq()) {
		mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
		mx6_dram_cfg(&mem_q, &mx6dq_mmdc_calib, &mt41j256);
	}

	udelay(100);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	gpr_init();

	/* iomux */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
