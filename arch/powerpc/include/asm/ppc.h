/*
 * Ugly header containing required header files. This could  be adjusted
 * so that including asm/arch/hardware includes the correct file.
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_PPC_H
#define __ASM_PPC_H

#ifndef __ASSEMBLY__

#if defined(CONFIG_8xx)
#include <asm/8xx_immap.h>
#if defined(CONFIG_MPC859) || defined(CONFIG_MPC859T) || \
	defined(CONFIG_MPC866) ||  defined(CONFIG_MPC866P)
# define CONFIG_MPC866_FAMILY 1
#elif defined(CONFIG_MPC885)
# define CONFIG_MPC885_FAMILY   1
#endif
#if defined(CONFIG_MPC860) || defined(CONFIG_MPC860T) || \
	defined(CONFIG_MPC866_FAMILY) || defined(CONFIG_MPC885_FAMILY)
# define CONFIG_MPC86x 1
#endif
#elif defined(CONFIG_5xx)
#include <asm/5xx_immap.h>
#elif defined(CONFIG_MPC5xxx)
#include <mpc5xxx.h>
#elif defined(CONFIG_MPC512X)
#include <asm/immap_512x.h>
#elif defined(CONFIG_MPC8260)
#if defined(CONFIG_MPC8247) || defined(CONFIG_MPC8272)
#define CONFIG_MPC8272_FAMILY	1
#endif
#include <asm/immap_8260.h>
#endif
#ifdef CONFIG_MPC86xx
#include <mpc86xx.h>
#include <asm/immap_86xx.h>
#endif
#ifdef CONFIG_MPC85xx
#include <mpc85xx.h>
#include <asm/immap_85xx.h>
#endif
#ifdef CONFIG_MPC83xx
#include <mpc83xx.h>
#include <asm/immap_83xx.h>
#endif
#ifdef	CONFIG_4xx
#include <asm/ppc4xx.h>
#endif
#ifdef CONFIG_SOC_DA8XX
#include <asm/arch/hardware.h>
#endif
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#endif
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif

/*
 * enable common handling for all TQM8xxL/M boards:
 * - CONFIG_TQM8xxM will be defined for all TQM8xxM boards
 * - CONFIG_TQM8xxL will be defined for all TQM8xxL _and_ TQM8xxM boards
 *                  and for the TQM885D board
 */
#if defined(CONFIG_TQM823M) || defined(CONFIG_TQM850M) || \
	defined(CONFIG_TQM855M) || defined(CONFIG_TQM860M) || \
	defined(CONFIG_TQM862M) || defined(CONFIG_TQM866M)
# ifndef CONFIG_TQM8xxM
#  define CONFIG_TQM8xxM
# endif
#endif
#if defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L) || \
	defined(CONFIG_TQM855L) || defined(CONFIG_TQM860L) || \
	defined(CONFIG_TQM862L) || defined(CONFIG_TQM8xxM) || \
	defined(CONFIG_TQM885D)
# ifndef CONFIG_TQM8xxL
#  define CONFIG_TQM8xxL
# endif
#endif

#if defined(CONFIG_5xx) || defined(CONFIG_8xx)
uint get_immr(uint);
#endif
#if defined(CONFIG_MPC5xxx)
uint get_svr(void);
#endif
uint get_pvr(void);
uint get_svr(void);
uint rd_ic_cst(void);
void wr_ic_cst(uint);
void wr_ic_adr(uint);
uint rd_dc_cst(void);
void wr_dc_cst(uint);
void wr_dc_adr(uint);

#if defined(CONFIG_4xx)	|| \
	defined(CONFIG_MPC5xxx)	|| \
	defined(CONFIG_MPC85xx)	|| \
	defined(CONFIG_MPC86xx)	|| \
	defined(CONFIG_MPC83xx)
unsigned char	in8(unsigned int);
void		out8(unsigned int, unsigned char);
unsigned short	in16(unsigned int);
unsigned short	in16r(unsigned int);
void		out16(unsigned int, unsigned short value);
void		out16r(unsigned int, unsigned short value);
unsigned long	in32(unsigned int);
unsigned long	in32r(unsigned int);
void		out32(unsigned int, unsigned long value);
void		out32r(unsigned int, unsigned long value);
void		ppcDcbf(unsigned long value);
void		ppcDcbi(unsigned long value);
void		ppcSync(void);
void		ppcDcbz(unsigned long value);
#endif
#if defined(CONFIG_MPC83xx)
void		ppcDWload(unsigned int *addr, unsigned int *ret);
void		ppcDWstore(unsigned int *addr, unsigned int *value);
void disable_addr_trans(void);
void enable_addr_trans(void);
#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
void ddr_enable_ecc(unsigned int dram_size);
#endif
#endif

#if defined(CONFIG_MPC5xxx)
int	prt_mpc5xxx_clks(void);
#endif

#if defined(CONFIG_MPC85xx)
typedef MPC85xx_SYS_INFO sys_info_t;
void get_sys_info(sys_info_t *);
void ft_fixup_cpu(void *, u64);
void ft_fixup_num_cores(void *);
#endif
#if defined(CONFIG_MPC86xx)
ulong get_bus_freq(ulong);
typedef MPC86xx_SYS_INFO sys_info_t;
void   get_sys_info(sys_info_t *);
static inline ulong get_ddr_freq(ulong dummy)
{
	return get_bus_freq(dummy);
}
#else
ulong get_ddr_freq(ulong);
#endif

#endif /* !__ASSEMBLY__ */

#ifdef CONFIG_PPC
/*
 * Has to be included outside of the #ifndef __ASSEMBLY__ section.
 * Otherwise might lead to compilation errors in assembler files.
 */
#include <asm/cache.h>
#endif

#endif
