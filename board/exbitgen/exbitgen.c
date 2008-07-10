#include <common.h>
#include <asm/u-boot.h>
#include <asm/processor.h>
#include "exbitgen.h"

void sdram_init(void);

/* ************************************************************************ */
int board_early_init_f (void)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
	unsigned long i;

   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the Walnut board.
   | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
   |       IRQ 16    405GP internally generated; active low; level sensitive
   |       IRQ 17-24 RESERVED
   |       IRQ 25 (EXT IRQ 0) FPGA; active high; level sensitive
   |       IRQ 26 (EXT IRQ 1) SMI; active high; level sensitive
   |       IRQ 27 (EXT IRQ 2) Not Used
   |       IRQ 28 (EXT IRQ 3) PCI SLOT 3; active low; level sensitive
   |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
   |       IRQ 30 (EXT IRQ 5) PCI SLOT 1; active low; level sensitive
   |       IRQ 31 (EXT IRQ 6) PCI SLOT 0; active low; level sensitive
   | Note for Walnut board:
   |       An interrupt taken for the FPGA (IRQ 25) indicates that either
   |       the Mouse, Keyboard, IRDA, or External Expansion caused the
   |       interrupt. The FPGA must be read to determine which device
   |       caused the interrupt. The default setting of the FPGA clears
   |
   +-------------------------------------------------------------------------*/

	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000020);	/* set all but FPGA SMI to be non-critical */
	mtdcr (uicpr, 0xFFFFFF90);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */

	/* Perform reset of PHY connected to PPC via register in CPLD */
	out8 (PHY_CTRL_ADDR, 0x2e);	/* activate nRESET,FDX,F100,ANEN, enable output */
	for (i = 0; i < 10000000; i++) {
		;
	}
	out8 (PHY_CTRL_ADDR, 0x2f);	/* deactivate nRESET */

	return 0;
}


/* ************************************************************************ */
int checkboard (void)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
	printf ("Exbit H/W id: %d\n", in8 (HW_ID_ADDR));
	return (0);
}

/* ************************************************************************ */
phys_size_t initdram (int board_type)
/* ------------------------------------------------------------------------ --
 * Purpose     : Determines size of mounted DRAM.
 * Remarks     : Size is determined by reading SDRAM configuration registers as
 *               set up by sdram_init.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
	ulong tot_size;
	ulong bank_size;
	ulong tmp;

	/*
	 * ToDo: Move the asm init routine sdram_init() to this C file,
	 * or even better use some common ppc4xx code available
	 * in cpu/ppc4xx
	 */
	sdram_init();

	tot_size = 0;

	mtdcr (memcfga, mem_mb0cf);
	tmp = mfdcr (memcfgd);
	if (tmp & 0x00000001) {
		bank_size = 0x00400000 << ((tmp >> 17) & 0x7);
		tot_size += bank_size;
	}

	mtdcr (memcfga, mem_mb1cf);
	tmp = mfdcr (memcfgd);
	if (tmp & 0x00000001) {
		bank_size = 0x00400000 << ((tmp >> 17) & 0x7);
		tot_size += bank_size;
	}

	mtdcr (memcfga, mem_mb2cf);
	tmp = mfdcr (memcfgd);
	if (tmp & 0x00000001) {
		bank_size = 0x00400000 << ((tmp >> 17) & 0x7);
		tot_size += bank_size;
	}

	mtdcr (memcfga, mem_mb3cf);
	tmp = mfdcr (memcfgd);
	if (tmp & 0x00000001) {
		bank_size = 0x00400000 << ((tmp >> 17) & 0x7);
		tot_size += bank_size;
	}

	return tot_size;
}
