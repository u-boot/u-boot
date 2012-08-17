/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc
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
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SYS_FSL_ERRATUM_IFC_A003399) && !defined(CONFIG_SYS_RAMBOOT)
void setup_ifc(void)
{
	struct fsl_ifc *ifc_regs = (void *)CONFIG_SYS_IFC_ADDR;
	u32 _mas0, _mas1, _mas2, _mas3, _mas7;
	phys_addr_t flash_phys = CONFIG_SYS_FLASH_BASE_PHYS;

	/*
	 * Adjust the TLB we were running out of to match the phys addr of the
	 * chip select we are adjusting and will return to.
	 */
	flash_phys += (~CONFIG_SYS_AMASK0) + 1 - 4*1024*1024;

	_mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(15);
	_mas1 = MAS1_VALID | MAS1_TID(0) | MAS1_TS | MAS1_IPROT |
			MAS1_TSIZE(BOOKE_PAGESZ_4M);
	_mas2 = FSL_BOOKE_MAS2(CONFIG_SYS_TEXT_BASE, MAS2_I|MAS2_G);
	_mas3 = FSL_BOOKE_MAS3(flash_phys, 0, MAS3_SW|MAS3_SR|MAS3_SX);
	_mas7 = FSL_BOOKE_MAS7(flash_phys);

	mtspr(MAS0, _mas0);
	mtspr(MAS1, _mas1);
	mtspr(MAS2, _mas2);
	mtspr(MAS3, _mas3);
	mtspr(MAS7, _mas7);

	asm volatile("isync;msync;tlbwe;isync");

	out_be32(&(ifc_regs->cspr_cs[0].cspr), CONFIG_SYS_CSPR0);
	out_be32(&(ifc_regs->csor_cs[0].csor), CONFIG_SYS_CSOR0);
	out_be32(&(ifc_regs->amask_cs[0].amask), CONFIG_SYS_AMASK0);

	return ;
}
#endif

/* We run cpu_init_early_f in AS = 1 */
void cpu_init_early_f(void)
{
	u32 mas0, mas1, mas2, mas3, mas7;
	int i;
#ifdef CONFIG_SYS_FSL_ERRATUM_P1010_A003549
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_IFC_A003399) && !defined(CONFIG_SYS_RAMBOOT)
	ccsr_l2cache_t *l2cache = (void *)CONFIG_SYS_MPC85xx_L2_ADDR;
	u32  *dst, *src;
	void (*setup_ifc_sram)(void);
#endif

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/*
	 * Clear initial global data
	 *   we don't use memset so we can share this code with NAND_SPL
	 */
	for (i = 0; i < sizeof(gd_t); i++)
		((char *)gd)[i] = 0;

	mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(13);
	mas1 = MAS1_VALID | MAS1_TID(0) | MAS1_TS | MAS1_TSIZE(BOOKE_PAGESZ_1M);
	mas2 = FSL_BOOKE_MAS2(CONFIG_SYS_CCSRBAR, MAS2_I|MAS2_G);
	mas3 = FSL_BOOKE_MAS3(CONFIG_SYS_CCSRBAR_PHYS, 0, MAS3_SW|MAS3_SR);
	mas7 = FSL_BOOKE_MAS7(CONFIG_SYS_CCSRBAR_PHYS);

	write_tlb(mas0, mas1, mas2, mas3, mas7);

/*
 * Work Around for IFC Erratum A-003549. This issue is P1010
 * specific. LCLK(a free running clk signal) is muxed with IFC_CS3 on P1010 SOC
 * Hence specifically selecting CS3.
 */
#ifdef CONFIG_SYS_FSL_ERRATUM_P1010_A003549
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_LCLK_IFC_CS3);
#endif

	init_laws();

/*
 * Work Around for IFC Erratum A003399, issue will hit only when execution
 * from NOR Flash
 */
#if defined(CONFIG_SYS_FSL_ERRATUM_IFC_A003399) && !defined(CONFIG_SYS_RAMBOOT)
#define SRAM_BASE_ADDR	(0x00000000)
	/* TLB for SRAM */
	mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(9);
	mas1 = MAS1_VALID | MAS1_TID(0) | MAS1_TS |
		MAS1_TSIZE(BOOKE_PAGESZ_1M);
	mas2 = FSL_BOOKE_MAS2(SRAM_BASE_ADDR, MAS2_I);
	mas3 = FSL_BOOKE_MAS3(SRAM_BASE_ADDR, 0, MAS3_SX|MAS3_SW|MAS3_SR);
	mas7 = FSL_BOOKE_MAS7(0);

	write_tlb(mas0, mas1, mas2, mas3, mas7);

	out_be32(&l2cache->l2srbar0, SRAM_BASE_ADDR);

	out_be32(&l2cache->l2errdis,
		(MPC85xx_L2ERRDIS_MBECC | MPC85xx_L2ERRDIS_SBECC));

	out_be32(&l2cache->l2ctl,
		(MPC85xx_L2CTL_L2E | MPC85xx_L2CTL_L2SRAM_ENTIRE));

	/*
	 * Copy the code in setup_ifc to L2SRAM. Do a word copy
	 * because NOR Flash on P1010 does not support byte
	 * access (Erratum IFC-A002769)
	 */
	setup_ifc_sram = (void *)SRAM_BASE_ADDR;
	dst = (u32 *) SRAM_BASE_ADDR;
	src = (u32 *) setup_ifc;
	for (i = 0; i < 1024; i++)
		*dst++ = *src++;

	setup_ifc_sram();

	/* CLEANUP */
	clrbits_be32(&l2cache->l2ctl,
			(MPC85xx_L2CTL_L2E |
			 MPC85xx_L2CTL_L2SRAM_ENTIRE));
	out_be32(&l2cache->l2srbar0, 0x0);
#endif

	invalidate_tlb(1);

#if defined(CONFIG_SECURE_BOOT)
	/* Disable the TLBs created by ISBC */
	for (i = CONFIG_SYS_ISBC_START_TLB;
	     i < CONFIG_SYS_ISBC_START_TLB + CONFIG_SYS_ISBC_NUM_TLBS; i++)
			disable_tlb(i);
#endif

	init_tlbs();
}
