/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * Derived from code (arch/arm/lib/reset.c) that is:
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * (C) Copyright 2004 Texas Insturments
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/pmc.h>

static int do_enterrcm(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;

	puts("Entering RCM...\n");
	udelay(50000);

	pmc->pmc_scratch0 = 2;
	disable_interrupts();
	reset_cpu(0);

	return 0;
}

U_BOOT_CMD(
	enterrcm, 1, 0, do_enterrcm,
	"reset Tegra and enter USB Recovery Mode",
	""
);
