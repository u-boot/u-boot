/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
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
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/arch/clocks.h>
#include <asm/arch/clocks_omap3.h>
#include <asm/arch/ehci_omap3.h>
/* from drivers/usb/host/ehci-core.h */
extern struct ehci_hccr *hccr;
extern volatile struct ehci_hcor *hcor;
#endif
#include "beagle.h"
#include <command.h>

#define pr_debug(fmt, args...) debug(fmt, ##args)

#define TWL4030_I2C_BUS			0
#define EXPANSION_EEPROM_I2C_BUS	1
#define EXPANSION_EEPROM_I2C_ADDRESS	0x50

#define TINCANTOOLS_ZIPPY		0x01000100
#define TINCANTOOLS_ZIPPY2		0x02000100
#define TINCANTOOLS_TRAINER		0x04000100
#define TINCANTOOLS_SHOWDOG		0x03000100
#define KBADC_BEAGLEFPGA		0x01000600
#define LW_BEAGLETOUCH			0x01000700
#define BRAINMUX_LCDOG			0x01000800
#define BRAINMUX_LCDOGTOUCH		0x02000800
#define BBTOYS_WIFI			0x01000B00
#define BBTOYS_VGA			0x02000B00
#define BBTOYS_LCD			0x03000B00
#define BEAGLE_NO_EEPROM		0xffffffff

DECLARE_GLOBAL_DATA_PTR;

static struct {
	unsigned int device_vendor;
	unsigned char revision;
	unsigned char content;
	char fab_revision[8];
	char env_var[16];
	char env_setting[64];
} expansion_config;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_BEAGLE;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
#endif

	return 0;
}

/*
 * Routine: get_board_revision
 * Description: Detect if we are running on a Beagle revision Ax/Bx,
 *		C1/2/3, C4 or xM. This can be done by reading
 *		the level of GPIO173, GPIO172 and GPIO171. This should
 *		result in
 *		GPIO173, GPIO172, GPIO171: 1 1 1 => Ax/Bx
 *		GPIO173, GPIO172, GPIO171: 1 1 0 => C1/2/3
 *		GPIO173, GPIO172, GPIO171: 1 0 1 => C4
 *		GPIO173, GPIO172, GPIO171: 0 0 0 => xM
 */
int get_board_revision(void)
{
	int revision;

	if (!omap_request_gpio(171) &&
	    !omap_request_gpio(172) &&
	    !omap_request_gpio(173)) {

		omap_set_gpio_direction(171, 1);
		omap_set_gpio_direction(172, 1);
		omap_set_gpio_direction(173, 1);

		revision = omap_get_gpio_datain(173) << 2 |
			   omap_get_gpio_datain(172) << 1 |
			   omap_get_gpio_datain(171);

		omap_free_gpio(171);
		omap_free_gpio(172);
		omap_free_gpio(173);
	} else {
		printf("Error: unable to acquire board revision GPIOs\n");
		revision = -1;
	}

	return revision;
}

/*
 * Routine: get_expansion_id
 * Description: This function checks for expansion board by checking I2C
 *		bus 1 for the availability of an AT24C01B serial EEPROM.
 *		returns the device_vendor field from the EEPROM
 */
unsigned int get_expansion_id(void)
{
	i2c_set_bus_num(EXPANSION_EEPROM_I2C_BUS);

	/* return BEAGLE_NO_EEPROM if eeprom doesn't respond */
	if (i2c_probe(EXPANSION_EEPROM_I2C_ADDRESS) == 1) {
		i2c_set_bus_num(TWL4030_I2C_BUS);
		return BEAGLE_NO_EEPROM;
	}

	/* read configuration data */
	i2c_read(EXPANSION_EEPROM_I2C_ADDRESS, 0, 1, (u8 *)&expansion_config,
		 sizeof(expansion_config));

	i2c_set_bus_num(TWL4030_I2C_BUS);

	return expansion_config.device_vendor;
}

/*
 * Configure DSS to display background color on DVID
 * Configure VENC to display color bar on S-Video
 */
void beagle_display_init(void)
{
	omap3_dss_venc_config(&venc_config_std_tv, VENC_HEIGHT, VENC_WIDTH);
	switch (get_board_revision()) {
	case REVISION_AXBX:
	case REVISION_CX:
	case REVISION_C4:
		omap3_dss_panel_config(&dvid_cfg);
		break;
	case REVISION_XM_A:
	case REVISION_XM_B:
	case REVISION_XM_C:
	default:
		omap3_dss_panel_config(&dvid_cfg_xm);
		break;
	}
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;
	struct control_prog_io *prog_io_base = (struct control_prog_io *)OMAP34XX_CTRL_BASE;

	/* Enable i2c2 pullup resisters */
	writel(~(PRG_I2C2_PULLUPRESX), &prog_io_base->io1);

	switch (get_board_revision()) {
	case REVISION_AXBX:
		printf("Beagle Rev Ax/Bx\n");
		setenv("beaglerev", "AxBx");
		break;
	case REVISION_CX:
		printf("Beagle Rev C1/C2/C3\n");
		setenv("beaglerev", "Cx");
		MUX_BEAGLE_C();
		break;
	case REVISION_C4:
		printf("Beagle Rev C4\n");
		setenv("beaglerev", "C4");
		MUX_BEAGLE_C();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	case REVISION_XM_A:
		printf("Beagle xM Rev A\n");
		setenv("beaglerev", "xMA");
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	case REVISION_XM_B:
		printf("Beagle xM Rev B\n");
		setenv("beaglerev", "xMB");
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	case REVISION_XM_C:
		printf("Beagle xM Rev C\n");
		setenv("beaglerev", "xMC");
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	default:
		printf("Beagle unknown 0x%02x\n", get_board_revision());
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
	}

	switch (get_expansion_id()) {
	case TINCANTOOLS_ZIPPY:
		printf("Recognized Tincantools Zippy board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_TINCANTOOLS_ZIPPY();
		setenv("buddy", "zippy");
		break;
	case TINCANTOOLS_ZIPPY2:
		printf("Recognized Tincantools Zippy2 board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_TINCANTOOLS_ZIPPY();
		setenv("buddy", "zippy2");
		break;
	case TINCANTOOLS_TRAINER:
		printf("Recognized Tincantools Trainer board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_TINCANTOOLS_ZIPPY();
		MUX_TINCANTOOLS_TRAINER();
		setenv("buddy", "trainer");
		break;
	case TINCANTOOLS_SHOWDOG:
		printf("Recognized Tincantools Showdow board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		/* Place holder for DSS2 definition for showdog lcd */
		setenv("defaultdisplay", "showdoglcd");
		setenv("buddy", "showdog");
		break;
	case KBADC_BEAGLEFPGA:
		printf("Recognized KBADC Beagle FPGA board\n");
		MUX_KBADC_BEAGLEFPGA();
		setenv("buddy", "beaglefpga");
		break;
	case LW_BEAGLETOUCH:
		printf("Recognized Liquidware BeagleTouch board\n");
		setenv("buddy", "beagletouch");
		break;
	case BRAINMUX_LCDOG:
		printf("Recognized Brainmux LCDog board\n");
		setenv("buddy", "lcdog");
		break;
	case BRAINMUX_LCDOGTOUCH:
		printf("Recognized Brainmux LCDog Touch board\n");
		setenv("buddy", "lcdogtouch");
		break;
	case BBTOYS_WIFI:
		printf("Recognized BeagleBoardToys WiFi board\n");
		MUX_BBTOYS_WIFI()
		setenv("buddy", "bbtoys-wifi");
		break;;
	case BBTOYS_VGA:
		printf("Recognized BeagleBoardToys VGA board\n");
		break;;
	case BBTOYS_LCD:
		printf("Recognized BeagleBoardToys LCD board\n");
		break;;
	case BEAGLE_NO_EEPROM:
		printf("No EEPROM on expansion board\n");
		setenv("buddy", "none");
		break;
	default:
		printf("Unrecognized expansion board: %x\n",
			expansion_config.device_vendor);
		setenv("buddy", "unknown");
	}

	if (expansion_config.content == 1)
		setenv(expansion_config.env_var, expansion_config.env_setting);

	twl4030_power_init();
	switch (get_board_revision()) {
	case REVISION_XM_A:
	case REVISION_XM_B:
		twl4030_led_init(TWL4030_LED_LEDEN_LEDBON);
		break;
	default:
		twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);
		break;
	}

	/* Set GPIO states before they are made outputs */
	writel(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1,
		&gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);

	/* Configure GPIOs to output */
	writel(~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);

	dieid_num_r();
	beagle_display_init();
	omap3_dss_enable();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_BEAGLE();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

#ifdef CONFIG_USB_EHCI

#define GPIO_PHY_RESET 147

/* Reset is needed otherwise the kernel-driver will throw an error. */
int ehci_hcd_stop(void)
{
	pr_debug("Resetting OMAP3 EHCI\n");
	omap_set_gpio_dataout(GPIO_PHY_RESET, 0);
	writel(OMAP_UHH_SYSCONFIG_SOFTRESET, OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);
	/* disable USB clocks */
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	sr32(&prcm_base->iclken_usbhost, 0, 1, 0);
	sr32(&prcm_base->fclken_usbhost, 0, 2, 0);
	sr32(&prcm_base->iclken3_core, 2, 1, 0);
	sr32(&prcm_base->fclken3_core, 2, 1, 0);
	return 0;
}

/* Call usb_stop() before starting the kernel */
void show_boot_progress(int val)
{
	if(val == 15)
		usb_stop();
}

/*
 * Initialize the OMAP3 EHCI controller and PHY on the BeagleBoard.
 * Based on "drivers/usb/host/ehci-omap.c" from Linux 2.6.37.
 * See there for additional Copyrights.
 */
int ehci_hcd_init(void)
{
	pr_debug("Initializing OMAP3 ECHI\n");

	/* Put the PHY in RESET */
	omap_request_gpio(GPIO_PHY_RESET);
	omap_set_gpio_direction(GPIO_PHY_RESET, 0);
	omap_set_gpio_dataout(GPIO_PHY_RESET, 0);

	/* Hold the PHY in RESET for enough time till DIR is high */
	/* Refer: ISSUE1 */
	udelay(10);

	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	/* Enable USBHOST_L3_ICLK (USBHOST_MICLK) */
	sr32(&prcm_base->iclken_usbhost, 0, 1, 1);
	/*
	 * Enable USBHOST_48M_FCLK (USBHOST_FCLK1)
	 * and USBHOST_120M_FCLK (USBHOST_FCLK2)
	 */
	sr32(&prcm_base->fclken_usbhost, 0, 2, 3);
	/* Enable USBTTL_ICLK */
	sr32(&prcm_base->iclken3_core, 2, 1, 1);
	/* Enable USBTTL_FCLK */
	sr32(&prcm_base->fclken3_core, 2, 1, 1);
	pr_debug("USB clocks enabled\n");

	/* perform TLL soft reset, and wait until reset is complete */
	writel(OMAP_USBTLL_SYSCONFIG_SOFTRESET,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);
	/* Wait for TLL reset to complete */
	while (!(readl(OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSSTATUS)
			& OMAP_USBTLL_SYSSTATUS_RESETDONE));
	pr_debug("TLL reset done\n");

	writel(OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
		OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
		OMAP_USBTLL_SYSCONFIG_CACTIVITY,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);

	/* Put UHH in NoIdle/NoStandby mode */
	writel(OMAP_UHH_SYSCONFIG_ENAWAKEUP
		| OMAP_UHH_SYSCONFIG_SIDLEMODE
		| OMAP_UHH_SYSCONFIG_CACTIVITY
		| OMAP_UHH_SYSCONFIG_MIDLEMODE,
		OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);

	/* setup burst configurations */
	writel(OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN,
		OMAP3_UHH_BASE + OMAP_UHH_HOSTCONFIG);

	/*
	 * Refer ISSUE1:
	 * Hold the PHY in RESET for enough time till
	 * PHY is settled and ready
	 */
	udelay(10);
	omap_set_gpio_dataout(GPIO_PHY_RESET, 1);

	hccr = (struct ehci_hccr *)(OMAP3_EHCI_BASE);
	hcor = (struct ehci_hcor *)(OMAP3_EHCI_BASE + 0x10);

	pr_debug("OMAP3 EHCI init done\n");
	return 0;
}

#endif /* CONFIG_USB_EHCI */

/*
 * This command returns the status of the user button on beagle xM
 * Input - none
 * Returns - 	1 if button is held down
 *		0 if button is not held down
 */
int do_userbutton (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int     button = 0;
	int	gpio;

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	switch (get_board_revision()) {
	case REVISION_AXBX:
	case REVISION_CX:
	case REVISION_C4:
		gpio = 7;
		break;
	case REVISION_XM_A:
	case REVISION_XM_B:
	case REVISION_XM_C:
	default:
		gpio = 4;
		break;
	}
	omap_request_gpio(gpio);
	omap_set_gpio_direction(gpio, 1);
	printf("The user button is currently ");
	if(omap_get_gpio_datain(gpio))
	{
		button = 1;
		printf("PRESSED.\n");
	}
	else
	{
		button = 0;
		printf("NOT pressed.\n");
	}

	omap_free_gpio(gpio);

	return !button;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	userbutton, CONFIG_SYS_MAXARGS, 1,	do_userbutton,
	"Return the status of the BeagleBoard USER button",
	""
);
