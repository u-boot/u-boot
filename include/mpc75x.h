/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
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

/*
 * mpc75x.h
 *
 * MPC75x/MPC74xx specific definitions
 */

#ifndef __MPC75X_H__
#define __MPC75X_H__

/*----------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET        0x0100      /* default system reset offset */

/*----------------------------------------------------------------
 * l2cr values
 */
#define l2cr 1017

#define L2CR_L2E         0x80000000 /* bit 0 - enable */
#define L2CR_L2PE        0x40000000 /* bit 1 - data parity */
#define L2CR_L2SIZ_2M    0x00000000 /* bits 2-3 - 2MB, MPC7400 only! */
#define L2CR_L2SIZ_1M    0x30000000 /* ... 1MB */
#define L2CR_L2SIZ_HM    0x20000000 /* ... 512K */
#define L2CR_L2SIZ_QM    0x10000000 /* ... 256k */
#define L2CR_L2CLK_1     0x02000000 /* bits 4-6 clock ratio div 1 */
#define L2CR_L2CLK_1_5   0x04000000 /* bits 4-6 clock ratio div 1.5 */
#define L2CR_L2CLK_2     0x08000000 /* bits 4-6 clock ratio div 2 */
#define L2CR_L2CLK_2_5   0x0a000000 /* bits 4-6 clock ratio div 2.5 */
#define L2CR_L2CLK_3     0x0c000000 /* bits 4-6 clock ratio div 3 */
#define L2CR_L2CLK_3_5   0x06000000 /* bits 4-6 clock ratio div 3.5 */
#define L2CR_L2CLK_4     0x0e000000 /* bits 4-6 clock ratio div 4 */
#define L2CR_L2RAM_BURST 0x01000000 /* bits 7-8 - burst SRAM */
#define L2CR_DO          0x00400000 /* bit 9 - enable caching of instr. in L2 */
#define L2CR_L2I         0x00200000 /* bit 10 - global invalidate bit */
#define L2CR_L2CTL       0x00100000 /* bit 11 - l2 ram control */
#define L2CR_L2WT        0x00080000 /* bit 12 - l2 write-through */
#define L2CR_TS          0x00040000 /* bit 13 - test support on */
#define L2CR_TS_OFF      -L2CR_TS   /* bit 13 - test support off */
#define L2CR_L2OH_5      0x00000000 /* bits 14-15 - output hold time = short */
#define L2CR_L2OH_1      0x00010000 /* bits 14-15 - output hold time = medium */
#define L2CR_L2OH_INV    0x00020000 /* bits 14-15 - output hold time = long */
#define L2CR_L2IP        0x00000001 /* global invalidate in progress */

/*----------------------------------------------------------------
 * BAT settings.  Look in config_<BOARD>.h for the actual setup
 */

#define BATU_BL_128K             0x00000000
#define BATU_BL_256K             0x00000004
#define BATU_BL_512K             0x0000000c
#define BATU_BL_1M               0x0000001c
#define BATU_BL_2M               0x0000003c
#define BATU_BL_4M               0x0000007c
#define BATU_BL_8M               0x000000fc
#define BATU_BL_16M              0x000001fc
#define BATU_BL_32M              0x000003fc
#define BATU_BL_64M              0x000007fc
#define BATU_BL_128M             0x00000ffc
#define BATU_BL_256M             0x00001ffc

#define BATU_VS                  0x00000002
#define BATU_VP                  0x00000001
#define BATU_INVALID             0x00000000

#define BATL_WRITETHROUGH        0x00000080
#define BATL_CACHEINHIBIT        0x00000040
#define BATL_COHERENT            0x00000020
#define BATL_GUARDED             0x00000010

#define BATL_NO_ACCESS           0x00000000
#define BATL_RO                  0x00000001
#define BATL_RW                  0x00000002

#endif  /* __MPC75X_H__ */
