/*
 * (C) Copyright 2001
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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
#include <mpc824x.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <i2c.h>
#include <netdev.h>

int sysControlDisplay(int digit, uchar ascii_code);
extern void Plx9030Init(void);
extern void SPD67290Init(void);

	/* We have to clear the initial data area here. Couldn't have done it
	 * earlier because DRAM had not been initialized.
	 */
int board_early_init_f(void)
{

	/* enable DUAL UART Mode on CPC45 */
	*(uchar*)DUART_DCR |= 0x1;	/* set DCM bit */

	return 0;
}

int checkboard(void)
{
/*
	char  revision = BOARD_REV;
*/
	ulong busfreq  = get_bus_freq(0);
	char  buf[32];

	puts ("CPC45  ");
/*
	printf("Revision %d ", revision);
*/
	printf("Local Bus at %s MHz\n", strmhz(buf, busfreq));

	return 0;
}

phys_size_t initdram (int board_type)
{
	int m, row, col, bank, i, ref;
	unsigned long start, end;
	uint32_t mccr1, mccr2;
	uint32_t mear1 = 0, emear1 = 0, msar1 = 0, emsar1 = 0;
	uint32_t mear2 = 0, emear2 = 0, msar2 = 0, emsar2 = 0;
	uint8_t mber = 0;
	unsigned int tmp;

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (i2c_reg_read (0x50, 2) != 0x04)
		return 0;	/* Memory type */

	m = i2c_reg_read (0x50, 5);	/* # of physical banks */
	row = i2c_reg_read (0x50, 3);	/* # of rows */
	col = i2c_reg_read (0x50, 4);	/* # of columns */
	bank = i2c_reg_read (0x50, 17);	/* # of logical banks */
	ref  = i2c_reg_read (0x50, 12);	/* refresh rate / type */

	CONFIG_READ_WORD(MCCR1, mccr1);
	mccr1 &= 0xffff0000;

	CONFIG_READ_WORD(MCCR2, mccr2);
	mccr2 &= 0xffff0000;

	start = CONFIG_SYS_SDRAM_BASE;
	end = start + (1 << (col + row + 3) ) * bank - 1;

	for (i = 0; i < m; i++) {
		mccr1 |= ((row == 13)? 2 : (bank == 4)? 0 : 3) << i * 2;
		if (i < 4) {
			msar1  |= ((start >> 20) & 0xff) << i * 8;
			emsar1 |= ((start >> 28) & 0xff) << i * 8;
			mear1  |= ((end >> 20) & 0xff) << i * 8;
			emear1 |= ((end >> 28) & 0xff) << i * 8;
		} else {
			msar2  |= ((start >> 20) & 0xff) << (i-4) * 8;
			emsar2 |= ((start >> 28) & 0xff) << (i-4) * 8;
			mear2  |= ((end >> 20) & 0xff) << (i-4) * 8;
			emear2 |= ((end >> 28) & 0xff) << (i-4) * 8;
		}
		mber |= 1 << i;
		start += (1 << (col + row + 3) ) * bank;
		end += (1 << (col + row + 3) ) * bank;
	}
	for (; i < 8; i++) {
		if (i < 4) {
			msar1  |= 0xff << i * 8;
			emsar1 |= 0x30 << i * 8;
			mear1  |= 0xff << i * 8;
			emear1 |= 0x30 << i * 8;
		} else {
			msar2  |= 0xff << (i-4) * 8;
			emsar2 |= 0x30 << (i-4) * 8;
			mear2  |= 0xff << (i-4) * 8;
			emear2 |= 0x30 << (i-4) * 8;
		}
	}

	switch(ref) {
		case 0x00:
		case 0x80:
			tmp = get_bus_freq(0) / 1000000 * 15625 / 1000 - 22;
			break;
		case 0x01:
		case 0x81:
			tmp = get_bus_freq(0) / 1000000 * 3900 / 1000 - 22;
			break;
		case 0x02:
		case 0x82:
			tmp = get_bus_freq(0) / 1000000 * 7800 / 1000 - 22;
			break;
		case 0x03:
		case 0x83:
			tmp = get_bus_freq(0) / 1000000 * 31300 / 1000 - 22;
			break;
		case 0x04:
		case 0x84:
			tmp = get_bus_freq(0) / 1000000 * 62500 / 1000 - 22;
			break;
		case 0x05:
		case 0x85:
			tmp = get_bus_freq(0) / 1000000 * 125000 / 1000 - 22;
			break;
		default:
			tmp = 0x512;
			break;
	}

	CONFIG_WRITE_WORD(MCCR1, mccr1);
	CONFIG_WRITE_WORD(MCCR2, tmp << MCCR2_REFINT_SHIFT);
	CONFIG_WRITE_WORD(MSAR1, msar1);
	CONFIG_WRITE_WORD(EMSAR1, emsar1);
	CONFIG_WRITE_WORD(MEAR1, mear1);
	CONFIG_WRITE_WORD(EMEAR1, emear1);
	CONFIG_WRITE_WORD(MSAR2, msar2);
	CONFIG_WRITE_WORD(EMSAR2, emsar2);
	CONFIG_WRITE_WORD(MEAR2, mear2);
	CONFIG_WRITE_WORD(EMEAR2, emear2);
	CONFIG_WRITE_BYTE(MBER, mber);

	return (1 << (col + row + 3) ) * bank * m;
}


/*
 * Initialize PCI Devices, report devices found.
 */

static struct pci_config_table pci_cpc45_config_table[] = {
#ifndef CONFIG_PCI_PNP
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0F, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0D, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_PLX9030_IOADDR,
				       PCI_PLX9030_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0E, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCMCIA_IO_BASE,
				       PCMCIA_IO_BASE,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_IO }},
#endif /*CONFIG_PCI_PNP*/
	{ }
};

struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_cpc45_config_table,
#endif
};

void pci_init_board(void)
{
	pci_mpc824x_init(&hose);

	/* init PCI_to_LOCAL Bus BRIDGE */
	Plx9030Init();

	/* Clear Display */
	DISP_CWORD = 0x0;

	sysControlDisplay(0,' ');
	sysControlDisplay(1,'C');
	sysControlDisplay(2,'P');
	sysControlDisplay(3,'C');
	sysControlDisplay(4,' ');
	sysControlDisplay(5,'4');
	sysControlDisplay(6,'5');
	sysControlDisplay(7,' ');

}

/**************************************************************************
*
* sysControlDisplay - controls one of the Alphanum. Display digits.
*
* This routine will write an ASCII character to the display digit requested.
*
* SEE ALSO:
*
* RETURNS: NA
*/

int sysControlDisplay (int digit,	/* number of digit 0..7 */
		       uchar ascii_code	/* ASCII code */
		      )
{
	if ((digit < 0) || (digit > 7))
		return (-1);

	*((volatile uchar *) (DISP_CHR_RAM + digit)) = ascii_code;

	return (0);
}

#if defined(CONFIG_CMD_PCMCIA)

#ifdef CONFIG_SYS_PCMCIA_MEM_ADDR
volatile unsigned char *pcmcia_mem = (unsigned char*)CONFIG_SYS_PCMCIA_MEM_ADDR;
#endif

int pcmcia_init(void)
{
	u_int rc;

	debug ("Enable PCMCIA " PCMCIA_SLOT_MSG "\n");

	rc = i82365_init();

	return rc;
}

#endif

# ifdef CONFIG_IDE_LED
void ide_led (uchar led, uchar status)
{
	u_char  val;
	/* We have one PCMCIA slot and use LED H4 for the IDE Interface */
	val = readb(BCSR_BASE + 0x04);
	if (status) {				/* led on */
		val |= B_CTRL_LED0;
	} else {
		val &= ~B_CTRL_LED0;
	}
	writeb(val, BCSR_BASE + 0x04);
}
# endif

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
