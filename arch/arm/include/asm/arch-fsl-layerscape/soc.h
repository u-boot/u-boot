/*
 * Copyright 2017 NXP
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#define SVR_DEV_LS2080A		0x8701

#define SVR_MAJ(svr)		(((svr) >> 4) & 0xf)
#define SVR_MIN(svr)		(((svr) >> 0) & 0xf)
#define SVR_REV(svr)		(((svr) >> 0) & 0xff)
#define SVR_SOC_VER(svr)	(((svr) >> 8) & SVR_WO_E)
#define IS_E_PROCESSOR(svr)	(!((svr >> 8) & 0x1))
#define IS_SVR_REV(svr, maj, min) \
		((SVR_MAJ(svr) == (maj)) && (SVR_MIN(svr) == (min)))

/* ahci port register default value */
#define AHCI_PORT_PHY_1_CFG    0xa003fffe
#define AHCI_PORT_TRANS_CFG    0x08000029
#define AHCI_PORT_AXICC_CFG	0x3fffffff

#ifndef __ASSEMBLY__
/* AHCI (sata) register map */
struct ccsr_ahci {
	u32 res1[0xa4/4];	/* 0x0 - 0xa4 */
	u32 pcfg;	/* port config */
	u32 ppcfg;	/* port phy1 config */
	u32 pp2c;	/* port phy2 config */
	u32 pp3c;	/* port phy3 config */
	u32 pp4c;	/* port phy4 config */
	u32 pp5c;	/* port phy5 config */
	u32 axicc;	/* AXI cache control */
	u32 paxic;	/* port AXI config */
	u32 axipc;	/* AXI PROT control */
	u32 ptc;	/* port Trans Config */
	u32 pts;	/* port Trans Status */
	u32 plc;	/* port link config */
	u32 plc1;	/* port link config1 */
	u32 plc2;	/* port link config2 */
	u32 pls;	/* port link status */
	u32 pls1;	/* port link status1 */
	u32 pcmdc;	/* port CMD config */
	u32 ppcs;	/* port phy control status */
	u32 pberr;	/* port 0/1 BIST error */
	u32 cmds;	/* port 0/1 CMD status error */
};

#ifdef CONFIG_FSL_LSCH3
void fsl_lsch3_early_init_f(void);
#elif defined(CONFIG_FSL_LSCH2)
void fsl_lsch2_early_init_f(void);
int setup_chip_volt(void);
/* Setup core vdd in unit mV */
int board_setup_core_volt(u32 vdd);
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
