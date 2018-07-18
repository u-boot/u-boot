// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/dma.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/rdc-sema.h>
#include <asm/arch/imx-rdc.h>
#include <asm/arch/crm_regs.h>
#include <dm.h>
#include <imx_thermal.h>
#include <fsl_sec.h>
#include <asm/setup.h>

#if defined(CONFIG_IMX_THERMAL)
static const struct imx_thermal_plat imx7_thermal_plat = {
	.regs = (void *)ANATOP_BASE_ADDR,
	.fuse_bank = 3,
	.fuse_word = 3,
};

U_BOOT_DEVICE(imx7_thermal) = {
	.name = "imx_thermal",
	.platdata = &imx7_thermal_plat,
};
#endif

#if CONFIG_IS_ENABLED(IMX_RDC)
/*
 * In current design, if any peripheral was assigned to both A7 and M4,
 * it will receive ipg_stop or ipg_wait when any of the 2 platforms enter
 * low power mode. So M4 sleep will cause some peripherals fail to work
 * at A7 core side. At default, all resources are in domain 0 - 3.
 *
 * There are 26 peripherals impacted by this IC issue:
 * SIM2(sim2/emvsim2)
 * SIM1(sim1/emvsim1)
 * UART1/UART2/UART3/UART4/UART5/UART6/UART7
 * SAI1/SAI2/SAI3
 * WDOG1/WDOG2/WDOG3/WDOG4
 * GPT1/GPT2/GPT3/GPT4
 * PWM1/PWM2/PWM3/PWM4
 * ENET1/ENET2
 * Software Workaround:
 * Here we setup some resources to domain 0 where M4 codes will move
 * the M4 out of this domain. Then M4 is not able to access them any longer.
 * This is a workaround for ic issue. So the peripherals are not shared
 * by them. This way requires the uboot implemented the RDC driver and
 * set the 26 IPs above to domain 0 only. M4 code will assign resource
 * to its own domain, if it want to use the resource.
 */
static rdc_peri_cfg_t const resources[] = {
	(RDC_PER_SIM1 | RDC_DOMAIN(0)),
	(RDC_PER_SIM2 | RDC_DOMAIN(0)),
	(RDC_PER_UART1 | RDC_DOMAIN(0)),
	(RDC_PER_UART2 | RDC_DOMAIN(0)),
	(RDC_PER_UART3 | RDC_DOMAIN(0)),
	(RDC_PER_UART4 | RDC_DOMAIN(0)),
	(RDC_PER_UART5 | RDC_DOMAIN(0)),
	(RDC_PER_UART6 | RDC_DOMAIN(0)),
	(RDC_PER_UART7 | RDC_DOMAIN(0)),
	(RDC_PER_SAI1 | RDC_DOMAIN(0)),
	(RDC_PER_SAI2 | RDC_DOMAIN(0)),
	(RDC_PER_SAI3 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG1 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG2 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG3 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG4 | RDC_DOMAIN(0)),
	(RDC_PER_GPT1 | RDC_DOMAIN(0)),
	(RDC_PER_GPT2 | RDC_DOMAIN(0)),
	(RDC_PER_GPT3 | RDC_DOMAIN(0)),
	(RDC_PER_GPT4 | RDC_DOMAIN(0)),
	(RDC_PER_PWM1 | RDC_DOMAIN(0)),
	(RDC_PER_PWM2 | RDC_DOMAIN(0)),
	(RDC_PER_PWM3 | RDC_DOMAIN(0)),
	(RDC_PER_PWM4 | RDC_DOMAIN(0)),
	(RDC_PER_ENET1 | RDC_DOMAIN(0)),
	(RDC_PER_ENET2 | RDC_DOMAIN(0)),
};

static void isolate_resource(void)
{
	imx_rdc_setup_peripherals(resources, ARRAY_SIZE(resources));
}
#endif

#if defined(CONFIG_SECURE_BOOT)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 1,
	.word = 3,
};
#endif

static bool is_mx7d(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	int val;

	val = readl(&fuse->tester4);
	if (val & 1)
		return false;
	else
		return true;
}

u32 get_cpu_rev(void)
{
	struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
						 ANATOP_BASE_ADDR;
	u32 reg = readl(&ccm_anatop->digprog);
	u32 type = (reg >> 16) & 0xff;

	if (!is_mx7d())
		type = MXC_CPU_MX7S;

	reg &= 0xff;
	return (type << 12) | reg;
}

#ifdef CONFIG_REVISION_TAG
u32 __weak get_board_rev(void)
{
	return get_cpu_rev();
}
#endif

/* enable all periherial can be accessed in nosec mode */
static void init_csu(void)
{
	int i = 0;
	for (i = 0; i < CSU_NUM_REGS; i++)
		writel(CSU_INIT_SEC_LEVEL0, CSU_IPS_BASE_ADDR + i * 4);
}

static void imx_enet_mdio_fixup(void)
{
	struct iomuxc_gpr_base_regs *gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/*
	 * The management data input/output (MDIO) requires open-drain,
	 * i.MX7D TO1.0 ENET MDIO pin has no open drain, but TO1.1 supports
	 * this feature. So to TO1.1, need to enable open drain by setting
	 * bits GPR0[8:7].
	 */

	if (soc_rev() >= CHIP_REV_1_1) {
		setbits_le32(&gpr_regs->gpr[0],
			     IOMUXC_GPR_GPR0_ENET_MDIO_OPEN_DRAIN_MASK);
	}
}

int arch_cpu_init(void)
{
	init_aips();

	init_csu();
	/* Disable PDE bit of WMCR register */
	imx_wdog_disable_powerdown();

	imx_enet_mdio_fixup();

#ifdef CONFIG_APBH_DMA
	/* Start APBH DMA */
	mxs_dma_init();
#endif

#if CONFIG_IS_ENABLED(IMX_RDC)
	isolate_resource();
#endif

	init_snvs();

	return 0;
}

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_mx7d())
		env_set("soc", "imx7d");
	else
		env_set("soc", "imx7s");
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	return 0;
}
#endif

#ifdef CONFIG_SERIAL_TAG
/*
 * OCOTP_TESTER
 * i.MX 7Solo Applications Processor Reference Manual, Rev. 0.1, 08/2016
 * OCOTP_TESTER describes a unique ID based on silicon wafer
 * and die X/Y position
 *
 * OCOTOP_TESTER offset 0x410
 * 31:0 fuse 0
 * FSL-wide unique, encoded LOT ID STD II/SJC CHALLENGE/ Unique ID
 *
 * OCOTP_TESTER1 offset 0x420
 * 31:24 fuse 1
 * The X-coordinate of the die location on the wafer/SJC CHALLENGE/ Unique ID
 * 23:16 fuse 1
 * The Y-coordinate of the die location on the wafer/SJC CHALLENGE/ Unique ID
 * 15:11 fuse 1
 * The wafer number of the wafer on which the device was fabricated/SJC
 * CHALLENGE/ Unique ID
 * 10:0 fuse 1
 * FSL-wide unique, encoded LOT ID STD II/SJC CHALLENGE/ Unique ID
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;

	serialnr->low = fuse->tester0;
	serialnr->high = fuse->tester1;
}
#endif

void set_wdog_reset(struct wdog_regs *wdog)
{
	u32 reg = readw(&wdog->wcr);
	/*
	 * Output WDOG_B signal to reset external pmic or POR_B decided by
	 * the board desgin. Without external reset, the peripherals/DDR/
	 * PMIC are not reset, that may cause system working abnormal.
	 */
	reg = readw(&wdog->wcr);
	reg |= 1 << 3;
	/*
	 * WDZST bit is write-once only bit. Align this bit in kernel,
	 * otherwise kernel code will have no chance to set this bit.
	 */
	reg |= 1 << 0;
	writew(reg, &wdog->wcr);
}

/*
 * cfg_val will be used for
 * Boot_cfg4[7:0]:Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 * After reset, if GPR10[28] is 1, ROM will copy GPR9[25:0]
 * to SBMR1, which will determine the boot device.
 */
const struct boot_mode soc_boot_modes[] = {
	{"ecspi1:0",	MAKE_CFGVAL(0x00, 0x60, 0x00, 0x00)},
	{"ecspi1:1",	MAKE_CFGVAL(0x40, 0x62, 0x00, 0x00)},
	{"ecspi1:2",	MAKE_CFGVAL(0x80, 0x64, 0x00, 0x00)},
	{"ecspi1:3",	MAKE_CFGVAL(0xc0, 0x66, 0x00, 0x00)},

	{"weim",	MAKE_CFGVAL(0x00, 0x50, 0x00, 0x00)},
	{"qspi1",	MAKE_CFGVAL(0x10, 0x40, 0x00, 0x00)},
	/* 4 bit bus width */
	{"usdhc1",	MAKE_CFGVAL(0x10, 0x10, 0x00, 0x00)},
	{"usdhc2",	MAKE_CFGVAL(0x10, 0x14, 0x00, 0x00)},
	{"usdhc3",	MAKE_CFGVAL(0x10, 0x18, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x10, 0x20, 0x00, 0x00)},
	{"mmc2",	MAKE_CFGVAL(0x10, 0x24, 0x00, 0x00)},
	{"mmc3",	MAKE_CFGVAL(0x10, 0x28, 0x00, 0x00)},
	{NULL,		0},
};

void s_init(void)
{
	/* clock configuration. */
	clock_init();

	return;
}

void reset_misc(void)
{
#ifdef CONFIG_VIDEO_MXS
	lcdif_power_down();
#endif
}

