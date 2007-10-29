/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 * York Sun <yorksun@freescale.com>
 *
 * FSL DIU Framebuffer driver
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
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_FSL_DIU_FB

#include "../common/pixis.h"
#include "../common/fsl_diu_fb.h"


extern unsigned int FSL_Logo_BMP[];


void mpc8610hpcd_diu_init(void)
{
	char *monitor_port;
	int xres, gamma_fix;
	unsigned int pixel_format;
	unsigned char tmp_val;

	tmp_val = in8(PIXIS_BASE + PIXIS_BRDCFG0);
	monitor_port = getenv ("monitor");

	if (!strncmp(monitor_port, "0", 1)) {	/* 0 - DVI */
		xres = 1280;
		pixel_format = 0x88882317;
		gamma_fix = 0;
		out8(PIXIS_BASE + PIXIS_BRDCFG0, tmp_val | 0x08);

	} else if (!strncmp(monitor_port, "1", 1)) { /* 1 - Single link LVDS */
		xres = 1024;
		pixel_format = 0x88883316;
		gamma_fix = 0;
		out8(PIXIS_BASE + PIXIS_BRDCFG0, (tmp_val & 0xf7) | 0x10);

	} else if (!strncmp(monitor_port, "2", 1)) { /* 2 - Double link LVDS */
		xres = 1280;
		pixel_format = 0x88883316;
		gamma_fix = 1;
		out8(PIXIS_BASE + PIXIS_BRDCFG0, tmp_val & 0xe7);

	} else {	/* DVI */
		xres = 1280;
		pixel_format = 0x88882317;
		gamma_fix = 0;
		out8(PIXIS_BASE + PIXIS_BRDCFG0, tmp_val | 0x08);
	}

	fsl_diu_init(xres, pixel_format, gamma_fix,
		     (unsigned char *)FSL_Logo_BMP);
}

int mpc8610diu_init_show_bmp(cmd_tbl_t *cmdtp,
			     int flag, int argc, char *argv[])
{
	unsigned int addr;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (!strncmp(argv[1],"init",4)) {
		mpc8610hpcd_diu_init();
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
		fsl_diu_clear_screen();
		fsl_diu_display_bmp((unsigned char *)addr, 0, 0, 0);
	}

	return 0;
}

U_BOOT_CMD(
	diufb, CFG_MAXARGS, 1, mpc8610diu_init_show_bmp,
	"diufb init | addr - Init or Display BMP file\n",
	"init\n    - initialize DIU\n"
	"addr\n    - display bmp at address 'addr'\n"
	);
#endif /* CONFIG_FSL_DIU_FB */
