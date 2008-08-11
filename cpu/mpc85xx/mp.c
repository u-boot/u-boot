/*
 * Copyright 2008 Freescale Semiconductor.
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
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

u32 get_my_id()
{
	return mfspr(SPRN_PIR);
}

int cpu_reset(int nr)
{
	volatile ccsr_pic_t *pic = (void *)(CFG_MPC85xx_PIC_ADDR);
	out_be32(&pic->pir, 1 << nr);
	(void)in_be32(&pic->pir);
	out_be32(&pic->pir, 0x0);

	return 0;
}

int cpu_status(int nr)
{
	u32 *table, id = get_my_id();

	if (nr == id) {
		table = (u32 *)get_spin_addr();
		printf("table base @ 0x%p\n", table);
	} else {
		table = (u32 *)get_spin_addr() + nr * NUM_BOOT_ENTRY;
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

static u8 boot_entry_map[4] = {
	0,
	BOOT_ENTRY_PIR,
	BOOT_ENTRY_R3_LOWER,
	BOOT_ENTRY_R6_LOWER,
};

int cpu_release(int nr, int argc, char *argv[])
{
	u32 i, val, *table = (u32 *)get_spin_addr() + nr * NUM_BOOT_ENTRY;
	u64 boot_addr;

	if (nr == get_my_id()) {
		printf("Invalid to release the boot core.\n\n");
		return 1;
	}

	if (argc != 4) {
		printf("Invalid number of arguments to release.\n\n");
		return 1;
	}

#ifdef CFG_64BIT_STRTOUL
	boot_addr = simple_strtoull(argv[0], NULL, 16);
#else
	boot_addr = simple_strtoul(argv[0], NULL, 16);
#endif

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

ulong get_spin_addr(void)
{
	extern ulong __secondary_start_page;
	extern ulong __spin_table;

	ulong addr =
		(ulong)&__spin_table - (ulong)&__secondary_start_page;
	addr += 0xfffff000;

	return addr;
}

static void pq3_mp_up(unsigned long bootpg)
{
	u32 up, cpu_up_mask, whoami;
	u32 *table = (u32 *)get_spin_addr();
	volatile u32 bpcr;
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_pic_t *pic = (void *)(CFG_MPC85xx_PIC_ADDR);
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
	up = ((1 << CONFIG_NUM_CPUS) - 1);
	bpcr = in_be32(&ecm->eebpcr);
	bpcr |= (up << 24);
	out_be32(&ecm->eebpcr, bpcr);
	asm("sync; isync; msync");

	cpu_up_mask = 1 << whoami;
	/* wait for everyone */
	while (timeout) {
		int i;
		for (i = 0; i < CONFIG_NUM_CPUS; i++) {
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
}

void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg;

	/* if we have 4G or more of memory, put the boot page at 4Gb-4k */
	if ((u64)gd->ram_size > 0xfffff000)
		bootpg = 0xfffff000;
	else
		bootpg = gd->ram_size - 4096;

	lmb_reserve(lmb, bootpg, 4096);
}

void setup_mp(void)
{
	extern ulong __secondary_start_page;
	ulong fixup = (ulong)&__secondary_start_page;
	u32 bootpg;

	/* if we have 4G or more of memory, put the boot page at 4Gb-4k */
	if ((u64)gd->ram_size > 0xfffff000)
		bootpg = 0xfffff000;
	else
		bootpg = gd->ram_size - 4096;

	memcpy((void *)bootpg, (void *)fixup, 4096);
	flush_cache(bootpg, 4096);

	pq3_mp_up(bootpg);
}
