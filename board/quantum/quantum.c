/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <mpc8xx.h>
#include "fpga.h"

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
unsigned long flash_init (void);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFCC25

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 00h in UPMA RAM)
	 */
	0x0F03CC04, 0x00ACCC24, 0x1FF74C20, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Read. (Offset 08h in UPMA RAM)
	 */
	0x0F03CC04, 0x00ACCC24, 0x00FFCC20, 0x00FFCC20,
	0x01FFCC20, 0x1FF74C20, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18h in UPMA RAM)
	 */
	0x0F03CC02, 0x00AC0C24, 0x1FF74C25, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20h in UPMA RAM)
	 */
	0x0F03CC00, 0x00AC0C20, 0x00FFFC20, 0x00FFFC22,
	0x01FFFC24, 0x1FF74C25, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Refresh. (Offset 30h in UPMA RAM)
	 * (Initialization code at 0x36)
	 */
	0x0FF0CC24, 0xFFFFCC24, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, 0xEFFB8C34, 0x0FF74C34,
	0x0FFACCB4, 0x0FF5CC34, 0x0FFCC34, 0x0FFFCCB4,

	/*
	 * Exception. (Offset 3Ch in UPMA RAM)
	 */
	0x0FEA8C34, 0x1FB54C34, 0xFFFFCC34, _NOT_USED_
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	char *s = getenv ("serial#");

	puts ("Board QUANTUM, Serial No: ");

	for (; s && *s; ++s) {
		if (*s == ' ')
			break;
		putc (*s);
	}
	putc ('\n');
	return (0);		/* success */
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size9;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/* Refresh clock prescalar */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

	memctl->memc_mar = 0x00000088;

	/* Map controller banks 1 to the SDRAM bank */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_9COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002136;	/* SDRAM bank 0 */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/* Check Bank 0 Memory Size,
	 * 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, (long *) SDRAM_BASE_PRELIM,
			   SDRAM_MAX_SIZE);
	/*
	 * Final mapping:
	 */
	memctl->memc_or1 = ((-size9) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	udelay (1000);

	return (size9);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base,
			   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile ulong *addr;
	ulong cnt, val, size;
	ulong save[32];		/* to make test non-destructive */
	unsigned char i = 0;

	memctl->memc_mamr = mamr_value;

	for (cnt = maxsize / sizeof (long); cnt > 0; cnt >>= 1) {
		addr = (volatile ulong *)(base + cnt);	/* pointer arith! */

		save[i++] = *addr;
		*addr = ~cnt;
	}

	/* write 0 to base address */
	addr = (volatile ulong *)base;
	save[i] = *addr;
	*addr = 0;

	/* check at base address */
	if ((val = *addr) != 0) {
		/* Restore the original data before leaving the function.
		 */
		*addr = save[i];
		for (cnt = 1; cnt <= maxsize / sizeof (long); cnt <<= 1) {
			addr = (volatile ulong *) base + cnt;
			*addr = save[--i];
		}
		return (0);
	}

	for (cnt = 1; cnt <= maxsize / sizeof (long); cnt <<= 1) {
		addr = (volatile ulong *)(base + cnt);	/* pointer arith! */

		val = *addr;
		*addr = save[--i];

		if (val != (~cnt)) {
			size = cnt * sizeof (long);
			/* Restore the original data before returning
			 */
			for (cnt <<= 1; cnt <= maxsize / sizeof (long);
			     cnt <<= 1) {
				addr = (volatile ulong *) base + cnt;
				*addr = save[--i];
			}
			return (size);
		}
	}
	return (maxsize);
}

/*
 * Miscellaneous intialization
 */
int misc_init_r (void)
{
	char *fpga_data_str = getenv ("fpgadata");
	char *fpga_size_str = getenv ("fpgasize");
	void *fpga_data;
	int fpga_size;
	int status;
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int flash_size;

	/* Remap FLASH according to real size */
	flash_size = flash_init ();
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-flash_size & 0xFFFF8000);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_V;

	if (fpga_data_str && fpga_size_str) {
		fpga_data = (void *) simple_strtoul (fpga_data_str, NULL, 16);
		fpga_size = simple_strtoul (fpga_size_str, NULL, 10);

		status = fpga_boot (fpga_data, fpga_size);
		if (status != 0) {
			printf ("\nFPGA: Booting failed ");
			switch (status) {
			case ERROR_FPGA_PRG_INIT_LOW:
				printf ("(Timeout: INIT not low after asserting PROGRAM*)\n ");
				break;
			case ERROR_FPGA_PRG_INIT_HIGH:
				printf ("(Timeout: INIT not high after deasserting PROGRAM*)\n ");
				break;
			case ERROR_FPGA_PRG_DONE:
				printf ("(Timeout: DONE not high after programming FPGA)\n ");
				break;
			}
		}
	}
	return 0;
}
