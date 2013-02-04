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
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long ddr_freq_mhz;

void sdram_init(void)
{
	ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC8xxx_DDR_ADDR;
	/* mask off E bit */
	u32 svr = SVR_SOC_VER(mfspr(SPRN_SVR));

	__raw_writel(CONFIG_SYS_DDR_CONTROL | SDRAM_CFG_32_BE, &ddr->sdram_cfg);
	__raw_writel(CONFIG_SYS_DDR_CS0_BNDS, &ddr->cs0_bnds);
	__raw_writel(CONFIG_SYS_DDR_CS0_CONFIG, &ddr->cs0_config);
	__raw_writel(CONFIG_SYS_DDR_CONTROL_2, &ddr->sdram_cfg_2);
	__raw_writel(CONFIG_SYS_DDR_DATA_INIT, &ddr->sdram_data_init);

	if (ddr_freq_mhz < 700) {
		__raw_writel(CONFIG_SYS_DDR_TIMING_3_667, &ddr->timing_cfg_3);
		__raw_writel(CONFIG_SYS_DDR_TIMING_0_667, &ddr->timing_cfg_0);
		__raw_writel(CONFIG_SYS_DDR_TIMING_1_667, &ddr->timing_cfg_1);
		__raw_writel(CONFIG_SYS_DDR_TIMING_2_667, &ddr->timing_cfg_2);
		__raw_writel(CONFIG_SYS_DDR_MODE_1_667, &ddr->sdram_mode);
		__raw_writel(CONFIG_SYS_DDR_MODE_2_667, &ddr->sdram_mode_2);
		__raw_writel(CONFIG_SYS_DDR_INTERVAL_667, &ddr->sdram_interval);
		__raw_writel(CONFIG_SYS_DDR_CLK_CTRL_667, &ddr->sdram_clk_cntl);
		__raw_writel(CONFIG_SYS_DDR_WRLVL_CONTROL_667, &ddr->ddr_wrlvl_cntl);
	} else {
		__raw_writel(CONFIG_SYS_DDR_TIMING_3_800, &ddr->timing_cfg_3);
		__raw_writel(CONFIG_SYS_DDR_TIMING_0_800, &ddr->timing_cfg_0);
		__raw_writel(CONFIG_SYS_DDR_TIMING_1_800, &ddr->timing_cfg_1);
		__raw_writel(CONFIG_SYS_DDR_TIMING_2_800, &ddr->timing_cfg_2);
		__raw_writel(CONFIG_SYS_DDR_MODE_1_800, &ddr->sdram_mode);
		__raw_writel(CONFIG_SYS_DDR_MODE_2_800, &ddr->sdram_mode_2);
		__raw_writel(CONFIG_SYS_DDR_INTERVAL_800, &ddr->sdram_interval);
		__raw_writel(CONFIG_SYS_DDR_CLK_CTRL_800, &ddr->sdram_clk_cntl);
		__raw_writel(CONFIG_SYS_DDR_WRLVL_CONTROL_800, &ddr->ddr_wrlvl_cntl);
	}

	__raw_writel(CONFIG_SYS_DDR_TIMING_4, &ddr->timing_cfg_4);
	__raw_writel(CONFIG_SYS_DDR_TIMING_5, &ddr->timing_cfg_5);
	__raw_writel(CONFIG_SYS_DDR_ZQ_CONTROL, &ddr->ddr_zq_cntl);

	/* P1014 and it's derivatives support max 16bit DDR width */
	if (svr == SVR_P1014) {
		__raw_writel(ddr->sdram_cfg & ~SDRAM_CFG_DBW_MASK, &ddr->sdram_cfg);
		__raw_writel(ddr->sdram_cfg | SDRAM_CFG_16_BE, &ddr->sdram_cfg);
		/* For CS0_BNDS we divide the start and end address by 2, so we can just
		 * shift the entire register to achieve the desired result and the mask
		 * the value so we don't write reserved fields */
		__raw_writel((CONFIG_SYS_DDR_CS0_BNDS >> 1) & 0x0fff0fff, &ddr->cs0_bnds);
	}

	udelay(500);

	/* Let the controller go */
	out_be32(&ddr->sdram_cfg, in_be32(&ddr->sdram_cfg) | SDRAM_CFG_MEM_EN);

	set_next_law(CONFIG_SYS_NAND_DDR_LAW, LAW_SIZE_1G, LAW_TRGT_IF_DDR_1);
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio, ddr_ratio;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	ddr_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO;
	ddr_ratio = ddr_ratio >> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
	ddr_freq_mhz = (CONFIG_SYS_CLK_FREQ * ddr_ratio) / 0x1000000;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
			gd->bus_clk / 16 / CONFIG_BAUDRATE);

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
