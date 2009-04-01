#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <ioports.h>
#include <lmb.h>
#include <asm/io.h>
#include <asm/mp.h>

DECLARE_GLOBAL_DATA_PTR;

int cpu_reset(int nr)
{
	/* dummy function so common/cmd_mp.c will build
	 * should be implemented in the future, when cpu_release()
	 * is supported.  Be aware there may be a similiar bug
	 * as exists on MPC85xx w/its PIC having a timing window
	 * associated to resetting the core */
	return 1;
}

int cpu_status(int nr)
{
	/* dummy function so common/cmd_mp.c will build */
	return 0;
}

int cpu_release(int nr, int argc, char *argv[])
{
	/* dummy function so common/cmd_mp.c will build
	 * should be implemented in the future */
	return 1;
}

u32 determine_mp_bootpg(void)
{
	/* if we have 4G or more of memory, put the boot page at 4Gb-1M */
	if ((u64)gd->ram_size > 0xfffff000)
		return (0xfff00000);

	return (gd->ram_size - (1024 * 1024));
}

void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg = determine_mp_bootpg();

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
	u32 bootpg = determine_mp_bootpg();
	u32 bootpg_va;

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
