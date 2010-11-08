/*
 * Author :
 *     Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on mem.c and sdrc.c
 *
 * Copyright (C) 2010
 * Texas Instruments Incorporated - http://www.ti.com/
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/emif4.h>

extern omap3_sysinfo sysinfo;

static emif4_t *emif4_base = (emif4_t *)OMAP34XX_SDRC_BASE;

/*
 * is_mem_sdr -
 *  - Return 1 if mem type in use is SDR
 */
u32 is_mem_sdr(void)
{
	return 0;
}

/*
 * get_sdr_cs_size -
 *  - Get size of chip select 0/1
 */
u32 get_sdr_cs_size(u32 cs)
{
	u32 size;

	/* TODO: Calculate the size based on EMIF4 configuration */
	size = CONFIG_SYS_CS0_SIZE;

	return size;
}

/*
 * get_sdr_cs_offset -
 *  - Get offset of cs from cs0 start
 */
u32 get_sdr_cs_offset(u32 cs)
{
	u32 offset = 0;

	return offset;
}

/*
 * do_emif4_init -
 *  - Init the emif4 module for DDR access
 *  - Early init routines, called from flash or SRAM.
 */
void do_emif4_init(void)
{
	unsigned int regval;
	/* Set the DDR PHY parameters in PHY ctrl registers */
	regval = (EMIF4_DDR1_READ_LAT | EMIF4_DDR1_PWRDN_DIS |
		EMIF4_DDR1_EXT_STRB_DIS);
	writel(regval, &emif4_base->ddr_phyctrl1);
	writel(regval, &emif4_base->ddr_phyctrl1_shdw);
	writel(0, &emif4_base->ddr_phyctrl2);

	/* Reset the DDR PHY and wait till completed */
	regval = readl(&emif4_base->sdram_iodft_tlgc);
	regval |= (1<<10);
	writel(regval, &emif4_base->sdram_iodft_tlgc);
	/*Wait till that bit clears*/
	while ((readl(&emif4_base->sdram_iodft_tlgc) & (1<<10)) == 0x1);
	/*Re-verify the DDR PHY status*/
	while ((readl(&emif4_base->sdram_sts) & (1<<2)) == 0x0);

	regval |= (1<<0);
	writel(regval, &emif4_base->sdram_iodft_tlgc);
	/* Set SDR timing registers */
	regval = (EMIF4_TIM1_T_WTR | EMIF4_TIM1_T_RRD |
		EMIF4_TIM1_T_RC | EMIF4_TIM1_T_RAS |
		EMIF4_TIM1_T_WR | EMIF4_TIM1_T_RCD |
		EMIF4_TIM1_T_RP);
	writel(regval, &emif4_base->sdram_time1);
	writel(regval, &emif4_base->sdram_time1_shdw);

	regval = (EMIF4_TIM2_T_CKE | EMIF4_TIM2_T_RTP |
		EMIF4_TIM2_T_XSRD | EMIF4_TIM2_T_XSNR |
		EMIF4_TIM2_T_ODT | EMIF4_TIM2_T_XP);
	writel(regval, &emif4_base->sdram_time2);
	writel(regval, &emif4_base->sdram_time2_shdw);

	regval = (EMIF4_TIM3_T_RAS_MAX | EMIF4_TIM3_T_RFC);
	writel(regval, &emif4_base->sdram_time3);
	writel(regval, &emif4_base->sdram_time3_shdw);

	/* Set the PWR control register */
	regval = (EMIF4_PWR_PM_TIM | EMIF4_PWR_LP_MODE |
		EMIF4_PWR_DPD_DIS | EMIF4_PWR_IDLE_MODE);
	writel(regval, &emif4_base->sdram_pwr_mgmt);
	writel(regval, &emif4_base->sdram_pwr_mgmt_shdw);

	/* Set the DDR refresh rate control register */
	regval = (EMIF4_REFRESH_RATE | EMIF4_INITREF_DIS);
	writel(regval, &emif4_base->sdram_refresh_ctrl);
	writel(regval, &emif4_base->sdram_refresh_ctrl_shdw);

	/* set the SDRAM configuration register */
	regval = (EMIF4_CFG_PGSIZE | EMIF4_CFG_EBANK |
		EMIF4_CFG_IBANK | EMIF4_CFG_ROWSIZE |
		EMIF4_CFG_CL | EMIF4_CFG_NARROW_MD |
		EMIF4_CFG_SDR_DRV | EMIF4_CFG_DDR_DIS_DLL |
		EMIF4_CFG_DDR2_DDQS | EMIF4_CFG_DDR_TERM |
		EMIF4_CFG_IBANK_POS | EMIF4_CFG_SDRAM_TYP);
	writel(regval, &emif4_base->sdram_config);
}

/*
 * dram_init -
 *  - Sets uboots idea of sdram size
 */
int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int size0 = 0, size1 = 0;

	size0 = get_sdr_cs_size(CS0);
	/*
	 * If a second bank of DDR is attached to CS1 this is
	 * where it can be started.  Early init code will init
	 * memory on CS0.
	 */
	if ((sysinfo.mtype == DDR_COMBO) || (sysinfo.mtype == DDR_STACKED))
		size1 = get_sdr_cs_size(CS1);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = size0;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_1 + get_sdr_cs_offset(CS1);
	gd->bd->bi_dram[1].size = size1;

	return 0;
}

/*
 * mem_init() -
 *  - Initialize memory subsystem
 */
void mem_init(void)
{
	do_emif4_init();
}
