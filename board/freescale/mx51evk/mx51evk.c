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
#include <asm/arch/clock.h>
#include <asm/imx-common/mx5_video.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <usb/ehci-fsl.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
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

#ifdef CONFIG_USB_EHCI_MX5
#define MX51EVK_USBH1_HUB_RST	IOMUX_TO_GPIO(MX51_PIN_GPIO1_7) /* GPIO1_7 */
#define MX51EVK_USBH1_STP	IOMUX_TO_GPIO(MX51_PIN_USBH1_STP) /* GPIO1_27 */
#define MX51EVK_USB_CLK_EN_B	IOMUX_TO_GPIO(MX51_PIN_EIM_D18) /* GPIO2_1 */
#define MX51EVK_USB_PHY_RESET	IOMUX_TO_GPIO(MX51_PIN_EIM_D21) /* GPIO2_5 */

#define USBH1_PAD	(PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |		\
			 PAD_CTL_100K_PU | PAD_CTL_PUE_PULL |		\
			 PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE)
#define GPIO_PAD	(PAD_CTL_DRV_HIGH | PAD_CTL_PKE_ENABLE |	\
			 PAD_CTL_SRE_FAST)
#define NO_PAD		(1 << 16)

static void setup_usb_h1(void)
{
	setup_iomux_usb_h1();

	/* GPIO_1_7 for USBH1 hub reset */
	mxc_request_iomux(MX51_PIN_GPIO1_7, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_GPIO1_7, NO_PAD);

	/* GPIO_2_1 */
	mxc_request_iomux(MX51_PIN_EIM_D17, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_EIM_D17, GPIO_PAD);

	/* GPIO_2_5 for USB PHY reset */
	mxc_request_iomux(MX51_PIN_EIM_D21, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_EIM_D21, GPIO_PAD);
}

int board_ehci_hcd_init(int port)
{
	/* Set USBH1_STP to GPIO and toggle it */
	mxc_request_iomux(MX51_PIN_USBH1_STP, IOMUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX51_PIN_USBH1_STP, USBH1_PAD);

	gpio_direction_output(MX51EVK_USBH1_STP, 0);
	gpio_direction_output(MX51EVK_USB_PHY_RESET, 0);
	mdelay(10);
	gpio_set_value(MX51EVK_USBH1_STP, 1);

	/* Set back USBH1_STP to be function */
	mxc_request_iomux(MX51_PIN_USBH1_STP, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_STP, USBH1_PAD);

	/* De-assert USB PHY RESETB */
	gpio_set_value(MX51EVK_USB_PHY_RESET, 1);

	/* Drive USB_CLK_EN_B line low */
	gpio_direction_output(MX51EVK_USB_CLK_EN_B, 0);

	/* Reset USB hub */
	gpio_direction_output(MX51EVK_USBH1_HUB_RST, 0);
	mdelay(2);
	gpio_set_value(MX51EVK_USBH1_HUB_RST, 1);
	return 0;
}
#endif

static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(I2C_PMIC);
	if (ret)
		return;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return;

	/* Write needed to Power Gate 2 register */
	pmic_reg_read(p, REG_POWER_MISC, &val);
	val &= ~PWGT2SPIEN;
	pmic_reg_write(p, REG_POWER_MISC, val);

	/* Externally powered */
	pmic_reg_read(p, REG_CHARGE, &val);
	val |= ICHRG0 | ICHRG1 | ICHRG2 | ICHRG3 | CHGAUTOB;
	pmic_reg_write(p, REG_CHARGE, val);

	/* power up the system first */
	pmic_reg_write(p, REG_POWER_MISC, PWUP);

	/* Set core voltage to 1.1V */
	pmic_reg_read(p, REG_SW_0, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_100V;
	pmic_reg_write(p, REG_SW_0, val);

	/* Setup VCC (SW2) to 1.25 */
	pmic_reg_read(p, REG_SW_1, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_1, val);

	/* Setup 1V2_DIG1 (SW3) to 1.25 */
	pmic_reg_read(p, REG_SW_2, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_2, val);
	udelay(50);

	/* Raise the core frequency to 800MHz */
	writel(0x0, &mxc_ccm->cacrr);

	/* Set switchers in Auto in NORMAL mode & STANDBY mode */
	/* Setup the switcher mode for SW1 & SW2*/
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val = (val & ~((SWMODE_MASK << SWMODE3_SHIFT) |
		(SWMODE_MASK << SWMODE4_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE3_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE4_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.6V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_65 | VGEN3_1_8 | VCAM_2_6;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);

	mxc_request_iomux(MX51_PIN_EIM_A20, IOMUX_CONFIG_ALT1);
	gpio_direction_output(IMX_GPIO_NR(2, 14), 0);

	udelay(500);

	gpio_set_value(IMX_GPIO_NR(2, 14), 1);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	mxc_request_iomux(MX51_PIN_GPIO1_0, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(1, 0));
	mxc_request_iomux(MX51_PIN_GPIO1_6, IOMUX_CONFIG_ALT0);
	gpio_direction_input(IMX_GPIO_NR(1, 6));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 0));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 6));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	s32 status = 0;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

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
#ifdef CONFIG_USB_EHCI_MX5
	setup_usb_h1();
#endif
	setup_iomux_lcd();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	lcd_enable();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_MXC_SPI
	setup_iomux_spi();
	power_init();
#endif

	return 0;
}
#endif

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int checkboard(void)
{
	puts("Board: MX51EVK\n");

	return 0;
}
