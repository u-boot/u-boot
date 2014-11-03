/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/bootm.h>

#include "common.h"
#include "eeprom.h"

void cl_print_pcb_info(void)
{
	u32 board_rev = get_board_rev();
	u32 rev_major = board_rev / 100;
	u32 rev_minor = board_rev - (rev_major * 100);

	if ((rev_minor / 10) * 10 == rev_minor)
		rev_minor = rev_minor / 10;

	printf("PCB:   %u.%u\n", rev_major, rev_minor);
}
