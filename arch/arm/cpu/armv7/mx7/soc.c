/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/dma.h>
#include <asm/imx-common/hab.h>
#include <asm/arch/crm_regs.h>
#include <dm.h>
#include <imx_thermal.h>

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

#if defined(CONFIG_SECURE_BOOT)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 1,
	.word = 3,
};
#endif

/*
 * OCOTP_TESTER3[9:8] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_SPEED_SHIFT	8
#define OCOTP_TESTER3_SPEED_800MHZ	0
#define OCOTP_TESTER3_SPEED_850MHZ	1
#define OCOTP_TESTER3_SPEED_1GHZ	2

u32 get_cpu_speed_grade_hz(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	val >>= OCOTP_TESTER3_SPEED_SHIFT;
	val &= 0x3;

	switch(val) {
	case OCOTP_TESTER3_SPEED_800MHZ:
		return 792000000;
	case OCOTP_TESTER3_SPEED_850MHZ:
		return 852000000;
	case OCOTP_TESTER3_SPEED_1GHZ:
		return 996000000;
	}
	return 0;
}

/*
 * OCOTP_TESTER3[7:6] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_TEMP_SHIFT	6

u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	val >>= OCOTP_TESTER3_TEMP_SHIFT;
	val &= 0x3;

	if (minc && maxc) {
		if (val == TEMP_AUTOMOTIVE) {
			*minc = -40;
			*maxc = 125;
		} else if (val == TEMP_INDUSTRIAL) {
			*minc = -40;
			*maxc = 105;
		} else if (val == TEMP_EXTCOMMERCIAL) {
			*minc = -20;
			*maxc = 105;
		} else {
			*minc = 0;
			*maxc = 95;
		}
	}
	return val;
}

u32 get_cpu_rev(void)
{
	struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
						 ANATOP_BASE_ADDR;
	u32 reg = readl(&ccm_anatop->digprog);
	u32 type = (reg >> 16) & 0xff;

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

int arch_cpu_init(void)
{
	init_aips();

	init_csu();
	/* Disable PDE bit of WMCR register */
	imx_set_wdog_powerdown(false);

#ifdef CONFIG_APBH_DMA
	/* Start APBH DMA */
	mxs_dma_init();
#endif

	return 0;
}

#ifdef CONFIG_SERIAL_TAG
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

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[9];
	struct fuse_bank9_regs *fuse =
		(struct fuse_bank9_regs *)bank->fuse_regs;

	if (0 == dev_id) {
		u32 value = readl(&fuse->mac_addr1);
		mac[0] = (value >> 8);
		mac[1] = value;

		value = readl(&fuse->mac_addr0);
		mac[2] = value >> 24;
		mac[3] = value >> 16;
		mac[4] = value >> 8;
		mac[5] = value;
	} else {
		u32 value = readl(&fuse->mac_addr2);
		mac[0] = value >> 24;
		mac[1] = value >> 16;
		mac[2] = value >> 8;
		mac[3] = value;

		value = readl(&fuse->mac_addr1);
		mac[4] = value >> 24;
		mac[5] = value >> 16;
	}
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

enum boot_device get_boot_device(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;

	enum boot_device boot_dev = SD1_BOOT;
	u8 boot_type = (*p)->boot_dev_type;
	u8 boot_instance = (*p)->boot_dev_instance;

	switch (boot_type) {
	case BOOT_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BOOT_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BOOT_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BOOT_TYPE_QSPI:
		boot_dev = QSPI_BOOT;
		break;
	case BOOT_TYPE_WEIM:
		boot_dev = WEIM_NOR_BOOT;
		break;
	case BOOT_TYPE_SPINOR:
		boot_dev = SPI_NOR_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}

void s_init(void)
{
#if !defined CONFIG_SPL_BUILD
	/* Enable SMP mode for CPU0, by setting bit 6 of Auxiliary Ctl reg */
	asm volatile(
			"mrc p15, 0, r0, c1, c0, 1\n"
			"orr r0, r0, #1 << 6\n"
			"mcr p15, 0, r0, c1, c0, 1\n");
#endif
	/* clock configuration. */
	clock_init();

	return;
}
