/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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

#ifndef	__ASM_SH_GLOBALDATA_H_
#define __ASM_SH_GLOBALDATA_H_

typedef	struct global_data
{
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	cpu_clk;	/* CPU clock in Hz! */
	unsigned long	have_console;	/* serial_init() was called */
	unsigned long	ram_size;	/* RAM size */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	env_addr;	/* Address  of Environment struct */
	unsigned long	env_valid;	/* Checksum of Environment valid */
	void		**jt;		/* Standalone app jump table */
}gd_t;

#define	GD_FLG_RELOC	0x00001		/* Code was relocated to RAM		*/
#define	GD_FLG_DEVINIT	0x00002		/* Devices have been initialized	*/
#define	GD_FLG_SILENT	0x00004		/* Silent mode				*/
#define	GD_FLG_POSTFAIL	0x00008		/* Critical POST test failed		*/

#define DECLARE_GLOBAL_DATA_PTR	register gd_t *gd asm ("r13")

#endif /* __ASM_SH_GLOBALDATA_H_ */
