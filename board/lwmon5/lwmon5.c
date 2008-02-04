/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <command.h>
#include <ppc440.h>
#include <asm/processor.h>
#include <asm/gpio.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

ulong flash_get_size(ulong base, int banknum);
int misc_init_r_kbd(void);

int board_early_init_f(void)
{
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	/* PLB Write pipelining disabled. Denali Core workaround */
	mtdcr(plb0_acr, 0xDE000000);
	mtdcr(plb1_acr, 0xDE000000);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);  /* clear all. if write with 1 then the status is cleared  */
	mtdcr(uic0er, 0x00000000);  /* disable all */
	mtdcr(uic0cr, 0x00000000);  /* we have not critical interrupts at the moment */
	mtdcr(uic0pr, 0xFFBFF1EF);  /* Adjustment of the polarity */
	mtdcr(uic0tr, 0x00000900);  /* per ref-board manual */
	mtdcr(uic0vr, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(uic0sr, 0xffffffff);  /* clear all */

	mtdcr(uic1sr, 0xffffffff);  /* clear all */
	mtdcr(uic1er, 0x00000000);  /* disable all */
	mtdcr(uic1cr, 0x00000000);  /* all non-critical */
	mtdcr(uic1pr, 0xFFFFC6A5);  /* Adjustment of the polarity */
	mtdcr(uic1tr, 0x60000040);  /* per ref-board manual */
	mtdcr(uic1vr, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(uic1sr, 0xffffffff);  /* clear all */

	mtdcr(uic2sr, 0xffffffff);  /* clear all */
	mtdcr(uic2er, 0x00000000);  /* disable all */
	mtdcr(uic2cr, 0x00000000);  /* all non-critical */
	mtdcr(uic2pr, 0x27C00000);  /* Adjustment of the polarity */
	mtdcr(uic2tr, 0x3C000000);  /* per ref-board manual */
	mtdcr(uic2vr, 0x00000000);  /* int31 highest, base=0x000 is within DDRAM */
	mtdcr(uic2sr, 0xffffffff);  /* clear all */

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
	mfsdr(sdr_pci0, reg);
	reg = 0;
	mtsdr(sdr_pci0, 0x00000000 | reg);

	gpio_write_bit(CFG_GPIO_FLASH_WP, 1);

#if CONFIG_POST & CFG_POST_BSPEC1
	gpio_write_bit(CFG_GPIO_HIGHSIDE, 1);

	reg = 0; /* reuse as counter */
	out_be32((void *)CFG_DSPIC_TEST_ADDR,
		in_be32((void *)CFG_DSPIC_TEST_ADDR)
			& ~CFG_DSPIC_TEST_MASK);
	while (!gpio_read_in_bit(CFG_GPIO_DSPIC_READY) && reg++ < 1000) {
		udelay(1000);
	}
	gpio_write_bit(CFG_GPIO_HIGHSIDE, 0);
	if (gpio_read_in_bit(CFG_GPIO_DSPIC_READY)) {
		/* set "boot error" flag */
		out_be32((void *)CFG_DSPIC_TEST_ADDR,
			in_be32((void *)CFG_DSPIC_TEST_ADDR) |
			CFG_DSPIC_TEST_MASK);
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
	gpio_write_bit(CFG_GPIO_PHY0_RST, 1);
	gpio_write_bit(CFG_GPIO_PHY1_RST, 1);
	udelay(1000);
	gpio_write_bit(CFG_GPIO_PHY0_RST, 0);
	gpio_write_bit(CFG_GPIO_PHY1_RST, 0);
	udelay(1000);
	gpio_write_bit(CFG_GPIO_PHY0_RST, 1);
	gpio_write_bit(CFG_GPIO_PHY1_RST, 1);

	return 0;
}

/*---------------------------------------------------------------------------+
  | misc_init_r.
  +---------------------------------------------------------------------------*/
int misc_init_r(void)
{
	u32 pbcr;
	int size_val = 0;
	u32 reg;
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;

	/*
	 * FLASH stuff...
	 */

	/* Re-do sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	mfebc(pb0cr, pbcr);
	switch (gd->bd->bi_flashsize) {
	case 1 << 20:
		size_val = 0;
		break;
	case 2 << 20:
		size_val = 1;
		break;
	case 4 << 20:
		size_val = 2;
		break;
	case 8 << 20:
		size_val = 3;
		break;
	case 16 << 20:
		size_val = 4;
		break;
	case 32 << 20:
		size_val = 5;
		break;
	case 64 << 20:
		size_val = 6;
		break;
	case 128 << 20:
		size_val = 7;
		break;
	}
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtebc(pb0cr, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CFG_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[1]);

	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CFG_ENV_ADDR_REDUND,
			    CFG_ENV_ADDR_REDUND + 2*CFG_ENV_SECT_SIZE - 1,
			    &flash_info[1]);

	/*
	 * USB suff...
	 */
	/* SDR Setting */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_USB0, usb2d0cr);
	mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mfsdr(SDR0_USB2H0CR, usb2h0cr);

	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;	/*1*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;		/*0*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;		/*1*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;		/*1*/

	/* An 8-bit/60MHz interface is the only possible alternative
	   when connecting the Device to the PHY */
	usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
	usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;	/*1*/

	mtsdr(SDR0_PFC1, sdr0_pfc1);
	mtsdr(SDR0_USB0, usb2d0cr);
	mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mtsdr(SDR0_USB2H0CR, usb2h0cr);

	/*
	 * Clear resets
	 */
	udelay (1000);
	mtsdr(SDR0_SRST1, 0x00000000);
	udelay (1000);
	mtsdr(SDR0_SRST0, 0x00000000);

	printf("USB:   Host(int phy) Device(ext phy)\n");

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(plb4_acr) & ~PLB4_ACR_WRP;
	mtdcr(plb4_acr, reg);

	/*
	 * Init matrix keyboard
	 */
	misc_init_r_kbd();

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: lwmon5");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_MBYTES_SDRAM;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0) {
			printf("%3d MB\r", k / 1024);
		}

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");
	return 0;
}
#endif

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB3 devices to 0.
	  | Set PLB3 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	mtdcr(plb3_acr, addr | 0x80000000);

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB4 devices to 0.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(plb4_acr, addr);

	/*-------------------------------------------------------------------------+
	  | Set Nebula PLB4 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	/* Segment0 */
	addr = (mfdcr(plb0_acr) & ~plb0_acr_ppm_mask) | plb0_acr_ppm_fair;
	addr = (addr & ~plb0_acr_hbu_mask) | plb0_acr_hbu_enabled;
	addr = (addr & ~plb0_acr_rdp_mask) | plb0_acr_rdp_4deep;
	addr = (addr & ~plb0_acr_wrp_mask) | plb0_acr_wrp_2deep;
	mtdcr(plb0_acr, addr);

	/* Segment1 */
	addr = (mfdcr(plb1_acr) & ~plb1_acr_ppm_mask) | plb1_acr_ppm_fair;
	addr = (addr & ~plb1_acr_hbu_mask) | plb1_acr_hbu_enabled;
	addr = (addr & ~plb1_acr_rdp_mask) | plb1_acr_rdp_4deep;
	addr = (addr & ~plb1_acr_wrp_mask) | plb1_acr_wrp_2deep;
	mtdcr(plb1_acr, addr);

	return 1;
}
#endif	/* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Set up Direct MMIO registers
	 *--------------------------------------------------------------------------*/
	/*--------------------------------------------------------------------------+
	  | PowerPC440EPX PCI Master configuration.
	  | Map one 1Gig range of PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address 0xA0000000-0xDFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r(PCIX0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM0LA, CFG_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIX0_PMM0PCILA, CFG_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM0MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PMM1MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM1LA, CFG_PCI_MEMBASE2); /* PMM0 Local Address */
	out32r(PCIX0_PMM1PCILA, CFG_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM1MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM1LA, 0);	/* Local Addr. Reg */
	out32r(PCIX0_PTM2MS, 0);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM2LA, 0);	/* Local Addr. Reg */

	/*--------------------------------------------------------------------------+
	 * Set up Configuration registers
	 *--------------------------------------------------------------------------*/

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CFG_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*--------------------------------------------------------------------------+
	  | Write the PowerPC440 EP PCI Configuration regs.
	  |   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	  |   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	  +--------------------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT) */

/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	/* Cactus is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */

void hw_watchdog_reset(void)
{
	int val;

	/*
	 * Toggle watchdog output
	 */
	val = gpio_read_out_bit(CFG_GPIO_WATCHDOG) == 0 ? 1 : 0;
	gpio_write_bit(CFG_GPIO_WATCHDOG, val);
}

int do_eeprom_wp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((strcmp(argv[1], "on") == 0)) {
		gpio_write_bit(CFG_GPIO_EEPROM_EXT_WP, 1);
	} else if ((strcmp(argv[1], "off") == 0)) {
		gpio_write_bit(CFG_GPIO_EEPROM_EXT_WP, 0);
	} else {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}


	return 0;
}

U_BOOT_CMD(
	eepromwp,	2,	0,	do_eeprom_wp,
	"eepromwp- eeprom write protect off/on\n",
	"<on|off> - enable (on) or disable (off) I2C EEPROM write protect\n"
);

#if defined(CONFIG_VIDEO)
#include <video_fb.h>
#include <mb862xx.h>

extern GraphicDevice mb862xx;

static const gdc_regs init_regs [] =
{
	{0x0100, 0x00000f00},
	{0x0020, 0x801401df},
	{0x0024, 0x00000000},
	{0x0028, 0x00000000},
	{0x002c, 0x00000000},
	{0x0110, 0x00000000},
	{0x0114, 0x00000000},
	{0x0118, 0x01df0280},
	{0x0004, 0x031f0000},
	{0x0008, 0x027f027f},
	{0x000c, 0x015f028f},
	{0x0010, 0x020c0000},
	{0x0014, 0x01df01ea},
	{0x0018, 0x00000000},
	{0x001c, 0x01e00280},
	{0x0100, 0x80010f00},
	{0x0, 0x0}
};

const gdc_regs *board_get_regs (void)
{
	return init_regs;
}

/* Returns Lime base address */
unsigned int board_video_init (void)
{
	/*
	 * Reset Lime controller
	 */
	gpio_write_bit(CFG_GPIO_LIME_S, 1);
	udelay(500);
	gpio_write_bit(CFG_GPIO_LIME_RST, 1);

	/* Lime memory clock adjusted to 100MHz */
	out_be32((void *)CFG_LIME_SDRAM_CLOCK, CFG_LIME_CLOCK_100MHZ);
	/* Wait untill time expired. Because of requirements in lime manual */
	udelay(300);
	/* Write lime controller memory parameters */
	out_be32((void *)CFG_LIME_MMR, CFG_LIME_MMR_VALUE);

	mb862xx.winSizeX = 640;
	mb862xx.winSizeY = 480;
	mb862xx.gdfBytesPP = 2;
	mb862xx.gdfIndex = GDF_15BIT_555RGB;

	return CFG_LIME_BASE_0;
}

void board_backlight_switch (int flag)
{
	if (flag) {
		/* pwm duty, lamp on */
		out_be32((void *)(CFG_FPGA_BASE_0 + 0x00000024), 0x64);
		out_be32((void *)(CFG_FPGA_BASE_0 + 0x00000020), 0x701);
	} else {
		/* lamp off */
		out_be32((void *)(CFG_FPGA_BASE_0 + 0x00000024), 0x00);
		out_be32((void *)(CFG_FPGA_BASE_0 + 0x00000020), 0x00);
	}
}

#if defined(CONFIG_CONSOLE_EXTRA_INFO)
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str (int line_number, char *info)
{
	if (line_number == 1) {
		strcpy (info, " Board: Lwmon5 (Liebherr Elektronik GmbH)");
	} else {
		info [0] = '\0';
	}
}
#endif
#endif /* CONFIG_VIDEO */
