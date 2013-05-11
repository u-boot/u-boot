/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
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
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx51.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <mmc.h>
#include <power/pmic.h>
#include <fsl_esdhc.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <linux/fb.h>

#include <ipu_pixfmt.h>

DECLARE_GLOBAL_DATA_PTR;

static struct fb_videomode const nec_nl6448bc26_09c = {
	"NEC_NL6448BC26-09C",
	60,	/* Refresh */
	640,	/* xres */
	480,	/* yres */
	37650,	/* pixclock = 26.56Mhz */
	48,	/* left margin */
	16,	/* right margin */
	31,	/* upper margin */
	12,	/* lower margin */
	96,	/* hsync-len */
	2,	/* vsync-len */
	0,	/* sync */
	FB_VMODE_NONINTERLACED,	/* vmode */
	0,	/* flag */
};

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
void hw_watchdog_reset(void)
{
	int val;

	/* toggle watchdog trigger pin */
	val = gpio_get_value(IMX_GPIO_NR(3, 2));
	val = val ? 0 : 1;
	gpio_set_value(IMX_GPIO_NR(3, 2), val);
}
#endif

static void init_drive_strength(void)
{
	static const iomux_v3_cfg_t ddr_pads[] = {
		NEW_PAD_CTRL(MX51_GRP_PKEDDR, 0),
		NEW_PAD_CTRL(MX51_GRP_PKEADDR, PAD_CTL_PKE),
		NEW_PAD_CTRL(MX51_GRP_DDRAPKS, 0),
		NEW_PAD_CTRL(MX51_GRP_DDRAPUS, PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX51_GRP_DDR_SR_A1, PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_DDR_A0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX51_GRP_DDR_A1, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX51_PAD_DRAM_RAS__DRAM_RAS,
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_DRAM_CAS__DRAM_CAS,
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_PKEDDR, PAD_CTL_PKE),
		NEW_PAD_CTRL(MX51_GRP_DDRPKS, 0),
		NEW_PAD_CTRL(MX51_GRP_HYSDDR0, 0),
		NEW_PAD_CTRL(MX51_GRP_HYSDDR1, 0),
		NEW_PAD_CTRL(MX51_GRP_HYSDDR2, 0),
		NEW_PAD_CTRL(MX51_GRP_HYSDDR3, 0),
		NEW_PAD_CTRL(MX51_GRP_DRAM_SR_B0, PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_DRAM_SR_B1, PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_DRAM_SR_B2, PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_DRAM_SR_B4, PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_GRP_DDRPUS, PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX51_GRP_INMODE1, 0),
		NEW_PAD_CTRL(MX51_GRP_DRAM_B0, PAD_CTL_DSE_MED),
		NEW_PAD_CTRL(MX51_GRP_DRAM_B1, PAD_CTL_DSE_MED),
		NEW_PAD_CTRL(MX51_GRP_DRAM_B2, PAD_CTL_DSE_MED),
		NEW_PAD_CTRL(MX51_GRP_DRAM_B4, PAD_CTL_DSE_MED),

		NEW_PAD_CTRL(MX51_PAD_DRAM_SDWE__DRAM_SDWE, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDCKE0__DRAM_SDCKE0,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDCKE1__DRAM_SDCKE1,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDCLK__DRAM_SDCLK,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDQS0__DRAM_SDQS0,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDQS1__DRAM_SDQS1,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDQS2__DRAM_SDQS2,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_SDQS3__DRAM_SDQS3,
				MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_CS0__DRAM_CS0, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_CS1__DRAM_CS1, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_DQM0__DRAM_DQM0, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_DQM1__DRAM_DQM1, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_DQM2__DRAM_DQM2, MX51_GPIO_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_DRAM_DQM3__DRAM_DQM3, MX51_GPIO_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(ddr_pads, ARRAY_SIZE(ddr_pads));
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1,
		PHYS_SDRAM_1_SIZE);

	return 0;
}

static void setup_weim(void)
{
	struct weim  *pweim = (struct weim *)WEIM_BASE_ADDR;

	pweim->cs0gcr1 = 0x004100b9;
	pweim->cs0gcr2 = 0x00000001;
	pweim->cs0rcr1 = 0x0a018000;
	pweim->cs0rcr2 = 0;
	pweim->cs0wcr1 = 0x0704a240;
}

static void setup_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX51_PAD_EIM_D25__UART3_RXD, /* console RX */
		MX51_PAD_EIM_D26__UART3_TXD, /* console TX */
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#ifdef CONFIG_MXC_SPI
void spi_io_init(void)
{
	static const iomux_v3_cfg_t spi_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_CSPI1_MOSI__ECSPI1_MOSI, PAD_CTL_HYS |
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_MISO__ECSPI1_MISO, PAD_CTL_HYS |
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_SS0__ECSPI1_SS0, PAD_CTL_HYS |
			PAD_CTL_PKE | PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_SS1__ECSPI1_SS1, PAD_CTL_HYS |
			PAD_CTL_PKE | PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_DI1_PIN11__ECSPI1_SS2, PAD_CTL_HYS |
			PAD_CTL_PKE | PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_SCLK__ECSPI1_SCLK, PAD_CTL_HYS |
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
	};

	imx_iomux_v3_setup_multiple_pads(spi_pads, ARRAY_SIZE(spi_pads));
}

static void reset_peripherals(int reset)
{
#ifdef CONFIG_VISION2_HW_1_0
	static const iomux_v3_cfg_t fec_cfg_pads[] = {
		/* RXD1 */
		NEW_PAD_CTRL(MX51_PAD_EIM_EB3__GPIO2_23, NO_PAD_CTRL),
		/* RXD2 */
		NEW_PAD_CTRL(MX51_PAD_EIM_CS2__GPIO2_27, NO_PAD_CTRL),
		/* RXD3 */
		NEW_PAD_CTRL(MX51_PAD_EIM_CS3__GPIO2_28, NO_PAD_CTRL),
		/* RXER */
		NEW_PAD_CTRL(MX51_PAD_EIM_CS4__GPIO2_29, NO_PAD_CTRL),
		/* COL */
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB2__GPIO3_10, NO_PAD_CTRL),
		/* RCLK */
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB3__GPIO3_11, NO_PAD_CTRL),
		/* RXD0 */
		NEW_PAD_CTRL(MX51_PAD_NANDF_D9__GPIO3_31, NO_PAD_CTRL),
	};

	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_EIM_CS3__FEC_RDATA3, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_CS2__FEC_RDATA2, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_EB3__FEC_RDATA1, MX51_PAD_CTRL_2),
		MX51_PAD_NANDF_D9__FEC_RDATA0,
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB3__FEC_RX_CLK, MX51_PAD_CTRL_4),
		MX51_PAD_EIM_CS4__FEC_RX_ER,
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB2__FEC_COL, MX51_PAD_CTRL_4),
	};
#endif

	if (reset) {

		/* reset_n is on NANDF_D15 */
		gpio_direction_output(IMX_GPIO_NR(3, 25), 0);

#ifdef CONFIG_VISION2_HW_1_0
		/*
		 * set FEC Configuration lines
		 * set levels of FEC config lines
		 */
		gpio_direction_output(IMX_GPIO_NR(3, 11), 0);
		gpio_direction_output(IMX_GPIO_NR(3, 10), 1);
		gpio_direction_output(IMX_GPIO_NR(3, 31), 1);

		/* set direction of FEC config lines */
		gpio_direction_output(IMX_GPIO_NR(2, 27), 0);
		gpio_direction_output(IMX_GPIO_NR(2, 28), 0);
		gpio_direction_output(IMX_GPIO_NR(2, 29), 0);
		gpio_direction_output(IMX_GPIO_NR(2, 23), 1);

		imx_iomux_v3_setup_multiple_pads(fec_cfg_pads,
						 ARRAY_SIZE(fec_cfg_pads));
#endif

		/* activate reset_n pin */
		imx_iomux_v3_setup_pad(
				NEW_PAD_CTRL(MX51_PAD_NANDF_D15__GPIO3_25,
						PAD_CTL_DSE_MAX));
	} else {
		/* set FEC Control lines */
		gpio_direction_input(IMX_GPIO_NR(3, 25));
		udelay(500);

#ifdef CONFIG_VISION2_HW_1_0
		imx_iomux_v3_setup_multiple_pads(fec_pads,
							ARRAY_SIZE(fec_pads));
#endif
	}
}

static void power_init_mx51(void)
{
	unsigned int val;
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

	/* enable VCAM with 2.775V to enable read from PMIC */
	val = VCAMCONFIG | VCAMEN;
	pmic_reg_write(p, REG_MODE_1, val);

	/*
	 * Set switchers in Auto in NORMAL mode & STANDBY mode
	 * Setup the switcher mode for SW1 & SW2
	 */
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val &= ~((SWMODE_MASK << SWMODE4_SHIFT) |
		(SWMODE_MASK << SWMODE3_SHIFT));
	val |= (SWMODE_AUTO_AUTO << SWMODE4_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE3_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);


	/* Set VGEN3 to 1.8V, VCAM to 3.0V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK);
	val |= VCAM_3_0;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V0, VSD to 1.8V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VVIDEO_2_775 | VAUDIO_3_0 | VSD_1_8;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);

	pmic_reg_read(p, REG_POWER_CTL2, &val);
	val |= WDIRESET;
	pmic_reg_write(p, REG_POWER_CTL2, val);

	udelay(2500);

}
#endif

static void setup_gpios(void)
{
	static const iomux_v3_cfg_t gpio_pads_1[] = {
		NEW_PAD_CTRL(MX51_PAD_GPIO1_7__GPIO1_7, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* CAM_SUP_DISn */
		NEW_PAD_CTRL(MX51_PAD_DI1_PIN12__GPIO3_1, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* DAB Display EN */
		NEW_PAD_CTRL(MX51_PAD_DI1_PIN13__GPIO3_2, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* WDOG_TRIGGER */
	};

	static const iomux_v3_cfg_t gpio_pads_2[] = {
		NEW_PAD_CTRL(MX51_PAD_DI1_D0_CS__GPIO3_3, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* Display2 TxEN */
		NEW_PAD_CTRL(MX51_PAD_DI1_D1_CS__GPIO3_4, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* DAB Light EN */
		NEW_PAD_CTRL(MX51_PAD_DISPB2_SER_DIN__GPIO3_5, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* AUDIO_MUTE */
		NEW_PAD_CTRL(MX51_PAD_DISPB2_SER_DIO__GPIO3_6, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* SPARE_OUT */
		NEW_PAD_CTRL(MX51_PAD_NANDF_D14__GPIO3_26, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* BEEPER_EN */
		NEW_PAD_CTRL(MX51_PAD_NANDF_D13__GPIO3_27, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* POWER_OFF */
		NEW_PAD_CTRL(MX51_PAD_NANDF_D10__GPIO3_30, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* FRAM_WE */
		NEW_PAD_CTRL(MX51_PAD_CSPI1_RDY__GPIO4_26, PAD_CTL_PKE |
				PAD_CTL_DSE_MED), /* EXPANSION_EN */
		MX51_PAD_GPIO1_2__PWM1_PWMO,
	};

	unsigned int i;

	imx_iomux_v3_setup_multiple_pads(gpio_pads_1, ARRAY_SIZE(gpio_pads_1));

	/* Now we need to trigger the watchdog */
	WATCHDOG_RESET();

	imx_iomux_v3_setup_multiple_pads(gpio_pads_2, ARRAY_SIZE(gpio_pads_2));

	/*
	 * Set GPIO1_4 to high and output; it is used to reset
	 * the system on reboot
	 */
	gpio_direction_output(IMX_GPIO_NR(1, 4), 1);

	gpio_direction_output(IMX_GPIO_NR(1, 7), 0);
	for (i = IMX_GPIO_NR(3, 1); i < IMX_GPIO_NR(3, 7); i++)
		gpio_direction_output(i, 0);

	gpio_direction_output(IMX_GPIO_NR(3, 30), 0);

	/* Set POWER_OFF high */
	gpio_direction_output(IMX_GPIO_NR(3, 27), 1);

	gpio_direction_output(IMX_GPIO_NR(3, 26), 0);

	gpio_direction_output(IMX_GPIO_NR(4, 26), 0);

	gpio_direction_output(IMX_GPIO_NR(4, 25), 1);

	WATCHDOG_RESET();
}

static void setup_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_EIM_EB2__FEC_MDIO, PAD_CTL_HYS |
				PAD_CTL_PUS_22K_UP | PAD_CTL_ODE |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		MX51_PAD_NANDF_CS3__FEC_MDC,
		NEW_PAD_CTRL(MX51_PAD_EIM_CS3__FEC_RDATA3, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_CS2__FEC_RDATA2, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_EB3__FEC_RDATA1, MX51_PAD_CTRL_2),
		MX51_PAD_NANDF_D9__FEC_RDATA0,
		MX51_PAD_NANDF_CS6__FEC_TDATA3,
		MX51_PAD_NANDF_CS5__FEC_TDATA2,
		MX51_PAD_NANDF_CS4__FEC_TDATA1,
		MX51_PAD_NANDF_D8__FEC_TDATA0,
		MX51_PAD_NANDF_CS7__FEC_TX_EN,
		MX51_PAD_NANDF_CS2__FEC_TX_ER,
		MX51_PAD_NANDF_RDY_INT__FEC_TX_CLK,
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB2__FEC_COL, MX51_PAD_CTRL_4),
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB3__FEC_RX_CLK, MX51_PAD_CTRL_4),
		MX51_PAD_EIM_CS5__FEC_CRS,
		MX51_PAD_EIM_CS4__FEC_RX_ER,
		NEW_PAD_CTRL(MX51_PAD_NANDF_D11__FEC_RX_DV, MX51_PAD_CTRL_4),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{MMC_SDHC1_BASE_ADDR},
};

int get_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		*cd = gpio_get_value(IMX_GPIO_NR(1, 0));
	else
		*cd = 0;

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD1_CMD__SD1_CMD, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_CLK__SD1_CLK, PAD_CTL_DSE_MAX |
			PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA0__SD1_DATA0, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA1__SD1_DATA1, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA2__SD1_DATA2, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA3__SD1_DATA3, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_0__SD1_CD, PAD_CTL_HYS),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_1__SD1_WP, PAD_CTL_HYS),
	};

	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

void lcd_enable(void)
{
	static const iomux_v3_cfg_t lcd_pads[] = {
		MX51_PAD_DI1_PIN2__DI1_PIN2,
		MX51_PAD_DI1_PIN3__DI1_PIN3,
	};

	int ret;

	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));

	gpio_set_value(IMX_GPIO_NR(1, 2), 1);
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_2__GPIO1_2,
						NO_PAD_CTRL));

	ret = ipuv3_fb_init(&nec_nl6448bc26_09c, 0, IPU_PIX_FMT_RGB666);
	if (ret)
		puts("LCD cannot be configured\n");
}

int board_early_init_f(void)
{


	init_drive_strength();

	/* Setup debug led */
	gpio_direction_output(IMX_GPIO_NR(1, 6), 0);
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6,
					PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST));

	/* wait a little while to give the pll time to settle */
	sdelay(100000);

	setup_weim();
	setup_uart();
	setup_fec();
	setup_gpios();

	spi_io_init();

	return 0;
}

static void backlight(int on)
{
	if (on) {
		gpio_set_value(IMX_GPIO_NR(3, 1), 1);
		udelay(10000);
		gpio_set_value(IMX_GPIO_NR(3, 4), 1);
	} else {
		gpio_set_value(IMX_GPIO_NR(3, 1), 0);
		gpio_set_value(IMX_GPIO_NR(3, 4), 0);
	}
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	lcd_enable();

	backlight(1);

	return 0;
}

int board_late_init(void)
{
	power_init_mx51();

	reset_peripherals(1);
	udelay(2000);
	reset_peripherals(0);
	udelay(2000);

	/* Early revisions require a second reset */
#ifdef CONFIG_VISION2_HW_1_0
	reset_peripherals(1);
	udelay(2000);
	reset_peripherals(0);
	udelay(2000);
#endif

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

int checkboard(void)
{
	puts("Board: TTControl Vision II CPU V\n");

	return 0;
}

int do_vision_lcd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int on;

	if (argc < 2)
		return cmd_usage(cmdtp);

	on = (strcmp(argv[1], "on") == 0);
	backlight(on);

	return 0;
}

U_BOOT_CMD(
	lcdbl, CONFIG_SYS_MAXARGS, 1, do_vision_lcd,
	"Vision2 Backlight",
	"lcdbl [on|off]\n"
);
