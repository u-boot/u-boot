/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
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
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <command.h>
#include <i2c.h>
#include <ppc440.h>
#include "du440.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];
extern ulong flash_get_size (ulong base, int banknum);

int usbhub_init(void);
int dvi_init(void);
int eeprom_write_enable (unsigned dev_addr, int state);
int board_revision(void);

static int du440_post_errors;

int board_early_init_f(void)
{
	u32 sdr0_cust0;
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	mtdcr(EBC0_CFGDATA, 0xb8400000);

	/*
	 * Setup the GPIO pins
	 */
	out_be32((void*)GPIO0_OR, 0x00000000 | CONFIG_SYS_GPIO0_EP_EEP);
	out_be32((void*)GPIO0_TCR, 0x0000001f | CONFIG_SYS_GPIO0_EP_EEP);
	out_be32((void*)GPIO0_OSRL, 0x50055400);
	out_be32((void*)GPIO0_OSRH, 0x55005000);
	out_be32((void*)GPIO0_TSRL, 0x50055400);
	out_be32((void*)GPIO0_TSRH, 0x55005000);
	out_be32((void*)GPIO0_ISR1L, 0x50000000);
	out_be32((void*)GPIO0_ISR1H, 0x00000000);
	out_be32((void*)GPIO0_ISR2L, 0x00000000);
	out_be32((void*)GPIO0_ISR2H, 0x00000000);
	out_be32((void*)GPIO0_ISR3L, 0x00000000);
	out_be32((void*)GPIO0_ISR3H, 0x00000000);

	out_be32((void*)GPIO1_OR, 0x00000000);
	out_be32((void*)GPIO1_TCR, 0xc2000000 |
		 CONFIG_SYS_GPIO1_IORSTN |
		 CONFIG_SYS_GPIO1_IORST2N |
		 CONFIG_SYS_GPIO1_LEDUSR1 |
		 CONFIG_SYS_GPIO1_LEDUSR2 |
		 CONFIG_SYS_GPIO1_LEDPOST |
		 CONFIG_SYS_GPIO1_LEDDU);
	out_be32((void*)GPIO1_ODR, CONFIG_SYS_GPIO1_LEDDU);
	out_be32((void*)GPIO1_OSRL, 0x0c280000);
	out_be32((void*)GPIO1_OSRH, 0x00000000);
	out_be32((void*)GPIO1_TSRL, 0xcc000000);
	out_be32((void*)GPIO1_TSRH, 0x00000000);
	out_be32((void*)GPIO1_ISR1L, 0x00005550);
	out_be32((void*)GPIO1_ISR1H, 0x00000000);
	out_be32((void*)GPIO1_ISR2L, 0x00050000);
	out_be32((void*)GPIO1_ISR2H, 0x00000000);
	out_be32((void*)GPIO1_ISR3L, 0x01400000);
	out_be32((void*)GPIO1_ISR3H, 0x00000000);

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffff7ff);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	/*
	 * UIC1:
	 *  bit30: ext. Irq 1: PLD : int 32+30
	 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xfffffffd);
	mtdcr(UIC1TR, 0x00000000);
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	/*
	 * UIC2
	 *  bit3: ext. Irq 2: DCF77 : int 64+3
	 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC2TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

	/* select Ethernet pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_PFC2, sdr0_pfc2);

	/* setup EMAC bridge interface */
	if (board_revision() == 0) {
		/* 1 x MII */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
			SDR0_PFC1_SELECT_CONFIG_1_2;
		sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
			SDR0_PFC2_SELECT_CONFIG_1_2;
	} else {
		/* 2 x SMII */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
			SDR0_PFC1_SELECT_CONFIG_6;
		sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
			SDR0_PFC2_SELECT_CONFIG_6;
	}

	/* enable 2nd IIC */
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL;

	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	/* PCI arbiter enabled */
	mfsdr(SDR0_PCI0, reg);
	mtsdr(SDR0_PCI0, 0x80000000 | reg);

	/* setup NAND FLASH */
	mfsdr(SDR0_CUST0, sdr0_cust0);
	sdr0_cust0 = SDR0_CUST0_MUX_NDFC_SEL	|
		SDR0_CUST0_NDFC_ENABLE		|
		SDR0_CUST0_NDFC_BW_8_BIT	|
		SDR0_CUST0_NDFC_ARE_MASK	|
		(0x80000000 >> (28 + CONFIG_SYS_NAND0_CS)) |
		(0x80000000 >> (28 + CONFIG_SYS_NAND1_CS));
	mtsdr(SDR0_CUST0, sdr0_cust0);

	return 0;
}

int misc_init_r(void)
{
	uint pbcr;
	int size_val = 0;
	u32 reg;
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;
	unsigned long sdr0_srst0, sdr0_srst1;
	int i, j;

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	mtdcr(EBC0_CFGADDR, PB0CR);
	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(gd->bd->bi_flashsize) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtdcr(EBC0_CFGADDR, PB0CR);
	mtdcr(EBC0_CFGDATA, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

	/*
	 * USB suff...
	 */
	/* SDR Setting */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_USB0, usb2d0cr);
	mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mfsdr(SDR0_USB2H0CR, usb2h0cr);

	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;

	/* An 8-bit/60MHz interface is the only possible alternative
	   when connecting the Device to the PHY */
	usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
	usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;

	/* To enable the USB 2.0 Device function through the UTMI interface */
	usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;

	sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
	sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

	mtsdr(SDR0_PFC1, sdr0_pfc1);
	mtsdr(SDR0_USB0, usb2d0cr);
	mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mtsdr(SDR0_USB2H0CR, usb2h0cr);

	/*
	 * Take USB out of reset:
	 * -Initial status = all cores are in reset
	 * -deassert reset to OPB1, P4OPB0, OPB2, PLB42OPB1 OPB2PLB40 cores
	 * -wait 1 ms
	 * -deassert reset to PHY
	 * -wait 1 ms
	 * -deassert  reset to HOST
	 * -wait 4 ms
	 * -deassert all other resets
	 */
	mfsdr(SDR0_SRST1, sdr0_srst1);
	sdr0_srst1 &= ~(SDR0_SRST1_OPBA1 |		\
			SDR0_SRST1_P4OPB0 |		\
			SDR0_SRST1_OPBA2 |		\
			SDR0_SRST1_PLB42OPB1 |		\
			SDR0_SRST1_OPB2PLB40);
	mtsdr(SDR0_SRST1, sdr0_srst1);
	udelay(1000);

	mfsdr(SDR0_SRST1, sdr0_srst1);
	sdr0_srst1 &= ~SDR0_SRST1_USB20PHY;
	mtsdr(SDR0_SRST1, sdr0_srst1);
	udelay(1000);

	mfsdr(SDR0_SRST0, sdr0_srst0);
	sdr0_srst0 &= ~SDR0_SRST0_USB2H;
	mtsdr(SDR0_SRST0, sdr0_srst0);
	udelay(4000);

	/* finally all the other resets */
	mtsdr(SDR0_SRST1, 0x00000000);
	mtsdr(SDR0_SRST0, 0x00000000);

	printf("USB:   Host(int phy)\n");

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(PLB4_ACR) & ~PLB4_ACR_WRP;
	mtdcr(PLB4_ACR, reg);

	/*
	 * release IO-RST#
	 * We have to wait at least 560ms until we may call usbhub_init
	 */
	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) |
		 CONFIG_SYS_GPIO1_IORSTN | CONFIG_SYS_GPIO1_IORST2N);

	/*
	 * flash USR1/2 LEDs (600ms)
	 * This results in the necessary delay from IORST# until
	 * calling usbhub_init will succeed
	 */
	for (j = 0; j < 3; j++) {
		out_be32((void*)GPIO1_OR,
			 (in_be32((void*)GPIO1_OR) & ~CONFIG_SYS_GPIO1_LEDUSR2) |
			 CONFIG_SYS_GPIO1_LEDUSR1);

		for (i = 0; i < 100; i++)
			udelay(1000);

		out_be32((void*)GPIO1_OR,
			 (in_be32((void*)GPIO1_OR) & ~CONFIG_SYS_GPIO1_LEDUSR1) |
			 CONFIG_SYS_GPIO1_LEDUSR2);

		for (i = 0; i < 100; i++)
			udelay(1000);
	}

	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) &
		 ~(CONFIG_SYS_GPIO1_LEDUSR1 | CONFIG_SYS_GPIO1_LEDUSR2));

	if (usbhub_init())
		du440_post_errors++;

	if (dvi_init())
		du440_post_errors++;

	return 0;
}

int pld_revision(void)
{
	out_8((void *)CONFIG_SYS_CPLD_BASE, 0x00);
	return (int)(in_8((void *)CONFIG_SYS_CPLD_BASE) & CPLD_VERSION_MASK);
}

int board_revision(void)
{
	int rpins = (int)((in_be32((void*)GPIO1_IR) & CONFIG_SYS_GPIO1_HWVER_MASK)
			  >> CONFIG_SYS_GPIO1_HWVER_SHIFT);

	return ((rpins & 1) << 3) | ((rpins & 2) << 1) |
		((rpins & 4) >> 1) | ((rpins & 8) >> 3);
}

#if defined(CONFIG_SHOW_ACTIVITY)
void board_show_activity (ulong timestamp)
{
	if ((timestamp % 100) == 0)
		out_be32((void*)GPIO1_OR,
			 in_be32((void*)GPIO1_OR) ^ CONFIG_SYS_GPIO1_LEDUSR1);
}

void show_activity(int arg)
{
}
#endif /* CONFIG_SHOW_ACTIVITY */

int du440_phy_addr(int devnum)
{
	if (board_revision() == 0)
		return devnum;

	return devnum + 1;
}

int checkboard(void)
{
	char serno[32];

	puts("Board: DU440");

	if (getenv_r("serial#", serno, sizeof(serno)) > 0) {
		puts(", serial# ");
		puts(serno);
	}

	printf(", HW-Rev. 1.%d, CPLD-Rev. 1.%d\n",
	       board_revision(), pld_revision());
	return (0);
}

int last_stage_init(void)
{
	int e, i;

	/* everyting is ok: turn on POST-LED */
	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | CONFIG_SYS_GPIO1_LEDPOST);

	/* slowly blink on errors and finally keep LED off */
	for (e = 0; e < du440_post_errors; e++) {
		out_be32((void*)GPIO1_OR,
			 in_be32((void*)GPIO1_OR) | CONFIG_SYS_GPIO1_LEDPOST);

		for (i = 0; i < 500; i++)
			udelay(1000);

		out_be32((void*)GPIO1_OR,
			 in_be32((void*)GPIO1_OR) & ~CONFIG_SYS_GPIO1_LEDPOST);

		for (i = 0; i < 500; i++)
			udelay(1000);
	}

	return 0;
}

#if defined(CONFIG_I2C_MULTI_BUS)
/*
 * read field strength from I2C ADC
 */
int dcf77_status(void)
{
	unsigned int oldbus;
	uchar u[2];
	int mv;

	oldbus = I2C_GET_BUS();
	I2C_SET_BUS(1);

	if (i2c_read (IIC1_MCP3021_ADDR, 0, 0, u, 2)) {
		I2C_SET_BUS(oldbus);
		return -1;
	}

	mv = (int)(((u[0] << 8) | u[1]) >> 2) * 3300 / 1024;

	I2C_SET_BUS(oldbus);
	return mv;
}

int do_dcf77(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int mv;
	u32 pin, pinold;
	unsigned long long t1, t2;
	bd_t *bd = gd->bd;

	printf("DCF77: ");
	mv = dcf77_status();
	if (mv > 0)
		printf("signal=%d mV\n", mv);
	else
		printf("ERROR - no signal\n");

	t1 = t2 = 0;
	pinold = in_be32((void*)GPIO1_IR) & CONFIG_SYS_GPIO1_DCF77;
	while (!ctrlc()) {
		pin = in_be32((void*)GPIO1_IR) & CONFIG_SYS_GPIO1_DCF77;
		if (pin && !pinold) { /* bit start */
			t1 = get_ticks();
			if (t2 && ((unsigned int)(t1 - t2) /
				   (bd->bi_procfreq / 1000) >= 1800))
				printf("Start of minute\n");

			t2 = t1;
		}
		if (t1 && !pin && pinold) { /* bit end */
			printf("%5d\n", (unsigned int)(get_ticks() - t1) /
			       (bd->bi_procfreq / 1000));
		}
		pinold = pin;
	}

	printf("Abort\n");
	return 0;
}
U_BOOT_CMD(
	dcf77, 1, 1, do_dcf77,
	"Check DCF77 receiver",
	""
);

/*
 * initialize USB hub via I2C1
 */
int usbhub_init(void)
{
	int reg;
	int ret = 0;
	unsigned int oldbus;
	uchar u[] = {0x04, 0x24, 0x04, 0x07, 0x25, 0x00, 0x00, 0xd3,
		     0x18, 0xe0, 0x00, 0x00, 0x01, 0x64, 0x01, 0x64,
		     0x32};
	uchar stcd;

	printf("Hub:   ");

	oldbus = I2C_GET_BUS();
	I2C_SET_BUS(1);

	for (reg = 0; reg < sizeof(u); reg++)
		if (i2c_write (IIC1_USB2507_ADDR, reg, 1, &u[reg], 1)) {
			ret = -1;
			break;
		}

	if (ret == 0) {
		stcd = 0x03;
		if (i2c_write (IIC1_USB2507_ADDR, 0, 1, &stcd, 1))
			ret = -1;
	}

	if (ret == 0)
		printf("initialized\n");
	else
		printf("failed - cannot initialize USB hub\n");

	I2C_SET_BUS(oldbus);
	return ret;
}

int do_hubinit(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	usbhub_init();
	return 0;
}
U_BOOT_CMD(
	hubinit, 1, 1, do_hubinit,
	"Initialize USB hub",
	""
);
#endif /* CONFIG_I2C_MULTI_BUS */

#define CONFIG_SYS_BOOT_EEPROM_PAGE_WRITE_BITS 3
int boot_eeprom_write (unsigned dev_addr,
		       unsigned offset,
		       uchar *buffer,
		       unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable(dev_addr, 1);
#endif
	/*
	 * Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */

	while (offset < end) {
		unsigned alen, len;
		unsigned maxlen;

		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen = 2;
		addr[0] |= dev_addr;		/* insert device address */

		len = end - offset;

		/*
		 * For a FRAM device there is no limit on the number of the
		 * bytes that can be ccessed with the single read or write
		 * operation.
		 */
#if defined(CONFIG_SYS_BOOT_EEPROM_PAGE_WRITE_BITS)

#define	BOOT_EEPROM_PAGE_SIZE (1 << CONFIG_SYS_BOOT_EEPROM_PAGE_WRITE_BITS)
#define BOOT_EEPROM_PAGE_OFFSET(x) ((x) & (BOOT_EEPROM_PAGE_SIZE - 1))

		maxlen = BOOT_EEPROM_PAGE_SIZE -
			BOOT_EEPROM_PAGE_OFFSET(blk_off);
#else
		maxlen = 0x100 - blk_off;
#endif
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;

		if (len > maxlen)
			len = maxlen;

		if (i2c_write (addr[0], offset, alen - 1, buffer, len) != 0)
			rcode = 1;

		buffer += len;
		offset += len;

#if defined(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS)
		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}
#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable(dev_addr, 0);
#endif
	return rcode;
}

int do_setup_boot_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong sdsdp[4];

	if (argc > 1) {
		if (!strcmp(argv[1], "533")) {
			printf("Bootstrapping for 533MHz\n");
			sdsdp[0] = 0x87788252;
			/* PLB-PCI-divider = 3 : sync PCI clock=44MHz */
			sdsdp[1] = 0x095fa030;
			sdsdp[2] = 0x40082350;
			sdsdp[3] = 0x0d050000;
		} else if (!strcmp(argv[1], "533-66")) {
			printf("Bootstrapping for 533MHz (66MHz PCI)\n");
			sdsdp[0] = 0x87788252;
			/* PLB-PCI-divider = 2 : sync PCI clock=66MHz */
			sdsdp[1] = 0x0957a030;
			sdsdp[2] = 0x40082350;
			sdsdp[3] = 0x0d050000;
		} else if (!strcmp(argv[1], "667")) {
			printf("Bootstrapping for 667MHz\n");
			sdsdp[0] = 0x8778a256;
			/* PLB-PCI-divider = 4 : sync PCI clock=33MHz */
			sdsdp[1] = 0x0947a030;
			/* PLB-PCI-divider = 3 : sync PCI clock=44MHz
			 * -> not working when overclocking 533MHz chips
			 * -> untested on 667MHz chips */
			/* sdsdp[1]=0x095fa030; */
			sdsdp[2] = 0x40082350;
			sdsdp[3] = 0x0d050000;
		} else if (!strcmp(argv[1], "667-166")) {
			printf("Bootstrapping for 667-166MHz\n");
			sdsdp[0] = 0x8778a252;
			sdsdp[1] = 0x09d7a030;
			sdsdp[2] = 0x40082350;
			sdsdp[3] = 0x0d050000;
		}
	} else {
		printf("Bootstrapping for 533MHz (default)\n");
		sdsdp[0] = 0x87788252;
		/* PLB-PCI-divider = 3 : sync PCI clock=44MHz */
		sdsdp[1] = 0x095fa030;
		sdsdp[2] = 0x40082350;
		sdsdp[3] = 0x0d050000;
	}

	printf("Writing boot EEPROM ...\n");
	if (boot_eeprom_write(CONFIG_SYS_I2C_BOOT_EEPROM_ADDR,
			      0, (uchar*)sdsdp, 16) != 0)
		printf("boot_eeprom_write failed\n");
	else
		printf("done (dump via 'i2c md 52 0.1 10')\n");

	return 0;
}
U_BOOT_CMD(
	sbe, 2, 0, do_setup_boot_eeprom,
	"setup boot eeprom",
	""
);

#if defined(CONFIG_SYS_EEPROM_WREN)
/*
 * Input: <dev_addr>  I2C address of EEPROM device to enable.
 *         <state>     -1: deliver current state
 *                      0: disable write
 *                      1: enable write
 * Returns:            -1: wrong device address
 *                      0: dis-/en- able done
 *                    0/1: current state if <state> was -1.
 */
int eeprom_write_enable (unsigned dev_addr, int state)
{
	if ((CONFIG_SYS_I2C_EEPROM_ADDR != dev_addr) &&
	    (CONFIG_SYS_I2C_BOOT_EEPROM_ADDR != dev_addr))
		return -1;
	else {
		switch (state) {
		case 1:
			/* Enable write access, clear bit GPIO_SINT2. */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) & ~CONFIG_SYS_GPIO0_EP_EEP);
			state = 0;
			break;
		case 0:
			/* Disable write access, set bit GPIO_SINT2. */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) | CONFIG_SYS_GPIO0_EP_EEP);
			state = 0;
			break;
		default:
			/* Read current status back. */
			state = (0 == (in_be32((void*)GPIO0_OR) &
				       CONFIG_SYS_GPIO0_EP_EEP));
			break;
		}
	}
	return state;
}

int do_eep_wren (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int query = argc == 1;
	int state = 0;

	if (query) {
		/* Query write access state. */
		state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, -1);
		if (state < 0)
			puts ("Query of write access state failed.\n");
		else {
			printf ("Write access for device 0x%0x is %sabled.\n",
				CONFIG_SYS_I2C_EEPROM_ADDR, state ? "en" : "dis");
			state = 0;
		}
	} else {
		if ('0' == argv[1][0]) {
			/* Disable write access. */
			state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, 0);
		} else {
			/* Enable write access. */
			state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, 1);
		}
		if (state < 0)
			puts ("Setup of write access state failed.\n");
	}

	return state;
}

U_BOOT_CMD(eepwren, 2, 0, do_eep_wren,
	"Enable / disable / query EEPROM write access",
	""
);
#endif /* #if defined(CONFIG_SYS_EEPROM_WREN) */

static int got_pldirq;

static int pld_interrupt(u32 arg)
{
	int rc = -1; /* not for us */
	u8 status = in_8((void *)CONFIG_SYS_CPLD_BASE);

	/* check for PLD interrupt */
	if (status & PWR_INT_FLAG) {
		/* reset this int */
		out_8((void *)CONFIG_SYS_CPLD_BASE, 0);
		rc = 0;
		got_pldirq = 1; /* trigger backend */
	}

	return rc;
}

int do_waitpwrirq(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	got_pldirq = 0;

	/* clear any pending interrupt */
	out_8((void *)CONFIG_SYS_CPLD_BASE, 0);

	irq_install_handler(CPLD_IRQ,
			    (interrupt_handler_t *)pld_interrupt, 0);

	printf("Waiting ...\n");
	while(!got_pldirq) {
		/* Abort if ctrl-c was pressed */
		if (ctrlc()) {
			puts("\nAbort\n");
			break;
		}
	}
	if (got_pldirq) {
		printf("Got interrupt!\n");
		printf("Power %sready!\n",
		       in_8((void *)CONFIG_SYS_CPLD_BASE) &
		       PWR_RDY ? "":"NOT ");
	}

	irq_free_handler(CPLD_IRQ);
	return 0;
}
U_BOOT_CMD(
	wpi,	1,	1,	do_waitpwrirq,
	"Wait for power change interrupt",
	""
);

/*
 * initialize DVI panellink transmitter
 */
int dvi_init(void)
{
	int i;
	int ret = 0;
	unsigned int oldbus;
	uchar u[] = {0x08, 0x34,
		     0x09, 0x20,
		     0x0a, 0x90,
		     0x0c, 0x89,
		     0x08, 0x35};

	printf("DVI:   ");

	oldbus = I2C_GET_BUS();
	I2C_SET_BUS(0);

	for (i = 0; i < sizeof(u); i += 2)
		if (i2c_write (0x38, u[i], 1, &u[i + 1], 1)) {
			ret = -1;
			break;
		}

	if (ret == 0)
		printf("initialized\n");
	else
		printf("failed - cannot initialize DVI transmitter\n");

	I2C_SET_BUS(oldbus);
	return ret;
}

int do_dviinit(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	dvi_init();
	return 0;
}
U_BOOT_CMD(
	dviinit, 1, 1, do_dviinit,
	"Initialize DVI Panellink transmitter",
	""
);

/*
 * TODO: 'time' command might be useful for others as well.
 *       Move to 'common' directory.
 */
int do_time(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long long start, end;
	char c, cmd[CONFIG_SYS_CBSIZE];
	char *p, *d = cmd;
	int ret, i;
	ulong us;

	for (i = 1; i < argc; i++) {
		p = argv[i];

		if (i > 1)
			*d++ = ' ';

		while ((c = *p++) != '\0') {
			*d++ = c;
		}
	}
	*d = '\0';

	start = get_ticks();
	ret = run_command (cmd, 0);
	end = get_ticks();

	printf("ticks=%ld\n", (ulong)(end - start));
	us = (ulong)((1000L * (end - start)) / (get_tbclk() / 1000));
	printf("usec=%ld\n", us);

	return ret;
}
U_BOOT_CMD(
	time,	CONFIG_SYS_MAXARGS,	1,	do_time,
	"run command and output execution time",
	""
);

extern void video_hw_rectfill (
	unsigned int bpp,		/* bytes per pixel */
	unsigned int dst_x,		/* dest pos x */
	unsigned int dst_y,		/* dest pos y */
	unsigned int dim_x,		/* frame width */
	unsigned int dim_y,		/* frame height */
	unsigned int color		/* fill color */
	);

/*
 * graphics demo
 * draw rectangles using pseudorandom number generator
 * (see http://www.embedded.com/columns/technicalinsights/20900500)
 */
unsigned int rprime = 9972;
static unsigned int r;
static unsigned int Y;

unsigned int prng(unsigned int max)
{
	if (r == 0 || r == 1 || r == -1)
		r = rprime; /* keep from getting stuck */

	r = (9973 * ~r) + ((Y) % 701); /* the actual algorithm */
	Y = (r >> 16) % max; /* choose upper bits and reduce */
	return Y;
}

int do_gfxdemo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int color;
	unsigned int x, y, dx, dy;

	while (!ctrlc()) {
		x = prng(1280 - 1);
		y = prng(1024 - 1);
		dx = prng(1280- x - 1);
		dy = prng(1024 - y - 1);
		color = prng(0x10000);
		video_hw_rectfill(2, x, y, dx, dy, color);
	}

	return 0;
}
U_BOOT_CMD(
	gfxdemo,	CONFIG_SYS_MAXARGS,	1,	do_gfxdemo,
	"demo",
	""
);
