/*
 * (C) Copyright 2011 Alex Dubov <oakad@yahoo.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

/*
 * Initialize Local Bus
 */
void local_bus_init(void)
{
	fsl_lbc_t *lbc = LBC_BASE_ADDR;

	out_be32(&lbc->ltesr, 0xffffffff); /* Clear LBC error interrupts */
	out_be32(&lbc->lteir, 0xffffffff); /* Enable LBC error interrupts */
}

int checkboard(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);

	puts("Board: Mercury Computer Systems, Inc. MPQ-101 ");
#ifdef CONFIG_PHYS_64BIT
	puts("(36-bit addrmap) ");
#endif
	putc('\n');

	/*
	 * Initialize local bus.
	 */
	local_bus_init();

	/*
	 * Hack TSEC 3 and 4 IO voltages.
	 */
	out_be32(&gur->tsec34ioovcr, 0xe7e0); /*  1110 0111 1110 0xxx */

	out_be32(&ecm->eedr, 0xffffffff); /* clear ecm errors */
	out_be32(&ecm->eeer, 0xffffffff); /* enable ecm errors */
	return 0;
}

phys_size_t fixed_sdram(void)
{
	ccsr_ddr_t *ddr = (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);
	const char *p_mode = getenv("perf_mode");

	puts("Initializing....");

	out_be32(&ddr->cs0_bnds, CONFIG_SYS_DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, CONFIG_SYS_DDR_CS0_CONFIG);

	out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);

	if (p_mode && !strcmp("performance", p_mode)) {
		out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1_PERF);
		out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2_PERF);
		out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_MODE_1_PERF);
		out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_MODE_2_PERF);
		out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_INTERVAL_PERF);
	} else {
		out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
		out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
		out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_MODE_1);
		out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_MODE_2);
		out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	}

	out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CTRL);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_CONTROL2);

	asm("sync;isync");
	udelay(500);

	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
	asm("sync; isync");
	udelay(500);

	return ((phys_size_t)1) << CONFIG_SYS_SDRAM_SIZE_LOG;
}

void pci_init_board(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* PCI is disabled */
	out_be32(&gur->devdisr, in_be32(&gur->devdisr)
				| MPC85xx_DEVDISR_PCI1
				| MPC85xx_DEVDISR_PCI2
				| MPC85xx_DEVDISR_PCIE);
}


#if defined(CONFIG_OF_BOARD_SETUP)

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}

#endif
