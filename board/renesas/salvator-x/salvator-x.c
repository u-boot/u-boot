/*
 * board/renesas/salvator-x/salvator-x.c
 *     This file is Salvator-X/Salvator-XS board support.
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define CPGWPCR	0xE6150904
#define CPGWPR  0xE615090C

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	writel(0xA5A50000, CPGWPCR);
	writel(0xFFFFFFFF, CPGWPR);
}

#define GSX_MSTP112		BIT(12)	/* 3DG */
#define TMU0_MSTP125		BIT(25)	/* secure */
#define TMU1_MSTP124		BIT(24)	/* non-secure */
#define SCIF2_MSTP310		BIT(10)	/* SCIF2 */
#define ETHERAVB_MSTP812	BIT(12)
#define DVFS_MSTP926		BIT(26)
#define SD0_MSTP314		BIT(14)
#define SD1_MSTP313		BIT(13)
#define SD2_MSTP312		BIT(12)	/* either MMC0 */
#define SD3_MSTP311		BIT(11)	/* either MMC1 */

#define SD0CKCR			0xE6150074
#define SD1CKCR			0xE6150078
#define SD2CKCR			0xE6150268
#define SD3CKCR			0xE615026C

int board_early_init_f(void)
{
	/* TMU0,1 */		/* which use ? */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125 | TMU1_MSTP124);
	/* SCIF2 */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, SCIF2_MSTP310);
	/* EHTERAVB */
	mstp_clrbits_le32(MSTPSR8, SMSTPCR8, ETHERAVB_MSTP812);
	/* eMMC */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, SD1_MSTP313 | SD2_MSTP312);
	/* SDHI0, 3 */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, SD0_MSTP314 | SD3_MSTP311);

	writel(0, SD0CKCR);
	writel(0, SD1CKCR);
	writel(0, SD2CKCR);
	writel(0, SD3CKCR);

#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	/* DVFS for reset */
	mstp_clrbits_le32(MSTPSR9, SMSTPCR9, DVFS_MSTP926);
#endif
	return 0;
}

/* SYSC */
/* R/- 32 Power status register 2(3DG) */
#define	SYSC_PWRSR2	0xE6180100
/* -/W 32 Power resume control register 2 (3DG) */
#define	SYSC_PWRONCR2	0xE618010C

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* Init PFC controller */
#if defined(CONFIG_R8A7795)
	r8a7795_pinmux_init();
#elif defined(CONFIG_R8A7796)
	r8a7796_pinmux_init();
#endif

#if defined(CONFIG_R8A7795)
	/* GSX: force power and clock supply */
	writel(0x0000001F, SYSC_PWRONCR2);
	while (readl(SYSC_PWRSR2) != 0x000003E0)
		mdelay(20);

	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, GSX_MSTP112);
#endif

	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

#ifdef CONFIG_RAVB
	/* EtherAVB Enable */
	/* GPSR2 */
	gpio_request(GPIO_GFN_AVB_AVTP_CAPTURE_A, NULL);
	gpio_request(GPIO_GFN_AVB_AVTP_MATCH_A, NULL);
	gpio_request(GPIO_GFN_AVB_LINK, NULL);
	gpio_request(GPIO_GFN_AVB_PHY_INT, NULL);
	gpio_request(GPIO_GFN_AVB_MAGIC, NULL);
	gpio_request(GPIO_GFN_AVB_MDC, NULL);

	/* IPSR0 */
	gpio_request(GPIO_IFN_AVB_MDC, NULL);
	gpio_request(GPIO_IFN_AVB_MAGIC, NULL);
	gpio_request(GPIO_IFN_AVB_PHY_INT, NULL);
	gpio_request(GPIO_IFN_AVB_LINK, NULL);
	gpio_request(GPIO_IFN_AVB_AVTP_MATCH_A, NULL);
	gpio_request(GPIO_IFN_AVB_AVTP_CAPTURE_A, NULL);
	/* IPSR1 */
	gpio_request(GPIO_FN_AVB_AVTP_PPS, NULL);
	/* IPSR2 */
	gpio_request(GPIO_FN_AVB_AVTP_MATCH_B, NULL);
	/* IPSR3 */
	gpio_request(GPIO_FN_AVB_AVTP_CAPTURE_B, NULL);

#if defined(CONFIG_R8A7795)
	/* USB2_OVC */
	gpio_request(GPIO_GP_6_15, NULL);
	gpio_direction_input(GPIO_GP_6_15);

	/* USB2_PWEN */
	gpio_request(GPIO_GP_6_14, NULL);
	gpio_direction_output(GPIO_GP_6_14, 1);
	gpio_set_value(GPIO_GP_6_14, 1);
#endif
	/* AVB_PHY_RST */
	gpio_request(GPIO_GP_2_10, NULL);
	gpio_direction_output(GPIO_GP_2_10, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_2_10, 1);
	udelay(1);
#endif

	return 0;
}

static struct eth_pdata salvator_x_ravb_platdata = {
	.iobase		= 0xE6800000,
	.phy_interface	= 0,
	.max_speed	= 1000,
};

U_BOOT_DEVICE(salvator_x_ravb) = {
	.name		= "ravb",
	.platdata	= &salvator_x_ravb_platdata,
};

#ifdef CONFIG_SH_SDHI
int board_mmc_init(bd_t *bis)
{
	int ret = -ENODEV;

	/* SDHI0 */
	gpio_request(GPIO_GFN_SD0_DAT0, NULL);
	gpio_request(GPIO_GFN_SD0_DAT1, NULL);
	gpio_request(GPIO_GFN_SD0_DAT2, NULL);
	gpio_request(GPIO_GFN_SD0_DAT3, NULL);
	gpio_request(GPIO_GFN_SD0_CLK, NULL);
	gpio_request(GPIO_GFN_SD0_CMD, NULL);
	gpio_request(GPIO_GFN_SD0_CD, NULL);
	gpio_request(GPIO_GFN_SD0_WP, NULL);

	gpio_request(GPIO_GP_5_2, NULL);
	gpio_request(GPIO_GP_5_1, NULL);
	gpio_direction_output(GPIO_GP_5_2, 1);	/* power on */
	gpio_direction_output(GPIO_GP_5_1, 1);	/* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE, 0,
			   SH_SDHI_QUIRK_64BIT_BUF);
	if (ret)
		return ret;

	/* SDHI1/SDHI2 eMMC */
	gpio_request(GPIO_GFN_SD1_DAT0, NULL);
	gpio_request(GPIO_GFN_SD1_DAT1, NULL);
	gpio_request(GPIO_GFN_SD1_DAT2, NULL);
	gpio_request(GPIO_GFN_SD1_DAT3, NULL);
	gpio_request(GPIO_GFN_SD2_DAT0, NULL);
	gpio_request(GPIO_GFN_SD2_DAT1, NULL);
	gpio_request(GPIO_GFN_SD2_DAT2, NULL);
	gpio_request(GPIO_GFN_SD2_DAT3, NULL);
	gpio_request(GPIO_GFN_SD2_CLK, NULL);
#if defined(CONFIG_R8A7795)
	gpio_request(GPIO_GFN_SD2_CMD, NULL);
#elif defined(CONFIG_R8A7796)
	gpio_request(GPIO_FN_SD2_CMD, NULL);
#else
#error Only R8A7795 and R87796 is supported
#endif
	gpio_request(GPIO_GP_5_3, NULL);
	gpio_request(GPIO_GP_5_9, NULL);
	gpio_direction_output(GPIO_GP_5_3, 0);	/* 1: 3.3V, 0: 1.8V */
	gpio_direction_output(GPIO_GP_5_9, 0);	/* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI2_BASE, 1,
			   SH_SDHI_QUIRK_64BIT_BUF);
	if (ret)
		return ret;

#if defined(CONFIG_R8A7795)
	/* SDHI3 */
	gpio_request(GPIO_GFN_SD3_DAT0, NULL);	/* GP_4_9 */
	gpio_request(GPIO_GFN_SD3_DAT1, NULL);	/* GP_4_10 */
	gpio_request(GPIO_GFN_SD3_DAT2, NULL);	/* GP_4_11 */
	gpio_request(GPIO_GFN_SD3_DAT3, NULL);	/* GP_4_12 */
	gpio_request(GPIO_GFN_SD3_CLK, NULL);	/* GP_4_7 */
	gpio_request(GPIO_GFN_SD3_CMD, NULL);	/* GP_4_8 */
#elif defined(CONFIG_R8A7796)
	gpio_request(GPIO_FN_SD3_DAT0, NULL);	/* GP_4_9 */
	gpio_request(GPIO_FN_SD3_DAT1, NULL);	/* GP_4_10 */
	gpio_request(GPIO_FN_SD3_DAT2, NULL);	/* GP_4_11 */
	gpio_request(GPIO_FN_SD3_DAT3, NULL);	/* GP_4_12 */
	gpio_request(GPIO_FN_SD3_CLK, NULL);	/* GP_4_7 */
	gpio_request(GPIO_FN_SD3_CMD, NULL);	/* GP_4_8 */
#else
#error Only R8A7795 and R87796 is supported
#endif
	/* IPSR10 */
	gpio_request(GPIO_FN_SD3_CD, NULL);
	gpio_request(GPIO_FN_SD3_WP, NULL);

	gpio_request(GPIO_GP_3_15, NULL);
	gpio_request(GPIO_GP_3_14, NULL);
	gpio_direction_output(GPIO_GP_3_15, 1);	/* power on */
	gpio_direction_output(GPIO_GP_3_14, 1);	/* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI3_BASE, 2,
			   SH_SDHI_QUIRK_64BIT_BUF);
	return ret;
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
#if (CONFIG_NR_DRAM_BANKS >= 2)
	gd->ram_size += PHYS_SDRAM_2_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 3)
	gd->ram_size += PHYS_SDRAM_3_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 4)
	gd->ram_size += PHYS_SDRAM_4_SIZE;
#endif

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#if (CONFIG_NR_DRAM_BANKS >= 2)
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 3)
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 4)
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;
#endif
	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RCAR_BOARD_STRING
};

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CODE	0xA5A5000F

void reset_cpu(ulong addr)
{
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	i2c_reg_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x20, 0x80);
#else
	/* only CA57 ? */
	writel(RST_CODE, RST_CA57RESCNT);
#endif
}

static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF2_BASE,
	.type = PORT_SCIF,
	.clk = CONFIG_SH_SCIF_CLK_FREQ,
	.clk_mode = INT_CLK,
};

U_BOOT_DEVICE(salvator_x_scif2) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};
