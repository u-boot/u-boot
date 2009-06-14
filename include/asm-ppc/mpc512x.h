/*
 * include/asm-ppc/mpc512x.h
 *
 * Prototypes, etc. for the Freescale MPC512x embedded cpu chips
 *
 * 2009 (C) Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#ifndef __ASMPPC_MPC512X_H
#define __ASMPPC_MPC512X_H

/*
 * macros for manipulating CSx_START/STOP
 */
#define CSAW_START(start)	((start) & 0xFFFF0000)
#define CSAW_STOP(start, size)	(((start) + (size) - 1) >> 16)

/*
 * Inlines
 */

/*
 * According to MPC5121e RM, configuring local access windows should
 * be followed by a dummy read of the config register that was
 * modified last and an isync.
 */
static inline void sync_law(volatile void *addr)
{
	in_be32(addr);
	__asm__ __volatile__ ("isync");
}

/*
 * Prototypes
 */
extern long int fixed_sdram(void);
extern int mpc5121_diu_init(void);
extern void ide_set_reset(int idereset);

#endif /* __ASMPPC_MPC512X_H */
