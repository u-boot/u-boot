/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ppc4xx.h>
#include <malloc.h>
#include <command.h>
#include <crc.h>
#include <asm/processor.h>
#include <spd_sdram.h>
#include <status_led.h>
#include <sha1.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

unsigned char	sha1_checksum[SHA1_SUM_LEN];

/* swap 4 Bits (Bit0 = Bit3, Bit1 = Bit2, Bit2 = Bit1 and Bit3 = Bit0) */
unsigned char swapbits[16] = {0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
			      0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf};

static void set_leds (int val)
{
	out32(GPIO0_OR, (in32 (GPIO0_OR) & ~0x78000000) | (val << 27));
}

#define GET_LEDS ((in32 (GPIO0_OR) & 0x78000000) >> 27)

void __led_init (led_id_t mask, int state)
{
	int	val = GET_LEDS;

	if (state == STATUS_LED_ON)
		val |= mask;
	else
		val &= ~mask;
	set_leds (val);
}

void __led_set (led_id_t mask, int state)
{
	int	val = GET_LEDS;

	if (state == STATUS_LED_ON)
		val |= mask;
	else if (state == STATUS_LED_OFF)
		val &= ~mask;
	set_leds (val);
}

void __led_toggle (led_id_t mask)
{
	int	val = GET_LEDS;

	val ^= mask;
	set_leds (val);
}

static void status_led_blink (void)
{
	int	i;
	int	val = GET_LEDS;

	/* set all LED which are on, to state BLINKING */
	for (i = 0; i < 4; i++) {
		if (val & 0x08) status_led_set (i, STATUS_LED_BLINKING);
		val = val << 1;
	}
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress (int val)
{
	/* find all valid Codes for val in README */
	if (val == -30) return;
	if (val < 0) {
		/* smthing goes wrong */
		status_led_blink ();
		return;
	}
	switch (val) {
		case 1:
			/* validating Image */
			status_led_set (0, STATUS_LED_OFF);
			status_led_set (1, STATUS_LED_ON);
			status_led_set (2, STATUS_LED_ON);
			break;
		case 15:
			/* booting */
			status_led_set (0, STATUS_LED_ON);
			status_led_set (1, STATUS_LED_ON);
			status_led_set (2, STATUS_LED_ON);
			break;
		case 64:
			/* starting Ethernet configuration */
			status_led_set (0, STATUS_LED_OFF);
			status_led_set (1, STATUS_LED_OFF);
			status_led_set (2, STATUS_LED_ON);
			break;
		case 80:
			/* loading Image */
			status_led_set (0, STATUS_LED_ON);
			status_led_set (1, STATUS_LED_OFF);
			status_led_set (2, STATUS_LED_ON);
			break;
	}
}
#endif

int board_early_init_f(void)
{
	register uint reg;

	set_leds(0);			/* display boot info counter */

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr(ebccfga, xbcfg);
	reg = mfdcr(ebccfgd);
	mtdcr(ebccfgd, reg | 0x04000000);	/* Set ATC */

	/*--------------------------------------------------------------------
	 * GPIO's are alreay setup in cpu/ppc4xx/cpu_init.c
	 * via define from board config file.
	 *-------------------------------------------------------------------*/

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000001);	/* UIC1 crit is critical */
	mtdcr(uic0pr, 0xfffffe1f);	/* per ref-board manual */
	mtdcr(uic0tr, 0x01c00000);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	/*--------------------------------------------------------------------
	 * Setup other serial configuration
	 *-------------------------------------------------------------------*/
	mfsdr(sdr_pci0, reg);
	mtsdr(sdr_pci0, 0x80000000 | reg);	/* PCI arbiter enabled */
	mtsdr(sdr_pfc0, 0x00000100);	/* Pin function: enable GPIO49-63 */
	mtsdr(sdr_pfc1, 0x00048000);	/* Pin function: UART0 has 4 pins, select IRQ5 */

	return 0;
}

#define EEPROM_LEN	256
void load_sernum_ethaddr (void)
{
	int	ret;
	char	buf[EEPROM_LEN];
	char	mac[32];
	char	*use_eeprom;
	u16	checksumcrc16 = 0;

	/* read the MACs from EEprom */
	status_led_set (0, STATUS_LED_ON);
	status_led_set (1, STATUS_LED_ON);
	ret = eeprom_read (CFG_I2C_EEPROM_ADDR, 0, (uchar *)buf, EEPROM_LEN);
	if (ret == 0) {
		checksumcrc16 = cyg_crc16 ((uchar *)buf, EEPROM_LEN - 2);
		/* check, if the EEprom is programmed:
		 * - The Prefix(Byte 0,1,2) is equal to "ATR"
		 * - The checksum, stored in the last 2 Bytes, is correct
		 */
		if ((strncmp (buf,"ATR",3) != 0) ||
			((checksumcrc16 >> 8) != buf[EEPROM_LEN - 2]) ||
			((checksumcrc16 & 0xff) != buf[EEPROM_LEN - 1])) 
		{
			/* EEprom is not programmed */
			printf("%s: EEPROM Checksum not OK\n", __FUNCTION__);
		} else {
			/* get the MACs */
			sprintf (mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
				buf[3],
				buf[4],
				buf[5],
				buf[6],
				buf[7],
				buf[8]);
			setenv ("ethaddr", (char *) mac);
			sprintf (mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
				buf[9],
				buf[10],
				buf[11],
				buf[12],
				buf[13],
				buf[14]);
			setenv ("eth1addr", (char *) mac);
			return;
		}
	}

	/* some error reading the EEprom */
	if ((use_eeprom = getenv ("use_eeprom_ethaddr")) == NULL) {
		/* dont use bootcmd */
		setenv("bootdelay", "-1");
		return;
	}
	/* == default ? use standard */
	if (strncmp (use_eeprom, "default", 7) == 0) {
		return;
	}
	/* Env doesnt exist -> hang */
	status_led_blink ();
	hang ();
	return;
}

#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[]		= "key_magic";
static uchar kbd_command_prefix[]	= "key_cmd";

struct kbd_data_t {
	char s1;
	char s2;
};

struct kbd_data_t* get_keys (struct kbd_data_t *kbd_data)
{
	char *val;
	unsigned long tmp;

	/* use the DIPs for some bootoptions */
	val = getenv (ENV_NAME_DIP);
	tmp = simple_strtoul (val, NULL, 16);

	kbd_data->s2 = (tmp & 0x0f);
	kbd_data->s1 = (tmp & 0xf0) >> 4;
	return kbd_data;
}

static int compare_magic (const struct kbd_data_t *kbd_data, char *str)
{
	char s1 = str[0];

	if (s1 >= '0' && s1 <= '9')
		s1 -= '0';
	else if (s1 >= 'a' && s1 <= 'f')
		s1 = s1 - 'a' + 10;
	else if (s1 >= 'A' && s1 <= 'F')
		s1 = s1 - 'A' + 10;
	else
		return -1;

	if (s1 != kbd_data->s1) return -1;

	s1 = str[1];
	if (s1 >= '0' && s1 <= '9')
		s1 -= '0';
	else if (s1 >= 'a' && s1 <= 'f')
		s1 = s1 - 'a' + 10;
	else if (s1 >= 'A' && s1 <= 'F')
		s1 = s1 - 'A' + 10;
	else
		return -1;

	if (s1 != kbd_data->s2) return -1;
	return 0;
}

static char *key_match (const struct kbd_data_t *kbd_data)
{
	char magic[sizeof (kbd_magic_prefix) + 1];
	char *suffix;
	char *kbd_magic_keys;

	/*
	 * The following string defines the characters that can be appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	if ((kbd_magic_keys = getenv ("magic_keys")) == NULL)
		kbd_magic_keys = "";

	/* loop over all magic keys;
	 * use '\0' suffix in case of empty string
	 */
	for (suffix = kbd_magic_keys; *suffix ||
		     suffix == kbd_magic_keys; ++suffix) {
		sprintf (magic, "%s%c", kbd_magic_prefix, *suffix);
		if (compare_magic (kbd_data, getenv (magic)) == 0) {
			char cmd_name[sizeof (kbd_command_prefix) + 1];
			char *cmd;

			sprintf (cmd_name, "%s%c", kbd_command_prefix, *suffix);
			cmd = getenv (cmd_name);

			return (cmd);
		}
	}
	return (NULL);
}

#endif /* CONFIG_PREBOOT */

static int pcs440ep_readinputs (void)
{
	int	i;
	char	value[20];

	/* read the inputs and set the Envvars */
	/* Revision Level Bit 26 - 29 */
	i = ((in32 (GPIO0_IR) & 0x0000003c) >> 2);
	i = swapbits[i];
	sprintf (value, "%02x", i);
	setenv (ENV_NAME_REVLEV, value);
	/* Solder Switch Bit 30 - 33 */
	i = (in32 (GPIO0_IR) & 0x00000003) << 2;
	i += (in32 (GPIO1_IR) & 0xc0000000) >> 30;
	i = swapbits[i];
	sprintf (value, "%02x", i);
	setenv (ENV_NAME_SOLDER, value);
	/* DIP Switch Bit 49 - 56 */
	i = ((in32 (GPIO1_IR) & 0x00007f80) >> 7);
	i = (swapbits[i & 0x0f] << 4) + swapbits[(i & 0xf0) >> 4];
	sprintf (value, "%02x", i);
	setenv (ENV_NAME_DIP, value);
	return 0;
}


#if defined(CONFIG_SHA1_CHECK_UB_IMG)
/*************************************************************************
 * calculate a SHA1 sum for the U-Boot image in Flash.
 *
 ************************************************************************/
static int pcs440ep_sha1 (int docheck)
{
	unsigned char *data;
	unsigned char *ptroff;
	unsigned char output[20];
	unsigned char org[20];
	int	i, len = CONFIG_SHA1_LEN;

	memcpy ((char *)CFG_LOAD_ADDR, (char *)CONFIG_SHA1_START, len);
	data = (unsigned char *)CFG_LOAD_ADDR;
	ptroff = &data[len + SHA1_SUM_POS];

	for (i = 0; i < SHA1_SUM_LEN; i++) {
		org[i] = ptroff[i];
		ptroff[i] = 0;
	}
	
	sha1_csum ((unsigned char *) data, len, (unsigned char *)output);

	if (docheck == 2) {
		for (i = 0; i < 20 ; i++) {
			printf("%02X ", output[i]);
		}
		printf("\n");
	}
	if (docheck == 1) {
		for (i = 0; i < 20 ; i++) {
			if (org[i] != output[i]) return 1;
		}
	}
	return 0;
}

/*************************************************************************
 * do some checks after the SHA1 checksum from the U-Boot Image was
 * calculated.
 *
 ************************************************************************/
static void pcs440ep_checksha1 (void)
{
	int	ret;
	char	*cs_test;

	ret = pcs440ep_sha1 (1);
	if (ret == 0) return;

	if ((cs_test = getenv ("cs_test")) == NULL) {
		/* Env doesnt exist -> hang */
		status_led_blink ();
		hang ();
	}

	if (strncmp (cs_test, "off", 3) == 0) {
		printf ("SHA1 U-Boot sum NOT ok!\n");
		setenv ("bootdelay", "-1");
	}
}
#else
static __inline__ void pcs440ep_checksha1 (void) { do {} while (0);}
#endif

int misc_init_r (void)
{
	uint pbcr;
	int size_val = 0;

	/* Re-do sizing to get full correct info */
	mtdcr(ebccfga, pb0cr);
	pbcr = mfdcr(ebccfgd);
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
	mtdcr(ebccfga, pb0cr);
	mtdcr(ebccfgd, pbcr);

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

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

	pcs440ep_readinputs ();
	pcs440ep_checksha1 ();
#ifdef CONFIG_PREBOOT
	{
		struct kbd_data_t kbd_data;
		/* Decode keys */
		char *str = strdup (key_match (get_keys (&kbd_data)));
		/* Set or delete definition */
		setenv ("preboot", str);
		free (str);
	}
#endif /* CONFIG_PREBOOT */
	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: PCS440EP");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

void spd_ddr_init_hang (void)
{
	status_led_set (0, STATUS_LED_OFF);
	status_led_set (1, STATUS_LED_ON);
	/* we cannot use hang() because we are still running from
	   Flash, and so the status_led driver is not initialized */
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;) {
		__led_toggle (4);
		udelay (100000);
	}
}

long int initdram (int board_type)
{
	long dram_size = 0;

	status_led_set (0, STATUS_LED_ON);
	status_led_set (1, STATUS_LED_OFF);
	dram_size = spd_sdram();
	status_led_set (0, STATUS_LED_OFF);
	status_led_set (1, STATUS_LED_ON);
	if (dram_size == 0) {
		hang();
	}

	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_KBYTES_SDRAM;
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
#if defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT)
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
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT) */

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
	  | PowerPC440 EP PCI Master configuration.
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
	out32r(PCIX0_PMM1LA, CFG_PCI_MEMBASE2);	/* PMM0 Local Address */
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
	/* PCS440EP is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */

/*************************************************************************
 *  hw_watchdog_reset
 *
 *	This routine is called to reset (keep alive) the watchdog timer
 *
 ************************************************************************/
#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{

}
#endif

/*************************************************************************
 * "led" Commando for the U-Boot shell
 *
 ************************************************************************/
int do_led (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int	rcode = 0;
	ulong	pattern = 0;

	pattern = simple_strtoul (argv[1], NULL, 10);
	if (pattern > 200) {
		status_led_blink ();
		hang ();
		return rcode;
	}
	if (pattern > 100) {
		status_led_blink ();
		return rcode;
	}
	pattern &= 0x0f;
	set_leds (pattern);
	return rcode;
}

U_BOOT_CMD(
 	led,	2,	1,	do_led,
 	"led    - set the led\n",
	NULL
);

#if defined(CONFIG_SHA1_CHECK_UB_IMG)
/*************************************************************************
 * "sha1" Commando for the U-Boot shell
 *
 ************************************************************************/
int do_sha1 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int	rcode = -1;

	if (argc < 2) {
  usage:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (argc >= 3) {
		unsigned char *data;
		unsigned char output[20];
		int	len;
		int	i;
		
		data = (unsigned char *)simple_strtoul (argv[1], NULL, 16);
		len = simple_strtoul (argv[2], NULL, 16);
		sha1_csum (data, len, (unsigned char *)output);
		printf ("U-Boot sum:\n");
		for (i = 0; i < 20 ; i++) {
			printf ("%02X ", output[i]);
		}
		printf ("\n");
		if (argc == 4) {
			data = (unsigned char *)simple_strtoul (argv[3], NULL, 16);
			memcpy (data, output, 20);
		}
		return 0;
	}
	if (argc == 2) {
		char *ptr = argv[1];
		if (*ptr != '-') goto usage;
		ptr++;
		if ((*ptr == 'c') || (*ptr == 'C')) {
			rcode = pcs440ep_sha1 (1);
			printf ("SHA1 U-Boot sum %sok!\n", (rcode != 0) ? "not " : "");
		} else if ((*ptr == 'p') || (*ptr == 'P')) {
			rcode = pcs440ep_sha1 (2);
		} else {
			rcode = pcs440ep_sha1 (0);
		}
		return rcode;	
	}
	return rcode;
}

U_BOOT_CMD(
 	sha1,	4,	1,	do_sha1,
 	"sha1    - calculate the SHA1 Sum\n",
	"address len [addr]  calculate the SHA1 sum [save at addr]\n"
	"     -p calculate the SHA1 sum from the U-Boot image in flash and print\n"
	"     -c check the U-Boot image in flash\n"
);
#endif

#ifdef CONFIG_IDE_PREINIT
int ide_preinit (void)
{
	/* Set True IDE Mode */
	out32 (GPIO0_OR, (in32 (GPIO0_OR) | 0x00100000));
	out32 (GPIO0_OR, (in32 (GPIO0_OR) | 0x00200000));
	out32 (GPIO1_OR, (in32 (GPIO1_OR) & ~0x00008040));
	udelay (100000);
	return 0;
}
#endif

#if defined (CFG_CMD_IDE) && defined (CONFIG_IDE_RESET)
void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);
	if (idereset == 0) {
		out32 (GPIO0_OR, (in32 (GPIO0_OR) | 0x00200000));
	} else {
		out32 (GPIO0_OR, (in32 (GPIO0_OR) & ~0x00200000));
	}
	udelay (10000);
}
#endif /* defined (CFG_CMD_IDE) && defined (CONFIG_IDE_RESET) */

