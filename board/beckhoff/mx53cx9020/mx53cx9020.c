// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 *
 * Based on <u-boot>/board/freescale/mx53loco/mx53loco.c
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/mx5_video.h>
#include <ACEX1K.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <input.h>
#include <fs.h>
#include <dm/platform_data/serial_mxc.h>

enum LED_GPIOS {
	GPIO_SD1_CD = IMX_GPIO_NR(1, 1),
	GPIO_SD2_CD = IMX_GPIO_NR(1, 4),
	GPIO_LED_SD2_R = IMX_GPIO_NR(3, 16),
	GPIO_LED_SD2_B = IMX_GPIO_NR(3, 17),
	GPIO_LED_SD2_G = IMX_GPIO_NR(3, 18),
	GPIO_LED_SD1_R = IMX_GPIO_NR(3, 19),
	GPIO_LED_SD1_B = IMX_GPIO_NR(3, 20),
	GPIO_LED_SD1_G = IMX_GPIO_NR(3, 21),
	GPIO_LED_PWR_R = IMX_GPIO_NR(3, 22),
	GPIO_LED_PWR_B = IMX_GPIO_NR(3, 23),
	GPIO_LED_PWR_G = IMX_GPIO_NR(3, 24),
	GPIO_SUPS_INT = IMX_GPIO_NR(3, 31),
	GPIO_C3_CONFIG = IMX_GPIO_NR(6, 8),
	GPIO_C3_STATUS = IMX_GPIO_NR(6, 7),
	GPIO_C3_DONE = IMX_GPIO_NR(6, 9),
};

#define CCAT_BASE_ADDR ((void *)0xf0000000)
#define CCAT_END_ADDR (CCAT_BASE_ADDR + (1024 * 1024 * 32))
#define CCAT_SIZE 1191788
#define CCAT_SIGN_ADDR (CCAT_BASE_ADDR + 12)
static const char CCAT_SIGNATURE[] = "CCAT";

static const u32 CCAT_MODE_CONFIG = 0x0024DC81;
static const u32 CCAT_MODE_RUN = 0x0033DC8F;

DECLARE_GLOBAL_DATA_PTR;

u32 get_board_rev(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
	    (struct fuse_bank0_regs *)bank->fuse_regs;

	int rev = readl(&fuse->gp[6]);

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

/*
 * Set CCAT mode
 * @mode: use CCAT_MODE_CONFIG or CCAT_MODE_RUN
 */
void weim_cs0_settings(u32 mode)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(0x0, &weim_regs->cs0gcr1);
	writel(mode, &weim_regs->cs0gcr1);
	writel(0x00001002, &weim_regs->cs0gcr2);

	writel(0x04000000, &weim_regs->cs0rcr1);
	writel(0x00000000, &weim_regs->cs0rcr2);

	writel(0x04000000, &weim_regs->cs0wcr1);
	writel(0x00000000, &weim_regs->cs0wcr2);
}

static void setup_gpio_eim(void)
{
	gpio_direction_input(GPIO_C3_STATUS);
	gpio_direction_input(GPIO_C3_DONE);
	gpio_direction_output(GPIO_C3_CONFIG, 1);

	weim_cs0_settings(CCAT_MODE_RUN);
}

static void setup_gpio_sups(void)
{
	gpio_direction_input(GPIO_SUPS_INT);

	static const int BLINK_INTERVALL = 50000;
	int status = 1;
	while (gpio_get_value(GPIO_SUPS_INT)) {
		/* signal "CX SUPS power fail" */
		gpio_set_value(GPIO_LED_PWR_R,
			       (++status / BLINK_INTERVALL) % 2);
	}

	/* signal "CX power up" */
	gpio_set_value(GPIO_LED_PWR_R, 1);
}

static void setup_gpio_leds(void)
{
	gpio_direction_output(GPIO_LED_SD2_R, 0);
	gpio_direction_output(GPIO_LED_SD2_B, 0);
	gpio_direction_output(GPIO_LED_SD2_G, 0);
	gpio_direction_output(GPIO_LED_SD1_R, 0);
	gpio_direction_output(GPIO_LED_SD1_B, 0);
	gpio_direction_output(GPIO_LED_SD1_G, 0);
	gpio_direction_output(GPIO_LED_PWR_R, 0);
	gpio_direction_output(GPIO_LED_PWR_B, 0);
	gpio_direction_output(GPIO_LED_PWR_G, 0);
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	/* request VBUS power enable pin, GPIO7_8 */
	gpio_direction_output(IMX_GPIO_NR(7, 8), 1);
	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	gpio_direction_input(GPIO_SD1_CD);
	gpio_direction_input(GPIO_SD2_CD);

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(GPIO_SD1_CD);
	else
		ret = !gpio_get_value(GPIO_SD2_CD);

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			break;
		case 1:
			break;
		default:
			printf("Warning: you configured more ESDHC controller(%d) as supported by the board(2)\n",
			       CONFIG_SYS_FSL_ESDHC_NUM);
			return -EINVAL;
		}
		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

static int power_init(void)
{
	/* nothing to do on CX9020 */
	return 0;
}

static void clock_1GHz(void)
{
	int ret;
	u32 ref_clk = MXC_HCLK;
	/*
	 * After increasing voltage to 1.25V, we can switch
	 * CPU clock to 1GHz and DDR to 400MHz safely
	 */
	ret = mxc_set_clock(ref_clk, 1000, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to 1GHZ failed\n");

	ret = mxc_set_clock(ref_clk, 400, MXC_PERIPH_CLK);
	ret |= mxc_set_clock(ref_clk, 400, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to 400MHz failed\n");
}

int board_early_init_f(void)
{
	setup_gpio_leds();
	setup_gpio_sups();
	setup_gpio_eim();
	setup_iomux_lcd();

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	mxc_set_sata_internal_clock();

	return 0;
}

int checkboard(void)
{
	puts("Board: Beckhoff CX9020\n");

	return 0;
}

static int ccat_config_fn(int assert_config, int flush, int cookie)
{
	/* prepare FPGA for programming */
	weim_cs0_settings(CCAT_MODE_CONFIG);
	gpio_set_value(GPIO_C3_CONFIG, 0);
	udelay(1);
	gpio_set_value(GPIO_C3_CONFIG, 1);
	udelay(230);

	return FPGA_SUCCESS;
}

static int ccat_status_fn(int cookie)
{
	return FPGA_FAIL;
}

static int ccat_write_fn(const void *buf, size_t buf_len, int flush, int cookie)
{
	const uint8_t *const buffer = buf;

	/* program CCAT */
	int i;
	for (i = 0; i < buf_len; ++i)
		writeb(buffer[i], CCAT_BASE_ADDR);

	writeb(0xff, CCAT_BASE_ADDR);
	writeb(0xff, CCAT_BASE_ADDR);

	return FPGA_SUCCESS;
}

static int ccat_done_fn(int cookie)
{
	/* programming complete? */
	return gpio_get_value(GPIO_C3_DONE);
}

static int ccat_post_fn(int cookie)
{
	/* switch to FPGA run mode */
	weim_cs0_settings(CCAT_MODE_RUN);
	invalidate_dcache_range((ulong) CCAT_BASE_ADDR, (ulong) CCAT_END_ADDR);

	if (memcmp(CCAT_SIGN_ADDR, CCAT_SIGNATURE, sizeof(CCAT_SIGNATURE))) {
		printf("Verifing CCAT firmware failed, signature not found\n");
		return FPGA_FAIL;
	}

	/* signal "CX booting OS" */
	gpio_set_value(GPIO_LED_PWR_R, 1);
	gpio_set_value(GPIO_LED_PWR_G, 1);
	gpio_set_value(GPIO_LED_PWR_B, 0);
	return FPGA_SUCCESS;
}

static Altera_CYC2_Passive_Serial_fns ccat_fns = {
	.config = ccat_config_fn,
	.status = ccat_status_fn,
	.done = ccat_done_fn,
	.write = ccat_write_fn,
	.abort = ccat_post_fn,
	.post = ccat_post_fn,
};

static Altera_desc ccat_fpga = {
	.family = Altera_CYC2,
	.iface = passive_serial,
	.size = CCAT_SIZE,
	.iface_fns = &ccat_fns,
	.base = CCAT_BASE_ADDR,
};

int board_late_init(void)
{
	if (!power_init())
		clock_1GHz();

	fpga_init();
	fpga_add(fpga_altera, &ccat_fpga);

	return 0;
}
