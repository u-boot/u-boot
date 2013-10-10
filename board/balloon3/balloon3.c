/*
 * Balloon3 Support
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pxa.h>
#include <serial.h>
#include <asm/io.h>
#include <spartan3.h>
#include <command.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

void balloon3_init_fpga(void);

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* arch number of vpac270 */
	gd->bd->bi_arch_number = MACH_TYPE_BALLOON3;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Init the FPGA */
	balloon3_init_fpga();

	return 0;
}

int dram_init(void)
{
	pxa2xx_dram_init();
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;

	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
}

#ifdef	CONFIG_CMD_USB
int board_usb_init(int index, enum usb_init_type init)
{
	writel((readl(UHCHR) | UHCHR_PCPL | UHCHR_PSPL) &
		~(UHCHR_SSEP0 | UHCHR_SSEP1 | UHCHR_SSEP2 | UHCHR_SSE),
		UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);

	while (readl(UHCHR) & UHCHR_FSBIR)
		;

	writel(readl(UHCHR) & ~UHCHR_SSE, UHCHR);
	writel((UHCHIE_UPRIE | UHCHIE_RWIE), UHCHIE);

	/* Clear any OTG Pin Hold */
	if (readl(PSSR) & PSSR_OTGPH)
		writel(readl(PSSR) | PSSR_OTGPH, PSSR);

	writel(readl(UHCRHDA) & ~(0x200), UHCRHDA);
	writel(readl(UHCRHDA) | 0x100, UHCRHDA);

	/* Set port power control mask bits, only 3 ports. */
	writel(readl(UHCRHDB) | (0x7<<17), UHCRHDB);

	/* enable port 2 */
	writel(readl(UP2OCR) | UP2OCR_HXOE | UP2OCR_HXS |
		UP2OCR_DMPDE | UP2OCR_DPPDE, UP2OCR);

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}

void usb_board_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | 1, UHCCOMS);
	udelay(10);

	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);

	return;
}
#endif

#if defined(CONFIG_FPGA)
/* Toggle GPIO103 and GPIO104 --  PROGB and RDnWR */
int fpga_pgm_fn(int nassert, int nflush, int cookie)
{
	if (nassert)
		writel(0x80, GPCR3);
	else
		writel(0x80, GPSR3);
	if (nflush)
		writel(0x100, GPCR3);
	else
		writel(0x100, GPSR3);
	return nassert;
}

/* Check GPIO83 -- INITB */
int fpga_init_fn(int cookie)
{
	return !(readl(GPLR2) & 0x80000);
}

/* Check GPIO84 -- BUSY */
int fpga_busy_fn(int cookie)
{
	return !(readl(GPLR2) & 0x100000);
}

/* Check GPIO111 -- DONE */
int fpga_done_fn(int cookie)
{
	return readl(GPLR3) & 0x8000;
}

/* Configure GPIO104 as GPIO and deassert it */
int fpga_pre_config_fn(int cookie)
{
	writel(readl(GAFR3_L) & ~0x30000, GAFR3_L);
	writel(0x100, GPCR3);
	return 0;
}

/* Configure GPIO104 as nSKTSEL */
int fpga_post_config_fn(int cookie)
{
	writel(readl(GAFR3_L) | 0x10000, GAFR3_L);
	return 0;
}

/* Toggle RDnWR */
int fpga_wr_fn(int nassert_write, int flush, int cookie)
{
	udelay(1000);

	if (nassert_write)
		writel(0x100, GPCR3);
	else
		writel(0x100, GPSR3);

	return nassert_write;
}

/* Write program to the FPGA */
int fpga_wdata_fn(uchar data, int flush, int cookie)
{
	writeb(data, 0x10f00000);
	return 0;
}

/* Toggle Clock pin -- NO-OP */
int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	return assert_clk;
}

/* Toggle ChipSelect pin -- NO-OP */
int fpga_cs_fn(int assert_clk, int flush, int cookie)
{
	return assert_clk;
}

Xilinx_Spartan3_Slave_Parallel_fns balloon3_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_init_fn,
	NULL,	/* err */
	fpga_done_fn,
	fpga_clk_fn,
	fpga_cs_fn,
	fpga_wr_fn,
	NULL,	/* rdata */
	fpga_wdata_fn,
	fpga_busy_fn,
	NULL,	/* abort */
	fpga_post_config_fn,
};

Xilinx_desc fpga = XILINX_XC3S1000_DESC(slave_parallel,
			(void *)&balloon3_fpga_fns, 0);

/* Initialize the FPGA */
void balloon3_init_fpga(void)
{
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
}
#else
void balloon3_init_fpga(void) {}
#endif /* CONFIG_FPGA */
