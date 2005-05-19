/*
 * (C) Copyright 2005
 * Greg Ungerer, OpenGear Inc, <greg.ungerer@opengear.com>
 *
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

#include <common.h>
#include <asm/arch/platform.h>

/* ------------------------------------------------------------------------- */

#define	ks8695_read(a)	  *((volatile unsigned int *) (KS8695_IO_BASE+(a)))
#define	ks8695_write(a,b) *((volatile unsigned int *) (KS8695_IO_BASE+(a))) = (b)

/* ------------------------------------------------------------------------- */


/*
 * Miscelaneous platform dependent initialisations
 */
int env_flash_cmdline (void)
{
	unsigned char *sp = (unsigned char *) 0x0201c020;
	unsigned char *ep;
	int len;

	/* Check if "erase" push button is depressed */
	if ((ks8695_read(KS8695_GPIO_DATA) & 0x8) == 0) {
		printf("### Entering network recovery mode...\n");
		setenv("bootargs", "console=ttyAM0,115200 mem=32M initrd=0x400000,8M root=/dev/ram0");
		setenv("bootcmd", "bootp 0x400000; gofsk 0x400000");
		setenv("bootdelay", "2");
		return 0;
	}

	/* Check for flash based kernel boot args to use as default */
	for (ep = sp, len = 0; ((len < 1024) && (*ep != 0)); ep++, len++)
		;

	if ((len > 0) && (len <1024))
		setenv("bootargs", sp);

	return 0;
}

int board_late_init (void)
{
	return 0;
}


int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* arch number of CM41xx */
	gd->bd->bi_arch_number = 672;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	/* power down all but port 0 on the switch */
	ks8695_write(KS8695_SWITCH_LPPM12, 0x00000005);
	ks8695_write(KS8695_SWITCH_LPPM34, 0x00050005);

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

	return (0);
}
