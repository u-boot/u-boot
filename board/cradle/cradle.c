/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

#include <asm/arch/pxa-regs.h>
#include <common.h>

/* ------------------------------------------------------------------------- */


/* local prototypes */
void set_led (int led, int color);
void error_code_halt (int code);
int init_sio (int led, unsigned long base);
inline void cradle_outb (unsigned short val, unsigned long base,
			 unsigned long reg);
inline unsigned char cradle_inb (unsigned long base, unsigned long reg);
inline void sleep (int i);

inline void
/**********************************************************/
sleep (int i)
/**********************************************************/
{
	while (i--) {
		udelay (1000000);
	}
}

void
/**********************************************************/
error_code_halt (int code)
/**********************************************************/
{
	while (1) {
		led_code (code, RED);
		sleep (1);
		led_code (0, OFF);
		sleep (1);
	}
}

void
/**********************************************************/
led_code (int code, int color)
/**********************************************************/
{
	int i;

	code &= 0xf;		/* only 4 leds */

	for (i = 0; i < 4; i++) {
		if (code & (1 << i)) {
			set_led (i, color);
		} else {
			set_led (i, OFF);
		}
	}
}

void
/**********************************************************/
set_led (int led, int color)
/**********************************************************/
{
	int shift = led * 2;
	unsigned long mask = 0x3 << shift;

	CRADLE_LED_CLR_REG = mask;	/* clear bits */
	CRADLE_LED_SET_REG = (color << shift);	/* set bits */
	udelay (5000);
}

inline void
/**********************************************************/
cradle_outb (unsigned short val, unsigned long base, unsigned long reg)
/**********************************************************/
{
	*(volatile unsigned short *) (base + (reg * 2)) = val;
}

inline unsigned char
/**********************************************************/
cradle_inb (unsigned long base, unsigned long reg)
/**********************************************************/
{
	unsigned short val;

	val = *(volatile unsigned short *) (base + (reg * 2));
	return (val & 0xff);
}

int
/**********************************************************/
init_sio (int led, unsigned long base)
/**********************************************************/
{
	unsigned char val;

	set_led (led, YELLOW);
	val = cradle_inb (base, CRADLE_SIO_INDEX);
	val = cradle_inb (base, CRADLE_SIO_INDEX);
	if (val != 0) {
		set_led (led, RED);
		return -1;
	}

	/* map SCC2 to COM1 */
	cradle_outb (0x01, base, CRADLE_SIO_INDEX);
	cradle_outb (0x00, base, CRADLE_SIO_DATA);

	/* enable SCC2 extended regs */
	cradle_outb (0x40, base, CRADLE_SIO_INDEX);
	cradle_outb (0xa0, base, CRADLE_SIO_DATA);

	/* enable SCC2 clock multiplier */
	cradle_outb (0x51, base, CRADLE_SIO_INDEX);
	cradle_outb (0x04, base, CRADLE_SIO_DATA);

	/* enable SCC2 */
	cradle_outb (0x00, base, CRADLE_SIO_INDEX);
	cradle_outb (0x04, base, CRADLE_SIO_DATA);

	/* map SCC2 DMA to channel 0 */
	cradle_outb (0x4f, base, CRADLE_SIO_INDEX);
	cradle_outb (0x09, base, CRADLE_SIO_DATA);

	/* read ID from SIO to check operation */
	cradle_outb (0xe4, base, 0x3f8 + 0x3);
	val = cradle_inb (base, 0x3f8 + 0x0);
	if ((val & 0xf0) != 0x20) {
		set_led (led, RED);
		/* disable SCC2 */
		cradle_outb (0, base, CRADLE_SIO_INDEX);
		cradle_outb (0, base, CRADLE_SIO_DATA);
		return -1;
	}
	/* set back to bank 0 */
	cradle_outb (0, base, 0x3f8 + 0x3);
	set_led (led, GREEN);
	return 0;
}

/*
 * Miscelaneous platform dependent initialisations
 */

int
/**********************************************************/
board_late_init (void)
/**********************************************************/
{
	return (0);
}

int
/**********************************************************/
board_init (void)
/**********************************************************/
{
	DECLARE_GLOBAL_DATA_PTR;

	led_code (0xf, YELLOW);

	/* arch number of HHP Cradle */
	gd->bd->bi_arch_number = MACH_TYPE_HHP_CRADLE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Init SIOs to enable SCC2 */
	udelay (100000);		/* delay makes it look neat */
	init_sio (0, CRADLE_SIO1_PHYS);
	udelay (100000);
	init_sio (1, CRADLE_SIO2_PHYS);
	udelay (100000);
	init_sio (2, CRADLE_SIO3_PHYS);
	udelay (100000);
	set_led (3, GREEN);

	return 1;
}

int
/**********************************************************/
dram_init (void)
/**********************************************************/
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size  = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size  = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size  = PHYS_SDRAM_4_SIZE;

	return (PHYS_SDRAM_1_SIZE +
		PHYS_SDRAM_2_SIZE +
		PHYS_SDRAM_3_SIZE +
		PHYS_SDRAM_4_SIZE );
}
