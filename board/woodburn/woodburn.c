/*
 * Copyright (C) 2012, Stefano Babic <sbabic@denx.de>
 *
 * Based on flea3.c and mx35pdk.c
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx35_pins.h>
#include <asm/arch/iomux.h>
#include <i2c.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <linux/types.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>
#include <spl.h>

#define CCM_CCMR_CONFIG		0x003F4208

#define ESDCTL_DDR2_CONFIG	0x007FFC3F

/* For MMC */
#define GPIO_MMC_CD	7
#define GPIO_MMC_WP	8

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1,
		PHYS_SDRAM_1_SIZE);

	return 0;
}

static void board_setup_sdram(void)
{
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;

	/* Initialize with default values both CSD0/1 */
	writel(0x2000, &esdc->esdctl0);
	writel(0x2000, &esdc->esdctl1);

	mx3_setup_sdram_bank(CSD0_BASE_ADDR, ESDCTL_DDR2_CONFIG,
		 13, 10, 2, 0x8080);
}

static void setup_iomux_fec(void)
{
	/* setup pins for FEC */
	mxc_request_iomux(MX35_PIN_FEC_TX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_DV, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_COL, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_EN, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDC, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDIO, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_CRS, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA3, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA3, MUX_CONFIG_FUNC);
}

int woodburn_init(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	/* initialize PLL and clock configuration */
	writel(CCM_CCMR_CONFIG, &ccm->ccmr);

	/* Set-up RAM */
	board_setup_sdram();

	/* enable clocks */
	writel(readl(&ccm->cgr0) |
		MXC_CCM_CGR0_EMI_MASK |
		MXC_CCM_CGR0_EDIO_MASK |
		MXC_CCM_CGR0_EPIT1_MASK,
		&ccm->cgr0);

	writel(readl(&ccm->cgr1) |
		MXC_CCM_CGR1_FEC_MASK |
		MXC_CCM_CGR1_GPIO1_MASK |
		MXC_CCM_CGR1_GPIO2_MASK |
		MXC_CCM_CGR1_GPIO3_MASK |
		MXC_CCM_CGR1_I2C1_MASK |
		MXC_CCM_CGR1_I2C2_MASK |
		MXC_CCM_CGR1_I2C3_MASK,
		&ccm->cgr1);

	/* Set-up NAND */
	__raw_writel(readl(&ccm->rcsr) | MXC_CCM_RCSR_NFC_FMS, &ccm->rcsr);

	/* Set pinmux for the required peripherals */
	setup_iomux_fec();

	/* setup GPIO1_4 FEC_ENABLE signal */
	mxc_request_iomux(MX35_PIN_SCKR, MUX_CONFIG_ALT5);
	gpio_direction_output(4, 1);
	mxc_request_iomux(MX35_PIN_HCKT, MUX_CONFIG_ALT5);
	gpio_direction_output(9, 1);

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
void board_init_f(ulong dummy)
{
	/* Set the stack pointer. */
	asm volatile("mov sp, %0\n" : : "r"(CONFIG_SPL_STACK));

	/* Initialize MUX and SDRAM */
	woodburn_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end__ - __bss_start);

	/* Set global data pointer. */
	gd = &gdata;

	preloader_console_init();
	timer_init();

	board_init_r(NULL, 0);
}

void spl_board_init(void)
{
}

#endif


/* Booting from NOR in external mode */
int board_early_init_f(void)
{
	return woodburn_init();
}


int board_init(void)
{
	struct pmic *p;
	u32 val;
	int ret;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	ret = pmic_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");

	/*
	 * Set switchers in Auto in NORMAL mode & STANDBY mode
	 * Setup the switcher mode for SW1 & SW2
	 */
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	/* Set SWILIMB */
	val |= (1 << 22);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val &= ~((SWMODE_MASK << SWMODE4_SHIFT) |
		(SWMODE_MASK << SWMODE3_SHIFT));
	val |= (SWMODE_AUTO_AUTO << SWMODE4_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE3_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VGEN1 to 3.15V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VGEN1_MASK);
	val |= VGEN1_3_15;
	pmic_reg_write(p, REG_SETTING_0, val);

	pmic_reg_read(p, REG_MODE_0, &val);
	val |= VGEN1EN;
	pmic_reg_write(p, REG_MODE_0, val);
	udelay(2000);

	return 0;
}

#if defined(CONFIG_FSL_ESDHC)
struct fsl_esdhc_cfg esdhc_cfg = {MMC_SDHC1_BASE_ADDR};

int board_mmc_init(bd_t *bis)
{
	/* configure pins for SDHC1 only */
	mxc_request_iomux(MX35_PIN_SD1_CMD, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_SD1_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_SD1_DATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_SD1_DATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_SD1_DATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_SD1_DATA3, MUX_CONFIG_FUNC);

	/* MMC Card Detect on GPIO1_7 */
	mxc_request_iomux(MX35_PIN_SCKT, MUX_CONFIG_ALT5);
	mxc_iomux_set_input(MUX_IN_GPIO1_IN_7, 0x1);
	gpio_direction_input(GPIO_MMC_CD);

	mxc_request_iomux(MX35_PIN_FST, MUX_CONFIG_ALT5);
	mxc_iomux_set_input(MUX_IN_GPIO1_IN_8, 0x1);
	gpio_direction_output(GPIO_MMC_WP, 0);

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);

	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return !gpio_get_value(GPIO_MMC_CD);
}
#endif

u32 get_board_rev(void)
{
	int rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}
