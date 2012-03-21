/*
 * (C) Copyright 2007-2010
 * Larry Johnson, lrj@acm.org
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,	    AMCC/IBM, alain.saurel@fr.ibm.com
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
#include <fdt_support.h>
#include <i2c.h>
#include <libfdt.h>
#include <asm/ppc440.h>
#include <asm/bitops.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/io.h>
#include <asm/ppc4xx-uic.h>
#include <asm/processor.h>
#include <asm/4xx_pci.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

ulong flash_get_size(ulong base, int banknum);

#if defined(CONFIG_KORAT_PERMANENT)
void korat_buzzer(int const on)
{
	if (on) {
		out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x05,
		      in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x05) | 0x80);
	} else {
		out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x05,
		      in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x05) & ~0x80);
	}
}
#endif

int board_early_init_f(void)
{
	uint32_t sdr0_pfc1, sdr0_pfc2;
	uint32_t reg;
	int eth;

#if defined(CONFIG_KORAT_PERMANENT)
	unsigned mscount;

	extern void korat_branch_absolute(uint32_t addr);

	for (mscount = 0;  mscount < CONFIG_SYS_KORAT_MAN_RESET_MS; ++mscount) {
		udelay(1000);
		if (gpio_read_in_bit(CONFIG_SYS_GPIO_RESET_PRESSED_)) {
			/* This call does not return. */
			korat_branch_absolute(
				CONFIG_SYS_FLASH1_TOP - 2 * CONFIG_ENV_SECT_SIZE - 4);
		}
	}
	korat_buzzer(1);
	while (!gpio_read_in_bit(CONFIG_SYS_GPIO_RESET_PRESSED_))
		udelay(1000);

	korat_buzzer(0);
#endif

	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	mtdcr(EBC0_CFGDATA, 0xb8400000);

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

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC2TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

	/*
	 * Take sim card reader and CF controller out of reset.  Also enable PHY
	 * auto-detect until board-specific PHY resets are available.
	 */
	out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x02, 0xC0);

	/* Configure the two Ethernet PHYs.  For each PHY, configure for fiber
	 * if the SFP module is present, and for copper if it is not present.
	 */
	for (eth = 0; eth < 2; ++eth) {
		if (gpio_read_in_bit(CONFIG_SYS_GPIO_SFP0_PRESENT_ + eth)) {
			/* SFP module not present: configure PHY for copper. */
			/* Set PHY to autonegotate 10 MB, 100MB, or 1 GB */
			out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x03,
			      in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x03) |
			      0x06 << (4 * eth));
		} else {
			/* SFP module present: configure PHY for fiber and
			   enable output */
			gpio_write_bit(CONFIG_SYS_GPIO_PHY0_FIBER_SEL + eth, 1);
			gpio_write_bit(CONFIG_SYS_GPIO_SFP0_TX_EN_ + eth, 0);
		}
	}
	/* enable Ethernet: set GPIO45 and GPIO46 to 1 */
	gpio_write_bit(CONFIG_SYS_GPIO_PHY0_EN, 1);
	gpio_write_bit(CONFIG_SYS_GPIO_PHY1_EN, 1);

	/* Wait 1 ms, then enable Fiber signal detect to PHYs. */
	udelay(1000);
	out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x03,
	      in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x03) | 0x88);

	/* select Ethernet (and optionally IIC1) pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
		SDR0_PFC1_SELECT_CONFIG_4;
#ifdef CONFIG_I2C_MULTI_BUS
	sdr0_pfc1 |= ((sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL);
#endif
	mfsdr(SDR0_PFC2, sdr0_pfc2);
	sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
		SDR0_PFC2_SELECT_CONFIG_4;
	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	/* PCI arbiter enabled */
	mfsdr(SDR0_PCI0, reg);
	mtsdr(SDR0_PCI0, 0x80000000 | reg);

	return 0;
}

/*
 * The boot flash on CS0 normally has its write-enable pin disabled, and so will
 * not respond to CFI commands.  This routine therefore fills in the flash
 * information for the boot flash.  (The flash at CS1 operates normally.)
 */
ulong board_flash_get_legacy (ulong base, int banknum, flash_info_t * info)
{
	uint32_t addr;
	int i;

	if (1 != banknum)
		return 0;

	info->size		= CONFIG_SYS_FLASH0_SIZE;
	info->sector_count	= CONFIG_SYS_FLASH0_SIZE / 0x20000;
	info->flash_id		= 0x01000000;
	info->portwidth		= 2;
	info->chipwidth		= 2;
	info->buffer_size	= 32;
	info->erase_blk_tout	= 16384;
	info->write_tout	= 2;
	info->buffer_write_tout	= 5;
	info->vendor		= 2;
	info->cmd_reset		= 0x00F0;
	info->interface		= 2;
	info->legacy_unlock	= 0;
	info->manufacturer_id	= 1;
	info->device_id		= 0x007E;

#if CONFIG_SYS_FLASH0_SIZE == 0x01000000
	info->device_id2	= 0x2101;
#elif CONFIG_SYS_FLASH0_SIZE == 0x04000000
	info->device_id2	= 0x2301;
#else
#error Unable to set device_id2 for current CONFIG_SYS_FLASH0_SIZE
#endif

	info->ext_addr		= 0x0040;
	info->cfi_version	= 0x3133;
	info->cfi_offset	= 0x0055;
	info->addr_unlock1	= 0x00000555;
	info->addr_unlock2	= 0x000002AA;
	info->name		= "CFI conformant";
	for (i = 0, addr = -info->size;
	     i < info->sector_count;
	     ++i, addr += 0x20000) {
		info->start[i] = addr;
		info->protect[i] = 0x00;
	}
	return 1;
}

static int man_data_read(unsigned int addr)
{
	/*
	 * Read an octet of data from address "addr" in the manufacturer's
	 * information serial EEPROM, or -1 on error.
	 */
	u8 data[2];

	if (0 != i2c_probe(MAN_DATA_EEPROM_ADDR) ||
	    0 != i2c_read(MAN_DATA_EEPROM_ADDR, addr, 1, data, 1)) {
		debug("man_data_read(0x%02X) failed\n", addr);
		return -1;
	}
	debug("man_info_read(0x%02X) returned 0x%02X\n", addr, data[0]);
	return data[0];
}

static unsigned int man_data_field_addr(unsigned int const field)
{
	/*
	 * The manufacturer's information serial EEPROM contains a sequence of
	 * zero-delimited fields.  Return the starting address of field "field",
	 * or 0 on error.
	 */
	unsigned addr, i;

	if (0 == field || 'A' != man_data_read(0) || '\0' != man_data_read(1))
		/* Only format "A" is currently supported */
		return 0;

	for (addr = 2, i = 1; i < field && addr < 256; ++addr) {
		if ('\0' == man_data_read(addr))
			++i;
	}
	return (addr < 256) ? addr : 0;
}

static char *man_data_read_field(char s[], unsigned const field,
				 unsigned const length)
{
	/*
	 * Place the null-terminated contents of field "field" of length
	 * "length" from the manufacturer's information serial EEPROM into
	 * string "s[length + 1]" and return a pointer to s, or return 0 on
	 * error. In either case the original contents of s[] is not preserved.
	 */
	unsigned addr, i;

	addr = man_data_field_addr(field);
	if (0 == addr || addr + length >= 255)
		return 0;

	for (i = 0; i < length; ++i) {
		int const c = man_data_read(addr++);

		if (c <= 0)
			return 0;

		s[i] = (char)c;
	}
	if (0 != man_data_read(addr))
		return 0;

	s[i] = '\0';
	return s;
}

static void set_serial_number(void)
{
	/*
	 * If the environmental variable "serial#" is not set, try to set it
	 * from the manufacturer's information serial EEPROM.
	 */
	char s[MAN_INFO_LENGTH + MAN_MAC_ADDR_LENGTH + 2];

	if (getenv("serial#"))
		return;

	if (!man_data_read_field(s, MAN_INFO_FIELD, MAN_INFO_LENGTH))
		return;

	s[MAN_INFO_LENGTH] = '-';
	if (!man_data_read_field(s + MAN_INFO_LENGTH + 1, MAN_MAC_ADDR_FIELD,
				 MAN_MAC_ADDR_LENGTH))
		return;

	setenv("serial#", s);
}

static void set_mac_addresses(void)
{
	/*
	 * If the environmental variables "ethaddr" and/or "eth1addr" are not
	 * set, try to set them from the manufacturer's information serial
	 * EEPROM.
	 */

#if MAN_MAC_ADDR_LENGTH % 2 != 0
#error MAN_MAC_ADDR_LENGTH must be an even number
#endif

	char s[(3 * MAN_MAC_ADDR_LENGTH) / 2];
	char *src;
	char *dst;

	if (0 != getenv("ethaddr") && 0 != getenv("eth1addr"))
		return;

	if (0 == man_data_read_field(s + (MAN_MAC_ADDR_LENGTH / 2) - 1,
				     MAN_MAC_ADDR_FIELD, MAN_MAC_ADDR_LENGTH))
		return;

	for (src = s + (MAN_MAC_ADDR_LENGTH / 2) - 1, dst = s; src != dst;) {
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = ':';
	}
	if (0 == getenv("ethaddr"))
		setenv("ethaddr", s);

	if (0 == getenv("eth1addr")) {
		++s[((3 * MAN_MAC_ADDR_LENGTH) / 2) - 2];
		setenv("eth1addr", s);
	}
}

int misc_init_r(void)
{
	uint32_t pbcr;
	int size_val;
	uint32_t reg;
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;
	uint32_t const flash1_size = gd->bd->bi_flashsize - CONFIG_SYS_FLASH0_SIZE;
	char const *const act = getenv("usbact");
	char const *const usbcf = getenv("korat_usbcf");

	/*
	 * Re-do FLASH1 sizing and adjust flash start and offset.
	 */
	gd->bd->bi_flashstart = CONFIG_SYS_FLASH1_TOP - flash1_size;
	gd->bd->bi_flashoffset = 0;

	mtdcr(EBC0_CFGADDR, PB1CR);
	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(flash1_size) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtdcr(EBC0_CFGADDR, PB1CR);
	mtdcr(EBC0_CFGDATA, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

	/*
	 * Re-do FLASH1 sizing and adjust flash offset to reserve space for
	 * environment
	 */
	gd->bd->bi_flashoffset =
		CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - CONFIG_SYS_FLASH1_ADDR;

	mtdcr(EBC0_CFGADDR, PB1CR);
	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(gd->bd->bi_flashsize - CONFIG_SYS_FLASH0_SIZE) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtdcr(EBC0_CFGADDR, PB1CR);
	mtdcr(EBC0_CFGDATA, pbcr);

	/* Monitor protection ON by default */
#if defined(CONFIG_KORAT_PERMANENT)
	(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
			    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
			    flash_info + 1);
#else
	(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
			    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
			    flash_info);
#endif
	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
			    CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			    flash_info);
	(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
			    CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			    flash_info);

	/*
	 * USB suff...
	 */
	/*
	 * Select the USB controller on the 440EPx ("ppc") or on the PCI bus
	 * ("pci") for the CompactFlash.
	 */
	if (usbcf != NULL && (strcmp(usbcf, "ppc") == 0)) {
		/*
		 * If environment variable "usbcf" is defined and set to "ppc",
		 * then connect the CompactFlash controller to the PowerPC USB
		 * port.
		 */
		printf("Attaching CompactFlash controller to PPC USB\n");
		out_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x02,
		      in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0x02) | 0x10);
	} else {
		if (usbcf != NULL && (strcmp(usbcf, "pci") != 0))
			printf("Warning: \"korat_usbcf\" is not set to a legal "
			       "value (\"ppc\" or \"pci\")\n");

		printf("Attaching CompactFlash controller to PCI USB\n");
	}
	if (act == NULL || strcmp(act, "hostdev") == 0) {
		/* SDR Setting */
		mfsdr(SDR0_PFC1, sdr0_pfc1);
		mfsdr(SDR0_USB2D0CR, usb2d0cr);
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

		/*
		 * An 8-bit/60MHz interface is the only possible alternative
		 * when connecting the Device to the PHY
		 */
		usb2h0cr = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;

		/*
		 * To enable the USB 2.0 Device function
		 * through the UTMI interface
		 */
		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_USB2DEV_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_USB2D_SEL;

		mtsdr(SDR0_PFC1, sdr0_pfc1);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2H0CR, usb2h0cr);

		/* clear resets */
		udelay(1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay(1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Host(int phy) Device(ext phy)\n");

	} else if (strcmp(act, "dev") == 0) {
		/*-------------------PATCH-------------------------------*/
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		udelay(1000);
		mtsdr(SDR0_SRST1, 0x672c6000);

		udelay(1000);
		mtsdr(SDR0_SRST0, 0x00000080);

		udelay(1000);
		mtsdr(SDR0_SRST1, 0x60206000);

		*(unsigned int *)(0xe0000350) = 0x00000001;

		udelay(1000);
		mtsdr(SDR0_SRST1, 0x60306000);
		/*-------------------PATCH-------------------------------*/

		/* SDR Setting */
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);
		mfsdr(SDR0_USB2D0CR, usb2d0cr);
		mfsdr(SDR0_PFC1, sdr0_pfc1);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PUREN;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_DEV;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_DEV;

		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_8BIT_60MHZ;

		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_EBC_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

		mtsdr(SDR0_USB2H0CR, usb2h0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_PFC1, sdr0_pfc1);

		/* clear resets */
		udelay(1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay(1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Device(int phy)\n");
	}

	mfsdr(SDR0_SRST1, reg);		/* enable security/kasumi engines */
	reg &= ~(SDR0_SRST1_CRYP0 | SDR0_SRST1_KASU0);
	mtsdr(SDR0_SRST1, reg);

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(PLB4A0_ACR) & ~PLB4Ax_ACR_WRP_MASK;
	mtdcr(PLB4A0_ACR, reg);

	set_serial_number();
	set_mac_addresses();
	gpio_write_bit(CONFIG_SYS_GPIO_ATMEGA_RESET_, 1);

	return 0;
}

int checkboard(void)
{
	char const *const s = getenv("serial#");
	u8 const rev = in_8((u8 *) CONFIG_SYS_CPLD_BASE + 0);

	printf("Board: Korat, Rev. %X", rev);
	if (s)
		printf(", serial# %s", s);

	printf(".\n       Ethernet PHY 0: ");
	if (gpio_read_out_bit(CONFIG_SYS_GPIO_PHY0_FIBER_SEL))
		printf("fiber");
	else
		printf("copper");

	printf(", PHY 1: ");
	if (gpio_read_out_bit(CONFIG_SYS_GPIO_PHY1_FIBER_SEL))
		printf("fiber");
	else
		printf("copper");

	printf(".\n");
#if defined(CONFIG_KORAT_PERMANENT)
	printf("       Executing permanent copy of U-Boot.\n");
#endif
	return 0;
}

#if defined(CONFIG_PCI) && defined(CONFIG_PCI_PNP)
/*
 * Assign interrupts to PCI devices.
 */
void board_pci_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, VECNUM_EIRQ2);
}
#endif

/*
 * pci_target_init
 *
 * The bootstrap configuration provides default settings for the pci
 * inbound map (PIM). But the bootstrap config choices are limited and
 * may not be sufficient for a given board.
 */
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/* First do 440EP(x) common setup */
	__pci_target_init(hose);

	/*
	 * Set up Configuration registers for on-board NEC uPD720101 USB
	 * controller.
	 */
	pci_write_config_dword(PCI_BDF(0x0, 0xC, 0x0), 0xE4, 0x00000020);
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[4];
	int rc;

	ft_cpu_setup(blob, bd);

	/* Fixup NOR mapping */
	val[0] = 1;				/* chip select number */
	val[1] = 0;				/* always 0 */
	val[2] = gd->bd->bi_flashstart;
	val[3] = gd->bd->bi_flashsize - CONFIG_SYS_FLASH0_SIZE;
	rc = fdt_find_and_setprop(blob, "/plb/opb/ebc", "ranges",
				  val, sizeof(val), 1);
	if (rc)
		printf("Unable to update property NOR mapping, err=%s\n",
		       fdt_strerror(rc));
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
