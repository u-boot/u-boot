#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <ioports.h>
#include <lmb.h>
#include <asm/io.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

#if (CONFIG_NUM_CPUS > 1)
void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg;

	/* if we have 4G or more of memory, put the boot page at 4Gb-1M */
	if ((u64)gd->ram_size > 0xfffff000)
		bootpg = 0xfff00000;
	else
		bootpg = gd->ram_size - (1024 * 1024);

	/* tell u-boot we stole a page */
	lmb_reserve(lmb, bootpg, 4096);
}

/*
 * Copy the code for other cpus to execute into an
 * aligned location accessible via BPTR
 */
void setup_mp(void)
{
	extern ulong __secondary_start_page;
	ulong fixup = (ulong)&__secondary_start_page;
	u32 bootpg;
	u32 bootpg_va;

	/*
	 * If we have 4G or more of memory, put the boot page at 4Gb-1M.
	 * Otherwise, put it at the very end of RAM.
	 */
	if (gd->ram_size > 0xfffff000)
		bootpg = 0xfff00000;
	else
		bootpg = gd->ram_size - (1024 * 1024);

	if (bootpg >= CONFIG_SYS_MAX_DDR_BAT_SIZE) {
		/* We're not covered by the DDR mapping, set up BAT  */
		write_bat(DBAT7, CONFIG_SYS_SCRATCH_VA | BATU_BL_128K |
			  BATU_VS | BATU_VP,
			  bootpg | BATL_PP_RW | BATL_MEMCOHERENCE);
		bootpg_va = CONFIG_SYS_SCRATCH_VA;
	} else {
		bootpg_va = bootpg;
	}

	memcpy((void *)bootpg_va, (void *)fixup, 4096);
	flush_cache(bootpg_va, 4096);

	/* remove the temporary BAT mapping */
	if (bootpg >= CONFIG_SYS_MAX_DDR_BAT_SIZE)
		write_bat(DBAT7, 0, 0);

	/* If the physical location of bootpg is not at fff00000, set BPTR */
	if (bootpg != 0xfff00000)
		out_be32((uint *)(CONFIG_SYS_CCSRBAR + 0x20), 0x80000000 |
			 (bootpg >> 12));
}
#endif
