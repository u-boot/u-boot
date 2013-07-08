
/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC4xx_ISRAM_H_
#define _PPC4xx_ISRAM_H_

/*
 * Internal SRAM
 */
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_APM821XX)
#define ISRAM0_DCR_BASE 0x380
#else
#define ISRAM0_DCR_BASE 0x020
#endif
#define ISRAM0_SB0CR	(ISRAM0_DCR_BASE+0x00)	/* SRAM bank config 0*/
#define ISRAM0_SB1CR	(ISRAM0_DCR_BASE+0x01)	/* SRAM bank config 1*/
#define ISRAM0_SB2CR	(ISRAM0_DCR_BASE+0x02)	/* SRAM bank config 2*/
#define ISRAM0_SB3CR	(ISRAM0_DCR_BASE+0x03)	/* SRAM bank config 3*/
#define ISRAM0_BEAR	(ISRAM0_DCR_BASE+0x04)	/* SRAM bus error addr reg */
#define ISRAM0_BESR0	(ISRAM0_DCR_BASE+0x05)	/* SRAM bus error status reg 0 */
#define ISRAM0_BESR1	(ISRAM0_DCR_BASE+0x06)	/* SRAM bus error status reg 1 */
#define ISRAM0_PMEG	(ISRAM0_DCR_BASE+0x07)	/* SRAM power management */
#define ISRAM0_CID	(ISRAM0_DCR_BASE+0x08)	/* SRAM bus core id reg */
#define ISRAM0_REVID	(ISRAM0_DCR_BASE+0x09)	/* SRAM bus revision id reg */
#define ISRAM0_DPC	(ISRAM0_DCR_BASE+0x0a)	/* SRAM data parity check reg */

#if defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_APM821XX)
#define ISRAM1_DCR_BASE 0x0B0
#define ISRAM1_SB0CR	(ISRAM1_DCR_BASE+0x00)	/* SRAM1 bank config 0*/
#define ISRAM1_BEAR	(ISRAM1_DCR_BASE+0x04)	/* SRAM1 bus error addr reg */
#define ISRAM1_BESR0	(ISRAM1_DCR_BASE+0x05)	/* SRAM1 bus error status reg 0 */
#define ISRAM1_BESR1	(ISRAM1_DCR_BASE+0x06)	/* SRAM1 bus error status reg 1 */
#define ISRAM1_PMEG	(ISRAM1_DCR_BASE+0x07)	/* SRAM1 power management */
#define ISRAM1_CID	(ISRAM1_DCR_BASE+0x08)	/* SRAM1 bus core id reg */
#define ISRAM1_REVID	(ISRAM1_DCR_BASE+0x09)	/* SRAM1 bus revision id reg */
#define ISRAM1_DPC	(ISRAM1_DCR_BASE+0x0a)	/* SRAM1 data parity check reg */
#endif /* CONFIG_460EX || CONFIG_460GT */

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define ISRAM1_SIZE 0x0984 /* OCM size 64k */
#elif defined(CONFIG_APM821XX)
#define ISRAM1_SIZE 0x0784 /* OCM size 32k */
#endif

/*
 * L2 Cache
 */
#if defined (CONFIG_440GX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX) || defined(CONFIG_APM821XX)
#define L2_CACHE_BASE	0x030
#define L2_CACHE_CFG	(L2_CACHE_BASE+0x00)	/* L2 Cache Config      */
#define L2_CACHE_CMD	(L2_CACHE_BASE+0x01)	/* L2 Cache Command     */
#define L2_CACHE_ADDR	(L2_CACHE_BASE+0x02)	/* L2 Cache Address     */
#define L2_CACHE_DATA	(L2_CACHE_BASE+0x03)	/* L2 Cache Data        */
#define L2_CACHE_STAT	(L2_CACHE_BASE+0x04)	/* L2 Cache Status      */
#define L2_CACHE_CVER	(L2_CACHE_BASE+0x05)	/* L2 Cache Revision ID */
#define L2_CACHE_SNP0	(L2_CACHE_BASE+0x06)	/* L2 Cache Snoop reg 0 */
#define L2_CACHE_SNP1	(L2_CACHE_BASE+0x07)	/* L2 Cache Snoop reg 1 */
#endif /* CONFIG_440GX */

#endif /* _PPC4xx_ISRAM_H_ */
