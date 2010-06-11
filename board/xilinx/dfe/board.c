/*
 * Just to satisfy init routines..
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* BHILL FIXME - should this be here? lib_arm/board.c */
	interrupt_init();

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	return 0;
}

/*
 * OK, and resets too.
 */
void reset_cpu(ulong addr)
{
	puts("Warning: this reset doesn't work.");
	/* hah. */
	goto *((void*)0x0);
}

void do_reset(void)
{
	reset_cpu(0);
	goto *((void*)0x0); /* call optimized out? */
}
