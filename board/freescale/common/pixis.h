/*
 * Copyright 2006 Freescale Semiconductor
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

extern void pixis_reset(void);
extern int set_px_sysclk(ulong sysclk);
extern int set_px_mpxpll(ulong mpxpll);
extern int set_px_corepll(ulong corepll);
extern void read_from_px_regs(int set);
extern void read_from_px_regs_altbank(int set);
extern void set_altbank(void);
extern void set_px_go(void);
extern void set_px_go_with_watchdog(void);
