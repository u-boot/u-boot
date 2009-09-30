/*
 * (C) Copyright 2005
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
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
#include <asm/processor.h>
#include <asm/io.h>
#include <command.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f (void)
{
	unsigned long CPC0_CR0Reg;

	/*
	 * Setup GPIO pins
	 */
	CPC0_CR0Reg = mfdcr(CPC0_CR0);
	mtdcr(CPC0_CR0, CPC0_CR0Reg |
	      ((CONFIG_SYS_EEPROM_WP | CONFIG_SYS_PB_LED |
		CONFIG_SYS_SELF_RST | CONFIG_SYS_INTA_FAKE) << 5));

	/* set output pins to high */
	out_be32((void *)GPIO0_OR,  CONFIG_SYS_EEPROM_WP);
	/* setup for output (LED=off) */
	out_be32((void *)GPIO0_TCR, CONFIG_SYS_EEPROM_WP | CONFIG_SYS_PB_LED);

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) PB0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) PB1; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) PCI SLOT 0; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) PCI SLOT 1; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI SLOT 3; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) unused
	 */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000000);	/* set all to be non-critical*/
	mtdcr(UIC0PR, 0xFFFFFF81);	/* set int polarities */

	mtdcr(UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority*/
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	return 0;
}

int misc_init_r (void)
{
	unsigned long CPC0_CR0Reg;

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Select cts (and not dsr) on uart1
	 */
	CPC0_CR0Reg = mfdcr(CPC0_CR0);
	mtdcr(CPC0_CR0, CPC0_CR0Reg | 0x00001000);

	return (0);
}


/*
 * Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_r ("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming CPCI2DP");
	} else {
		puts(str);
	}

	printf(" (Ver 1.0)");

	putc ('\n');

	return 0;
}

#if defined(CONFIG_SYS_EEPROM_WREN)
/* Input: <dev_addr>  I2C address of EEPROM device to enable.
 *	   <state>     -1: deliver current state
 *		       0: disable write
 *		       1: enable write
 *  Returns:	       -1: wrong device address
 *			0: dis-/en- able done
 *		     0/1: current state if <state> was -1.
 */
int eeprom_write_enable (unsigned dev_addr, int state) {
	if (CONFIG_SYS_I2C_EEPROM_ADDR != dev_addr) {
		return -1;
	} else {
		switch (state) {
		case 1:
			/* Enable write access, clear bit GPIO_SINT2. */
			out_be32((void *)GPIO0_OR,
				 in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_EEPROM_WP);
			state = 0;
			break;
		case 0:
			/* Disable write access, set bit GPIO_SINT2. */
			out_be32((void *)GPIO0_OR,
				 in_be32((void *)GPIO0_OR) | CONFIG_SYS_EEPROM_WP);
			state = 0;
			break;
		default:
			/* Read current status back. */
			state = (0 == (in_be32((void *)GPIO0_OR) &
				       CONFIG_SYS_EEPROM_WP));
			break;
		}
	}
	return state;
}
#endif

#if defined(CONFIG_SYS_EEPROM_WREN)
int do_eep_wren (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int query = argc == 1;
	int state = 0;

	if (query) {
		/* Query write access state. */
		state = eeprom_write_enable (CONFIG_SYS_I2C_EEPROM_ADDR, -1);
		if (state < 0) {
			puts ("Query of write access state failed.\n");
		} else {
			printf ("Write access for device 0x%0x is %sabled.\n",
				CONFIG_SYS_I2C_EEPROM_ADDR, state ? "en" : "dis");
			state = 0;
		}
	} else {
		if ('0' == argv[1][0]) {
			/* Disable write access. */
			state = eeprom_write_enable (CONFIG_SYS_I2C_EEPROM_ADDR, 0);
		} else {
			/* Enable write access. */
			state = eeprom_write_enable (CONFIG_SYS_I2C_EEPROM_ADDR, 1);
		}
		if (state < 0) {
			puts ("Setup of write access state failed.\n");
		}
	}

	return state;
}

U_BOOT_CMD(
	eepwren,	2,	0,	do_eep_wren,
	"Enable / disable / query EEPROM write access",
	""
);
#endif /* #if defined(CONFIG_SYS_EEPROM_WREN) */
