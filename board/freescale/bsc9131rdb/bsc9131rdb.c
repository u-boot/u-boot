/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <netdev.h>


DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_GPIO42);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_DSP_TMS);

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_GPIO43);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_DSP_TCK |
			MPC85xx_PMUXCR2_UART_CTS_B1_SIM_PD);
	setbits_be32(&gur->halt_req_mask, HALTED_TO_HALT_REQ_MASK_0);
	clrsetbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_IFC_AD_GPIO_MASK |
			MPC85xx_PMUXCR_IFC_AD17_GPO_MASK,
			MPC85xx_PMUXCR_IFC_AD_GPIO |
			MPC85xx_PMUXCR_IFC_AD17_GPO | MPC85xx_PMUXCR_SDHC_USIM);

	return 0;
}

int checkboard(void)
{
	struct cpu_type *cpu;

	cpu = gd->cpu;
	printf("Board: %sRDB\n", cpu->name);

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	fdt_fixup_dr_usb(blob, bd);
}
#endif
