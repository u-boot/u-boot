/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <common.h>
#include <mpc85xx.h>
#include <asm/io.h>
#include <ns16550.h>
#include <nand.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_law.h>

#define udelay(x) { int j; for (j = 0; j < x * 10000; j++) isync(); }

unsigned long ddr_freq_mhz;

void sdram_init(void)
{
	ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC85xx_DDR_ADDR;

	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL | SDRAM_CFG_32_BE);
	out_be32(&ddr->cs0_bnds, CONFIG_SYS_DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, CONFIG_SYS_DDR_CS0_CONFIG);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_CONTROL_2);
	out_be32(&ddr->sdram_data_init, CONFIG_SYS_DDR_DATA_INIT);

	if (ddr_freq_mhz < 700) {
		out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3_667);
		out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0_667);
		out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1_667);
		out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2_667);
		out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_MODE_1_667);
		out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_MODE_2_667);
		out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_INTERVAL_667);
		out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CTRL_667);
		out_be32(&ddr->ddr_wrlvl_cntl,
				CONFIG_SYS_DDR_WRLVL_CONTROL_667);
	} else {
		out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3_800);
		out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0_800);
		out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1_800);
		out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2_800);
		out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_MODE_1_800);
		out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_MODE_2_800);
		out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_INTERVAL_800);
		out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CTRL_800);
		out_be32(&ddr->ddr_wrlvl_cntl,
				CONFIG_SYS_DDR_WRLVL_CONTROL_800);
	}

	out_be32(&ddr->timing_cfg_4, CONFIG_SYS_DDR_TIMING_4);
	out_be32(&ddr->timing_cfg_5, CONFIG_SYS_DDR_TIMING_5);
	out_be32(&ddr->ddr_zq_cntl, CONFIG_SYS_DDR_ZQ_CONTROL);

	/* mimic 500us delay, with busy isync() loop */
	udelay(100);

	/* Let the controller go */
	out_be32(&ddr->sdram_cfg, in_be32(&ddr->sdram_cfg) | SDRAM_CFG_MEM_EN);

	set_next_law(CONFIG_SYS_NAND_DDR_LAW, LAW_SIZE_1G, LAW_TRGT_IF_DDR_1);
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio, ddr_ratio;
	unsigned long bus_clk;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	ddr_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO;
	ddr_ratio = ddr_ratio >> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
	ddr_freq_mhz = (CONFIG_SYS_CLK_FREQ * ddr_ratio) / 0x1000000;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
			bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

	/* Initialize the DDR3 */
	sdram_init();

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SYS_NAND_U_BOOT_RELOC_SP, 0,
			CONFIG_SYS_NAND_U_BOOT_RELOC);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	nand_boot();
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, '\r');

	NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, c);
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}
