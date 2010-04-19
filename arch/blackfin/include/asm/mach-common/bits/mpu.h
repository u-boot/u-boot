/*
 * MPU Masks
 */

#ifndef __BFIN_PERIPHERAL_MPU__
#define __BFIN_PERIPHERAL_MPU__

/*
 * DMEM_CONTROL Register
 */

/* ** Bit Positions */
#define ENDM_P			0x00	/* (doesn't really exist) Enable Data Memory L1 */
#define DMCTL_ENDM_P		ENDM_P	/* "" (older define) */
#define ENDCPLB_P		0x01	/* Enable DCPLBS */
#define DMCTL_ENDCPLB_P		ENDCPLB_P	/* "" (older define) */
#define DMC0_P			0x02	/* L1 Data Memory Configure bit 0 */
#define DMCTL_DMC0_P		DMC0_P	/* "" (older define) */
#define DMC1_P			0x03	/* L1 Data Memory Configure bit 1 */
#define DMCTL_DMC1_P		DMC1_P	/* "" (older define) */
#define DCBS_P			0x04	/* L1 Data Cache Bank Select */
#define PORT_PREF0_P		0x12	/* DAG0 Port Preference */
#define PORT_PREF1_P		0x13	/* DAG1 Port Preference */

/* ** Masks */
#define ENDM			0x00000001	/* (doesn't really exist) Enable Data Memory L1 */
#define ENDCPLB			0x00000002	/* Enable DCPLB */
#define ASRAM_BSRAM		0x00000000
#define ACACHE_BSRAM		0x00000008
#define ACACHE_BCACHE		0x0000000C
#define DCBS			0x00000010	/*  L1 Data Cache Bank Select */
#define PORT_PREF0		0x00001000	/* DAG0 Port Preference */
#define PORT_PREF1		0x00002000	/* DAG1 Port Preference */

/* IMEM_CONTROL Register */
/* ** Bit Positions */
#define ENIM_P			0x00	/* Enable L1 Code Memory */
#define IMCTL_ENIM_P		0x00	/* "" (older define) */
#define ENICPLB_P		0x01	/* Enable ICPLB */
#define IMCTL_ENICPLB_P		0x01	/* "" (older define) */
#define IMC_P			0x02	/* Enable */
#define IMCTL_IMC_P		0x02	/* Configure L1 code memory as cache (0=SRAM) */
#define ILOC0_P			0x03	/* Lock Way 0 */
#define ILOC1_P			0x04	/* Lock Way 1 */
#define ILOC2_P			0x05	/* Lock Way 2 */
#define ILOC3_P			0x06	/* Lock Way 3 */
#define LRUPRIORST_P		0x0D	/* Least Recently Used Replacement Priority */

/* ** Masks */
#define ENIM			0x00000001	/* Enable L1 Code Memory */
#define ENICPLB			0x00000002	/* Enable ICPLB */
#define IMC			0x00000004	/* Configure L1 code memory as cache (0=SRAM) */
#define ILOC0			0x00000008	/* Lock Way 0 */
#define ILOC1			0x00000010	/* Lock Way 1 */
#define ILOC2			0x00000020	/* Lock Way 2 */
#define ILOC3			0x00000040	/* Lock Way 3 */
#define LRUPRIORST		0x00002000	/* Least Recently Used Replacement Priority */

/* DCPLB_DATA and ICPLB_DATA Registers */
/* ** Bit Positions */
#define CPLB_VALID_P		0x00000000	/* 0=invalid entry, 1=valid entry */
#define CPLB_LOCK_P		0x00000001	/* 0=entry may be replaced, 1=entry locked */
#define CPLB_USER_RD_P		0x00000002	/* 0=no read access, 1=read access allowed (user mode) */

/* ** Masks */
#define CPLB_VALID		0x00000001	/* 0=invalid entry, 1=valid entry */
#define CPLB_LOCK		0x00000002	/* 0=entry may be replaced, 1=entry locked */
#define CPLB_USER_RD		0x00000004	/* 0=no read access, 1=read access allowed (user mode) */
#define PAGE_SIZE_1KB		0x00000000	/* 1 KB page size */
#define PAGE_SIZE_4KB		0x00010000	/* 4 KB page size */
#define PAGE_SIZE_1MB		0x00020000	/* 1 MB page size */
#define PAGE_SIZE_4MB		0x00030000	/* 4 MB page size */
#define PAGE_SIZE_MASK		0x00030000	/* the bits for the page_size field */
#define PAGE_SIZE_SHIFT		16
#define CPLB_L1SRAM		0x00000020	/* 0=SRAM mapped in L1, 0=SRAM not mapped to L1 */
#define CPLB_PORTPRIO		0x00000200	/* 0=low priority port, 1= high priority port */
#define CPLB_L1_CHBL		0x00001000	/* 0=non-cacheable in L1, 1=cacheable in L1 */

/* ICPLB_DATA only */
#define CPLB_LRUPRIO		0x00000100	/* 0=can be replaced by any line, 1=priority for non-replacement */

/* DCPLB_DATA only */
#define CPLB_USER_WR		0x00000008	/* 0=no write access, 0=write access allowed (user mode) */
#define CPLB_SUPV_WR		0x00000010	/* 0=no write access, 0=write access allowed (supervisor mode) */
#define CPLB_DIRTY		0x00000080	/* 1=dirty, 0=clean */
#define CPLB_L1_AOW		0x00008000	/* 0=do not allocate cache lines on write-through writes, */
						/* 1= allocate cache lines on write-through writes. */
#define CPLB_WT			0x00004000	/* 0=write-back, 1=write-through */

/* ITEST_COMMAND and DTEST_COMMAND Registers */
/* ** Masks */
#define TEST_READ		0x00000000	/* Read Access */
#define TEST_WRITE		0x00000002	/* Write Access */
#define TEST_TAG		0x00000000	/* Access TAG */
#define TEST_DATA		0x00000004	/* Access DATA */
#define TEST_DW0		0x00000000	/* Select Double Word 0 */
#define TEST_DW1		0x00000008	/* Select Double Word 1 */
#define TEST_DW2		0x00000010	/* Select Double Word 2 */
#define TEST_DW3		0x00000018	/* Select Double Word 3 */
#define TEST_MB0		0x00000000	/* Select Mini-Bank 0 */
#define TEST_MB1		0x00010000	/* Select Mini-Bank 1 */
#define TEST_MB2		0x00020000	/* Select Mini-Bank 2 */
#define TEST_MB3		0x00030000	/* Select Mini-Bank 3 */
#define TEST_SET(x)		((x << 5) & 0x03E0)	/* Set Index 0->31 */
#define TEST_WAY0		0x00000000	/* Access Way0 */
#define TEST_WAY1		0x04000000	/* Access Way1 */

/* ** ITEST_COMMAND only */
#define TEST_WAY2		0x08000000	/* Access Way2 */
#define TEST_WAY3		0x0C000000	/* Access Way3 */

/* ** DTEST_COMMAND only */
#define TEST_BNKSELA		0x00000000	/* Access SuperBank A */
#define TEST_BNKSELB		0x00800000	/* Access SuperBank B */

#endif
