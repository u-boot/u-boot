/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

struct cpu_type {
	char name[15];
	u32 soc_ver;
	u32 num_cores;
};

#define CPU_TYPE_ENTRY(n, v, nc) \
	{ .name = #n, .soc_ver = SVR_##v, .num_cores = (nc)}

#define SVR_WO_E		0xFFFFFE
#define SVR_LS2045		0x870120
#define SVR_LS2080		0x870110
#define SVR_LS2085		0x870100

#define SVR_MAJ(svr)		(((svr) >> 4) & 0xf)
#define SVR_MIN(svr)		(((svr) >> 0) & 0xf)
#define SVR_SOC_VER(svr)	(((svr) >> 8) & SVR_WO_E)
#define IS_E_PROCESSOR(svr)	(!((svr >> 8) & 0x1))

void fsl_lsch3_early_init_f(void);
void cpu_name(char *name);

