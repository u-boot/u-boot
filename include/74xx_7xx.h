/*
 * (C) Copyright 2001
 * Josh Huber, Mission Critical Linux, Inc. <huber@mclx.com>
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
 * 74xx_7xx.h
 *
 * 74xx/7xx specific definitions
 */

#ifndef __MPC74XX_H__
#define __MPC74XX_H__

/*----------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET        0x0100      /* default system reset offset */
#define _START_OFFSET		EXC_OFF_SYS_RESET

/*----------------------------------------------------------------
 * l2cr values
 */
#define l2cr		 1017

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

#define BATU_BL_128K            0x00000000
#define BATU_BL_256K            0x00000004
#define BATU_BL_512K            0x0000000c
#define BATU_BL_1M              0x0000001c
#define BATU_BL_2M              0x0000003c
#define BATU_BL_4M              0x0000007c
#define BATU_BL_8M              0x000000fc
#define BATU_BL_16M             0x000001fc
#define BATU_BL_32M             0x000003fc
#define BATU_BL_64M             0x000007fc
#define BATU_BL_128M            0x00000ffc
#define BATU_BL_256M            0x00001ffc

#define BATU_VS                 0x00000002
#define BATU_VP                 0x00000001
#define BATU_INVALID            0x00000000

#define BATL_WRITETHROUGH       0x00000040
#define BATL_CACHEINHIBIT       0x00000020
#define BATL_MEMCOHERENCE	0x00000010
#define BATL_GUARDEDSTORAGE     0x00000008
#define BATL_NO_ACCESS		0x00000000

#define BATL_PP_MSK		0x00000003
#define BATL_PP_00		0x00000000 /* No access */
#define BATL_PP_01		0x00000001 /* Read-only */
#define BATL_PP_10		0x00000002 /* Read-write */
#define BATL_PP_11		0x00000003

#define BATL_PP_NO_ACCESS	BATL_PP_00
#define BATL_PP_RO		BATL_PP_01
#define BATL_PP_RW		BATL_PP_10

#ifndef __ASSEMBLY__
/* cpu ids we detect */
typedef enum __cpu_t {
	CPU_740, CPU_750,
	CPU_740P, CPU_750P,
	CPU_745, CPU_755,
	CPU_750CX, CPU_750FX, CPU_750GX,
	CPU_7400,
	CPU_7410,
	CPU_7447A, CPU_7448,
	CPU_7450, CPU_7455, CPU_7457,
	CPU_UNKNOWN} cpu_t;

extern cpu_t get_cpu_type(void);

#define l1icache_enable	icache_enable

void l2cache_enable(void);
void l1dcache_enable(void);

static __inline__ unsigned long get_msr (void)
{
	unsigned long msr;
	asm volatile("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void set_msr (unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}

static __inline__ unsigned long get_hid0 (void)
{
	unsigned long hid0;
	asm volatile("mfspr %0, 1008" : "=r" (hid0) :);
	return hid0;
}

static __inline__ unsigned long get_hid1 (void)
{
	unsigned long hid1;
	asm volatile("mfspr %0, 1009" : "=r" (hid1) :);
	return hid1;
}

static __inline__ void set_hid0 (unsigned long hid0)
{
	asm volatile("mtspr 1008, %0" : : "r" (hid0));
}

static __inline__ void set_hid1 (unsigned long hid1)
{
	asm volatile("mtspr 1009, %0" : : "r" (hid1));
}

#endif	/* __ASSEMBLY__ */
#endif  /* __MPC74XX_H__ */
