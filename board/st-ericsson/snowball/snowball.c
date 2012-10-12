/*
 * Copyright (C) ST-Ericsson SA 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <i2c.h>
#include <mmc.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/db8500_pincfg.h>
#include <asm/arch/prcmu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#ifdef CONFIG_MMC
#include "../../../drivers/mmc/arm_pl180_mmci.h"
#endif
#include "db8500_pins.h"

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

/*
 * Memory controller register
 */
#define DMC_BASE_ADDR			0x80156000
#define DMC_CTL_97			(DMC_BASE_ADDR + 0x184)

/*
 * GPIO pin config common for MOP500/HREF boards
 */
unsigned long gpio_cfg_common[] = {
	/* I2C */
	GPIO147_I2C0_SCL,
	GPIO148_I2C0_SDA,
	GPIO16_I2C1_SCL,
	GPIO17_I2C1_SDA,
	GPIO10_I2C2_SDA,
	GPIO11_I2C2_SCL,
	GPIO229_I2C3_SDA,
	GPIO230_I2C3_SCL,

	/* SSP0, to AB8500 */
	GPIO143_SSP0_CLK,
	GPIO144_SSP0_FRM,
	GPIO145_SSP0_RXD | PIN_PULL_DOWN,
	GPIO146_SSP0_TXD,

	/* MMC0 (MicroSD card) */
	GPIO18_MC0_CMDDIR	| PIN_OUTPUT_HIGH,
	GPIO19_MC0_DAT0DIR	| PIN_OUTPUT_HIGH,
	GPIO20_MC0_DAT2DIR	| PIN_OUTPUT_HIGH,
	GPIO21_MC0_DAT31DIR	| PIN_OUTPUT_HIGH,
	GPIO22_MC0_FBCLK	| PIN_INPUT_NOPULL,
	GPIO23_MC0_CLK		| PIN_OUTPUT_LOW,
	GPIO24_MC0_CMD		| PIN_INPUT_PULLUP,
	GPIO25_MC0_DAT0		| PIN_INPUT_PULLUP,
	GPIO26_MC0_DAT1		| PIN_INPUT_PULLUP,
	GPIO27_MC0_DAT2		| PIN_INPUT_PULLUP,
	GPIO28_MC0_DAT3		| PIN_INPUT_PULLUP,

	/* MMC4 (On-board eMMC) */
	GPIO197_MC4_DAT3	| PIN_INPUT_PULLUP,
	GPIO198_MC4_DAT2	| PIN_INPUT_PULLUP,
	GPIO199_MC4_DAT1	| PIN_INPUT_PULLUP,
	GPIO200_MC4_DAT0	| PIN_INPUT_PULLUP,
	GPIO201_MC4_CMD		| PIN_INPUT_PULLUP,
	GPIO202_MC4_FBCLK	| PIN_INPUT_NOPULL,
	GPIO203_MC4_CLK		| PIN_OUTPUT_LOW,
	GPIO204_MC4_DAT7	| PIN_INPUT_PULLUP,
	GPIO205_MC4_DAT6	| PIN_INPUT_PULLUP,
	GPIO206_MC4_DAT5	| PIN_INPUT_PULLUP,
	GPIO207_MC4_DAT4	| PIN_INPUT_PULLUP,

	/* UART2, console */
	GPIO29_U2_RXD	| PIN_INPUT_PULLUP,
	GPIO30_U2_TXD	| PIN_OUTPUT_HIGH,
	GPIO31_U2_CTSn	| PIN_INPUT_PULLUP,
	GPIO32_U2_RTSn	| PIN_OUTPUT_HIGH,

	/*
	 * USB, pin 256-267 USB, Is probably already setup correctly from
	 * BootROM/boot stages, but we don't trust that and set it up anyway
	 */
	GPIO256_USB_NXT,
	GPIO257_USB_STP,
	GPIO258_USB_XCLK,
	GPIO259_USB_DIR,
	GPIO260_USB_DAT7,
	GPIO261_USB_DAT6,
	GPIO262_USB_DAT5,
	GPIO263_USB_DAT4,
	GPIO264_USB_DAT3,
	GPIO265_USB_DAT2,
	GPIO266_USB_DAT1,
	GPIO267_USB_DAT0,
};

unsigned long gpio_cfg_snowball[] = {
	/* MMC0 (MicroSD card) */
	GPIO217_GPIO    | PIN_OUTPUT_HIGH,      /* MMC_EN */
	GPIO218_GPIO    | PIN_INPUT_NOPULL,     /* MMC_CD */
	GPIO228_GPIO    | PIN_OUTPUT_HIGH,      /* SD_SEL */

	/* eMMC */
	GPIO167_GPIO    | PIN_OUTPUT_HIGH,      /* RSTn_MLC */

	/* LAN */
	GPIO131_SM_ADQ8,
	GPIO132_SM_ADQ9,
	GPIO133_SM_ADQ10,
	GPIO134_SM_ADQ11,
	GPIO135_SM_ADQ12,
	GPIO136_SM_ADQ13,
	GPIO137_SM_ADQ14,
	GPIO138_SM_ADQ15,

	/* RSTn_LAN */
	GPIO141_GPIO	| PIN_OUTPUT_HIGH,
};

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	/*
	 * Setup board (bd) and board-info (bi).
	 * bi_arch_number: Unique id for this board. It will passed in r1 to
	 *    Linux startup code and is the machine_id.
	 * bi_boot_params: Where this board expects params.
	 */
	gd->bd->bi_arch_number = MACH_TYPE_SNOWBALL;
	gd->bd->bi_boot_params = 0x00000100;

	/* Configure GPIO pins needed by U-boot */
	db8500_gpio_config_pins(gpio_cfg_common, ARRAY_SIZE(gpio_cfg_common));

	db8500_gpio_config_pins(gpio_cfg_snowball,
						ARRAY_SIZE(gpio_cfg_snowball));

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->ram_size = gd->bd->bi_dram[0].size =
		get_ram_size(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_MAX_RAM_SIZE);

	return 0;
}

static int raise_ab8500_gpio16(void)
{
	int ret;

	/* selection */
	ret = ab8500_read(AB8500_MISC, AB8500_GPIO_SEL2_REG);
	if (ret < 0)
		goto out;

	ret |= 0x80;
	ret = ab8500_write(AB8500_MISC, AB8500_GPIO_SEL2_REG, ret);
	if (ret < 0)
		goto out;

	/* direction */
	ret = ab8500_read(AB8500_MISC, AB8500_GPIO_DIR2_REG);
	if (ret < 0)
		goto out;

	ret |= 0x80;
	ret = ab8500_write(AB8500_MISC, AB8500_GPIO_DIR2_REG, ret);
	if (ret < 0)
		goto out;

	/* out */
	ret = ab8500_read(AB8500_MISC, AB8500_GPIO_OUT2_REG);
	if (ret < 0)
		goto out;

	ret |= 0x80;
	ret = ab8500_write(AB8500_MISC, AB8500_GPIO_OUT2_REG, ret);

out:
	return ret;
}

static int raise_ab8500_gpio26(void)
{
	int ret;

	/* selection */
	ret = ab8500_read(AB8500_MISC, AB8500_GPIO_DIR4_REG);
	if (ret < 0)
		goto out;

	ret |= 0x2;
	ret = ab8500_write(AB8500_MISC, AB8500_GPIO_DIR4_REG, ret);
	if (ret < 0)
		goto out;

	/* out */
	ret = ab8500_read(AB8500_MISC, AB8500_GPIO_OUT4_REG);
	if (ret < 0)
		goto out;

	ret |= 0x2;
	ret = ab8500_write(AB8500_MISC, AB8500_GPIO_OUT4_REG, ret);

out:
	return ret;
}

int board_late_init(void)
{
	/* enable 3V3 for LAN controller */
	if (raise_ab8500_gpio26() >= 0) {
		/* Turn on FSMC device */
		writel(0x1, 0x8000f000);
		writel(0x1, 0x8000f008);

		/* setup FSMC for LAN controler */
		writel(0x305b, 0x80000000);

		/* run at the highest possible speed */
		writel(0x01010210, 0x80000004);
	} else
		printf("error: can't raise GPIO26\n");

	/* enable 3v6 for GBF chip */
	if ((raise_ab8500_gpio16() < 0))
		printf("error: cant' raise GPIO16\n");

	/* empty UART RX FIFO */
	while (tstc())
		(void) getc();

	return 0;
}

#ifdef CONFIG_MMC
/*
 * emmc_host_init - initialize the emmc controller.
 * Configure GPIO settings, set initial clock and power for emmc slot.
 * Initialize mmc struct and register with mmc framework.
 */
static int emmc_host_init(void)
{
	struct pl180_mmc_host *host;

	host = malloc(sizeof(struct pl180_mmc_host));
	if (!host)
		return -ENOMEM;
	memset(host, 0, sizeof(*host));

	host->base = (struct sdi_registers *)CFG_EMMC_BASE;
	host->pwr_init = SDI_PWR_OPD | SDI_PWR_PWRCTRL_ON;
	host->clkdiv_init = SDI_CLKCR_CLKDIV_INIT_V2 |
				 SDI_CLKCR_CLKEN | SDI_CLKCR_HWFC_EN;
	strcpy(host->name, "EMMC");
	host->caps = MMC_MODE_8BIT | MMC_MODE_HS | MMC_MODE_HS_52MHz;
	host->voltages = VOLTAGE_WINDOW_MMC;
	host->clock_min = ARM_MCLK / (2 + SDI_CLKCR_CLKDIV_INIT_V2);
	host->clock_max = ARM_MCLK / 2;
	host->clock_in = ARM_MCLK;
	host->version2 = 1;

	return arm_pl180_mmci_init(host);
}

/*
 * mmc_host_init - initialize the external mmc controller.
 * Configure GPIO settings, set initial clock and power for mmc slot.
 * Initialize mmc struct and register with mmc framework.
 */
static int mmc_host_init(void)
{
	struct pl180_mmc_host *host;
	u32 sdi_u32;

	host = malloc(sizeof(struct pl180_mmc_host));
	if (!host)
		return -ENOMEM;
	memset(host, 0, sizeof(*host));

	host->base = (struct sdi_registers *)CFG_MMC_BASE;
	sdi_u32 = 0xBF;
	writel(sdi_u32, &host->base->power);
	host->pwr_init = 0xBF;
	host->clkdiv_init = SDI_CLKCR_CLKDIV_INIT_V2 |
				 SDI_CLKCR_CLKEN | SDI_CLKCR_HWFC_EN;
	strcpy(host->name, "MMC");
	host->caps = MMC_MODE_8BIT;
	host->b_max = 0;
	host->voltages = VOLTAGE_WINDOW_SD;
	host->clock_min = ARM_MCLK / (2 + SDI_CLKCR_CLKDIV_INIT_V2);
	host->clock_max = ARM_MCLK / 2;
	host->clock_in = ARM_MCLK;
	host->version2 = 1;

	return arm_pl180_mmci_init(host);
}

/*
 * board_mmc_init - initialize all the mmc/sd host controllers.
 * Called by generic mmc framework.
 */
int board_mmc_init(bd_t *bis)
{
	int error;

	(void) bis;

	error = emmc_host_init();
	if (error) {
		printf("emmc_host_init() %d\n", error);
		return -1;
	}

	u8500_mmc_power_init();

	error = mmc_host_init();
	if (error) {
		printf("mmc_host_init() %d\n", error);
		return -1;
	}

	return 0;
}
#endif /* CONFIG_MMC */
