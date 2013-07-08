/*
 * (C) Copyright 2005 Freescale Semiconductor, Inc.
 *
 * Roy Zang <tie-fei.zang@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * modifications for the Tsi108 Emul Board by avb@Tundra
 */

/*
 * board support/init functions for the
 * Freescale MPC7448 HPC2 (High-Performance Computing 2 Platform).
 */

#include <common.h>
#include <74xx_7xx.h>
#include <fdt_support.h>
#include <netdev.h>

#undef	DEBUG

DECLARE_GLOBAL_DATA_PTR;

extern void tsi108_init_f (void);

int display_mem_map (void);

void after_reloc (ulong dest_addr)
{
	/*
	 * Jump to the main U-Boot board init code
	 */
	board_init_r ((gd_t *) gd, dest_addr);
	/* NOTREACHED */
}

/*
 * Check Board Identity:
 * report board type
 */

int checkboard (void)
{
	int l_type = 0;

	printf ("BOARD: %s\n", CONFIG_SYS_BOARD_NAME);
	return (l_type);
}

/*
 * Read Processor ID:
 *
 * report calling processor number
 */

int read_pid (void)
{
	return 0;		/* we are on single CPU platform for a while */
}

long int dram_size (int board_type)
{
	return 0x20000000;	/* 256M bytes */
}

phys_size_t initdram (int board_type)
{
	return dram_size (board_type);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#if defined(CONFIG_TSI108_ETH)
	rc = tsi108_eth_initialize(bis);
#endif
	return rc;
}
