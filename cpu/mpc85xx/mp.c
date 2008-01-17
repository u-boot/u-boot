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
#include <asm/io.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

#define BOOT_ENTRY_ADDR	0
#define BOOT_ENTRY_PIR	1
#define BOOT_ENTRY_R3	2
#define BOOT_ENTRY_R4	3
#define BOOT_ENTRY_R6	4
#define BOOT_ENTRY_R7	5
#define NUM_BOOT_ENTRY	6

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
		printf("table base @ 0x%08x\n", table);
	} else {
		table = (u32 *)get_spin_addr() + nr * NUM_BOOT_ENTRY;
		printf("Running on cpu %d\n", id);
		printf("\n");
		printf("table @ 0x%08x:\n", table);
		printf("   addr - 0x%08x\n", table[BOOT_ENTRY_ADDR]);
		printf("   pir  - 0x%08x\n", table[BOOT_ENTRY_PIR]);
		printf("   r3   - 0x%08x\n", table[BOOT_ENTRY_R3]);
		printf("   r4   - 0x%08x\n", table[BOOT_ENTRY_R4]);
		printf("   r6   - 0x%08x\n", table[BOOT_ENTRY_R6]);
		printf("   r7   - 0x%08x\n", table[BOOT_ENTRY_R7]);
	}

	return 0;
}

int cpu_release(int nr, unsigned long boot_addr, int argc, char *argv[])
{
	u32 i, val, *table = (u32 *)get_spin_addr() + nr * NUM_BOOT_ENTRY;

	if (nr == get_my_id()) {
		printf("Invalid to release the boot core.\n\n");
		return 1;
	}

	if (argc != 5) {
		printf("Invalid number of arguments to release.\n\n");
		return 1;
	}

	/* handle pir, r3, r4, r6, r7 */
	for (i = 0; i < 5; i++) {
		if (argv[i][0] != '-') {
			val = simple_strtoul(argv[i], NULL, 16);
			table[i+BOOT_ENTRY_PIR] = val;
		}
	}

	table[BOOT_ENTRY_ADDR] = boot_addr;

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
	up = ((1 << CONFIG_NR_CPUS) - 1);
	bpcr = in_be32(&ecm->eebpcr);
	bpcr |= (up << 24);
	out_be32(&ecm->eebpcr, bpcr);
	asm("sync; isync; msync");

	cpu_up_mask = 1 << whoami;
	/* wait for everyone */
	while (timeout) {
		int i;
		for (i = 1; i < CONFIG_NR_CPUS; i++) {
			if (table[i * NUM_BOOT_ENTRY])
				cpu_up_mask |= (1 << i);
		};

		if ((cpu_up_mask & up) == up)
			break;

		udelay(100);
		timeout--;
	}

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
