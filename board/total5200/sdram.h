/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
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

typedef struct {
	ulong ddr;
	ulong mode;
	ulong emode;
	ulong control;
	ulong config1;
	ulong config2;
	ulong tapdelay;
} sdram_conf_t;

long int mpc5xxx_sdram_init (sdram_conf_t *sdram_conf);
