#ifndef __MINI2440_BOARD_CONF_H__
#define __MINI2440_BOARD_CONF_H__

/* PLL Parameters */
#define CLKDIVN_VAL	7
#define M_MDIV		0x7f
#define M_PDIV		0x2
#define M_SDIV		0x1

#define U_M_MDIV	0x38
#define U_M_PDIV	0x2
#define U_M_SDIV	0x2

/* BWSCON */
#define DW8				0x0
#define DW16			0x1
#define DW32			0x2
#define WAIT			(0x1<<2)
#define UBLB			(0x1<<3)

#define B1_BWSCON		(DW32)
#define B2_BWSCON		(DW16)
#define B3_BWSCON		(DW16 + WAIT + UBLB)
#define B4_BWSCON		(DW16 + WAIT + UBLB)
#define B5_BWSCON		(DW16)
#define B6_BWSCON		(DW32)
#define B7_BWSCON		(DW32)

/*
 * Bank Configuration
 */
#define B0_Tacs			0x0	/*  0clk */
#define B0_Tcos			0x0	/*  0clk */
#define B0_Tacc			0x7	/* 14clk */
#define B0_Tcoh			0x0	/*  0clk */
#define B0_Tah			0x0	/*  0clk */
#define B0_Tacp			0x0 /*  0clk */
#define B0_PMC			0x0	/* normal */

#define B1_Tacs			0x0
#define B1_Tcos			0x0
#define B1_Tacc			0x7
#define B1_Tcoh			0x0
#define B1_Tah			0x0
#define B1_Tacp			0x0
#define B1_PMC			0x0

#define B2_Tacs			0x0
#define B2_Tcos			0x0
#define B2_Tacc			0x7
#define B2_Tcoh			0x0
#define B2_Tah			0x0
#define B2_Tacp			0x0
#define B2_PMC			0x0

#define B3_Tacs			0x0
#define B3_Tcos			0x3	/*  4clk */
#define B3_Tacc			0x7
#define B3_Tcoh			0x1	/*  1clk */
#define B3_Tah			0x3	/*  4clk */
#define B3_Tacp			0x0
#define B3_PMC			0x0

#define B4_Tacs			0x0
#define B4_Tcos			0x3
#define B4_Tacc			0x7
#define B4_Tcoh			0x1
#define B4_Tah			0x3
#define B4_Tacp			0x0
#define B4_PMC			0x0

#define B5_Tacs			0x0
#define B5_Tcos			0x0
#define B5_Tacc			0x7
#define B5_Tcoh			0x0
#define B5_Tah			0x0
#define B5_Tacp			0x0
#define B5_PMC			0x0

/*
 * SDRAM Configuration
 */
#define SDRAM_MT		0x3	/* SDRAM */
#define SDRAM_Trcd		0x0	/* 2clk */
#define SDRAM_SCAN_9	0x1	/* 9bit */
#define SDRAM_SCAN_10	0x2	/* 10bit */

#define SDRAM_64MB	((SDRAM_MT<<15) + (SDRAM_Trcd<<2) + (SDRAM_SCAN_9))

/*
 * Refresh Parameter
 */
#define REFEN		0x1	/* Refresh enable */
#define TREFMD		0x0	/* CBR(CAS before RAS)/Auto refresh */
#define Trp			0x1	/* 3clk */
#define Trc			0x3	/* 7clk */
#define Tchr		0x0	/* unused */
#define REFCNT	1012 /* period=10.37us, HCLK=100Mhz, (2048 + 1-10.37*100) */

/*
 * MRSR Parameter
 */
#define BL	0x0
#define BT	0x0
#define CL	0x3 /* 3 clocks */
#define TM	0x0
#define WBL	0x0

/*
 * BankSize Parameter
 */
#define BK76MAP	0x2 /* 128MB/128MB */
#define SCLK_EN	0x1 /* SCLK active */
#define SCKE_EN	0x1 /* SDRAM power down mode enable */
#define BURST_EN	0x1 /* Burst enable */

/*
 * Register values
 */
#define BANK_CONF ((0 + (B1_BWSCON<<4) + (B2_BWSCON<<8) + (B3_BWSCON<<12) + \
			(B4_BWSCON<<16) + (B5_BWSCON<<20) + (B6_BWSCON<<24) + \
			(B7_BWSCON<<28)))

#define B0_CONF	((B0_Tacs<<13) + (B0_Tcos<<11) + (B0_Tacc<<8) + \
		(B0_Tcoh<<6) + (B0_Tah<<4) + (B0_Tacp<<2) + (B0_PMC))
#define B1_CONF	((B1_Tacs<<13) + (B1_Tcos<<11) + (B1_Tacc<<8) + \
		(B1_Tcoh<<6) + (B1_Tah<<4) + (B1_Tacp<<2) + (B1_PMC))
#define B2_CONF	((B2_Tacs<<13) + (B2_Tcos<<11) + (B2_Tacc<<8) + \
		(B2_Tcoh<<6) + (B2_Tah<<4) + (B2_Tacp<<2) + (B2_PMC))
#define B3_CONF	((B3_Tacs<<13) + (B3_Tcos<<11) + (B3_Tacc<<8) + \
		(B3_Tcoh<<6) + (B3_Tah<<4) + (B3_Tacp<<2) + (B3_PMC))
#define B4_CONF	((B4_Tacs<<13) + (B4_Tcos<<11) + (B4_Tacc<<8) + \
		(B4_Tcoh<<6) + (B4_Tah<<4) + (B4_Tacp<<2) + (B4_PMC))
#define B5_CONF	((B5_Tacs<<13) + (B5_Tcos<<11) + (B5_Tacc<<8) + \
		(B5_Tcoh<<6) + (B5_Tah<<4) + (B5_Tacp<<2) + (B5_PMC))

#define MEM_TIMING (REFEN<<23) + (TREFMD<<22) + (Trp<<20) + \
	(Trc<<18) + (Tchr<<16) + REFCNT

#define BANKSIZE_CONF	(BK76MAP) + (SCLK_EN<<4) + (SCKE_EN<<5) + (BURST_EN<<7)
#define B6_MRSR			(CL<<4)
#define B7_MRSR			(CL<<4)

#endif
