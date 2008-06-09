/*
 * (C)	Copyright 2001
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 * U-Boot port on RPXClassic LF (CLLF_BW31) board
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <i2c.h>
#include <config.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
static unsigned char aschex_to_byte (unsigned char *cp);

/* ------------------------------------------------------------------------- */

#define _NOT_USED_	0xFFFFCC25

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 00h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0X0CAFCC04, 0X03AFCC08,
	0x3FBFCC27, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Read. (Offset 08h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CAFCC84, 0x03AFCC88,
	0x3FBFCC27, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CFFCC04, 0x03FFCC00,
	0x3FFFCC27, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CFFCC80, 0x03FFCC8C,
	0x0CFFCC00, 0x33FFCC27, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/*
	 * Refresh. (Offset 30h in UPMA RAM)
	 */
	0xC0FFCC24, 0x03FFCC24, 0x0FFFCC24, 0x0FFFCC24,
	0x3FFFCC27, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Exception. (Offset 3Ch in UPMA RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: RPXClassic\n");
	return (0);
}

/*-----------------------------------------------------------------------------
 * board_get_enetaddr -- Read the MAC Address in the I2C EEPROM
 *-----------------------------------------------------------------------------
 */
void board_get_enetaddr (uchar * enet)
{
	int i;
	char buff[256], *cp;

	/* Initialize I2C					*/
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);

	/* Read 256 bytes in EEPROM				*/
	i2c_read (0x54, 0, 1, (uchar *)buff, 128);
	i2c_read (0x54, 128, 1, (uchar *)buff + 128, 128);

	/* Retrieve MAC address in buffer (key EA)		*/
	for (cp = buff;;) {
		if (cp[0] == 'E' && cp[1] == 'A') {
			cp += 3;
			/* Read MAC address			*/
			for (i = 0; i < 6; i++, cp += 2) {
				enet[i] = aschex_to_byte ((unsigned char *)cp);
			}
		}
		/* Scan to the end of the record		*/
		while ((*cp != '\n') && (*cp != (char)0xff)) {
			cp++;
		}
		/* If the next character is a \n, 0 or ff, we are done.	*/
		cp++;
		if ((*cp == '\n') || (*cp == 0) || (*cp == (char)0xff))
			break;
	}

#ifdef CONFIG_FEC_ENET
	/* The MAC address is the same as normal ethernet except the 3rd byte	 */
	/* (See the E.P. Planet Core Overview manual		*/
	enet[3] |= 0x80;
#endif

	printf ("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n",
		enet[0], enet[1], enet[2], enet[3], enet[4], enet[5]);

}

void rpxclassic_init (void)
{
	/* Enable NVRAM */
	*((uchar *) BCSR0) |= BCSR0_ENNVRAM;

#ifdef CONFIG_FEC_ENET

	/* Validate the fast ethernet tranceiver                             */
	*((volatile uchar *) BCSR2) &= ~BCSR2_MIICTL;
	*((volatile uchar *) BCSR2) &= ~BCSR2_MIIPWRDWN;
	*((volatile uchar *) BCSR2) |= BCSR2_MIIRST;
	*((volatile uchar *) BCSR2) |= BCSR2_MIIPWRDWN;
#endif

}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size10;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/* Refresh clock prescalar */
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_mar = 0x00000000;

	/* Map controller banks 1 to the SDRAM bank */
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;

	memctl->memc_mamr = CFG_MAMR_10COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002230;	/* SDRAM bank 0 - refresh twice */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE; /* enable refresh */

	udelay (1000);

	/* Check Bank 0 Memory Size
	 * try 10 column mode
	 */

	size10 = dram_size (CFG_MAMR_10COL, SDRAM_BASE_PRELIM,
						SDRAM_MAX_SIZE);

	return (size10);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}
/*-----------------------------------------------------------------------------
 * aschex_to_byte --
 *-----------------------------------------------------------------------------
 */
static unsigned char aschex_to_byte (unsigned char *cp)
{
	u_char byte, c;

	c = *cp++;

	if ((c >= 'A') && (c <= 'F')) {
		c -= 'A';
		c += 10;
	} else if ((c >= 'a') && (c <= 'f')) {
		c -= 'a';
		c += 10;
	} else {
		c -= '0';
	}

	byte = c * 16;

	c = *cp;

	if ((c >= 'A') && (c <= 'F')) {
		c -= 'A';
		c += 10;
	} else if ((c >= 'a') && (c <= 'f')) {
		c -= 'a';
		c += 10;
	} else {
		c -= '0';
	}

	byte += c;

	return (byte);
}
