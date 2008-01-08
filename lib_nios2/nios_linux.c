/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
#include <asm/byteorder.h>

extern image_header_t header;	/* common/cmd_bootm.c */

void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		ulong addr, ulong *len_ptr, int   verify)
{
	image_header_t *hdr = &header;
	void (*kernel)(void) = (void (*)(void))image_get_ep (hdr);

	/* For now we assume the Microtronix linux ... which only
	 * needs to be called ;-)
	 */
	kernel ();
}
