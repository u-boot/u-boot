/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

/*
 * Miscellaneous platform dependent initialisations
 */

int board_early_init_f (void)
{
	/*
	 * set clock frequency:
	 *	VERSATILE_REFCLK is 32KHz
	 *	VERSATILE_TIMCLK is 1MHz
	 */
	*(volatile unsigned int *)(VERSATILE_SCTL_BASE) |=
	  ((VERSATILE_TIMCLK << VERSATILE_TIMER1_EnSel) | (VERSATILE_TIMCLK << VERSATILE_TIMER2_EnSel) |
	   (VERSATILE_TIMCLK << VERSATILE_TIMER3_EnSel) | (VERSATILE_TIMCLK << VERSATILE_TIMER4_EnSel));

	return 0;
}

int board_init (void)
{
	/* arch number of Versatile Board */
#ifdef CONFIG_ARCH_VERSATILE_AB
	gd->bd->bi_arch_number = MACH_TYPE_VERSATILE_AB;
#else
	gd->bd->bi_arch_number = MACH_TYPE_VERSATILE_PB;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

	icache_enable ();

	return 0;
}


int misc_init_r (void)
{
	setenv("verify", "n");
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	return rc;
}
#endif
