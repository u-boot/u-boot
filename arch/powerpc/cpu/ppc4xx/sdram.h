/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * DAVE Srl <www.dave-tech.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SDRAM_H_
#define _SDRAM_H_

#include <config.h>

#define ONE_BILLION	1000000000

struct sdram_conf_s {
	unsigned long size;
	int rows;
	unsigned long reg;
};

typedef struct sdram_conf_s sdram_conf_t;

/* Bitfields offsets */
#define SDRAM0_TR_CASL		(31 - 8)
#define SDRAM0_TR_PTA		(31 - 13)
#define SDRAM0_TR_CTP		(31 - 15)
#define SDRAM0_TR_LDF		(31 - 17)
#define SDRAM0_TR_RFTA		(31 - 29)
#define SDRAM0_TR_RCD		(31 - 31)

#ifdef CONFIG_SYS_SDRAM_CL
/* SDRAM timings [ns] according to AMCC/IBM names (see SDRAM_faq.doc) */
#define CONFIG_SYS_SDRAM_CASL		CONFIG_SYS_SDRAM_CL
#define CONFIG_SYS_SDRAM_PTA		CONFIG_SYS_SDRAM_tRP
#define CONFIG_SYS_SDRAM_CTP		(CONFIG_SYS_SDRAM_tRC - CONFIG_SYS_SDRAM_tRCD - CONFIG_SYS_SDRAM_tRP)
#define CONFIG_SYS_SDRAM_LDF		0
#ifdef CONFIG_SYS_SDRAM_tRFC
#define CONFIG_SYS_SDRAM_RFTA		CONFIG_SYS_SDRAM_tRFC
#else
#define CONFIG_SYS_SDRAM_RFTA		CONFIG_SYS_SDRAM_tRC
#endif
#define CONFIG_SYS_SDRAM_RCD		CONFIG_SYS_SDRAM_tRCD
#endif /* #ifdef CONFIG_SYS_SDRAM_CL */

/*
 * Some defines for the 440 DDR controller
 */
#define SDRAM_CFG0_DC_EN	0x80000000	/* SDRAM Controller Enable	*/
#define SDRAM_CFG0_MEMCHK	0x30000000	/* Memory data error checking mask*/
#define SDRAM_CFG0_MEMCHK_NON	0x00000000	/* No ECC generation		*/
#define SDRAM_CFG0_MEMCHK_GEN	0x20000000	/* ECC generation		*/
#define SDRAM_CFG0_MEMCHK_CHK	0x30000000	/* ECC generation and checking	*/
#define SDRAM_CFG0_DRAMWDTH	0x02000000	/* DRAM width mask		*/
#define SDRAM_CFG0_DRAMWDTH_32	0x00000000	/* 32 bits			*/
#define SDRAM_CFG0_DRAMWDTH_64	0x02000000	/* 64 bits			*/

#endif
