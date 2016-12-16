/*
 * (C) Copyright 2007-2013
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/ppc440.h>
#include <asm/processor.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/io.h>
#include <post.h>
#include <flash.h>
#include <video.h>
#include <mtd/cfi_flash.h>

DECLARE_GLOBAL_DATA_PTR;

static phys_addr_t lwmon5_cfi_flash_bank_addr[2] = CONFIG_SYS_FLASH_BANKS_LIST;

ulong flash_get_size(ulong base, int banknum);
int misc_init_r_kbd(void);

int board_early_init_f(void)
{
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	/* PLB Write pipelining disabled. Denali Core workaround */
	mtdcr(PLB4A0_ACR, 0xDE000000);
	mtdcr(PLB4A1_ACR, 0xDE000000);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(UIC0SR, 0xffffffff);  /* clear all. if write with 1 then the status is cleared  */
	mtdcr(UIC0ER, 0x00000000);  /* disable all */
	mtdcr(UIC0CR, 0x00000000);  /* we have not critical interrupts at the moment */
	mtdcr(UIC0PR, 0xFFBFF1EF);  /* Adjustment of the polarity */
	mtdcr(UIC0TR, 0x00000900);  /* per ref-board manual */
	mtdcr(UIC0VR, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(UIC0SR, 0xffffffff);  /* clear all */

	mtdcr(UIC1SR, 0xffffffff);  /* clear all */
	mtdcr(UIC1ER, 0x00000000);  /* disable all */
	mtdcr(UIC1CR, 0x00000000);  /* all non-critical */
	mtdcr(UIC1PR, 0xFFFFC6A5);  /* Adjustment of the polarity */
	mtdcr(UIC1TR, 0x60000040);  /* per ref-board manual */
	mtdcr(UIC1VR, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(UIC1SR, 0xffffffff);  /* clear all */

	mtdcr(UIC2SR, 0xffffffff);  /* clear all */
	mtdcr(UIC2ER, 0x00000000);  /* disable all */
	mtdcr(UIC2CR, 0x00000000);  /* all non-critical */
	mtdcr(UIC2PR, 0x27C00000);  /* Adjustment of the polarity */
	mtdcr(UIC2TR, 0x3C000000);  /* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(UIC2SR, 0xffffffff);  /* clear all */

	/* Trace Pins are disabled. SDR0_PFC0 Register */
	mtsdr(SDR0_PFC0, 0x0);

	/* select Ethernet pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	/* SMII via ZMII */
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
		SDR0_PFC1_SELECT_CONFIG_6;
	mfsdr(SDR0_PFC2, sdr0_pfc2);
	sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
		SDR0_PFC2_SELECT_CONFIG_6;

	/* enable SPI (SCP) */
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_SCP_SEL;

	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	mtsdr(SDR0_PFC4, 0x80000000);

	/* PCI arbiter disabled */
	/* PCI Host Configuration disbaled */
	mfsdr(SDR0_PCI0, reg);
	reg = 0;
	mtsdr(SDR0_PCI0, 0x00000000 | reg);

	gpio_write_bit(CONFIG_SYS_GPIO_FLASH_WP, 1);

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC1
	/* enable the LSB transmitter */
	gpio_write_bit(CONFIG_SYS_GPIO_LSB_ENABLE, 1);
	/* enable the CAN transmitter */
	gpio_write_bit(CONFIG_SYS_GPIO_CAN_ENABLE, 1);

	reg = 0; /* reuse as counter */
	out_be32((void *)CONFIG_SYS_DSPIC_TEST_ADDR,
		in_be32((void *)CONFIG_SYS_DSPIC_TEST_ADDR)
			& ~CONFIG_SYS_DSPIC_TEST_MASK);
	while (gpio_read_in_bit(CONFIG_SYS_GPIO_DSPIC_READY) && reg++ < 1000) {
		udelay(1000);
	}
	if (gpio_read_in_bit(CONFIG_SYS_GPIO_DSPIC_READY)) {
		/* set "boot error" flag */
		out_be32((void *)CONFIG_SYS_DSPIC_TEST_ADDR,
			in_be32((void *)CONFIG_SYS_DSPIC_TEST_ADDR) |
			CONFIG_SYS_DSPIC_TEST_MASK);
	}
#endif

	/*
	 * Reset PHY's:
	 * The PHY's need a 2nd reset pulse, since the MDIO address is latched
	 * upon reset, and with the first reset upon powerup, the addresses are
	 * not latched reliable, since the IRQ line is multiplexed with an
	 * MDIO address. A 2nd reset at this time will make sure, that the
	 * correct address is latched.
	 */
	gpio_write_bit(CONFIG_SYS_GPIO_PHY0_RST, 1);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY1_RST, 1);
	udelay(1000);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY0_RST, 0);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY1_RST, 0);
	udelay(1000);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY0_RST, 1);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY1_RST, 1);

	return 0;
}

/*
 * Override weak default with board specific version
 */
phys_addr_t cfi_flash_bank_addr(int bank)
{
	return lwmon5_cfi_flash_bank_addr[bank];
}

/*
 * Override the weak default mapping function with a board specific one
 */
u32 flash_get_bank_size(int cs, int idx)
{
	return flash_info[idx].size;
}

int board_early_init_r(void)
{
	u32 val0, val1;

	/*
	 * lwmon5 is manufactured in 2 different board versions:
	 * The lwmon5a board has 64MiB NOR flash instead of the
	 * 128MiB of the original lwmon5. Unfortunately the CFI driver
	 * will report 2 banks of 64MiB even for the smaller flash
	 * chip, since the bank is mirrored. To fix this, we bring
	 * one bank into CFI query mode and read its response. This
	 * enables us to detect the real number of flash devices/
	 * banks which will be used later on by the common CFI driver.
	 */

	/* Put bank 0 into CFI command mode and read */
	out_be32((void *)CONFIG_SYS_FLASH0, 0x00980098);
	val0 = in_be32((void *)CONFIG_SYS_FLASH0 + FLASH_OFFSET_CFI_RESP);
	val1 = in_be32((void *)CONFIG_SYS_FLASH1 + FLASH_OFFSET_CFI_RESP);

	/* Reset flash again out of query mode */
	out_be32((void *)CONFIG_SYS_FLASH0, 0x00f000f0);

	/* When not identical, we have 2 different flash devices/banks */
	if (val0 != val1)
		return 0;

	/*
	 * Now we're sure that we're running on a LWMON5a board with
	 * only 64MiB NOR flash in one bank:
	 *
	 * Set flash base address and bank count for CFI driver probing.
	 */
	cfi_flash_num_flash_banks = 1;
	lwmon5_cfi_flash_bank_addr[0] = CONFIG_SYS_FLASH0;

	return 0;
}

int misc_init_r(void)
{
	u32 pbcr;
	int size_val = 0;
	u32 reg;
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1, sdr0_srst;

	/*
	 * FLASH stuff...
	 */

	/* Re-do sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	mfebc(PB0CR, pbcr);
	size_val = ffs(gd->bd->bi_flashsize) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtebc(PB0CR, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

	/* Monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET, -CONFIG_SYS_MONITOR_LEN, 0xffffffff,
		      &flash_info[cfi_flash_num_flash_banks - 1]);

	/* Env protection ON by default */
	flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
		      CONFIG_ENV_ADDR_REDUND + 2 * CONFIG_ENV_SECT_SIZE - 1,
		      &flash_info[cfi_flash_num_flash_banks - 1]);

	/*
	 * USB suff...
	 */

	/* Reset USB */
	/* Reset of USB2PHY0 must be active at least 10 us  */
	mtsdr(SDR0_SRST0, SDR0_SRST0_USB2H | SDR0_SRST0_USB2D);
	udelay(2000);

	mtsdr(SDR0_SRST1, SDR0_SRST1_USB20PHY | SDR0_SRST1_USB2HUTMI |
	      SDR0_SRST1_USB2HPHY | SDR0_SRST1_OPBA2 |
	      SDR0_SRST1_PLB42OPB1 | SDR0_SRST1_OPB2PLB40);
	udelay(2000);

	/* Errata CHIP_6 */

	/* 1. Set internal PHY configuration */
	/* SDR Setting */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_USB0, usb2d0cr);
	mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mfsdr(SDR0_USB2H0CR, usb2h0cr);

	usb2phy0cr = usb2phy0cr & ~SDR0_USB2PHY0CR_XOCLK_MASK;
	usb2phy0cr = usb2phy0cr |  SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
	usb2phy0cr = usb2phy0cr & ~SDR0_USB2PHY0CR_WDINT_MASK;
	usb2phy0cr = usb2phy0cr |  SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;	/*1*/
	usb2phy0cr = usb2phy0cr & ~SDR0_USB2PHY0CR_DVBUS_MASK;
	usb2phy0cr = usb2phy0cr |  SDR0_USB2PHY0CR_DVBUS_PUREN;		/*1*/
	usb2phy0cr = usb2phy0cr & ~SDR0_USB2PHY0CR_DWNSTR_MASK;
	usb2phy0cr = usb2phy0cr |  SDR0_USB2PHY0CR_DWNSTR_HOST;		/*1*/
	usb2phy0cr = usb2phy0cr & ~SDR0_USB2PHY0CR_UTMICN_MASK;
	usb2phy0cr = usb2phy0cr |  SDR0_USB2PHY0CR_UTMICN_HOST;		/*1*/

	/*
	 * An 8-bit/60MHz interface is the only possible alternative
	 * when connecting the Device to the PHY
	 */
	usb2h0cr   = usb2h0cr & ~SDR0_USB2H0CR_WDINT_MASK;
	usb2h0cr   = usb2h0cr |  SDR0_USB2H0CR_WDINT_16BIT_30MHZ;	/*1*/

	mtsdr(SDR0_PFC1, sdr0_pfc1);
	mtsdr(SDR0_USB0, usb2d0cr);
	mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mtsdr(SDR0_USB2H0CR, usb2h0cr);

	/* 2. De-assert internal PHY reset */
	mfsdr(SDR0_SRST1, sdr0_srst);
	sdr0_srst = sdr0_srst & ~SDR0_SRST1_USB20PHY;
	mtsdr(SDR0_SRST1, sdr0_srst);

	/* 3. Wait for more than 1 ms */
	udelay(2000);

	/* 4. De-assert USB 2.0 Host main reset */
	mfsdr(SDR0_SRST0, sdr0_srst);
	sdr0_srst = sdr0_srst &~ SDR0_SRST0_USB2H;
	mtsdr(SDR0_SRST0, sdr0_srst);
	udelay(1000);

	/* 5. De-assert reset of OPB2 cores */
	mfsdr(SDR0_SRST1, sdr0_srst);
	sdr0_srst = sdr0_srst &~ SDR0_SRST1_PLB42OPB1;
	sdr0_srst = sdr0_srst &~ SDR0_SRST1_OPB2PLB40;
	sdr0_srst = sdr0_srst &~ SDR0_SRST1_OPBA2;
	mtsdr(SDR0_SRST1, sdr0_srst);
	udelay(1000);

	/* 6. Set EHCI Configure FLAG */

	/* 7. Reassert internal PHY reset: */
	mtsdr(SDR0_SRST1, SDR0_SRST1_USB20PHY);
	udelay(1000);

	/*
	 * Clear resets
	 */
	mtsdr(SDR0_SRST1, 0x00000000);
	mtsdr(SDR0_SRST0, 0x00000000);

	printf("USB:   Host(int phy) Device(ext phy)\n");

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(PLB4A0_ACR) & ~PLB4Ax_ACR_WRP_MASK;
	mtdcr(PLB4A0_ACR, reg);

	/*
	 * Init matrix keyboard
	 */
	misc_init_r_kbd();

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: %s", __stringify(CONFIG_HOSTNAME));

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

void hw_watchdog_reset(void)
{
	int val;
#if defined(CONFIG_WD_MAX_RATE)
	unsigned long long ct = get_ticks();

	/*
	 * Don't allow watch-dog triggering more frequently than
	 * the predefined value CONFIG_WD_MAX_RATE [ticks].
	 */
	if (ct >= gd->arch.wdt_last) {
		if ((ct - gd->arch.wdt_last) < CONFIG_WD_MAX_RATE)
			return;
	} else {
		/* Time base counter had been reset */
		if (((unsigned long long)(-1) - gd->arch.wdt_last + ct) <
		    CONFIG_WD_MAX_RATE)
			return;
	}
	gd->arch.wdt_last = get_ticks();
#endif

	/*
	 * Toggle watchdog output
	 */
	val = gpio_read_out_bit(CONFIG_SYS_GPIO_WATCHDOG) == 0 ? 1 : 0;
	gpio_write_bit(CONFIG_SYS_GPIO_WATCHDOG, val);
}

int do_eeprom_wp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	if ((strcmp(argv[1], "on") == 0))
		gpio_write_bit(CONFIG_SYS_GPIO_EEPROM_EXT_WP, 1);
	else if ((strcmp(argv[1], "off") == 0))
		gpio_write_bit(CONFIG_SYS_GPIO_EEPROM_EXT_WP, 0);
	else
		return cmd_usage(cmdtp);

	return 0;
}

U_BOOT_CMD(
	eepromwp,	2,	0,	do_eeprom_wp,
	"eeprom write protect off/on",
	"<on|off> - enable (on) or disable (off) I2C EEPROM write protect"
);

#if defined(CONFIG_VIDEO)
#include <video_fb.h>
#include <mb862xx.h>

extern GraphicDevice mb862xx;

static const gdc_regs init_regs [] = {
	{ 0x0100, 0x00000f00 },
	{ 0x0020, 0x801401df },
	{ 0x0024, 0x00000000 },
	{ 0x0028, 0x00000000 },
	{ 0x002c, 0x00000000 },
	{ 0x0110, 0x00000000 },
	{ 0x0114, 0x00000000 },
	{ 0x0118, 0x01df0280 },
	{ 0x0004, 0x031f0000 },
	{ 0x0008, 0x027f027f },
	{ 0x000c, 0x015f028f },
	{ 0x0010, 0x020c0000 },
	{ 0x0014, 0x01df01ea },
	{ 0x0018, 0x00000000 },
	{ 0x001c, 0x01e00280 },
	{ 0x0100, 0x80010f00 },
	{ 0x0, 0x0 }
};

const gdc_regs *board_get_regs(void)
{
	return init_regs;
}

/* Returns Lime base address */
unsigned int board_video_init(void)
{
	/*
	 * Reset Lime controller
	 */
	gpio_write_bit(CONFIG_SYS_GPIO_LIME_S, 1);
	udelay(500);
	gpio_write_bit(CONFIG_SYS_GPIO_LIME_RST, 1);

	mb862xx.winSizeX = 640;
	mb862xx.winSizeY = 480;
	mb862xx.gdfBytesPP = 2;
	mb862xx.gdfIndex = GDF_15BIT_555RGB;

	return CONFIG_SYS_LIME_BASE_0;
}

#define DEFAULT_BRIGHTNESS	0x64

static void board_backlight_brightness(int brightness)
{
	if (brightness > 0) {
		/* pwm duty, lamp on */
		out_be32((void *)(CONFIG_SYS_FPGA_BASE_0 + 0x00000024), brightness);
		out_be32((void *)(CONFIG_SYS_FPGA_BASE_0 + 0x00000020), 0x701);
	} else {
		/* lamp off */
		out_be32((void *)(CONFIG_SYS_FPGA_BASE_0 + 0x00000024), 0x00);
		out_be32((void *)(CONFIG_SYS_FPGA_BASE_0 + 0x00000020), 0x00);
	}
}

void board_backlight_switch(int flag)
{
	char * param;
	int rc;

	if (flag) {
		param = getenv("brightness");
		rc = param ? simple_strtol(param, NULL, 10) : -1;
		if (rc < 0)
			rc = DEFAULT_BRIGHTNESS;
	} else {
		rc = 0;
	}
	board_backlight_brightness(rc);
}

#if defined(CONFIG_CONSOLE_EXTRA_INFO)
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str(int line_number, char *info)
{
	if (line_number == 1)
		strcpy(info, " Board: Lwmon5 (Liebherr Elektronik GmbH)");
	else
		info [0] = '\0';
}
#endif /* CONFIG_CONSOLE_EXTRA_INFO */
#endif /* CONFIG_VIDEO */

void board_reset(void)
{
	gpio_write_bit(CONFIG_SYS_GPIO_BOARD_RESET, 1);
}

#ifdef CONFIG_SPL_OS_BOOT
/*
 * lwmon5 specific implementation of spl_start_uboot()
 *
 * RETURN
 * 0 if booting into OS is selected (default)
 * 1 if booting into U-Boot is selected
 */
int spl_start_uboot(void)
{
	char s[8];

	env_init();
	getenv_f("boot_os", s, sizeof(s));
	if ((s != NULL) && (strcmp(s, "yes") == 0))
		return 0;

	return 1;
}

/*
 * This function is called from the SPL U-Boot version for
 * early init stuff, that needs to be done for OS (e.g. Linux)
 * booting. Doing it later in the real U-Boot would not work
 * in case that the SPL U-Boot boots Linux directly.
 */
void spl_board_init(void)
{
	const gdc_regs *regs = board_get_regs();

	/*
	 * Setup PFC registers, mainly for ethernet support
	 * later on in Linux
	 */
	board_early_init_f();

	/* enable the LSB transmitter */
	gpio_write_bit(CONFIG_SYS_GPIO_LSB_ENABLE, 1);

	/*
	 * Clear resets
	 */
	mtsdr(SDR0_SRST1, 0x00000000);
	mtsdr(SDR0_SRST0, 0x00000000);

	/*
	 * Reset Lime controller
	 */
	gpio_write_bit(CONFIG_SYS_GPIO_LIME_S, 1);
	udelay(500);
	gpio_write_bit(CONFIG_SYS_GPIO_LIME_RST, 1);

	out_be32((void *)CONFIG_SYS_LIME_SDRAM_CLOCK, CONFIG_SYS_MB862xx_CCF);
	udelay(300);
	out_be32((void *)CONFIG_SYS_LIME_MMR, CONFIG_SYS_MB862xx_MMR);

	while (regs->index) {
		out_be32((void *)(CONFIG_SYS_LIME_BASE_0 + GC_DISP_BASE) +
			 regs->index, regs->value);
		regs++;
	}

	board_backlight_brightness(DEFAULT_BRIGHTNESS);
}
#endif
