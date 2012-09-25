/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Author: Dipen Dudhat <dipen.dudhat@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_ifc.h>

void print_ifc_regs(void)
{
	int i, j;

	printf("IFC Controller Registers\n");
	for (i = 0; i < FSL_IFC_BANK_COUNT; i++) {
		printf("CSPR%d:0x%08X\tAMASK%d:0x%08X\tCSOR%d:0x%08X\n",
			i, get_ifc_cspr(i), i, get_ifc_amask(i),
			i, get_ifc_csor(i));
		for (j = 0; j < 4; j++)
			printf("IFC_FTIM%d:0x%08X\n", j, get_ifc_ftim(i, j));
	}
}

void init_early_memctl_regs(void)
{
#if defined(CONFIG_SYS_CSPR0) && defined(CONFIG_SYS_CSOR0)
	set_ifc_ftim(IFC_CS0, IFC_FTIM0, CONFIG_SYS_CS0_FTIM0);
	set_ifc_ftim(IFC_CS0, IFC_FTIM1, CONFIG_SYS_CS0_FTIM1);
	set_ifc_ftim(IFC_CS0, IFC_FTIM2, CONFIG_SYS_CS0_FTIM2);
	set_ifc_ftim(IFC_CS0, IFC_FTIM3, CONFIG_SYS_CS0_FTIM3);

#if !defined(CONFIG_SYS_FSL_ERRATUM_IFC_A003399) || defined(CONFIG_SYS_RAMBOOT)
#ifdef CONFIG_SYS_CSPR0_EXT
	set_ifc_cspr_ext(IFC_CS0, CONFIG_SYS_CSPR0_EXT);
#endif
	set_ifc_cspr(IFC_CS0, CONFIG_SYS_CSPR0);
	set_ifc_amask(IFC_CS0, CONFIG_SYS_AMASK0);
	set_ifc_csor(IFC_CS0, CONFIG_SYS_CSOR0);
#endif
#endif

#ifdef CONFIG_SYS_CSPR1_EXT
	set_ifc_cspr_ext(IFC_CS1, CONFIG_SYS_CSPR1_EXT);
#endif
#if defined(CONFIG_SYS_CSPR1) && defined(CONFIG_SYS_CSOR1)
	set_ifc_ftim(IFC_CS1, IFC_FTIM0, CONFIG_SYS_CS1_FTIM0);
	set_ifc_ftim(IFC_CS1, IFC_FTIM1, CONFIG_SYS_CS1_FTIM1);
	set_ifc_ftim(IFC_CS1, IFC_FTIM2, CONFIG_SYS_CS1_FTIM2);
	set_ifc_ftim(IFC_CS1, IFC_FTIM3, CONFIG_SYS_CS1_FTIM3);

	set_ifc_csor(IFC_CS1, CONFIG_SYS_CSOR1);
	set_ifc_amask(IFC_CS1, CONFIG_SYS_AMASK1);
	set_ifc_cspr(IFC_CS1, CONFIG_SYS_CSPR1);
#endif

#ifdef CONFIG_SYS_CSPR2_EXT
	set_ifc_cspr_ext(IFC_CS2, CONFIG_SYS_CSPR2_EXT);
#endif
#if defined(CONFIG_SYS_CSPR2) && defined(CONFIG_SYS_CSOR2)
	set_ifc_ftim(IFC_CS2, IFC_FTIM0, CONFIG_SYS_CS2_FTIM0);
	set_ifc_ftim(IFC_CS2, IFC_FTIM1, CONFIG_SYS_CS2_FTIM1);
	set_ifc_ftim(IFC_CS2, IFC_FTIM2, CONFIG_SYS_CS2_FTIM2);
	set_ifc_ftim(IFC_CS2, IFC_FTIM3, CONFIG_SYS_CS2_FTIM3);

	set_ifc_csor(IFC_CS2, CONFIG_SYS_CSOR2);
	set_ifc_amask(IFC_CS2, CONFIG_SYS_AMASK2);
	set_ifc_cspr(IFC_CS2, CONFIG_SYS_CSPR2);
#endif

#ifdef CONFIG_SYS_CSPR3_EXT
	set_ifc_cspr_ext(IFC_CS3, CONFIG_SYS_CSPR3_EXT);
#endif
#if defined(CONFIG_SYS_CSPR3) && defined(CONFIG_SYS_CSOR3)
	set_ifc_ftim(IFC_CS3, IFC_FTIM0, CONFIG_SYS_CS3_FTIM0);
	set_ifc_ftim(IFC_CS3, IFC_FTIM1, CONFIG_SYS_CS3_FTIM1);
	set_ifc_ftim(IFC_CS3, IFC_FTIM2, CONFIG_SYS_CS3_FTIM2);
	set_ifc_ftim(IFC_CS3, IFC_FTIM3, CONFIG_SYS_CS3_FTIM3);

	set_ifc_cspr(IFC_CS3, CONFIG_SYS_CSPR3);
	set_ifc_amask(IFC_CS3, CONFIG_SYS_AMASK3);
	set_ifc_csor(IFC_CS3, CONFIG_SYS_CSOR3);
#endif
}
