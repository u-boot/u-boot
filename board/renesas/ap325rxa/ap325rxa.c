/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* PRI control register */
#define PRPRICR5	0xFF800048 /* LMB */
#define PRPRICR5_D	0x2a

/* FPGA control */
#define FPGA_NAND_CTL	0xB410020C
#define FPGA_NAND_RST	0x0008
#define FPGA_NAND_INIT	0x0000
#define FPGA_NAND_RST_WAIT	10000

/* I/O port data */
#define PACR_D	0x0000
#define PBCR_D	0x0000
#define PCCR_D	0x1000
#define PDCR_D	0x0000
#define PECR_D	0x0410
#define PFCR_D	0xffff
#define PGCR_D	0x0000
#define PHCR_D	0x5011
#define PJCR_D	0x4400
#define PKCR_D	0x7c00
#define PLCR_D	0x0000
#define PMCR_D	0x0000
#define PNCR_D	0x0000
#define PQCR_D	0x0000
#define PRCR_D	0x0000
#define PSCR_D	0x0000
#define PTCR_D	0x0010
#define PUCR_D	0x0fff
#define PVCR_D	0xffff
#define PWCR_D	0x0000
#define PXCR_D	0x7500
#define PYCR_D	0x0000
#define PZCR_D	0x5540

/* Pin Function Controler data */
#define PSELA_D	0x1410
#define PSELB_D	0x0140
#define PSELC_D	0x0000
#define PSELD_D	0x0400

/* I/O Buffer Hi-Z data */
#define	HIZCRA_D	0x0000
#define HIZCRB_D	0x1000
#define HIZCRC_D	0x0000
#define HIZCRD_D	0x0000

/* Module select reg data */
#define MSELCRA_D	0x0014
#define MSELCRB_D	0x0018

/* Module Stop reg Data */
#define MSTPCR2_D	0xFFD9F280

/* CPLD loader */
extern void init_cpld(void);

int checkboard(void)
{
	puts("BOARD: AP325RXA\n");
	return 0;
}

int board_init(void)
{
	/* Pin Function Controler Init */
	outw(PSELA_D, PSELA);
	outw(PSELB_D, PSELB);
	outw(PSELC_D, PSELC);
	outw(PSELD_D, PSELD);

	/* I/O Buffer Hi-Z Init */
	outw(HIZCRA_D, HIZCRA);
	outw(HIZCRB_D, HIZCRB);
	outw(HIZCRC_D, HIZCRC);
	outw(HIZCRD_D, HIZCRD);

	/* Module select reg Init */
	outw(MSELCRA_D, MSELCRA);
	outw(MSELCRB_D, MSELCRB);

	/* Module Stop reg Init */
	outl(MSTPCR2_D, MSTPCR2);

	/* I/O ports */
	outw(PACR_D, PACR);
	outw(PBCR_D, PBCR);
	outw(PCCR_D, PCCR);
	outw(PDCR_D, PDCR);
	outw(PECR_D, PECR);
	outw(PFCR_D, PFCR);
	outw(PGCR_D, PGCR);
	outw(PHCR_D, PHCR);
	outw(PJCR_D, PJCR);
	outw(PKCR_D, PKCR);
	outw(PLCR_D, PLCR);
	outw(PMCR_D, PMCR);
	outw(PNCR_D, PNCR);
	outw(PQCR_D, PQCR);
	outw(PRCR_D, PRCR);
	outw(PSCR_D, PSCR);
	outw(PTCR_D, PTCR);
	outw(PUCR_D, PUCR);
	outw(PVCR_D, PVCR);
	outw(PWCR_D, PWCR);
	outw(PXCR_D, PXCR);
	outw(PYCR_D, PYCR);
	outw(PZCR_D, PZCR);

	/* PRI control register Init */
	outl(PRPRICR5_D, PRPRICR5);

	/* cpld init */
	init_cpld();

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

void led_set_state(unsigned short value)
{
}

void ide_set_reset(int idereset)
{
	outw(FPGA_NAND_RST, FPGA_NAND_CTL);	/* NAND RESET */
	udelay(FPGA_NAND_RST_WAIT);
	outw(FPGA_NAND_INIT, FPGA_NAND_CTL);
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
