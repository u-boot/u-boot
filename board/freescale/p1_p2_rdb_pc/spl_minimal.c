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
#include <ns16550.h>
#include <asm/io.h>
#include <nand.h>
#include <linux/compiler.h>
#include <asm/fsl_law.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_INIT_L2_ADDR
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
static void sdram_init(void)
{
	ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC8xxx_DDR_ADDR;

	__raw_writel(CONFIG_SYS_DDR_CS0_BNDS, &ddr->cs0_bnds);
	__raw_writel(CONFIG_SYS_DDR_CS0_CONFIG, &ddr->cs0_config);
#if CONFIG_CHIP_SELECTS_PER_CTRL > 1
	__raw_writel(CONFIG_SYS_DDR_CS1_BNDS, &ddr->cs1_bnds);
	__raw_writel(CONFIG_SYS_DDR_CS1_CONFIG, &ddr->cs1_config);
#endif
	__raw_writel(CONFIG_SYS_DDR_TIMING_3, &ddr->timing_cfg_3);
	__raw_writel(CONFIG_SYS_DDR_TIMING_0, &ddr->timing_cfg_0);
	__raw_writel(CONFIG_SYS_DDR_TIMING_1, &ddr->timing_cfg_1);
	__raw_writel(CONFIG_SYS_DDR_TIMING_2, &ddr->timing_cfg_2);

	__raw_writel(CONFIG_SYS_DDR_CONTROL_2, &ddr->sdram_cfg_2);
	__raw_writel(CONFIG_SYS_DDR_MODE_1, &ddr->sdram_mode);
	__raw_writel(CONFIG_SYS_DDR_MODE_2, &ddr->sdram_mode_2);

	__raw_writel(CONFIG_SYS_DDR_INTERVAL, &ddr->sdram_interval);
	__raw_writel(CONFIG_SYS_DDR_DATA_INIT, &ddr->sdram_data_init);
	__raw_writel(CONFIG_SYS_DDR_CLK_CTRL, &ddr->sdram_clk_cntl);

	__raw_writel(CONFIG_SYS_DDR_TIMING_4, &ddr->timing_cfg_4);
	__raw_writel(CONFIG_SYS_DDR_TIMING_5, &ddr->timing_cfg_5);
	__raw_writel(CONFIG_SYS_DDR_ZQ_CONTROL, &ddr->ddr_zq_cntl);
	__raw_writel(CONFIG_SYS_DDR_WRLVL_CONTROL, &ddr->ddr_wrlvl_cntl);

	/* Set, but do not enable the memory */
	__raw_writel(CONFIG_SYS_DDR_CONTROL & ~SDRAM_CFG_MEM_EN, &ddr->sdram_cfg);

	asm volatile("sync;isync");
	udelay(500);

	/* Let the controller go */
	out_be32(&ddr->sdram_cfg, in_be32(&ddr->sdram_cfg) | SDRAM_CFG_MEM_EN);

	set_next_law(0, CONFIG_SYS_SDRAM_SIZE_LAW, LAW_TRGT_IF_DDR_1);
}
#endif

void board_init_f(ulong bootflag)
{
	u32 plat_ratio;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
#ifndef CONFIG_QE
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
#endif

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
			gd->bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

#ifndef CONFIG_QE
	/* init DDR3 reset signal */
	__raw_writel(0x02000000, &pgpio->gpdir);
	__raw_writel(0x00200000, &pgpio->gpodr);
	__raw_writel(0x00000000, &pgpio->gpdat);
	udelay(1000);
	__raw_writel(0x00200000, &pgpio->gpdat);
	udelay(1000);
	__raw_writel(0x00000000, &pgpio->gpdir);
#endif

#ifndef CONFIG_SYS_INIT_L2_ADDR
	/* Initialize the DDR3 */
	sdram_init();
#endif

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SPL_RELOC_STACK, 0, CONFIG_SPL_RELOC_TEXT_BASE);
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
