/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
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

#ifndef _INIT_WRAPPERS_H_
#define _INIT_WRAPPERS_H_

int serial_initialize_r(void);
int env_relocate_r(void);
int pci_init_r(void);
int jumptable_init_r(void);
int pcmcia_init_r(void);
int kgdb_init_r(void);
int enable_interrupts_r(void);
int eth_initialize_r(void);
int reset_phy_r(void);
int ide_init_r(void);
int scsi_init_r(void);
int doc_init_r(void);
int bb_miiphy_init_r(void);
int post_run_r(void);

#endif	/* !_INIT_WRAPPERS_H_ */
