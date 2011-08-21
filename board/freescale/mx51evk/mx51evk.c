/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <fsl_pmic.h>
#include <mc13892.h>

DECLARE_GLOBAL_DATA_PTR;

static u32 system_rev;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR, 1},
	{MMC_SDHC2_BASE_ADDR, 1},
};
#endif

u32 get_board_rev(void)
{
	return system_rev;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

static void setup_iomux_uart(void)
{
	unsigned int pad = PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE |
			PAD_CTL_PUE_PULL | PAD_CTL_DRV_HIGH;

	mxc_request_iomux(MX51_PIN_UART1_RXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_RXD, pad | PAD_CTL_SRE_FAST);
	mxc_request_iomux(MX51_PIN_UART1_TXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_TXD, pad | PAD_CTL_SRE_FAST);
	mxc_request_iomux(MX51_PIN_UART1_RTS, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_RTS, pad);
	mxc_request_iomux(MX51_PIN_UART1_CTS, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_CTS, pad);
}

static void setup_iomux_fec(void)
{
	/*FEC_MDIO*/
	mxc_request_iomux(MX51_PIN_EIM_EB2 , IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_EB2 , 0x1FD);

	/*FEC_MDC*/
	mxc_request_iomux(MX51_PIN_NANDF_CS3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS3, 0x2004);

	/* FEC RDATA[3] */
	mxc_request_iomux(MX51_PIN_EIM_CS3, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS3, 0x180);

	/* FEC RDATA[2] */
	mxc_request_iomux(MX51_PIN_EIM_CS2, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS2, 0x180);

	/* FEC RDATA[1] */
	mxc_request_iomux(MX51_PIN_EIM_EB3, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_EB3, 0x180);

	/* FEC RDATA[0] */
	mxc_request_iomux(MX51_PIN_NANDF_D9, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D9, 0x2180);

	/* FEC TDATA[3] */
	mxc_request_iomux(MX51_PIN_NANDF_CS6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS6, 0x2004);

	/* FEC TDATA[2] */
	mxc_request_iomux(MX51_PIN_NANDF_CS5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS5, 0x2004);

	/* FEC TDATA[1] */
	mxc_request_iomux(MX51_PIN_NANDF_CS4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS4, 0x2004);

	/* FEC TDATA[0] */
	mxc_request_iomux(MX51_PIN_NANDF_D8, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D8, 0x2004);

	/* FEC TX_EN */
	mxc_request_iomux(MX51_PIN_NANDF_CS7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS7, 0x2004);

	/* FEC TX_ER */
	mxc_request_iomux(MX51_PIN_NANDF_CS2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS2, 0x2004);

	/* FEC TX_CLK */
	mxc_request_iomux(MX51_PIN_NANDF_RDY_INT, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RDY_INT, 0x2180);

	/* FEC TX_COL */
	mxc_request_iomux(MX51_PIN_NANDF_RB2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RB2, 0x2180);

	/* FEC RX_CLK */
	mxc_request_iomux(MX51_PIN_NANDF_RB3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RB3, 0x2180);

	/* FEC RX_CRS */
	mxc_request_iomux(MX51_PIN_EIM_CS5, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS5, 0x180);

	/* FEC RX_ER */
	mxc_request_iomux(MX51_PIN_EIM_CS4, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS4, 0x180);

	/* FEC RX_DV */
	mxc_request_iomux(MX51_PIN_NANDF_D11, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D11, 0x2180);
}

#ifdef CONFIG_MXC_SPI
static void setup_iomux_spi(void)
{
	/* 000: Select mux mode: ALT0 mux port: MOSI of instance: ecspi1 */
	mxc_request_iomux(MX51_PIN_CSPI1_MOSI, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_MOSI, 0x105);

	/* 000: Select mux mode: ALT0 mux port: MISO of instance: ecspi1. */
	mxc_request_iomux(MX51_PIN_CSPI1_MISO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_MISO, 0x105);

	/* de-select SS1 of instance: ecspi1. */
	mxc_request_iomux(MX51_PIN_CSPI1_SS1, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_SS1, 0x85);

	/* 000: Select mux mode: ALT0 mux port: SS0 ecspi1 */
	mxc_request_iomux(MX51_PIN_CSPI1_SS0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_SS0, 0x185);

	/* 000: Select mux mode: ALT0 mux port: RDY of instance: ecspi1. */
	mxc_request_iomux(MX51_PIN_CSPI1_RDY, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_RDY, 0x180);

	/* 000: Select mux mode: ALT0 mux port: SCLK of instance: ecspi1. */
	mxc_request_iomux(MX51_PIN_CSPI1_SCLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_CSPI1_SCLK, 0x105);
}
#endif

static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;

	/* Write needed to Power Gate 2 register */
	val = pmic_reg_read(REG_POWER_MISC);
	val &= ~PWGT2SPIEN;
	pmic_reg_write(REG_POWER_MISC, val);

	/* Externally powered */
	val = pmic_reg_read(REG_CHARGE);
	val |= ICHRG0 | ICHRG1 | ICHRG2 | ICHRG3 | CHGAUTOB;
	pmic_reg_write(REG_CHARGE, val);

	/* power up the system first */
	pmic_reg_write(REG_POWER_MISC, PWUP);

	/* Set core voltage to 1.1V */
	val = pmic_reg_read(REG_SW_0);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_100V;
	pmic_reg_write(REG_SW_0, val);

	/* Setup VCC (SW2) to 1.25 */
	val = pmic_reg_read(REG_SW_1);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(REG_SW_1, val);

	/* Setup 1V2_DIG1 (SW3) to 1.25 */
	val = pmic_reg_read(REG_SW_2);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(REG_SW_2, val);
	udelay(50);

	/* Raise the core frequency to 800MHz */
	writel(0x0, &mxc_ccm->cacrr);

	/* Set switchers in Auto in NORMAL mode & STANDBY mode */
	/* Setup the switcher mode for SW1 & SW2*/
	val = pmic_reg_read(REG_SW_4);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	pmic_reg_write(REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	val = pmic_reg_read(REG_SW_5);
	val = (val & ~((SWMODE_MASK << SWMODE3_SHIFT) |
		(SWMODE_MASK << SWMODE4_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE3_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE4_SHIFT);
	pmic_reg_write(REG_SW_5, val);

	/* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.6V */
	val = pmic_reg_read(REG_SETTING_0);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_65 | VGEN3_1_8 | VCAM_2_6;
	pmic_reg_write(REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	val = pmic_reg_read(REG_SETTING_1);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775;
	pmic_reg_write(REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(REG_MODE_1, val);
	udelay(200);

	gpio_direction_output(46, 0);

	/* Reset the ethernet controller over GPIO */
	writel(0x1, IOMUXC_BASE_ADDR + 0x0AC);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(REG_MODE_1, val);

	udelay(500);

	gpio_set_value(46, 1);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		*cd = gpio_get_value(0);
	else
		*cd = gpio_get_value(6);

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	s32 status = 0;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
			index++) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX51_PIN_SD1_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_CLK,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA0,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA1,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA2,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA3,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_SD1_CMD,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_CLK,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_NONE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA0,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA1,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA2,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA3,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PD |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_request_iomux(MX51_PIN_GPIO1_0,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_0,
				PAD_CTL_HYS_ENABLE);
			mxc_request_iomux(MX51_PIN_GPIO1_1,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_1,
				PAD_CTL_HYS_ENABLE);
			break;
		case 1:
			mxc_request_iomux(MX51_PIN_SD2_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD2_CLK,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD2_DATA0,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA1,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA2,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA3,
				IOMUX_CONFIG_ALT0);
			mxc_iomux_set_pad(MX51_PIN_SD2_CMD,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_CLK,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA0,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA1,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA2,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA3,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_request_iomux(MX51_PIN_SD2_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_GPIO1_6,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_6,
				PAD_CTL_HYS_ENABLE);
			mxc_request_iomux(MX51_PIN_GPIO1_5,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_5,
				PAD_CTL_HYS_ENABLE);
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}
	return status;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();

	return 0;
}

int board_init(void)
{
	system_rev = get_cpu_rev();

	gd->bd->bi_arch_number = MACH_TYPE_MX51_BABBAGE;
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_MXC_SPI
	setup_iomux_spi();
	power_init();
#endif
	return 0;
}
#endif

int checkboard(void)
{
	puts("Board: MX51EVK\n");

	return 0;
}
