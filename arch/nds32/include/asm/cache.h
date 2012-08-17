/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

#ifndef _ASM_CACHE_H
#define _ASM_CACHE_H

/* cache */
int	icache_status(void);
void	icache_enable(void);
void	icache_disable(void);
int	dcache_status(void);
void	dcache_enable(void);
void	dcache_disable(void);

#define DEFINE_GET_SYS_REG(reg) \
	static inline unsigned long GET_##reg(void)		\
	{							\
		unsigned long val;				\
		__asm__ volatile (				\
		"mfsr %0, $"#reg : "=&r" (val) : : "memory"	\
		);						\
		return val;					\
	}

enum cache_t {ICACHE, DCACHE};
DEFINE_GET_SYS_REG(ICM_CFG);
DEFINE_GET_SYS_REG(DCM_CFG);
#define ICM_CFG_OFF_ISZ	6	/* I-cache line size */
#define ICM_CFG_MSK_ISZ	(0x7UL << ICM_CFG_OFF_ISZ)
#define DCM_CFG_OFF_DSZ	6	/* D-cache line size */
#define DCM_CFG_MSK_DSZ	(0x7UL << DCM_CFG_OFF_DSZ)

/*
 * The current upper bound for NDS32 L1 data cache line sizes is 32 bytes.
 * We use that value for aligning DMA buffers unless the board config has
 * specified an alternate cache line size.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	32
#endif

#endif /* _ASM_CACHE_H */
