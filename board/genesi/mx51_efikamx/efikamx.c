/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 * Copyright (C) 2009-2012 Genesi USA, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/iomux-mx51.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Compile-time error checking
 */
#ifndef	CONFIG_MXC_SPI
#error "CONFIG_MXC_SPI not set, this is essential for board's operation!"
#endif

/*
 * Board revisions
 *
 * Note that we get these revisions here for convenience, but we only set
 * up for the production model Smarttop (1.3) and Smartbook (2.0).
 *
 */
#define	EFIKAMX_BOARD_REV_11	0x1
#define	EFIKAMX_BOARD_REV_12	0x2
#define	EFIKAMX_BOARD_REV_13	0x3
#define	EFIKAMX_BOARD_REV_14	0x4

#define	EFIKASB_BOARD_REV_13	0x1
#define	EFIKASB_BOARD_REV_20	0x2

/*
 * Board identification
 */
static u32 get_mx_rev(void)
{
	u32 rev = 0;
	/*
	 * Retrieve board ID:
	 *
	 *  gpio: 16 17 11
	 *  ==============
	 *	r1.1:  1+ 1  1
	 *	r1.2:  1  1  0
	 *	r1.3:  1  0  1
	 *	r1.4:  1  0  0
	 *
	 * + note: r1.1 does not strap this pin properly so it needs to
	 *         be hacked or ignored.
	 */

	/* set to 1 in order to get correct value on board rev 1.1 */
	gpio_direction_output(IMX_GPIO_NR(3, 16), 1);
	gpio_direction_input(IMX_GPIO_NR(3, 11));
	gpio_direction_input(IMX_GPIO_NR(3, 16));
	gpio_direction_input(IMX_GPIO_NR(3, 17));

	rev |= (!!gpio_get_value(IMX_GPIO_NR(3, 16))) << 0;
	rev |= (!!gpio_get_value(IMX_GPIO_NR(3, 17))) << 1;
	rev |= (!!gpio_get_value(IMX_GPIO_NR(3, 11))) << 2;

	return (~rev & 0x7) + 1;
}

static iomux_v3_cfg_t const efikasb_revision_pads[] = {
	MX51_PAD_EIM_CS3__GPIO2_28,
	MX51_PAD_EIM_CS4__GPIO2_29,
};

static inline u32 get_sb_rev(void)
{
	u32 rev = 0;

	imx_iomux_v3_setup_multiple_pads(efikasb_revision_pads,
				ARRAY_SIZE(efikasb_revision_pads));
	gpio_direction_input(IMX_GPIO_NR(2, 28));
	gpio_direction_input(IMX_GPIO_NR(2, 29));

	rev |= (!!gpio_get_value(IMX_GPIO_NR(2, 28))) << 0;
	rev |= (!!gpio_get_value(IMX_GPIO_NR(2, 29))) << 1;

	return rev;
}

inline uint32_t get_efikamx_rev(void)
{
	if (machine_is_efikamx())
		return get_mx_rev();
	else if (machine_is_efikasb())
		return get_sb_rev();
}

u32 get_board_rev(void)
{
	return get_cpu_rev() | (get_efikamx_rev() << 8);
}

/*
 * DRAM initialization
 */
int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
						PHYS_SDRAM_1_SIZE);
	return 0;
}

/*
 * UART configuration
 */
static iomux_v3_cfg_t const efikamx_uart_pads[] = {
	MX51_PAD_UART1_RXD__UART1_RXD,
	MX51_PAD_UART1_TXD__UART1_TXD,
	MX51_PAD_UART1_RTS__UART1_RTS,
	MX51_PAD_UART1_CTS__UART1_CTS,
};

/*
 * SPI configuration
 */
static iomux_v3_cfg_t const efikamx_spi_pads[] = {
	MX51_PAD_CSPI1_MOSI__ECSPI1_MOSI,
	MX51_PAD_CSPI1_MISO__ECSPI1_MISO,
	MX51_PAD_CSPI1_SCLK__ECSPI1_SCLK,
	MX51_PAD_CSPI1_SS0__GPIO4_24,
	MX51_PAD_CSPI1_SS1__GPIO4_25,
	MX51_PAD_GPIO1_6__GPIO1_6,
};

#define EFIKAMX_SPI_SS0		IMX_GPIO_NR(4, 24)
#define EFIKAMX_SPI_SS1		IMX_GPIO_NR(4, 25)
#define EFIKAMX_PMIC_IRQ	IMX_GPIO_NR(1, 6)

/*
 * PMIC configuration
 */
#ifdef CONFIG_MXC_SPI
static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(CONFIG_FSL_PMIC_BUS);
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

	/* Set VDIG to 1.8V, VGEN3 to 1.8V, VCAM to 2.6V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_8 | VGEN3_1_8 | VCAM_2_6;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775 | VGEN1_1_2 | VGEN2_3_15;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Enable VGEN1, VGEN2, VDIG, VPLL */
	pmic_reg_read(p, REG_MODE_0, &val);
	val |= VGEN1EN | VDIGEN | VGEN2EN | VPLLEN;
	pmic_reg_write(p, REG_MODE_0, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);

	pmic_reg_read(p, REG_POWER_CTL2, &val);
	val |= WDIRESET;
	pmic_reg_write(p, REG_POWER_CTL2, val);

	udelay(2500);
}
#else
static inline void power_init(void) { }
#endif

/*
 * MMC configuration
 */
#ifdef CONFIG_FSL_ESDHC

struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};

static iomux_v3_cfg_t const efikamx_sdhc1_pads[] = {
	MX51_PAD_SD1_CMD__SD1_CMD,
	MX51_PAD_SD1_CLK__SD1_CLK,
	MX51_PAD_SD1_DATA0__SD1_DATA0,
	MX51_PAD_SD1_DATA1__SD1_DATA1,
	MX51_PAD_SD1_DATA2__SD1_DATA2,
	MX51_PAD_SD1_DATA3__SD1_DATA3,
	MX51_PAD_GPIO1_1__SD1_WP,
};

#define EFIKAMX_SDHC1_WP	IMX_GPIO_NR(1, 1)

static iomux_v3_cfg_t const efikamx_sdhc1_cd_pads[] = {
	MX51_PAD_GPIO1_0__SD1_CD,
	NEW_PAD_CTRL(MX51_PAD_EIM_CS2__GPIO2_27, MX51_ESDHC_PAD_CTRL),
};

#define EFIKAMX_SDHC1_CD	IMX_GPIO_NR(1, 0)
#define EFIKASB_SDHC1_CD	IMX_GPIO_NR(2, 27)

static iomux_v3_cfg_t const efikasb_sdhc2_pads[] = {
	MX51_PAD_SD2_CMD__SD2_CMD,
	MX51_PAD_SD2_CLK__SD2_CLK,
	MX51_PAD_SD2_DATA0__SD2_DATA0,
	MX51_PAD_SD2_DATA1__SD2_DATA1,
	MX51_PAD_SD2_DATA2__SD2_DATA2,
	MX51_PAD_SD2_DATA3__SD2_DATA3,
	MX51_PAD_GPIO1_7__SD2_WP,
	MX51_PAD_GPIO1_8__SD2_CD,
};

#define EFIKASB_SDHC2_CD	IMX_GPIO_NR(1, 8)
#define EFIKASB_SDHC2_WP	IMX_GPIO_NR(1, 7)

static inline uint32_t efikamx_mmc_getcd(u32 base)
{
	if (base == MMC_SDHC1_BASE_ADDR)
		if (machine_is_efikamx())
			return EFIKAMX_SDHC1_CD;
		else
			return EFIKASB_SDHC1_CD;
	else
		return EFIKASB_SDHC2_CD;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	uint32_t cd = efikamx_mmc_getcd(cfg->esdhc_base);
	int ret = !gpio_get_value(cd);

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int ret;

	/*
	 * All Efika MX boards use eSDHC1 with a common write-protect GPIO
	 */
	imx_iomux_v3_setup_multiple_pads(efikamx_sdhc1_pads,
					ARRAY_SIZE(efikamx_sdhc1_pads));
	gpio_direction_input(EFIKAMX_SDHC1_WP);

	/*
	 * Smartbook and Smarttop differ on the location of eSDHC1
	 * carrier-detect GPIO
	 */
	if (machine_is_efikamx()) {
		imx_iomux_v3_setup_pad(efikamx_sdhc1_cd_pads[0]);
		gpio_direction_input(EFIKAMX_SDHC1_CD);
	} else if (machine_is_efikasb()) {
		imx_iomux_v3_setup_pad(efikamx_sdhc1_cd_pads[1]);
		gpio_direction_input(EFIKASB_SDHC1_CD);
	}

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

	ret = fsl_esdhc_initialize(bis, &esdhc_cfg[0]);

	if (machine_is_efikasb()) {

		imx_iomux_v3_setup_multiple_pads(efikasb_sdhc2_pads,
						ARRAY_SIZE(efikasb_sdhc2_pads));
		gpio_direction_input(EFIKASB_SDHC2_CD);
		gpio_direction_input(EFIKASB_SDHC2_WP);
		if (!ret)
			ret = fsl_esdhc_initialize(bis, &esdhc_cfg[1]);
	}

	return ret;
}
#endif

/*
 * PATA
 */
static iomux_v3_cfg_t const efikamx_pata_pads[] = {
	MX51_PAD_NANDF_WE_B__PATA_DIOW,
	MX51_PAD_NANDF_RE_B__PATA_DIOR,
	MX51_PAD_NANDF_ALE__PATA_BUFFER_EN,
	MX51_PAD_NANDF_CLE__PATA_RESET_B,
	MX51_PAD_NANDF_WP_B__PATA_DMACK,
	MX51_PAD_NANDF_RB0__PATA_DMARQ,
	MX51_PAD_NANDF_RB1__PATA_IORDY,
	MX51_PAD_GPIO_NAND__PATA_INTRQ,
	MX51_PAD_NANDF_CS2__PATA_CS_0,
	MX51_PAD_NANDF_CS3__PATA_CS_1,
	MX51_PAD_NANDF_CS4__PATA_DA_0,
	MX51_PAD_NANDF_CS5__PATA_DA_1,
	MX51_PAD_NANDF_CS6__PATA_DA_2,
	MX51_PAD_NANDF_D15__PATA_DATA15,
	MX51_PAD_NANDF_D14__PATA_DATA14,
	MX51_PAD_NANDF_D13__PATA_DATA13,
	MX51_PAD_NANDF_D12__PATA_DATA12,
	MX51_PAD_NANDF_D11__PATA_DATA11,
	MX51_PAD_NANDF_D10__PATA_DATA10,
	MX51_PAD_NANDF_D9__PATA_DATA9,
	MX51_PAD_NANDF_D8__PATA_DATA8,
	MX51_PAD_NANDF_D7__PATA_DATA7,
	MX51_PAD_NANDF_D6__PATA_DATA6,
	MX51_PAD_NANDF_D5__PATA_DATA5,
	MX51_PAD_NANDF_D4__PATA_DATA4,
	MX51_PAD_NANDF_D3__PATA_DATA3,
	MX51_PAD_NANDF_D2__PATA_DATA2,
	MX51_PAD_NANDF_D1__PATA_DATA1,
	MX51_PAD_NANDF_D0__PATA_DATA0,
};

/*
 * EHCI USB
 */
#ifdef	CONFIG_CMD_USB
extern void setup_iomux_usb(void);
#else
static inline void setup_iomux_usb(void) { }
#endif

/*
 * LED configuration
 *
 * Smarttop LED pad config is done in the DCD
 *
 */
#define EFIKAMX_LED_BLUE	IMX_GPIO_NR(3, 13)
#define EFIKAMX_LED_GREEN	IMX_GPIO_NR(3, 14)
#define EFIKAMX_LED_RED		IMX_GPIO_NR(3, 15)

static iomux_v3_cfg_t const efikasb_led_pads[] = {
	MX51_PAD_GPIO1_3__GPIO1_3,
	MX51_PAD_EIM_CS0__GPIO2_25,
};

#define EFIKASB_CAPSLOCK_LED	IMX_GPIO_NR(2, 25)
#define EFIKASB_MESSAGE_LED	IMX_GPIO_NR(1, 3) /* Note: active low */

/*
 * Board initialization
 */
int board_early_init_f(void)
{
	if (machine_is_efikasb()) {
		imx_iomux_v3_setup_multiple_pads(efikasb_led_pads,
						ARRAY_SIZE(efikasb_led_pads));
		gpio_direction_output(EFIKASB_CAPSLOCK_LED, 0);
		gpio_direction_output(EFIKASB_MESSAGE_LED, 1);
	} else if (machine_is_efikamx()) {
		/*
		 * Set up GPIO directions for LEDs.
		 * IOMUX has been done in the DCD already.
		 * Turn the red LED on for pre-relocation code.
		 */
		gpio_direction_output(EFIKAMX_LED_BLUE, 0);
		gpio_direction_output(EFIKAMX_LED_GREEN, 0);
		gpio_direction_output(EFIKAMX_LED_RED, 1);
	}

	/*
	 * Both these pad configurations for UART and SPI are kind of redundant
	 * since they are the Power-On Defaults for the i.MX51. But, it seems we
	 * should make absolutely sure that they are set up correctly.
	 */
	imx_iomux_v3_setup_multiple_pads(efikamx_uart_pads,
					ARRAY_SIZE(efikamx_uart_pads));
	imx_iomux_v3_setup_multiple_pads(efikamx_spi_pads,
					ARRAY_SIZE(efikamx_spi_pads));

	/* not technically required for U-Boot operation but do it anyway. */
	gpio_direction_input(EFIKAMX_PMIC_IRQ);
	/* Deselect both CS for now, otherwise NOR doesn't probe properly. */
	gpio_direction_output(EFIKAMX_SPI_SS0, 0);
	gpio_direction_output(EFIKAMX_SPI_SS1, 1);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	if (machine_is_efikamx()) {
		/*
		 * Set up Blue LED for "In U-Boot" status.
		 * We're all relocated and ready to U-Boot!
		 */
		gpio_set_value(EFIKAMX_LED_RED, 0);
		gpio_set_value(EFIKAMX_LED_GREEN, 0);
		gpio_set_value(EFIKAMX_LED_BLUE, 1);
	}

	power_init();

	imx_iomux_v3_setup_multiple_pads(efikamx_pata_pads,
					ARRAY_SIZE(efikamx_pata_pads));
	setup_iomux_usb();

	return 0;
}

int checkboard(void)
{
	u32 rev = get_efikamx_rev();

	printf("Board: Genesi Efika MX ");
	if (machine_is_efikamx())
		printf("Smarttop (1.%i)\n", rev & 0xf);
	else if (machine_is_efikasb())
		printf("Smartbook\n");

	return 0;
}
