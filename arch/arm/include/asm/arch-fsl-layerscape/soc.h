/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2015 Freescale Semiconductor
 */

#ifndef _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_
#define _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_

#ifndef __ASSEMBLY__
#include <linux/types.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#endif
#endif

#ifdef CONFIG_SYS_FSL_CCSR_GUR_LE
#define gur_in32(a)       in_le32(a)
#define gur_out32(a, v)   out_le32(a, v)
#elif defined(CONFIG_SYS_FSL_CCSR_GUR_BE)
#define gur_in32(a)       in_be32(a)
#define gur_out32(a, v)   out_be32(a, v)
#endif

#ifdef CONFIG_SYS_FSL_CCSR_SCFG_LE
#define scfg_in32(a)       in_le32(a)
#define scfg_out32(a, v)   out_le32(a, v)
#define scfg_clrbits32(addr, clear) clrbits_le32(addr, clear)
#define scfg_clrsetbits32(addr, clear, set) clrsetbits_le32(addr, clear, set)
#elif defined(CONFIG_SYS_FSL_CCSR_SCFG_BE)
#define scfg_in32(a)       in_be32(a)
#define scfg_out32(a, v)   out_be32(a, v)
#define scfg_clrbits32(addr, clear) clrbits_be32(addr, clear)
#define scfg_clrsetbits32(addr, clear, set) clrsetbits_be32(addr, clear, set)
#endif

#ifdef CONFIG_SYS_FSL_PEX_LUT_LE
#define pex_lut_in32(a)       in_le32(a)
#define pex_lut_out32(a, v)   out_le32(a, v)
#elif defined(CONFIG_SYS_FSL_PEX_LUT_BE)
#define pex_lut_in32(a)       in_be32(a)
#define pex_lut_out32(a, v)   out_be32(a, v)
#endif
#ifndef __ASSEMBLY__
struct cpu_type {
	char name[15];
	u32 soc_ver;
	u32 num_cores;
};

#define CPU_TYPE_ENTRY(n, v, nc) \
	{ .name = #n, .soc_ver = SVR_##v, .num_cores = (nc)}
#endif
#define SVR_WO_E		0xFFFFFE
#define SVR_LS1012A		0x870400
#define SVR_LS1043A		0x879200
#define SVR_LS1023A		0x879208
#define SVR_LS1046A		0x870700
#define SVR_LS1026A		0x870708
#define SVR_LS1048A		0x870320
#define SVR_LS1084A		0x870302
#define SVR_LS1088A		0x870300
#define SVR_LS1044A		0x870322
#define SVR_LS2045A		0x870120
#define SVR_LS2080A		0x870110
#define SVR_LS2085A		0x870100
#define SVR_LS2040A		0x870130
#define SVR_LS2088A		0x870900
#define SVR_LS2084A		0x870910
#define SVR_LS2048A		0x870920
#define SVR_LS2044A		0x870930
#define SVR_LS2081A		0x870918
#define SVR_LS2041A		0x870914

#define SVR_MAJ(svr)		(((svr) >> 4) & 0xf)
#define SVR_MIN(svr)		(((svr) >> 0) & 0xf)
#define SVR_REV(svr)		(((svr) >> 0) & 0xff)
#define SVR_SOC_VER(svr)	(((svr) >> 8) & SVR_WO_E)
#define IS_E_PROCESSOR(svr)	(!((svr >> 8) & 0x1))
#define IS_SVR_REV(svr, maj, min) \
		((SVR_MAJ(svr) == (maj)) && (SVR_MIN(svr) == (min)))
#define SVR_DEV(svr)		((svr) >> 8)
#define IS_SVR_DEV(svr, dev)	(((svr) >> 16) == (dev))

#ifndef __ASSEMBLY__
#ifdef CONFIG_FSL_LSCH3
void fsl_lsch3_early_init_f(void);
int get_core_volt_from_fuse(void);
#elif defined(CONFIG_FSL_LSCH2)
void fsl_lsch2_early_init_f(void);
int setup_chip_volt(void);
/* Setup core vdd in unit mV */
int board_setup_core_volt(u32 vdd);
#ifdef CONFIG_FSL_PFE
void init_pfe_scfg_dcfg_regs(void);
#endif
#endif
#ifdef CONFIG_QSPI_AHB_INIT
int qspi_ahb_init(void);
#endif

void cpu_name(char *name);
#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
void erratum_a009635(void);
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
void erratum_a010315(void);
#endif

bool soc_has_dp_ddr(void);
bool soc_has_aiop(void);
#endif

#endif /* _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_ */
