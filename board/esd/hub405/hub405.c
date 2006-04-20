/*
 * (C) Copyright 2001-2003
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <asm/processor.h>
#include <command.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

extern void lxt971_no_sleep(void);

int board_revision(void)
{
	unsigned long osrl_reg;
	unsigned long isr1l_reg;
	unsigned long tcr_reg;
	unsigned long value;

	/*
	 * Get version of HUB405 board from GPIO's
	 */

	/*
	 * Setup GPIO pin(s) (IRQ6/GPIO23)
	 */
	osrl_reg = in32(GPIO0_OSRH);
	isr1l_reg = in32(GPIO0_ISR1H);
	tcr_reg = in32(GPIO0_TCR);
	out32(GPIO0_OSRH, osrl_reg & ~0x00030000);     /* output select */
	out32(GPIO0_ISR1H, isr1l_reg | 0x00030000);    /* input select  */
	out32(GPIO0_TCR, tcr_reg & ~0x00000100);       /* select input  */

	udelay(1000);            /* wait some time before reading input */
	value = in32(GPIO0_IR) & 0x00000100;         /* get config bits */

	/*
	 * Restore GPIO settings
	 */
	out32(GPIO0_OSRH, osrl_reg);                   /* output select */
	out32(GPIO0_ISR1H, isr1l_reg);                 /* input select  */
	out32(GPIO0_TCR, tcr_reg);  /* enable output driver for outputs */

	if (value & 0x00000100) {
		/* Revision 1.1 or 1.2 detected */
		return 1;
	}

	/* Revision 1.0 */
	return 0;
}


int board_early_init_f (void)
{
	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) SER0 ; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) SER1; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) FPGA 0; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) FPGA 1; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI INTA; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
	 */
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */
	mtdcr(uicer, 0x00000000);       /* disable all ints */
	mtdcr(uiccr, 0x00000000);       /* set all to be non-critical*/
	mtdcr(uicpr, 0xFFFFFF9F);       /* set int polarities */
	mtdcr(uictr, 0x10000000);       /* set int trigger levels */
	mtdcr(uicvcr, 0x00000001);      /* set vect base=0,INT0 highest priority*/
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */

	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks -> ca. 15 us
	 */
	mtebc (epcr, 0xa8400000); /* ebc always driven */

	return 0;
}


int misc_init_f (void)
{
	return 0;  /* dummy implementation */
}


int misc_init_r (void)
{
	volatile unsigned char *duart0_mcr = (unsigned char *)((ulong)DUART0_BA + 4);
	volatile unsigned char *duart1_mcr = (unsigned char *)((ulong)DUART1_BA + 4);
	volatile unsigned char *duart2_mcr = (unsigned char *)((ulong)DUART2_BA + 4);
	volatile unsigned char *duart3_mcr = (unsigned char *)((ulong)DUART3_BA + 4);
	volatile unsigned char *led_reg    = (unsigned char *)((ulong)DUART0_BA + 0x20);
	unsigned long val;
	int delay, flashcnt;
	char *str;
	char hw_rev[4];

	/*
	 * Enable interrupts in exar duart mcr[3]
	 */
	*duart0_mcr = 0x08;
	*duart1_mcr = 0x08;
	*duart2_mcr = 0x08;
	*duart3_mcr = 0x08;

	/*
	 * Set RS232/RS422 control (RS232 = high on GPIO)
	 */
	val = in32(GPIO0_OR);
	val &= ~(CFG_UART2_RS232 | CFG_UART3_RS232 | CFG_UART4_RS232 | CFG_UART5_RS232);

	str = getenv("phys0");
	if (!str || (str && (str[0] == '0')))
		val |= CFG_UART2_RS232;

	str = getenv("phys1");
	if (!str || (str && (str[0] == '0')))
		val |= CFG_UART3_RS232;

	str = getenv("phys2");
	if (!str || (str && (str[0] == '0')))
		val |= CFG_UART4_RS232;

	str = getenv("phys3");
	if (!str || (str && (str[0] == '0')))
		val |= CFG_UART5_RS232;

	out32(GPIO0_OR, val);

	/*
	 * Set NAND-FLASH GPIO signals to default
	 */
	out32(GPIO0_OR, in32(GPIO0_OR) & ~(CFG_NAND_CLE | CFG_NAND_ALE));
	out32(GPIO0_OR, in32(GPIO0_OR) | CFG_NAND_CE);

	/*
	 * check board type and setup AP power
	 */
	str = getenv("bd_type"); /* this is only set on non prototype hardware */
	if (str != NULL) {
		if ((strcmp(str, "swch405") == 0) || ((!strcmp(str, "hub405") && (gd->board_type >= 1)))) {
			unsigned char led_reg_default = 0;
			str = getenv("ap_pwr");
			if (!str || (str && (str[0] == '1')))
				led_reg_default = 0x04 | 0x02 ; /* U2_LED | AP_PWR */

			/*
			 * Flash LEDs
			 */
			for (flashcnt = 0; flashcnt < 3; flashcnt++) {
				*led_reg = led_reg_default;        /* LED_A..D off */
				for (delay = 0; delay < 100; delay++)
					udelay(1000);
				*led_reg = led_reg_default | 0xf0; /* LED_A..D on */
				for (delay = 0; delay < 50; delay++)
					udelay(1000);
			}
			*led_reg = led_reg_default;
		}
	}

	/*
	 * Reset external DUARTs
	 */
	out32(GPIO0_OR, in32(GPIO0_OR) | CFG_DUART_RST); /* set reset to high */
	udelay(10); /* wait 10us */
	out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_DUART_RST); /* set reset to low */
	udelay(1000); /* wait 1ms */

	/*
	 * Store hardware revision in environment for further processing
	 */
	sprintf(hw_rev, "1.%ld", gd->board_type);
	setenv("hw_rev", hw_rev);
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
		puts ("### No HW ID - assuming HUB405");
	} else {
		puts(str);
	}

	if (getenv_r("bd_type", str, sizeof(str)) != -1) {
		printf(" (%s", str);
	} else {
		puts(" (Missing bd_type!");
	}

	gd->board_type = board_revision();
	printf(", Rev 1.%ld)\n", gd->board_type);

	/*
	 * Disable sleep mode in LXT971
	 */
	lxt971_no_sleep();

	return 0;
}


long int initdram (int board_type)
{
	unsigned long val;

	mtdcr(memcfga, mem_mb0cf);
	val = mfdcr(memcfgd);

#if 0
	printf("\nmb0cf=%x\n", val); /* test-only */
	printf("strap=%x\n", mfdcr(strap)); /* test-only */
#endif

	return (4*1024*1024 << ((val & 0x000e0000) >> 17));
}


int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: 16 MB - ok\n");

	return (0);
}


#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <linux/mtd/nand_legacy.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];

void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
}
#endif
