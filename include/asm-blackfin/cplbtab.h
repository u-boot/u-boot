/*This file is subject to the terms and conditions of the GNU General Public
 * License.
 *
 * Blackfin BF533/2.6 support : LG Soft India
 * Updated : Ashutosh Singh / Jahid Khan : Rrap Software Pvt Ltd
 * Updated : 1. SDRAM_KERNEL, SDRAM_DKENEL are added as initial cplb's
 *	        shouldn't be victimized. cplbmgr.S search logic is corrected
 *	        to findout the appropriate victim.
 *	     2. SDRAM_IGENERIC in dpdt_table is replaced with SDRAM_DGENERIC
 *	     : LG Soft India
 */
#include <config.h>

#ifndef __ARCH_BFINNOMMU_CPLBTAB_H
#define __ARCH_BFINNOMMU_CPLBTAB_H

/*************************************************************************
 *  			ICPLB TABLE
 *************************************************************************/

.data

/* This table is configurable */

.align 4;

/* Data Attibutes*/

#define SDRAM_IGENERIC		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID)
#define SDRAM_IKERNEL		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID | CPLB_LOCK)
#define L1_IMEMORY            	(PAGE_SIZE_1MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_VALID | CPLB_LOCK)
#define SDRAM_INON_CHBL		(PAGE_SIZE_4MB | CPLB_USER_RD | CPLB_VALID)

/*Use the menuconfig cache policy here - CONFIG_BLKFIN_WT/CONFIG_BLKFIN_WB*/

#define ANOMALY_05000158		0x200
#ifdef CONFIG_BLKFIN_WB 	/*Write Back Policy */
	#define SDRAM_DGENERIC  	(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_DIRTY | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_USER_RD | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_DNON_CHBL         (PAGE_SIZE_4MB | CPLB_DIRTY | CPLB_SUPV_WR | CPLB_USER_RD | CPLB_USER_WR | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_DKERNEL 		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_USER_RD | CPLB_USER_WR | CPLB_DIRTY | CPLB_SUPV_WR | CPLB_VALID | CPLB_LOCK | ANOMALY_05000158)
	#define L1_DMEMORY		(PAGE_SIZE_4KB | CPLB_L1_CHBL | CPLB_DIRTY | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_USER_RD | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_EBIU		(PAGE_SIZE_1MB | CPLB_DIRTY | CPLB_USER_RD | CPLB_USER_WR | CPLB_SUPV_WR | CPLB_VALID | ANOMALY_05000158)

#else  /*Write Through*/
	#define SDRAM_DGENERIC 		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_WT | CPLB_L1_AOW | CPLB_SUPV_WR | CPLB_USER_RD | CPLB_USER_WR | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_DNON_CHBL         (PAGE_SIZE_4MB | CPLB_WT | CPLB_L1_AOW | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_USER_RD | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_DKERNEL 		(PAGE_SIZE_4MB | CPLB_L1_CHBL | CPLB_WT | CPLB_L1_AOW | CPLB_USER_RD | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_VALID | CPLB_LOCK | ANOMALY_05000158)
	#define L1_DMEMORY		(PAGE_SIZE_4KB | CPLB_L1_CHBL | CPLB_L1_AOW | CPLB_WT | CPLB_SUPV_WR | CPLB_USER_WR | CPLB_VALID | ANOMALY_05000158)
	#define SDRAM_EBIU		(PAGE_SIZE_1MB | CPLB_WT | CPLB_L1_AOW | CPLB_USER_RD | CPLB_USER_WR | CPLB_SUPV_WR | CPLB_VALID | ANOMALY_05000158)
#endif

.global icplb_table
icplb_table:
.byte4 0xFFA00000;
.byte4 (L1_IMEMORY);
.byte4 0x00000000;
.byte4 (SDRAM_IKERNEL);			/*SDRAM_Page1*/
.byte4 0x00400000;
.byte4 (SDRAM_IKERNEL);		/*SDRAM_Page1*/
.byte4 0x07C00000;
.byte4 (SDRAM_IKERNEL);		/*SDRAM_Page14*/
.byte4 0x00800000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page2*/
.byte4 0x00C00000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page2*/
.byte4 0x01000000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page4*/
.byte4 0x01400000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page5*/
.byte4 0x01800000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page6*/
.byte4 0x01C00000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page7*/
#ifndef CONFIG_EZKIT			/*STAMP Memory regions*/
.byte4 0x02000000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page8*/
.byte4 0x02400000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page9*/
.byte4 0x02800000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page10*/
.byte4 0x02C00000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page11*/
.byte4 0x03000000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page12*/
.byte4 0x03400000;
.byte4 (SDRAM_IGENERIC);		/*SDRAM_Page13*/
#endif
.byte4 0xffffffff;			/* end of section - termination*/

.align 4;
.global ipdt_table
ipdt_table:
#ifdef CONFIG_CPLB_INFO
.byte4 0x00000000;
.byte4 (SDRAM_IKERNEL);               /*SDRAM_Page0*/
.byte4 0x00400000;
.byte4 (SDRAM_IKERNEL);               /*SDRAM_Page1*/
#endif
.byte4 0x00800000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page2*/
.byte4 0x00C00000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page3*/
.byte4 0x01000000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page4*/
.byte4 0x01400000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page5*/
.byte4 0x01800000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page6*/
.byte4 0x01C00000;
.byte4 (SDRAM_IGENERIC);              /*SDRAM_Page7*/
#ifndef CONFIG_EZKIT                  /*STAMP Memory regions*/
.byte4  0x02000000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page8*/
.byte4  0x02400000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page9*/
.byte4  0x02800000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page10*/
.byte4  0x02C00000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page11*/
.byte4  0x03000000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page12*/
.byte4  0x03400000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page13*/
.byte4  0x03800000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page14*/
.byte4  0x03C00000;
.byte4  (SDRAM_IGENERIC);              /*SDRAM_Page15*/
#endif
.byte4  0x20200000;
.byte4  (SDRAM_EBIU);      /* Async Memory Bank 2 (Secnd)*/
.byte4  0x20100000;
.byte4  (SDRAM_EBIU);      /* Async Memory Bank 1 (Prim B)*/
.byte4  0x20000000;
.byte4  (SDRAM_EBIU);      /* Async Memory Bank 0 (Prim A)*/
.byte4  0x20300000;             /*Fix for Network*/
.byte4  (SDRAM_EBIU);    /*Async Memory bank 3*/

#ifdef CONFIG_STAMP
.byte4        0x04000000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x04400000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x04800000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x04C00000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x05000000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x05400000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x05800000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x05C00000;
.byte4  (SDRAM_IGENERIC);
.byte4        0x06000000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page25*/
.byte4        0x06400000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page26*/
.byte4        0x06800000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page27*/
.byte4        0x06C00000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page28*/
.byte4        0x07000000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page29*/
.byte4        0x07400000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page30*/
.byte4        0x07800000;
.byte4  (SDRAM_IGENERIC);       /*SDRAM_Page31*/
#ifdef CONFIG_CPLB_INFO
.byte4        0x07C00000;
.byte4  (SDRAM_IKERNEL);        /*SDRAM_Page32*/
#endif
#endif
.byte4 0xffffffff;                    /* end of section - termination*/

/*********************************************************************
 *			DCPLB TABLE
 ********************************************************************/

.global dcplb_table
dcplb_table:
.byte4	0x00000000;
.byte4	(SDRAM_DKERNEL);	/*SDRAM_Page1*/
.byte4	0x00400000;
.byte4	(SDRAM_DKERNEL);	/*SDRAM_Page1*/
.byte4	0x07C00000;
.byte4	(SDRAM_DKERNEL);	/*SDRAM_Page15*/
.byte4	0x00800000;
.byte4 	(SDRAM_DGENERIC);	/*SDRAM_Page2*/
.byte4 	0x00C00000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page3*/
.byte4	0x01000000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page4*/
.byte4	0x01400000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page5*/
.byte4	0x01800000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page6*/
.byte4	0x01C00000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page7*/
#ifndef CONFIG_EZKIT
.byte4	0x02000000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page8*/
.byte4	0x02400000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page9*/
.byte4	0x02800000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page10*/
.byte4	0x02C00000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page11*/
.byte4	0x03000000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page12*/
.byte4	0x03400000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page13*/
.byte4	0x03800000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page14*/
#endif
.byte4	0xffffffff;		/*end of section - termination*/

/**********************************************************************
 *		PAGE DESCRIPTOR TABLE
 *
 **********************************************************************/

/* Till here we are discussing about the static memory management model.
 * However, the operating envoronments commonly define more CPLB
 * descriptors to cover the entire addressable memory than will fit into
 * the available on-chip 16 CPLB MMRs. When this happens, the below table
 * will be used which will hold all the potentially required CPLB descriptors
 *
 * This is how Page descriptor Table is implemented in uClinux/Blackfin.
 */
.global dpdt_table
dpdt_table:
#ifdef CONFIG_CPLB_INFO
.byte4        0x00000000;
.byte4        (SDRAM_DKERNEL);        /*SDRAM_Page0*/
.byte4        0x00400000;
.byte4        (SDRAM_DKERNEL);        /*SDRAM_Page1*/
#endif
.byte4        0x00800000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page2*/
.byte4        0x00C00000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page3*/
.byte4        0x01000000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page4*/
.byte4        0x01400000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page5*/
.byte4        0x01800000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page6*/
.byte4        0x01C00000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page7*/

#ifndef CONFIG_EZKIT
.byte4        0x02000000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page8*/
.byte4        0x02400000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page9*/
.byte4        0x02800000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page10*/
.byte4        0x02C00000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page11*/
.byte4        0x03000000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page12*/
.byte4        0x03400000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page13*/
.byte4        0x03800000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page14*/
.byte4        0x03C00000;
.byte4        (SDRAM_DGENERIC);       /*SDRAM_Page15*/
#endif
.byte4	0x20200000;
.byte4	(SDRAM_EBIU);	/* Async Memory Bank 2 (Secnd)*/
.byte4	0x20100000;
.byte4	(SDRAM_EBIU);	/* Async Memory Bank 1 (Prim B)*/
.byte4	0x20000000;
.byte4	(SDRAM_EBIU);	/* Async Memory Bank 0 (Prim A)*/
.byte4	0x20300000;		/*Fix for Network*/
.byte4  (SDRAM_EBIU);	/*Async Memory bank 3*/

#ifdef CONFIG_STAMP
.byte4	0x04000000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x04400000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x04800000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x04C00000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x05000000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x05400000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x05800000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x05C00000;
.byte4  (SDRAM_DGENERIC);
.byte4	0x06000000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page25*/
.byte4	0x06400000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page26*/
.byte4	0x06800000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page27*/
.byte4	0x06C00000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page28*/
.byte4	0x07000000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page29*/
.byte4	0x07400000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page30*/
.byte4	0x07800000;
.byte4	(SDRAM_DGENERIC);	/*SDRAM_Page31*/
#ifdef CONFIG_CPLB_INFO
.byte4	0x07C00000;
.byte4	(SDRAM_DKERNEL);	/*SDRAM_Page32*/
#endif
#endif

.byte4  0xFF900000;
.byte4  (L1_DMEMORY);
.byte4  0xFF901000;
.byte4  (L1_DMEMORY);
.byte4  0xFF902000;
.byte4  (L1_DMEMORY);
.byte4  0xFF903000;
.byte4  (L1_DMEMORY);
.byte4  0xFF904000;
.byte4  (L1_DMEMORY);
.byte4  0xFF905000;
.byte4  (L1_DMEMORY);
.byte4  0xFF906000;
.byte4  (L1_DMEMORY);
.byte4  0xFF907000;
.byte4  (L1_DMEMORY);
.byte4  0xFF800000;
.byte4  (L1_DMEMORY);
.byte4  0xFF801000;
.byte4  (L1_DMEMORY);
.byte4  0xFF802000;
.byte4  (L1_DMEMORY);
.byte4  0xFF803000;
.byte4  (L1_DMEMORY);

.byte4	0xffffffff;		/*end of section - termination*/

#ifdef CONFIG_CPLB_INFO
.global ipdt_swapcount_table;	/* swapin count first, then swapout count*/
ipdt_swapcount_table:
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 10 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 20 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 30 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 40 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 50 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 60 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 70 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 80 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 90 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 100 */

.global dpdt_swapcount_table;	/* swapin count first, then swapout count*/
dpdt_swapcount_table:
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 10 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 20 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 30 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 40 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 50 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 60 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 70 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 80 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 80 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 100 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 110 */
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;
.byte4        0x00000000;	/* 120 */

#endif

#endif	/*__ARCH_BFINNOMMU_CPLBTAB_H*/
