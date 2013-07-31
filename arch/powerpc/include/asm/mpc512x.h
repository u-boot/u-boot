/*
 * include/asm-ppc/mpc512x.h
 *
 * Prototypes, etc. for the Freescale MPC512x embedded cpu chips
 *
 * 2009 (C) Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
extern long int fixed_sdram(ddr512x_config_t *mddrc_config,
				u32 *dram_init_seq, int seq_sz);
extern int mpc5121_diu_init(void);
extern void ide_set_reset(int idereset);

#endif /* __ASMPPC_MPC512X_H */
