// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <env.h>
#include <fsl_esdhc_imx.h>
#include <init.h>
#include <miiphy.h>
#include <mmc.h>
#include <sysinfo.h>
#include <version.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/bootm.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

/* Use a version without spaces so we don't have to quote it */
const char version_string[] = PLAIN_VERSION;

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, PHYS_SDRAM_SIZE);

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#if CONFIG_IS_ENABLED(ENV_MMC)
int board_mmc_get_env_dev(int devno)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}
#endif

int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_FEC_MXC
static int setup_fec(int fec_id)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	if (fec_id == 0) {
		/*
		 * Use 50MHz anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
	} else {
		/*
		 * Use 50MHz anatop loopback REF_CLK2 for ENET2,
		 * clear gpr1[14], set gpr1[18].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);
	}

	ret = enable_fec_anatop_clock(fec_id, ENET_50MHZ);
	if (ret)
		return ret;

	enable_enet_clk(1);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x8190);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec(0);
	setup_fec(1);
#endif

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"emmc", MAKE_CFGVAL(0x60, 0x48, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_CHAIN_OF_TRUST) || imx_hab_is_enabled())
		env_set("bootdelay", "-2");

#ifdef CONFIG_FASTBOOT
	unsigned int bmode = readl(&src_base->sbmr1);

	/*
	 * If BOOT_MODE = 1 then we booted from SDP.
	 * If USB0 is running, then we also (probably) booted from SDP
	 * In either case, start fastboot
	 */
	if (((bmode >> 24) & 3) == 1 || is_usbotg_phy_active())
		env_set("preboot", "setenv preboot; run altbootcmd");
#endif

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#if defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG) && !defined(CONFIG_SPL_BUILD)
	{
		int ret;
		char rev[64];
		struct udevice *sysinfo;

		ret = sysinfo_get(&sysinfo);
		if (ret)
			return ret;

		ret = sysinfo_detect(sysinfo);
		if (ret)
			return ret;

		ret = sysinfo_get_str(sysinfo, SYSINFO_ID_BOARD_MODEL, sizeof(rev), rev);
		if (ret)
			return ret;

		env_set("board_name", "Tiago");
		env_set("board_rev", rev);
	}
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

#define PAD_CTRL_SD (PAD_CTL_DSE_52ohm | PAD_CTL_SPEED_MED | PAD_CTL_PKE | \
		     PAD_CTL_PUE | PAD_CTL_PUS_100K_UP | PAD_CTL_HYS | \
		     PAD_CTL_SRE_FAST)

#define PAD_CTRL_CLK (PAD_CTL_SRE_FAST | PAD_CTL_DSE_52ohm | \
		      PAD_CTL_SPEED_MED | PAD_CTL_HYS)

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_NAND_ALE__USDHC2_RESET_B | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_WE_B__USDHC2_CMD	 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_RE_B__USDHC2_CLK | MUX_PAD_CTRL(PAD_CTRL_CLK),
	MX6_PAD_NAND_DATA00__USDHC2_DATA0 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA01__USDHC2_DATA1 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA02__USDHC2_DATA2 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA03__USDHC2_DATA3 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA04__USDHC2_DATA4 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA05__USDHC2_DATA5 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA06__USDHC2_DATA6 | MUX_PAD_CTRL(PAD_CTRL_SD),
	MX6_PAD_NAND_DATA07__USDHC2_DATA7 | MUX_PAD_CTRL(PAD_CTRL_SD),
};

static struct fsl_esdhc_cfg usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}

static const struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x00000018,
	.grp_ctlds = 0x00000018,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000018,
	.grp_b1ds = 0x00000018,
};

static const struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000018,
	.dram_cas = 0x00000018,
	.dram_ras = 0x00000018,
	.dram_reset = 0x00000018,
	.dram_sdba2 = 0x00000000,
	.dram_odt0 = 0x00000018,
	.dram_odt1 = 0x00000018,
	.dram_sdqs0 = 0x00000018,
	.dram_sdqs1 = 0x00000018,
	.dram_dqm0 = 0x00000018,
	.dram_dqm1 = 0x00000018,
};

static const struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize = 0,
	.cs_density = 4,
	.ncs = 2,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x8,
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
	.refsel = 0,	/* Refresh cycles at 64KHz */
	.refr = 1,	/* 2 refresh commands per refresh cycle */
};

static const struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 800,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
	writel(0xFFFFFFFF, &ccm->CCGR7);
}

static void spl_dram_init(void)
{
	int err;
	struct mx6_mmdc_calibration calibration = {
#define DYNAMIC
#ifdef DYNAMIC
		.p0_mpwrdlctl = 0x40404040,
		.p0_mprddlctl = 0x40404040,
#else
		.p0_mpwrdlctl = 0x40402E26,
		.p0_mpdgctrl0 = 0x0158015C,
		.p0_mprddlctl = 0x40403A3E,
		.p0_mpwrdlctl = 0x40402E26,
#endif
	};

	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &calibration, &mem_ddr);

#ifdef DYNAMIC
	err = mmdc_do_write_level_calibration(&ddr_sysinfo);
	if (err) {
		printf("error %d from write level calibration\n", err);
	} else {
		err = mmdc_do_dqs_calibration(&ddr_sysinfo);
		if (err)
			printf("error %d from dqs calibration\n", err);
	}
#endif
}

void board_init_f(ulong dummy)
{
	ulong size;

	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();
	size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, PHYS_SDRAM_SIZE);
	printf("DRAM: %lx\n", size);
	if (size < PHYS_SDRAM_SIZE)
		panic("Could not init DRAM\n");

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
