/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <ioports.h>
#include <lmb.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

u32 get_my_id()
{
	return mfspr(SPRN_PIR);
}

int cpu_reset(int nr)
{
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC85xx_PIC_ADDR);
	out_be32(&pic->pir, 1 << nr);
	/* the dummy read works around an errata on early 85xx MP PICs */
	(void)in_be32(&pic->pir);
	out_be32(&pic->pir, 0x0);

	return 0;
}

int cpu_status(int nr)
{
	u32 *table, id = get_my_id();

	if (nr == id) {
		table = (u32 *)get_spin_virt_addr();
		printf("table base @ 0x%p\n", table);
	} else {
		table = (u32 *)get_spin_virt_addr() + nr * NUM_BOOT_ENTRY;
		printf("Running on cpu %d\n", id);
		printf("\n");
		printf("table @ 0x%p\n", table);
		printf("   addr - 0x%08x\n", table[BOOT_ENTRY_ADDR_LOWER]);
		printf("   pir  - 0x%08x\n", table[BOOT_ENTRY_PIR]);
		printf("   r3   - 0x%08x\n", table[BOOT_ENTRY_R3_LOWER]);
		printf("   r6   - 0x%08x\n", table[BOOT_ENTRY_R6_LOWER]);
	}

	return 0;
}

#ifdef CONFIG_FSL_CORENET
int cpu_disable(int nr)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->coredisrl, 1 << nr);

	return 0;
}
#else
int cpu_disable(int nr)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	switch (nr) {
	case 0:
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_CPU0);
		break;
	case 1:
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_CPU1);
		break;
	default:
		printf("Invalid cpu number for disable %d\n", nr);
		return 1;
	}

	return 0;
}
#endif

static u8 boot_entry_map[4] = {
	0,
	BOOT_ENTRY_PIR,
	BOOT_ENTRY_R3_LOWER,
	BOOT_ENTRY_R6_LOWER,
};

int cpu_release(int nr, int argc, char *argv[])
{
	u32 i, val, *table = (u32 *)get_spin_virt_addr() + nr * NUM_BOOT_ENTRY;
	u64 boot_addr;

	if (nr == get_my_id()) {
		printf("Invalid to release the boot core.\n\n");
		return 1;
	}

	if (argc != 4) {
		printf("Invalid number of arguments to release.\n\n");
		return 1;
	}

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	/* handle pir, r3, r6 */
	for (i = 1; i < 4; i++) {
		if (argv[i][0] != '-') {
			u8 entry = boot_entry_map[i];
			val = simple_strtoul(argv[i], NULL, 16);
			table[entry] = val;
		}
	}

	table[BOOT_ENTRY_ADDR_UPPER] = (u32)(boot_addr >> 32);

	/* ensure all table updates complete before final address write */
	eieio();

	table[BOOT_ENTRY_ADDR_LOWER] = (u32)(boot_addr & 0xffffffff);

	return 0;
}

u32 determine_mp_bootpg(void)
{
	/* if we have 4G or more of memory, put the boot page at 4Gb-4k */
	if ((u64)gd->ram_size > 0xfffff000)
		return (0xfffff000);

	return (gd->ram_size - 4096);
}

ulong get_spin_phys_addr(void)
{
	extern ulong __secondary_start_page;
	extern ulong __spin_table;

	return (determine_mp_bootpg() +
		(ulong)&__spin_table - (ulong)&__secondary_start_page);
}

ulong get_spin_virt_addr(void)
{
	extern ulong __secondary_start_page;
	extern ulong __spin_table;

	return (CONFIG_BPTR_VIRT_ADDR +
		(ulong)&__spin_table - (ulong)&__secondary_start_page);
}

#ifdef CONFIG_FSL_CORENET
static void plat_mp_up(unsigned long bootpg)
{
	u32 up, cpu_up_mask, whoami;
	u32 *table = (u32 *)get_spin_virt_addr();
	volatile ccsr_gur_t *gur;
	volatile ccsr_local_t *ccm;
	volatile ccsr_rcpm_t *rcpm;
	volatile ccsr_pic_t *pic;
	int timeout = 10;
	u32 nr_cpus;
	struct law_entry e;

	gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	ccm = (void *)(CONFIG_SYS_FSL_CORENET_CCM_ADDR);
	rcpm = (void *)(CONFIG_SYS_FSL_CORENET_RCPM_ADDR);
	pic = (void *)(CONFIG_SYS_MPC85xx_PIC_ADDR);

	nr_cpus = ((in_be32(&pic->frr) >> 8) & 0xff) + 1;

	whoami = in_be32(&pic->whoami);
	cpu_up_mask = 1 << whoami;
	out_be32(&ccm->bstrl, bootpg);

	e = find_law(bootpg);
	out_be32(&ccm->bstrar, LAW_EN | e.trgt_id << 20 | LAW_SIZE_4K);

	/* readback to sync write */
	in_be32(&ccm->bstrar);

	/* disable time base at the platform */
	out_be32(&rcpm->ctbenrl, cpu_up_mask);

	/* release the hounds */
	up = ((1 << nr_cpus) - 1);
	out_be32(&gur->brrl, up);

	/* wait for everyone */
	while (timeout) {
		int i;
		for (i = 0; i < nr_cpus; i++) {
			if (table[i * NUM_BOOT_ENTRY + BOOT_ENTRY_ADDR_LOWER])
				cpu_up_mask |= (1 << i);
		};

		if ((cpu_up_mask & up) == up)
			break;

		udelay(100);
		timeout--;
	}

	if (timeout == 0)
		printf("CPU up timeout. CPU up mask is %x should be %x\n",
			cpu_up_mask, up);

	/* enable time base at the platform */
	out_be32(&rcpm->ctbenrl, 0);
	mtspr(SPRN_TBWU, 0);
	mtspr(SPRN_TBWL, 0);
	out_be32(&rcpm->ctbenrl, (1 << nr_cpus) - 1);

#ifdef CONFIG_MPC8xxx_DISABLE_BPTR
	/*
	 * Disabling Boot Page Translation allows the memory region 0xfffff000
	 * to 0xffffffff to be used normally.  Leaving Boot Page Translation
	 * enabled remaps 0xfffff000 to SDRAM which makes that memory region
	 * unusable for normal operation but it does allow OSes to easily
	 * reset a processor core to put it back into U-Boot's spinloop.
	 */
	clrbits_be32(&ecm->bptr, 0x80000000);
#endif
}
#else
static void plat_mp_up(unsigned long bootpg)
{
	u32 up, cpu_up_mask, whoami;
	u32 *table = (u32 *)get_spin_virt_addr();
	volatile u32 bpcr;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC85xx_PIC_ADDR);
	u32 devdisr;
	int timeout = 10;

	whoami = in_be32(&pic->whoami);
	out_be32(&ecm->bptr, 0x80000000 | (bootpg >> 12));

	/* disable time base at the platform */
	devdisr = in_be32(&gur->devdisr);
	if (whoami)
		devdisr |= MPC85xx_DEVDISR_TB0;
	else
		devdisr |= MPC85xx_DEVDISR_TB1;
	out_be32(&gur->devdisr, devdisr);

	/* release the hounds */
	up = ((1 << cpu_numcores()) - 1);
	bpcr = in_be32(&ecm->eebpcr);
	bpcr |= (up << 24);
	out_be32(&ecm->eebpcr, bpcr);
	asm("sync; isync; msync");

	cpu_up_mask = 1 << whoami;
	/* wait for everyone */
	while (timeout) {
		int i;
		for (i = 0; i < cpu_numcores(); i++) {
			if (table[i * NUM_BOOT_ENTRY + BOOT_ENTRY_ADDR_LOWER])
				cpu_up_mask |= (1 << i);
		};

		if ((cpu_up_mask & up) == up)
			break;

		udelay(100);
		timeout--;
	}

	if (timeout == 0)
		printf("CPU up timeout. CPU up mask is %x should be %x\n",
			cpu_up_mask, up);

	/* enable time base at the platform */
	if (whoami)
		devdisr |= MPC85xx_DEVDISR_TB1;
	else
		devdisr |= MPC85xx_DEVDISR_TB0;
	out_be32(&gur->devdisr, devdisr);
	mtspr(SPRN_TBWU, 0);
	mtspr(SPRN_TBWL, 0);

	devdisr &= ~(MPC85xx_DEVDISR_TB0 | MPC85xx_DEVDISR_TB1);
	out_be32(&gur->devdisr, devdisr);

#ifdef CONFIG_MPC8xxx_DISABLE_BPTR
	/*
	 * Disabling Boot Page Translation allows the memory region 0xfffff000
	 * to 0xffffffff to be used normally.  Leaving Boot Page Translation
	 * enabled remaps 0xfffff000 to SDRAM which makes that memory region
	 * unusable for normal operation but it does allow OSes to easily
	 * reset a processor core to put it back into U-Boot's spinloop.
	 */
	clrbits_be32(&ecm->bptr, 0x80000000);
#endif
}
#endif

void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg = determine_mp_bootpg();

	lmb_reserve(lmb, bootpg, 4096);
}

void setup_mp(void)
{
	extern ulong __secondary_start_page;
	extern ulong __bootpg_addr;
	ulong fixup = (ulong)&__secondary_start_page;
	u32 bootpg = determine_mp_bootpg();

	/* Store the bootpg's SDRAM address for use by secondary CPU cores */
	__bootpg_addr = bootpg;

	/* look for the tlb covering the reset page, there better be one */
	int i = find_tlb_idx((void *)CONFIG_BPTR_VIRT_ADDR, 1);

	/* we found a match */
	if (i != -1) {
		/* map reset page to bootpg so we can copy code there */
		disable_tlb(i);

		set_tlb(1, CONFIG_BPTR_VIRT_ADDR, bootpg, /* tlb, epn, rpn */
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G, /* perms, wimge */
			0, i, BOOKE_PAGESZ_4K, 1); /* ts, esel, tsize, iprot */

		memcpy((void *)CONFIG_BPTR_VIRT_ADDR, (void *)fixup, 4096);

		plat_mp_up(bootpg);
	} else {
		puts("WARNING: No reset page TLB. "
			"Skipping secondary core setup\n");
	}
}
